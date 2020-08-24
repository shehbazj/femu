#include "../stream_common/common.h"

#include <unistd.h>
#include <stdint.h>
int main(int argc, char **argv)
{
	static const char *perrstr;
	struct sdm f;
	int err, fd, i;
	int mode;

	next = time(NULL); // random seed

	if (posix_memalign((void **)&f.data_in, getpagesize(), IO_SEGMENT_SIZE))
                goto perror;

	if (posix_memalign((void **)&f.data_out, getpagesize(), IO_SEGMENT_SIZE))
                goto perror;

	memset(f.data_in, 0, IO_SEGMENT_SIZE);
	memset(f.data_out, 0, IO_SEGMENT_SIZE);

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <device> <mode>\n", argv[0]);
		fprintf(stderr, "mode : 0 enable 1 disable\n");
		return 1;
	}

	perrstr = argv[1];
	f.fn = argv[1];		
	fd = open(argv[1], O_RDWR | O_DIRECT | O_LARGEFILE);
	if (fd < 0){
		goto perror;
	}

	printf("Opened fd from main: %d\n", fd);

	mode = strtol(argv[2], NULL, 10);
//	if (mode == LONG_MIN || mode == LONG_MAX || errno) {
	if (errno) {
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

	if (mode == 1) {
		err = set_computational_stream_directive(fd, JBD2_CHECKSUM_ENABLE);
	} else {
		err = set_computational_stream_directive(fd, JBD2_CHECKSUM_DISABLE);
	}

     	close(fd);
	free(f.data_in);
	free(f.data_out);
	return 0;

perror:
	close(fd);
	free(f.data_in);
	free(f.data_out);
	perror(perrstr);
	return 1;
}
