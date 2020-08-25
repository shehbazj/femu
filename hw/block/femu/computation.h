// Temporary file defining what to do in computation. This will be removed later
// when we add the compute operations in NVMe commands.

// pointer chasing macros.

#ifndef COMPUTATION_H
#define COMPUTATION_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define END_BLOCK_MAGIC 99999
#define BLOCK_SIZE 4096

uint64_t count_bits(char *buf);
uint64_t get_disk_pointer(char *buf);
//void get_jbd2_checksum(char *buf);
void init_gzip(int);
#endif
