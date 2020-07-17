all: write_ones host_count nsc_count stream_count

write_ones: write_ones.c
	gcc write_ones.c -o write_ones

host_count: host_count.c
	gcc host_count.c -o host_count

nsc_count: nsc_count.c
	gcc nsc_count.c -o nsc_count

stream_count: stream_count.c
	gcc -D_GNU_SOURCE stream_count.c -o stream_count

clean:
	rm -f write_ones host_count nsc_count stream_count

