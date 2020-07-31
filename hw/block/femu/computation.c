#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "computation.h"

void init_gzip(int mode)
{
	typedef int (*some_func)(char *param, char *parm2 , int x);
	void *myso = dlopen("/home/shehbaz/femu/hw/block/femu/gzip_pipe_so/libgzip.so", RTLD_NOW);
	some_func *func = dlsym(myso, "gzip_me");
	char *i,*o;
	int ret;

	i = malloc(100);
	o = malloc(100);
	strncpy(i, "compression_pipe_send", strlen("compression_pipe_send") + 1);
	strncpy(o, "compression_pipe_recv", strlen("compression_pipe_recv") + 1);

	// gzip_me(recv pipe, send pipe and mode) where mode is 1 - compress, 2 - decompress.
	ret = (*func)(i ,o, mode);
	if (ret) {
		printf("unexpected end of gzip\n");
	}

	free(i);
	free(o);
	// close shared object when pipe gets closed.
	dlclose(myso);
}

uint64_t count_bits(char *buf)
{
        int i, j;
        char c;
        uint64_t count =0;

        for (i = 0 ; i < 4096 ; i++) {
                c = buf[i];
                for (j = 0 ; j < 8 ; j++) {
                        if (c & (1 << j)) {
                                count +=  1;
                        }
                }
        }
        return count;
}

uint64_t get_disk_pointer(char *buf)
{
	uint64_t *p = (uint64_t *)buf;
	return p[0];
}
