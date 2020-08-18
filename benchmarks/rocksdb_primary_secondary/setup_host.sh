echo "Run after guest"

echo "Press key"
read x

if [[ ! -d /mnt/rocksdb_host ]]; then
	echo "create rocksdb directory"
	sudo mkdir -p /mnt/rocksdb_host
else
	echo "rocksdb host directory already exists"
fi

echo "mount nvme0n1 to rocksdb_host"

sudo mount -o ro /dev/shm/memfile /mnt/rocksdb_host

#echo "change ownership"

#sudo chown -R shehbaz:shehbaz /mnt/rocksdb_guest

#echo "run secondary"
