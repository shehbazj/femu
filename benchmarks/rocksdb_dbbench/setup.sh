# This script is used to set everything up in the guest VM before
# running RocksDB

sudo mkfs.ext4 /dev/nvme0n1
sudo mount /dev/nvme0n1 /mnt
sudo chown -R vm:vm /mnt

