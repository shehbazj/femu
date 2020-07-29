all: gzip

gzip:
	gcc -c *.c -I . -I lib/ -fPIC
	gcc -shared -o libgzip.so *.o

clean:
	rm -f *.o *.so gzip_so_stub
