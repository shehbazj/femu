#include "../stream_common/common.h"
#define DEBUG 1
#include "common.h"

/* RAND_MAX assumed to be 32767 */
unsigned char myrand(void) {
	return (unsigned char)(0xAA);
//    next = next * 1103515245 + 12345;
//    return((unsigned)(next/65536) % 32768) & 0xFF;
}

void computational_read(void *x)
{
	int fd;
	struct sdm *f = (struct sdm *)x;

	FILE* head_pointer_list_ptr;
	int head_pointer_list_fd;
	struct stat statbuf;

	uint64_t current_block, next_block;
	uint64_t head_pointer;

	int ret;
	char *data = aligned_alloc(4096, 4096);
	char *region;
	off_t disk_off;

	head_pointer_list_ptr = fopen("hp.dat", "r");
	if (head_pointer_list_ptr == NULL) {
		printf("hp.dat open error: %s\n", strerror(errno));
		exit(1);
	}

	head_pointer_list_fd = fileno(head_pointer_list_ptr);
	if (fstat (head_pointer_list_fd, &statbuf) < 0)
	{
		printf ("fstat error\n");
		exit(1);
	}

	if ((region = mmap (0, statbuf.st_size, PROT_READ, MAP_SHARED,head_pointer_list_fd, 0)) == MAP_FAILED)
	{
		printf ("mmap error for input");
		exit(1);
	}

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
		err = set_computational_stream_directive(fd, POINTER_CHASE_ENABLE);
		if (err<0){
			fprintf(stderr, "enable computational stream directive status:%#x(%s)\n", errno, strerror(errno));
		}else{
			printf("enable computational stream directive successful\n");
		}

		// IO_TRANSFER_SIZE = 4K 
		// Grannularity of Transfer
		// A read on a pointer_chase_enabled stream should return end of block with END_BLOCK_MAGIC
		start_time = rdtsc();
//		for (current_offset = IO_OFFSET_NW ; current_offset < MAX_FILE_OFFSET ; current_offset += IO_TRANSFER_SIZE) {
		while (fscanf(head_pointer_list_ptr, "%lu", &head_pointer) != EOF) {
			debug_print("Parsing LL with head %lu\n", head_pointer);
			current_block = head_pointer;
			disk_off = lseek(fd, current_block * BLOCK_SIZE, SEEK_SET);
			if (disk_off != current_block * BLOCK_SIZE) {
				printf("Error: could not seek %s\n", strerror(errno));
				exit (1);
			}
			// the read is issued with fd pointed at head, it would call pointer chase 
			// which would eventually return the tail corresponding to the head that
			// would get stored in data buffer.
			err = read(fd, data, IO_TRANSFER_SIZE); 
			if (err<0) {
				fprintf(stderr, "nvme write from nw_thread status:%#x(%s) \n", errno, strerror(errno));
				break;
			}else if (err != IO_TRANSFER_SIZE) {
				printf("nvme size written from nw_thread: %d\n", err);
				break;
			}else {
				printf("READ DONE: \n");
			}
			memcpy(&next_block, data, sizeof(next_block));
			printf("next block = %lu\n", next_block);
		//	assert (next_block == END_BLOCK_MAGIC);
		}
		end_time = rdtsc();
		// disable computation
		err = set_computational_stream_directive(fd, POINTER_CHASE_DISABLE);
		if (err<0){
			fprintf(stderr, "enable computational stream directive status:%#x(%s)\n", errno, strerror(errno));
		}else{
			printf("enable computational stream directive successful\n");
		}
	}

	free(data);
	munmap(region, statbuf.st_size);

	start = ( ((uint64_t)cycles_high << 32) | cycles_low );
        end = ( ((uint64_t)cycles_high1 << 32) | cycles_low1 );
        printf("cycles spent: %lu\n",end - start);

	close(fd);
	fclose(head_pointer_list_ptr);
}

/* 0x401 = 0b 0100 0000 0001 this enables counting (dw12 << 10 = 01)
   0x801 = 0b 1000 0000 0001 this enables pointer chase (dw12 << 10 = 02)
*/

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

	computational_read(&f);

	// when computational_read is disabled, it reads host block one by one.

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
