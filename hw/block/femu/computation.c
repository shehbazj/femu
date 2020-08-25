#include "computation.h"

extern int zlib_me(FILE *ifstream, FILE *ofstream, int mode);

void init_gzip(int mode)
{
	char *i,*o;
	int ret;
	FILE *ifile, *ofile;

	i = malloc(100);
	o = malloc(100);
	strncpy(i, "compression_pipe_send", strlen("compression_pipe_send") + 1);
	strncpy(o, "compression_pipe_recv", strlen("compression_pipe_recv") + 1);

	printf("COMPUTATION PROCESS -- Wait for Compression\n");

	ifile = fopen(i, "r");
	ofile = fopen(o, "w");

	ret = zlib_me(ifile, ofile, 1);
	if (ret) {
		printf("unexpected end of gzip\n");
	}
	printf("COMPUTATION PROCESS -- Done with Compression\n");

	free(i);
	free(o);
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