/*
	gzip_so_stub.c - a stub file that shall be compiled with the shared
	library gzip.so located in hw/block/femu/gzip_pipe_so/
*/

#include <stdio.h>
#include <stdlib.h>

extern int gzip_me(char *infile, char *outfile, int mode);

int main(int argc, char *argv[])
{
        if (argc != 4) {
                printf("usage gzip infile outfile mode\n");
                printf("mode\n");
                printf("1-compression\n");
                printf("2-decompression\n");
                exit(0);
        }

        int option;
        option = strtol(argv[3], NULL, 10);
        int ret = gzip_me(argv[1], argv[2], option);
        return ret;
}
