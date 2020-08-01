	`host_compress.sh` and `host_decompress.sh`

	- run gzip and gunzip operation at the host respectively.
	- for compress, the resultant output gz file is redirected to /dev/nvme0n1.
	- for decompress, contents in /dev/nvme0n1 are decompressed.

	`stream_compress.c`
	
	- reads file from `inputfile_fd` and writes it to the underlying device.
	- the device will compress the data and only return when it receives end compression directive.
