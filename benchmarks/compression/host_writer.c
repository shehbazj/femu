/* zpipe.c: example of proper use of zlib's inflate() and deflate()
   Not copyrighted -- provided to the public domain
   Version 1.4  11 December 2005  Mark Adler */

/* Version history:
   1.0  30 Oct 2004  First version
   1.1   8 Nov 2004  Add void casting for unused return values
                     Use switch statement for inflate() return values
   1.2   9 Nov 2004  Add assertions to document zlib guarantees
   1.3   6 Apr 2005  Remove incorrect assertion in inf()
   1.4  11 Dec 2005  Add hack to avoid MSDOS end-of-line conversions
                     Avoid some compiler warnings for input and output buffers
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#define BLOCK_SIZE 4096

bool check_root()
{
	if (geteuid() == 0) {
		return true;
	}
	else {
		return false;
	}
}

static __inline__ unsigned long long rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

/* compress or decompress from stdin to stdout */
int main(int argc, char **argv)
{
    	int ret;
	unsigned long long start, end;
	int s_gb;
	size_t sz;
	int fd;

	FILE *ofstream;
	int input_fd;

	char buf[BLOCK_SIZE];
	if (argc != 2) {
		printf("./host_compression <size in GB>\n");
		exit (1);
	}
	
	if (check_root() == false) {
		printf("Please run as sudo\n");
		exit(1);
	}
	sscanf(argv[1], "%d", &s_gb);

	printf("creating file of size %d GB\n", s_gb);

	char cmd[200];
	char infile[100];
#ifdef RAMDISK
	strcpy (infile, "/mnt/ramdisk/IPFILE");
#else
	strcpy (infile, "IPFILE");
#endif
	sprintf (cmd, "dd if=/home/vm/10G of=%s bs=1G count=%d", infile, s_gb);
	system(cmd);

	input_fd = open(infile, O_RDONLY);
	if (input_fd < 0) {
		printf("inpipe failed\n");
		exit(1);
	}
	ofstream = fopen("/tmp/host_compression_pipe", "w");
	if (ofstream == NULL) {
		printf("infile failed %s\n", strerror(errno));
		exit(1);
	}

	/*
	char c;
	printf("SET CPU Limit\n");
	printf("cpulimit -p <process> -l <limit>\n");
	printf("Press key once CPU Limit has been set\n");
	scanf("%c", &c);
	*/

	/*
	char c;
	printf("Please run ./host_zlib if not running already\n");
	scanf("%c", &c);
	*/

	start = rdtsc();	
	// read data from disk into buffer
	ret = read(input_fd, buf, BLOCK_SIZE);
	while (ret > 0) {
		// write buffer to pipe
		ret = fwrite(buf, BLOCK_SIZE, 1, ofstream);
	       	if (ret < 0) {
			printf("write failed %s\n", strerror(errno));
			exit(1);
		}
		ret = read(input_fd, buf, BLOCK_SIZE);
		if(ret < 0) {
			printf("read failed %s\n", strerror(errno));
		}
	}
	end = rdtsc();

        printf("CYCLE: %llu\n",end - start);
        fclose(ofstream);

        return ret;
}
