all: stream_compression host_writer host_zlib host_fstream_compression

stream_compression: stream_compression.c
	gcc ../stream_common/common.c stream_compression.c -o stream_compression

stream_decompress: stream_decompress.c
	gcc ../stream_common/common.c stream_decompress.c -o stream_decompress

host_writer: host_writer.c
	gcc host_writer.c -o host_writer

host_zlib: host_zlib.c
	gcc host_zlib.c -lz -o host_zlib

host_fstream_compression: host_fstream_compression.c
	gcc host_fstream_compression.c -lz -o host_fstream_compression

clean:
	rm -rf stream_decompress stream_compression host_fstream_compression host_writer host_zlib
