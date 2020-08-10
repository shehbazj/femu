all: stream_compression stream_decompress host_compression

stream_compression: stream_compression.c
	gcc ../stream_common/common.c stream_compression.c -o stream_compression

stream_decompress: stream_decompress.c
	gcc ../stream_common/common.c stream_decompress.c -o stream_decompress

host_compression: host_compression.c
	gcc host_compression.c -lz -o host_compression


clean:
	rm -rf stream_decompress stream_compression host_compression
