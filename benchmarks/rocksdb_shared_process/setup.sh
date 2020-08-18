# A file is created in host Ramdisk.

dd if=/dev/zero of=/dev/shm/mem bs=1M count=1000

##this file is mmapped and data is
#written to this mmaped region. Next, `msync` is issued such that the
#mmaped data is written to the Ramdisk.

