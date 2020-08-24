echo "Unmount file system if mounted"

sudo umount -l /mnt

echo "Create a file system with an external Journal Device"
yes | sudo mke2fs -t ext4 /dev/sdb

echo "Mount and change permissions"
sudo mount -o journal_checksum /dev/sdb /mnt
sudo chown -R vm:vm /mnt

echo ""
