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

int main()
{
	int ram_fd = open("/dev/shm/mem", O_RDWR);
	struct stat statbuf;
	int ret;
	int size;
	int i;
	uint8_t buf[10];

	if (ram_fd < 0) {
		printf("Failed to open memory file %s\n", strerror(errno));
		exit(1);
	}

	ret = fstat(ram_fd, &statbuf);
	if (ret < 0) {
		printf("stat failed %s\n", strerror(errno));
		exit (1);
	}

	size = statbuf.st_size;

	while (1) {
		ret = lseek(ram_fd, 0, SEEK_SET);
		if (ret != 0) {
			printf("seek failed %s\n", strerror(errno));
			exit(1);
		}
		ret = read(ram_fd, buf, 10);
		if (ret < 0) {
			printf("read failed %s\n", strerror(errno));
			exit(1);
		}
		for (i = 0 ; i < 10 ; i++) {
			printf("%d %c\n", i, buf[i]);
		}
		sleep(10);
	}

	close(ram_fd);
	return 0;
}
