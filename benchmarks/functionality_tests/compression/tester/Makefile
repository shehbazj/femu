all: gzip_tester gzip_so_stub

gzip_tester: gzip_tester.c
	gcc gzip_tester.c -o gzip_tester

gzip_so_stub: gzip_so_stub.c
	gcc gzip_so_stub.c -L ../../../../femu/hw/block/femu/gzip_pipe_so -lgzip -o gzip_so_stub

clean:
	rm -rf gzip_tester gzip_so_stub
