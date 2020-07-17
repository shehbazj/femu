This folder contains the counting benchmark that writes ones to the first 10 blocks
in a file and then reads them back in two modes, host and nsc (near storage compute).

The following files are added:

`write_ones.c`
writes the digit 1 on each byte of first 10 4K blocks of a file

`host_count.c`
reads and counts the number of 1's in the first 10 4K blocks within the host. Please
run using `computation_mode=0` parameter while creating FEMU.

`nsc_count.c` (WARN: depricated, please use `stream_count.c`)
reads first 10 4K blocks in the device and does not do any computation. The computation
is done in host and individually printed for each block. This should be run when FEMU
is run with `computation_mode=1`

`stream_count.c`
enables directive to count data being read from a stream.
this replaces `nsc_count` that required hardcoded values in device to count bits being read.
It reads `MAX_FILE_OFFSET` bytes of data from the underlying device. the count directive
is enabled by default. a subsiquent read is done with the count directive disabled to test
if counting continues after disabling the directive are not.

First `write_ones` program is executed, followed by either `host_count` or `nsc_count`
based on the `computation_mode` parameter set while instantiating a FEMU VM.

Please also change `hw/block/femu/computation.h` and define COUNTING and comment any
other computational workload.
Computation mode will later be made configurable by the host.
