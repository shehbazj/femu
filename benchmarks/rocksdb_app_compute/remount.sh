echo "If mounted before, unmount.."
sudo umount /mnt/rocksdb_host
echo "mount nvme0n1 to rocksdb_host"
sudo mount -t ext4 -o dax /dev/pmem0 /mnt/rocksdb_host

while true; do
	ls -R /mnt/rocksdb_host
	sleep 1;
done


