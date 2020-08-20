// Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
// COPYING file in the root directory) and Apache 2.0 License
// (found in the LICENSE.Apache file in the root directory).

// How to use this example
// Open two terminals, in one of them, run `./multi_processes_example 0` to
// start a process running the primary instance. This will create a new DB in
// kDBPath. The process will run for a while inserting keys to the normal
// RocksDB database.
// Next, go to the other terminal and run `./multi_processes_example 1` to
// start a process running the secondary instance. This will create a secondary
// instance following the aforementioned primary instance. This process will
// run for a while, tailing the logs of the primary. After process with primary
// instance exits, this process will keep running until you hit 'CTRL+C'.

#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <thread>
#include <vector>
#include <errno.h>

#if defined(OS_LINUX)
#include <dirent.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif  // !OS_LINUX

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"

#include <sys/mount.h>

using ROCKSDB_NAMESPACE::ColumnFamilyDescriptor;
using ROCKSDB_NAMESPACE::ColumnFamilyHandle;
using ROCKSDB_NAMESPACE::ColumnFamilyOptions;
using ROCKSDB_NAMESPACE::DB;
using ROCKSDB_NAMESPACE::FlushOptions;
using ROCKSDB_NAMESPACE::Iterator;
using ROCKSDB_NAMESPACE::Options;
using ROCKSDB_NAMESPACE::ReadOptions;
using ROCKSDB_NAMESPACE::Slice;
using ROCKSDB_NAMESPACE::Status;
using ROCKSDB_NAMESPACE::WriteOptions;

const std::string kDBPath = "/mnt/rocksdb_host/rdb";
//const std::string kPrimaryStatusFile =  "/tmp/rocksdb_multi_processes_example_primary_status";
const uint64_t kMaxKey = 600000;
const size_t kMaxValueLength = 256;
const size_t kNumKeysPerFlush = 1000;

const std::vector<std::string>& GetColumnFamilyNames() {
  static std::vector<std::string> column_family_names = {
      ROCKSDB_NAMESPACE::kDefaultColumnFamilyName, "pikachu"};
  return column_family_names;
}

inline bool IsLittleEndian() {
  uint32_t x = 1;
  return *reinterpret_cast<char*>(&x) != 0;
}

static std::atomic<int>& ShouldSecondaryWait() {
  static std::atomic<int> should_secondary_wait{1};
  return should_secondary_wait;
}

static std::string Key(uint64_t k) {
  std::string ret;
  if (IsLittleEndian()) {
    ret.append(reinterpret_cast<char*>(&k), sizeof(k));
  } else {
    char buf[sizeof(k)];
    buf[0] = k & 0xff;
    buf[1] = (k >> 8) & 0xff;
    buf[2] = (k >> 16) & 0xff;
    buf[3] = (k >> 24) & 0xff;
    buf[4] = (k >> 32) & 0xff;
    buf[5] = (k >> 40) & 0xff;
    buf[6] = (k >> 48) & 0xff;
    buf[7] = (k >> 56) & 0xff;
    ret.append(buf, sizeof(k));
  }
  size_t i = 0, j = ret.size() - 1;
  while (i < j) {
    char tmp = ret[i];
    ret[i] = ret[j];
    ret[j] = tmp;
    ++i;
    --j;
  }
  return ret;
}

static uint64_t Key(std::string key) {
  assert(key.size() == sizeof(uint64_t));
  size_t i = 0, j = key.size() - 1;
  while (i < j) {
    char tmp = key[i];
    key[i] = key[j];
    key[j] = tmp;
    ++i;
    --j;
  }
  uint64_t ret = 0;
  if (IsLittleEndian()) {
    memcpy(&ret, key.c_str(), sizeof(uint64_t));
  } else {
    const char* buf = key.c_str();
    ret |= static_cast<uint64_t>(buf[0]);
    ret |= (static_cast<uint64_t>(buf[1]) << 8);
    ret |= (static_cast<uint64_t>(buf[2]) << 16);
    ret |= (static_cast<uint64_t>(buf[3]) << 24);
    ret |= (static_cast<uint64_t>(buf[4]) << 32);
    ret |= (static_cast<uint64_t>(buf[5]) << 40);
    ret |= (static_cast<uint64_t>(buf[6]) << 48);
    ret |= (static_cast<uint64_t>(buf[7]) << 56);
  }
  return ret;
}

static Slice GenerateRandomValue(const size_t max_length, char scratch[]) {
  size_t sz = 1 + (std::rand() % max_length);
  int rnd = std::rand();
  for (size_t i = 0; i != sz; ++i) {
    scratch[i] = static_cast<char>(rnd ^ i);
  }
  return Slice(scratch, sz);
}

static bool ShouldCloseDB() { return true; }

void secondary_instance_sigint_handler(int signal) {
  ShouldSecondaryWait().store(0, std::memory_order_relaxed);
  fprintf(stdout, "\n");
  fflush(stdout);
};

DB* openSecondaryDB()
{
  DB* db = nullptr;
  Options options;
  options.create_if_missing = false;
  options.max_open_files = -1;
  const std::string kSecondaryPath =
      "/mnt/rocksdb_host/rocksdb_multi_processes_example_secondary";
  Status s = DB::OpenAsSecondary(options, kDBPath, kSecondaryPath, &db);
  if (!s.ok()) {
    fprintf(stderr, "Failed to open in secondary mode: %s\n",
           s.ToString().c_str());
    assert(false);
  } else {
    fprintf(stdout, "Secondary instance starts\n");
  }
  return db;
}

void unmountDB(DB *db)
{
	delete db;
	db = nullptr;
	int ret = umount("/mnt/rocksdb_host");
	if (ret < 0) {
		printf("umount failed %s\n", strerror(errno));
//		exit(1);
	}
//const std::string kDBPath = "/mnt/rocksdb_host/rdb";
}

void mountDB()
{
	int ret = mount("/dev/pmem0", "/mnt/rocksdb_host", "ext4", 0 , NULL);
	if (ret < 0) {
		printf("mount failed %s\n", strerror(errno));
//		exit(1);
	}
}

void RunSecondary() {
  ::signal(SIGINT, secondary_instance_sigint_handler);
  // Create directory if necessary
  char c;
  DB *db = nullptr;

  printf("Unmount and mount\n");
	unmountDB(db);
  mountDB();
  const std::string kSecondaryPath =
      "/mnt/rocksdb_host/rocksdb_multi_processes_example_secondary";
	printf("opening secondary path dir\n");
  if (nullptr == opendir(kSecondaryPath.c_str())) {
    int ret =
        mkdir(kSecondaryPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (ret < 0) {
      perror("failed to create directory for secondary instance");
      exit(0);
    }
  }

	printf("start while loop\n");
  	while (1) {

		printf("Remount. Press Key\n");
		scanf("%c", &c);

		mountDB();

		printf("open secondary db\n");
		db = openSecondaryDB();

	  	printf("Umount. press key\n");
		scanf("%c", &c);
	
		unmountDB(db);

	}
  	delete db;
}

int main(int argc, char** argv) {
	char c;
	printf("Please run ./host_setup.sh first\n Press key to continue\n");
	scanf("%c", &c);
	RunSecondary();
  return 0;
}
