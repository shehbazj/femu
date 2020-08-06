#define _GNU_SOURCE

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <stdint.h>
#include <error.h>
#include <errno.h>
#include <signal.h>

#define BLOCK_SIZE 4096
//#define BLOCK_SIZE 1024

/*
void init_gzip(int mode)
{
  //      typedef int (*some_func)(char *param, char *parm2 , int x);
//        void *myso = dlopen("/home/shehbaz/femu/hw/block/femu/gzip_pipe_so/libgzip.so", RTLD_NOW);
//        some_func *func = dlsym(myso, "gzip_me");
        char *i,*o;
        int ret;

        i = malloc(100);
        o = malloc(100);

        strncpy(i, "compression_pipe_send", strlen("compression_pipe_send") + 1);
        strncpy(o, "compression_pipe_recv", strlen("compression_pipe_recv") + 1);

        //send_to_compression_fd = open("compression_pipe_send", O_WRONLY);
        //recieve_from_compression_fd  = open("compression_pipe_recv", O_RDONLY);

        // gzip_me(recv pipe, send pipe and mode) where mode is 1 - compress, 2 - decompress.
        printf("COMPUTATION PROCESS -- Wait for Compression\n");
        ret = (*func)(i ,o, mode);
        if (ret) {
                printf("unexpected end of gzip\n");
		exit(1);
        }
        printf("COMPUTATION PROCESS -- Done with Compression\n");

        free(i);
        free(o);
        // close shared object when pipe gets closed.
        dlclose(myso);
}
*/

int main(int argc, char *argv[])
{
	int fd;
	int pid;
	int recieve_from_compression_fd;
	int send_to_compression_fd;
	int compressed_file_fd;
	int ret;
	int rc;

	struct timespec t;

        t.tv_sec = 0;
        t.tv_nsec = 10;
        sigset_t origmask;

	//unlink("compression_pipe_send");
	//unlink("compression_pipe_recv");

	//mkfifo("compression_pipe_send", 0666);
	//mkfifo("compression_pipe_recv", 0666);

	compressed_file_fd = open("of.gz", O_CREAT | O_TRUNC | O_WRONLY, 0666);
	if (compressed_file_fd < 0) {
		printf("compressed file open error\n");
		exit(1);
	}

	/*
	pid = fork();
	if(pid == 0) {	
		init_gzip(1);
	}else {
	*/


		struct pollfd fds[1];
		memset(fds, 0 , sizeof(fds));

		printf("open fd\n");

		fds[0].fd = recieve_from_compression_fd;
		fds[0].events = POLLIN;

		uint8_t compress_buf[BLOCK_SIZE];

		uint8_t decompress_buf[BLOCK_SIZE];

	
		unlink("/tmp/inpipe");
		mkfifo("/tmp/inpipe", 0666);

		unlink("/tmp/outpipe");
		mkfifo("/tmp/outpipe", 0666);

		if (argc != 2) {
			printf("./gzip_so_stub <infile>\n");
			exit(1);
		}

		int ifd = open(argv[1], O_RDONLY);
		if(ifd < 0) {
			printf("could not open input file\n");
			exit(1);
		}
			printf("opened input file\n");

		printf("opening inpipe\n");
		send_to_compression_fd = open("/tmp/inpipe", O_WRONLY);
		if (send_to_compression_fd < 0) {
			printf("%s():open error\n", __func__);
			exit(1);
		}
		printf("opened compression pipe send\n");
		/*
		printf("open outpipe\n");
		recieve_from_compression_fd = open("/tmp/outpipe", O_RDONLY);
		if (recieve_from_compression_fd < 0) {
			printf("%s():open error\n", __func__);
			exit(1);
		}
		printf("opened compression pipe recv\n");
		*/
		// loop read from input file, write to gzip pipe

		printf("reading from input file\n");
		ret = read(ifd, compress_buf, BLOCK_SIZE);
		while (ret > 0) {
			printf("writing to compression fd\n");
			ret = write(send_to_compression_fd, compress_buf, BLOCK_SIZE);
		/*	
			printf("%s(): polling on fds\n",__func__);
			rc = ppoll (fds, 1, &t, &origmask);
			if (rc < 0) {
				perror( "poll failed");
				exit(1);
			}
	
			if (rc == 0) {
				printf("poll timed out\n");
			}else { 
				// optionally read back the data.
				printf("%s()trying to read from compression FD Now\n", __func__);
				if (fds[0].revents == POLLIN) {
					if (fds[0].fd == recieve_from_compression_fd) {
						ret = read (recieve_from_compression_fd, decompress_buf, BLOCK_SIZE);
						if (ret < 0) {
							printf("read from pipe failed %s\n", strerror(errno));
							exit(1);
						}
						printf("%s(): returned %d bytes\n", __func__,ret);
						//total_read_bytes += ret;
						ret = write (compressed_file_fd, decompress_buf, ret);
						if (ret < 0) {
							printf("write to outfile failed %s\n", strerror(errno));
							exit(1);
						}
					}
				}
			}
		*/
			ret = read(ifd, compress_buf, BLOCK_SIZE);
			if (ret < 0) {
				printf("read failed\n");
				exit(1);
			}
		}
		printf("done writing\n");
		ret = close(send_to_compression_fd);
		if(ret < 0) {
			printf("closing compression fd failed %s\n", strerror(errno));
			exit(1);
		}	
		/*	
		ret = read(recieve_from_compression_fd, decompress_buf, BLOCK_SIZE);
		if (ret < 0) {
			printf("reading from compression fd failed %s\n", strerror(errno));
			exit(1);
		}

		while(ret > 0) {
			printf("write decompress buf to compress file fd\n");
			ret = write (compressed_file_fd, decompress_buf, BLOCK_SIZE);
			if (ret < 0) {
				printf("read error\n");
				exit(1);
			}
			printf("read from recv compression fd\n");
			ret = read(recieve_from_compression_fd, decompress_buf, BLOCK_SIZE);
			if (ret < 0) {
				printf("read error\n");
				exit(1);
			}
		}
		
//}
	ret = close(recieve_from_compression_fd);
	if (ret < 0) {
		printf("didnt close recv compression fd properly %s\n", strerror(errno));
		exit(1);
	}
*/
	ret = close(compressed_file_fd);
	if (ret < 0) {
		printf("didnt close recv compression fd properly %s\n", strerror(errno));
		exit(1);
	}
	return 0;
}
