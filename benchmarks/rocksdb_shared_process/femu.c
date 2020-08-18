/* 
 * This file is mmapped and data is written to this mmaped region.
 * Next, `msync` is issued such that the mmaped data is written to the Ramdisk.
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>

void* create_shared_memory(int ram_fd, size_t size) {
  // Our memory buffer will be readable and writable:
  int protection = PROT_READ | PROT_WRITE;

  // The buffer will be shared (meaning other processes can access it), but
  // anonymous (meaning third-party processes cannot obtain an address for it),
  // so only this process and its children will be able to use it:
  int visibility = MAP_SHARED ;

  // XXX named/anonymous
 
  // The remaining parameters to `mmap()` are not important for this use case,
  // but the manpage for `mmap` explains their purpose.
  return mmap(NULL, size, protection, visibility, ram_fd, 0);
}

void print_mem (uint8_t *p)
{
	int i;
	for (i = 0 ; i < 10 ; i++) {
		printf("i = %d c = %c\n", i , p[i]);
	}
}

void print_disk ()
{
	uint8_t buf[10];
	int i;
	int new_fd;

	new_fd = open("/dev/shm/mem", O_RDWR);
	if (new_fd < 0) {
		printf("error opening new fd %s\n", strerror(errno));
		exit (1);
	}

	int ret = read(new_fd, buf, 10);
	if (ret < 0) {
		printf("read failed %s\n", strerror(errno));
		exit(1);
	}

	for (i = 0 ; i < 10 ; i++) {
		printf("buf %d %c\n", i, buf[i]);
	}
	close(new_fd);
}

int main()
{
	int ram_fd = open("/dev/shm/mem", O_RDWR);
	struct stat statbuf;
	int ret;
	int size;
	uint8_t *memaddr;
	int i;
	char c;

	if (ram_fd < 0) {
		printf("Failed to open memory file %s\n", strerror(errno));
		exit(1);
	}

	ret = fstat(ram_fd, &statbuf);
	if (ret < 0) {
		printf("stat failed %s\n", strerror(errno));
		exit (1);
	}
	
	printf("run ./setup.sh before contining\npress key to continue..\n");
	scanf("%c", &c);

	size = statbuf.st_size;

	memaddr = (uint8_t *)create_shared_memory(ram_fd, size);

	printf("start nsc in another terminal.\npress key to continue.\n");
	scanf("%c", &c);

	for (i = 0 ; i < 10 ; i++) {
		memaddr[i] = (char ) (i + 49);
	}

	print_mem(memaddr);
	print_disk(ram_fd);

	printf("wrote to in memory buffer, sleeping\n");
	sleep(10);

	msync(memaddr, size, MS_SYNC);	

	printf("called msync, sleeping\n");
	sleep(10);

	// msync
	
	print_disk(ram_fd);

	munmap(memaddr, size);
	close(ram_fd);
	return 0;
}
