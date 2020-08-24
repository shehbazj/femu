echo "Unmount file system if mounted"

sudo umount -l /mnt

echo "Create a Journal Device"

yes | sudo mke2fs -O journal_dev /dev/nvme0n1

echo "Create a file system with an external Journal Device"
yes | sudo mke2fs -t ext4 -J device=/dev/nvme0n1 /dev/sdb

echo "Mount and change permissions"
sudo mount -o journal_checksum /dev/sdb /mnt
sudo chown -R vm:vm /mnt

echo ""
