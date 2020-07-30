#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <linux/nvme_ioctl.h>

#define DEBUG 1
#include "common.h"
#define POSIX_FADV_STREAMID 8 
#define IO_TRANSFER_SIZE (4*1024) 
#define IO_ST_TRANSFER_SIZE (16*1024) 
#define IO_SEGMENT_SIZE (IO_TRANSFER_SIZE * 8) 
#define MAX_FILE_OFFSET (IO_TRANSFER_SIZE * 10)

#define IO_OFFSET_NW 0x000 //0xa000 

#define nvme_admin_directive_send 0x19
#define nvme_admin_directive_recv 0x1a

//#define IO_OPEN_OPTIONS (O_RDWR | O_DIRECT | O_LARGEFILE)
#define IO_OPEN_OPTIONS (O_RDWR | O_DIRECT | O_NONBLOCK | O_ASYNC)

static unsigned long next;

struct sdm {
	char *fn;
	unsigned int sid;
	unsigned char *data_in;
	unsigned char *data_out;
};

int enable_computational_stream_directive (int fd);
int disable_computational_stream_directive (int fd);

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
		err = enable_computational_stream_directive(fd);
		if (err<0){
			fprintf(stderr, "enable computational stream directive status:%#x(%s)\n", errno, strerror(errno));
		}else{
			printf("enable computational stream directive successful\n");
		}

		posix_fadvise(fd, f->sid, 0, POSIX_FADV_STREAMID);
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
		err = disable_computational_stream_directive(fd);
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

int nvme_dir_send(int fd, __u32 nsid, __u16 dspec, __u8 dtype, __u8 doper,
                  __u32 data_len, __u32 dw12, void *data, __u32 *result)
{
        struct nvme_admin_cmd cmd = {
                .opcode         = nvme_admin_directive_send,
                .addr           = (__u64)(uintptr_t) data,
                .data_len       = data_len,
                .nsid           = nsid,
                .cdw10          = data_len? (data_len >> 2) - 1 : 0,
                .cdw11          = dspec << 16 | dtype << 8 | doper,
                .cdw12          = dw12,
        };
        int err;

        err = ioctl(fd, NVME_IOCTL_ADMIN_CMD, &cmd);
        if (!err && result)
                *result = cmd.result;
        return err;
}

int nvme_dir_recv(int fd, __u32 nsid, __u16 dspec, __u8 dtype, __u8 doper,
                  __u32 data_len, __u32 dw12, void *data, __u32 *result)
{
        struct nvme_admin_cmd cmd = {
                .opcode         = nvme_admin_directive_recv,
                .addr           = (__u64)(uintptr_t) data,
                .data_len       = data_len,
                .nsid           = nsid,
                .cdw10          = data_len? (data_len >> 2) - 1 : 0,
                .cdw11          = dspec << 16 | dtype << 8 | doper,
                .cdw12          = dw12,
        };
        int err;

        err = ioctl(fd, NVME_IOCTL_ADMIN_CMD, &cmd);
        if (!err && result)
                *result = cmd.result;
        return err;
}

int nvme_get_nsid(int fd)
{
        static struct stat nvme_stat;
        int err = fstat(fd, &nvme_stat);

        if (err < 0)
                return err;

        if (!S_ISBLK(nvme_stat.st_mode)) {
                fprintf(stderr,
                        "Error: requesting namespace-id from non-block device\n");
                exit(ENOTBLK);
        }
        return ioctl(fd, NVME_IOCTL_ID);
}

int enable_stream_directive(int fd)
{
	__u32 result;
	int err;
	int nsid = nvme_get_nsid(fd);

	printf("Enable stream directive for nsid %d\n", nsid);
	// dspec, dtype, doper, length, dw12, data, result
	err = nvme_dir_send(fd, nsid, 0, 0, 1, 0, 0x101, NULL, &result);
	return err;
}

/* 0x401 = 0b 0100 0000 0001 this enables counting (dw12 << 10 = 01)
   0x801 = 0b 1000 0000 0001 this enables pointer chase (dw12 << 10 = 02)
*/

int enable_computational_stream_directive (int fd)
{
	__u32 result;
	int err;
	int nsid = nvme_get_nsid(fd);

	printf("Enable stream directive for nsid %d\n", nsid);
	// dspec, dtype, doper, length, dw12, data, result
	err = nvme_dir_send(fd, nsid, 0, 0, 1, 0, 0x801, NULL, &result);
	return err;
}

// disables pointer chase operation.
int disable_computational_stream_directive (int fd)
{
	__u32 result;
	int err;
	int nsid = nvme_get_nsid(fd);

	printf("Enable stream directive for nsid %d\n", nsid);
	// dspec, dtype, doper, length, dw12, data, result
	err = nvme_dir_send(fd, nsid, 0, 0, 1, 0, 0x800, NULL, &result);
	return err;
}

int alloc_stream_resources(int fd, unsigned int rsc_cnt)
{
	__u32 result;
	int err;
	int nsid = nvme_get_nsid(fd);

	printf("Allocate stream resource for nsid %d\n", nsid);
	err = nvme_dir_recv(fd, nsid, 0, 1, 3, 0, rsc_cnt, NULL, &result);
	if (err==0)
		printf("  requested %d; returned %d\n", rsc_cnt, result & 0xffff);
	return err;
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

	f.sid = 1;

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
