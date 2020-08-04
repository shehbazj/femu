CherryPicked Code from gzip and a simple makefile that just builds the piped implementation of gzip.

The resultant gzip.so is linked to FEMU so that FEMU can call `gzip_me` function with appropriate arguments.

A helper file that calls `gzip_me()` in gzip.so can be found in `benchmarks/functionality_tests/compression/tester`

Testing the so file can be done using the steps below:

```
rm -f gzip_so_stub.c
cd `pwd`
make
cp ~/femu/benchmarks/functionality_tests/compression/tester/gzip_so_stub.c .
export LD_LIBRARY_PATH=`pwd`
gcc -L`pwd` -Wall -o gzip_so_stub gzip_so_stub.c -lgzip
```

