all: stream_compress stream_decompress

stream_compress: stream_compress.c
	gcc ../stream_common/common.c stream_compress.c -o stream_compress

stream_decompress: stream_decompress.c
	gcc ../stream_common/common.c stream_decompress.c -o stream_decompress

clean:
	rm -rf stream_compress stream_compress
