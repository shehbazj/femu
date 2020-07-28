/*
	gzip_tester.c
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <poll.h>

#define DEBUG
#ifdef DEBUG
# define debug_print(x) printf x
#else
# define debug_print(x) do {} while (0)
#endif

#define BLOCK_SIZE 4096
//#define BLOCK_SIZE 1

int main(int argc, char *argv[])
{
	int input_fd;
	int output_fd;
	uint8_t buf[BLOCK_SIZE];
	uint8_t compress_buf[BLOCK_SIZE];
	int ret;

	struct pollfd fds[1];
	memset(fds, 0 , sizeof(fds));

	// read file and write to computational_pipe_send
	if (argc != 3) {
		printf("Usage: ./gzip_writer <infile> <mode>\n");
		printf("mode = 1 - compression, 2 - decompression\n");
		exit(1);
	}
	long option;
	option = strtol(argv[2], NULL, 10);

	debug_print(("unlink fifo\n"));
	unlink("computational_pipe_send");
	unlink("computational_pipe_recv");
	debug_print(("unlink fifo done\n"));

	debug_print(("open ip file\n"));
	input_fd = open(argv[1], O_RDONLY);
	if (input_fd < 0) {
		printf("error opening file %s\n", strerror(errno));
		exit(1);
	}

	if (option == 1) {
		output_fd = open("of.gz", O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if (output_fd < 0) {
			printf("error opening outfile %s\n", strerror(errno));
			exit(1);
		}
	} else if (option == 2) {
		output_fd = open("if", O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if (output_fd < 0) {
			printf("error opening outfile %s\n", strerror(errno));
			exit(1);
		}
	}else {
		printf("Mode can be\n1 - compression\n2 - decompression\n");
		exit(1);
	}

	debug_print(("open file done\n"));
	debug_print(("create pipe\n"));
	mkfifo("computational_pipe_send", 0666);
	mkfifo("computational_pipe_recv", 0666);

	debug_print(("create pipe done\n"));

	debug_print(("open write pipe\n"));
	int write_pipe = open("computational_pipe_recv", O_WRONLY, 0666);
	if (write_pipe < 0) {
		printf("error opening read pipe %s\n", strerror(errno));
	}

	debug_print(("open read pipe\n"));
	int read_pipe = open("computational_pipe_send", O_RDONLY);
	if (read_pipe < 0) {
		printf("error opening pipe %s\n", strerror(errno));
		exit(1);
	}

	debug_print(("open compute pipes done\n"));

	fds[0].fd = read_pipe;
	fds[0].events = POLLIN;
	int nfds = 1;
	// timeout in ms for poll()
	// int timeout = 1;

	// timeout in ns for ppoll()
	struct timespec t;
	t.tv_sec = 0;
	t.tv_nsec = 1;
	sigset_t origmask;

	int rc;
	int total_read_bytes = 0;

	debug_print(("read from file\n"));
	while ( (ret = read(input_fd, buf, BLOCK_SIZE)) > 0 ) {
		debug_print(("read %d bytes\n", ret));
		ret = write (write_pipe, buf, BLOCK_SIZE);
		if (ret < 0) {
			printf("write to pipe failed %s\n", strerror(errno));
			exit(1);
		}
		debug_print(("wrote %d bytes\n", ret));

//		rc = poll (fds, 1, timeout);
		rc = ppoll (fds, 1, &t, &origmask);
		if (rc < 0) {
			perror( "poll failed");
			exit(1);
		}

		if (rc == 0) {
			debug_print(("poll timed out\n"));
			continue;
		}

		if (fds[0].revents == POLLIN) {
			if (fds[0].fd == read_pipe) {
				ret = read (read_pipe, compress_buf, BLOCK_SIZE);
				if (ret < 0) {
					printf("read from pipe failed %s\n", strerror(errno));
					exit(1);
				}
				debug_print(("read %d bytes\n", ret));
				debug_print(("returned %d bytes\n", ret));
				total_read_bytes += ret;
				ret = write (output_fd, compress_buf, ret);
				if (ret < 0) {
					printf("write to outfile failed %s\n", strerror(errno));
					exit(1);
				}
			}
		}
	}

	close(write_pipe);

	debug_print(("read from pipe\n"));
	while ( (ret = read (read_pipe, compress_buf, BLOCK_SIZE)) ) {
		if (ret > 0) {
			total_read_bytes += ret;
			ret = write (output_fd, compress_buf, ret);
			if (ret < 0) {
				printf("write error %s\n", strerror(errno));
				exit (1);
			}
		}else {
			printf("read fail %s()\n", strerror(errno));
			break;
		}
	}
	
	printf("total read bytes = %d\n", total_read_bytes);
	close(output_fd);
	close(input_fd);
	close(read_pipe);
}
