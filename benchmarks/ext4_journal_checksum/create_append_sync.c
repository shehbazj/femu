/* Test program that creates(), appends() and syncs() a file
 * on a device.
 * */

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define BLOCK_SIZE 4096

int check_root()
{
	if (geteuid() != 0)
		return false;
	return true;
}

int main()
{
	int fd;
	char cmd[200];
	char *buf;
	int ret;

	if (!check_root()) {
		printf("Please run with sudo permissions\n");
		exit (1);
	}

	buf = aligned_alloc(BLOCK_SIZE, BLOCK_SIZE);
	if (buf == NULL) {
		printf("aligned alloc failed\n");
		exit(1);
	}
	buf = memset(buf, 'x', BLOCK_SIZE);
	if (buf == NULL) {
		printf("memset failed\n");
		exit(1);
	}

	strcpy(cmd, "./setup_journal.sh");
	system(cmd);


	printf("create file\n");
	fd = open("/mnt/testfile", O_RDWR | O_CREAT | O_DIRECT, 0666);
	if (fd < 0) {
		goto error;
	}

	ret = ftruncate(fd, 40960);
	if (ret < 0) {goto error;}

	printf("write\n");
	ret = write (fd, buf, BLOCK_SIZE);
	if (ret < 0) {goto error;}

	printf("sync\n");
	ret = fsync(fd);
	if (ret < 0) {goto error;}

	printf("write\n");
	ret = write (fd, buf, BLOCK_SIZE);
	if (ret < 0) {goto error;}

	printf("sync\n");
	ret = fsync(fd);
	if (ret < 0) {goto error;}

	printf("write\n");
	ret = write (fd, buf, BLOCK_SIZE);
	if (ret < 0) {goto error;}

	printf("sync\n");
	ret = fsync(fd);
	if (ret < 0) {goto error;}

	printf("Close file system\n");
	ret = close(fd);
	if (ret < 0) {goto error;}	

	printf("Umount\n");
	strcpy(cmd, "umount -l /mnt");
	system(cmd);

	return 0;
error:
	perror("error");
	exit(1);
}
