#include "common.h"

int main(int argc, char *argv[])
{
	int fd = open(argv[1], O_WRONLY | O_CREAT | O_DIRECT);
	void *buf; //[BLOCK_SIZE];
	int i;
	long unsigned c;
	int ret;

	int file_size = NUM_BLOCKS;

	if (posix_memalign(&buf, getpagesize(), getpagesize())) {
		printf("error allocating buffer\n");
	}

	if (fd < 0) {
		printf("Error opening %s\n", strerror(errno));
		exit(1);
	}
	
	for (i = 0 ; i < BLOCK_SIZE ; i++) {
		((uint8_t *)buf)[i] = 1;
	}

	for (i = 0; i < file_size ; i++) {
		ret = write(fd, buf, BLOCK_SIZE);
		if (ret < 0) {
			printf("Error writing file \n");
			exit(1);
		}	
	}

	close(fd);
	return 0;
}
