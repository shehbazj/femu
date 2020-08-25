#ifndef COMMON_H
#define COMMON_H

#define _GNU_SOURCE
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

#define POSIX_FADV_STREAMID 8 
#define IO_TRANSFER_SIZE (4*1024) 
#define IO_ST_TRANSFER_SIZE (16*1024) 
#define IO_SEGMENT_SIZE (IO_TRANSFER_SIZE * 8) 
#define MAX_FILE_OFFSET (IO_TRANSFER_SIZE * 10)

#define IO_OFFSET_ST 0x2000
#define IO_OFFSET_NW 0x0

#define nvme_admin_directive_send 0x19
#define nvme_admin_directive_recv 0x1a

#define IO_OPEN_OPTIONS (O_RDWR | O_DIRECT | O_NONBLOCK | O_ASYNC)

unsigned long next;

struct sdm {
	char *fn;
	unsigned char *data_in;
	unsigned char *data_out;
};

/* 
	0b is for binary.
   0x401 = 0b 0100 0000 0001 this enables counting (dw12 >> 10 = 01)
   0x801 = 0b 1000 0000 0001 this enables pointer chase (dw12 >> 10 = 02)
*/
enum ComputationalDirectiveType {
	COUNTING_ENABLE		= 0x401,
	COUNTING_DISABLE	= 0x400,
	POINTER_CHASE_ENABLE  = 0x801,
	POINTER_CHASE_DISABLE = 0x800,
	JBD2_CHECKSUM_ENABLE	= 0x0c01,
	JBD2_CHECKSUM_DISABLE	= 0x0c00,
	COMPRESSION_ENABLE	= 0x1001,
	COMPRESSION_DISABLE	= 0x1000,
};

int nvme_dir_send(int fd, __u32 nsid, __u16 dspec, __u8 dtype, __u8 doper,
                  __u32 data_len, __u32 dw12, void *data, __u32 *result);

int nvme_dir_recv(int fd, __u32 nsid, __u16 dspec, __u8 dtype, __u8 doper,
                  __u32 data_len, __u32 dw12, void *data, __u32 *result);


int nvme_get_nsid(int fd);
int enable_stream_directive(int fd);
int alloc_stream_resources(int fd, unsigned int rsc_cnt);
int set_computational_stream_directive (int fd, enum ComputationalDirectiveType computation_type);

#endif
