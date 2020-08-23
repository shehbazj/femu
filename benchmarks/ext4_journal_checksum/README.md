Mounting a separate journal device:

Create a journal device
`mke2fs -O journal_dev /dev/nvme0n1`

Point the journal to the file system device
`mke2fs -t ext4 -J device=/dev/nvme0n1`/dev/sdb`

