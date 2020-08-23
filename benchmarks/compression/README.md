	`stream_compress.c`
	
	- reads file from `inputfile_fd` and writes it to the underlying device.
	- the device will compress the data and only return when it receives end compression directive.

	`host_writer.c` and `host_zlib.c`

	- `host_writer` writes data at `write_chunk` grannularity to `/tmp/host_compression_pipe`
	- `host_zlib` reads data from `/tmp/host_compression_pipe` and writes to `/dev/nvme0n1`

	`host_fstream_compression.c` (Depricated)

	- run gzip operation in host by opening input file as a ifstream.
	- for compress, the resultant output gz file is redirected to `/dev/nvme0n1`.
