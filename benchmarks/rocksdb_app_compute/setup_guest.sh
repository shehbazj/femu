if [[ ! -d /mnt/rocksdb_guest ]]; then
	echo "create rocksdb directory"
	sudo mkdir -p /mnt/rocksdb_guest
else
	echo "rocksdb guest directory already exists"
fi

echo "Make ext4 file system"

sudo mkfs.ext4 /dev/nvme0n1

echo "mount nvme0n1 to rocksdb_guest"

sudo mount /dev/nvme0n1 /mnt/rocksdb_guest

echo "change ownership"

sudo chown -R vm:vm /mnt/rocksdb_guest
