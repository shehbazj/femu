This is a functionality test for checking if two rocksDB processes
can simulataneously access a shared database in read and write mode.

A file is created in host Ramdisk. this file is mmapped and data is
written to this mmaped region. Next, `msync` is issued such that the
mmaped data is written to the Ramdisk.

Another process opens and reads the file from host ramdisk in read-only
mode, and reads first few bytes every 10 seconds. After msync operation,
the data should be visible by the second process.
