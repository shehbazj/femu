This is a functionality test to check two processes primary and secondary
opening the same database instance are able to access the contents of the
database correctly.

`./setup_guest.sh` (to be run in guest)
mounts /dev/nvme0n1 in guest at `/mnt/rocksdb_guest`.
changes mode appropriately.

`./setup_host.sh` (to be run on host)
FEMU needs to be run before host rocksdb is setup.
mounts the file as rdonly in host at `/mnt/rocksdb_host`.
changes the mode as shehbaz:shehbaz in host,
waits for secondary to be run.

The `primary` will be used as part of FEMU.
Primary runs inside the VM and uses /dev/nvme0n1 disk that is mounted
within the device.

The `secondary` will be used as part of the NSC process.
secondary mounts the ramdisk file (/dev/shm/memfile) in the host
as readonly file and reads contents of the file.
