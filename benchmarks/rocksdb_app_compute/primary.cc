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

const std::string kDBPath = "/mnt/rocksdb_guest/rdb";
//const std::string kPrimaryStatusFile = "/tmp/rocksdb_multi_processes_example_primar";
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

// TODO: port this example to other systems. It should be straightforward for
// POSIX-compliant systems.
void CreateDB() {
  long my_pid = static_cast<long>(getpid());
  Options options;
  Status s = ROCKSDB_NAMESPACE::DestroyDB(kDBPath, options);
  if (!s.ok()) {
    fprintf(stderr, "[process %ld] Failed to destroy DB: %s\n", my_pid,
            s.ToString().c_str());
    assert(false);
  }
  options.create_if_missing = true;
  DB* db = nullptr;
  s = DB::Open(options, kDBPath, &db);
  if (!s.ok()) {
    fprintf(stderr, "[process %ld] Failed to open DB: %s\n", my_pid,
            s.ToString().c_str());
    assert(false);
  }
  std::vector<ColumnFamilyHandle*> handles;
  ColumnFamilyOptions cf_opts(options);
  for (const auto& cf_name : GetColumnFamilyNames()) {
    if (ROCKSDB_NAMESPACE::kDefaultColumnFamilyName != cf_name) {
      ColumnFamilyHandle* handle = nullptr;
      s = db->CreateColumnFamily(cf_opts, cf_name, &handle);
      if (!s.ok()) {
        fprintf(stderr, "[process %ld] Failed to create CF %s: %s\n", my_pid,
                cf_name.c_str(), s.ToString().c_str());
        assert(false);
      }
      handles.push_back(handle);
    }
  }
  fprintf(stdout, "[process %ld] Column families created\n", my_pid);
  for (auto h : handles) {
    delete h;
  }
  handles.clear();
  delete db;
}

void RunPrimary() {
  long my_pid = static_cast<long>(getpid());
  fprintf(stdout, "[process %ld] Primary instance starts\n", my_pid);
  CreateDB();
  std::srand(time(nullptr));
  DB* db = nullptr;
  Options options;
  options.create_if_missing = false;
  std::vector<ColumnFamilyDescriptor> column_families;
  for (const auto& cf_name : GetColumnFamilyNames()) {
    column_families.push_back(ColumnFamilyDescriptor(cf_name, options));
  }
  std::vector<ColumnFamilyHandle*> handles;
  WriteOptions write_opts;
  char val_buf[kMaxValueLength] = {0};
  uint64_t curr_key = 0;
  while (curr_key < kMaxKey) {
    Status s;
    if (nullptr == db) {
      s = DB::Open(options, kDBPath, column_families, &handles, &db);
      if (!s.ok()) {
        fprintf(stderr, "[process %ld] Failed to open DB: %s\n", my_pid,
                s.ToString().c_str());
        assert(false);
      }
    }
    assert(nullptr != db);
    assert(handles.size() == GetColumnFamilyNames().size());
    for (auto h : handles) {
      assert(nullptr != h);
      for (size_t i = 0; i != kNumKeysPerFlush; ++i) {
        Slice key = Key(curr_key + static_cast<uint64_t>(i));
        Slice value = GenerateRandomValue(kMaxValueLength, val_buf);
        s = db->Put(write_opts, h, key, value);
        if (!s.ok()) {
          fprintf(stderr, "[process %ld] Failed to insert\n", my_pid);
          assert(false);
        }
      }
      s = db->Flush(FlushOptions(), h);
      if (!s.ok()) {
        fprintf(stderr, "[process %ld] Failed to flush\n", my_pid);
        assert(false);
      }
    }
    curr_key += static_cast<uint64_t>(kNumKeysPerFlush);
    if (ShouldCloseDB()) {
      for (auto h : handles) {
        delete h;
      }
      handles.clear();
      delete db;
      db = nullptr;
    }
  }
  if (nullptr != db) {
    for (auto h : handles) {
      delete h;
    }
    handles.clear();
    delete db;
    db = nullptr;
  }
  fprintf(stdout, "[process %ld] Finished adding keys\n", my_pid);
}

int main(int argc, char** argv) {
	char c;
	printf("Please run ./setup_guest.sh first\n");
	printf("Press key\n");
	scanf("%c", &c);
    	RunPrimary();
  return 0;
}
