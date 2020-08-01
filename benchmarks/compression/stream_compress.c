#include "../stream_common/common.h"

void compress(int inputfile_fd, int fd)
{
	int err;
	off_t current_offset;

	unsigned char *buf;
	if (posix_memalign((void **)&buf, getpagesize(), IO_SEGMENT_SIZE)) {
                return;
	}

	memset(buf, 0, IO_SEGMENT_SIZE);

	printf("Opened fd from nw_thread: %d\n", fd);
	err = set_computational_stream_directive(fd, COMPRESSION_ENABLE);
	if (err<0){
		fprintf(stderr, "enable computational stream directive status:%#x(%s)\n", errno, strerror(errno));
		return;
	}else{
		printf("enable computational stream directive successful\n");
	}

	while (read(inputfile_fd, buf, IO_SEGMENT_SIZE) != 0) {
		err = write(fd, buf, IO_TRANSFER_SIZE);
		if (err < 0) {
			printf("compression failed\n");
		}
	}

	err = set_computational_stream_directive(fd, COMPRESSION_DISABLE);
	// disable before leaving...
	if (err<0){
		fprintf(stderr, "enable computational stream directive status:%#x(%s)\n", errno, strerror(errno));
	} else{
		printf("disable computational stream directive successful\n");
	}
	free(buf);
}

int main(int argc, char **argv)
{
	static const char *perrstr;
	int err, fd, i;
	int inputfile_fd;
	
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <inputfile> <device>\n", argv[0]);
		return 1;
	}

	perrstr = argv[1];
	fd = open(argv[1], IO_OPEN_OPTIONS);
	if (fd < 0){
		goto perror;
	}
	inputfile_fd = open(argv[2], IO_OPEN_OPTIONS);
	if (inputfile_fd < 0) {
		goto perror;
	}

	err = enable_stream_directive(fd);

	if (err<0){
		fprintf(stderr, "enable stream directive status:%#x(%s)\n", errno, strerror(errno));
	}else{
		printf("enable stream directive successful\n");
	}

	err = alloc_stream_resources(fd, 1);
	if (err<0){
		fprintf(stderr, "allocate stream resource status:%#x(%s)\n", errno, strerror(errno));
	}else{
		printf("allocate stream resource successful\n");
	}

	compress(inputfile_fd, fd);

//	testing if counting is disabled correctly.
//	computational_read(&f, 0);


   close(fd);
   close(inputfile_fd);
	return 0;

perror:
   close(fd);
	perror(perrstr);
	return 1;
}
