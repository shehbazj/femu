#include "../stream_common/common.h"

void computational_read(void *x, int enable)
{
	int fd;
	struct sdm *f = (struct sdm *)x;

	fd = open(f->fn, IO_OPEN_OPTIONS);
	if (fd < 0) {
		fprintf(stderr, "file %s open error\n", f->fn);
	}
	else {
		int err;
		off_t current_offset;
		printf("Opened fd from nw_thread: %d\n", fd);
		/* stream id is persistent in the kernel for an open fd.
		   If a normal write is intented while at a stream is open, it
		   is suggested to write a stream_id of 0 before the write */ 
		if (enable) {
			err = set_computational_stream_directive(fd, COUNTING_ENABLE);
		} else {
			err = set_computational_stream_directive(fd, COUNTING_DISABLE);
		}
		if (err<0){
			fprintf(stderr, "enable computational stream directive status:%#x(%s)\n", errno, strerror(errno));
		}else{
			printf("enable computational stream directive successful\n");
		}

		for (current_offset = IO_OFFSET_NW ; current_offset < MAX_FILE_OFFSET ; current_offset += IO_TRANSFER_SIZE) {
			err = pread(fd, f->data_in, IO_TRANSFER_SIZE, current_offset); 
			if (err<0) {
				fprintf(stderr, "nvme write from nw_thread status:%#x(%s) \n", errno, strerror(errno));
				break;
			}else if (err != IO_TRANSFER_SIZE) {
				printf("nvme size written from nw_thread: %d\n", err);
				break;
			}else {
				printf("READ DONE: \n");
			}
		}
		if(enable) {
			// disable before leaving...
			err = set_computational_stream_directive(fd, COUNTING_DISABLE);
			if (err<0){
				fprintf(stderr, "enable computational stream directive status:%#x(%s)\n", errno, strerror(errno));
			} else{
				printf("enable computational stream directive successful\n");
			}
		}
	}
	close(fd);
	return;
}

int main(int argc, char **argv)
{
	static const char *perrstr;
	struct sdm f;
	int err, fd, i;

	next = time(NULL); // random seed

	if (posix_memalign((void **)&f.data_in, getpagesize(), IO_SEGMENT_SIZE))
                goto perror;

	if (posix_memalign((void **)&f.data_out, getpagesize(), IO_SEGMENT_SIZE))
                goto perror;

	memset(f.data_in, 0, IO_SEGMENT_SIZE);
	memset(f.data_out, 0, IO_SEGMENT_SIZE);

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <device>\n", argv[0]);
		return 1;
	}

	perrstr = argv[1];
	f.fn = argv[1];		
	fd = open(argv[1], O_RDWR | O_DIRECT | O_LARGEFILE);
	if (fd < 0){
		goto perror;
	}
	printf("Opened fd from main: %d\n", fd);

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

	computational_read(&f, 1);

//	testing if counting is disabled correctly.
//	computational_read(&f, 0);


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
