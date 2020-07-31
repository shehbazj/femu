	`gzip_tester.c`

	This program is a test harness for writing input file to a piped gzipped implementation
	and reading compressed data back from the gzip file and writing the resultant data to
	an output file `of.gz`. gzip implementation does *not* guaranee that a block sent to it for
	compression will lead to a compressed block, and generally waits for more incomming data
	before writing to the disk. Hence this program uses a non-blocking `poll` instead of
	a blocking `read` call to the pipe to read compressed data from the gzip computation.

	Similarly, the `decompression` workflow writes a compressed data to the gzip `computational_pipe_send`
	pipe and receives a decompressed data that is written to the hardcoded file named `if`.

	* COMPRESSION:

	- This program writes to `computational_pipe_recv`.
	- The `computational_pipe_recv` is read by the gzip program which compresses the data 
	and writes it to the computational_pipe_send file.
	- This program then reads the compressed data off of `computational_pipe_send`
	and writes it to of.gz.
	- outfile when gunzipped and compared with input file shall match inputfile.

	* DECOMPRESSION:	

	- This program writes to `computation_pipe_recv`.
	- The `computational_pipe_recv` is read by the gzip program which decompresses the
	data and writes it to the `computational_pipe_send` file.
	- This program then reads the decompressed data off of `computational_pipe_send`
	and writes it to infile.
	- infile when matched with originalfile should match.

	Eg. Test Run:

	`export LD_LIBRARY_PATH=/home/shehbaz/femu/hw/block/femu/gzip_pipe_so`

	1. For compression:
	```
	./gzip_tester inputfile 1	
	../build/gzip computational_pipe_recv computational_pipe_send 1
	```
	2. For decompression:
	```
	./gzip_tester of.gz 2
	../build/gzip computational_pipe_recv computational_pipe_send 2
	```

	`gzip_so_stub.c`

	This is a stub to test the functionality of gzip.so located in `hw/block/femu/gzip_pipe_so`
	The code is compiled with the shared object file and can be used instead of `../build/gzip`
