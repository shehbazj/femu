echo "Run after guest"

echo "Press key"
read x

if [[ ! -d /mnt/rocksdb_host ]]; then
	echo "create rocksdb directory"
	sudo mkdir -p /mnt/rocksdb_host
else
	echo "rocksdb host directory already exists"
fi

echo "If mounted before, unmount.."
sudo umount /mnt/rocksdb_host
echo "mount nvme0n1 to rocksdb_host"
sudo mount -t ext4 -o dax /dev/pmem0 /mnt/rocksdb_host

#while true; do
#	ls -R /mnt/rocksdb_host
#	sleep 1;
#done
echo "change ownership"

sudo chown -R shehbaz:shehbaz /mnt/rocksdb_host

echo "run secondary"
