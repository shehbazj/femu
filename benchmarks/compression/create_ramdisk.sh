if [[ $# -ne 1 ]]; then
	"Usage: ./create_ramdisk.sh <size>"
	exit 1
fi

sz=$1

sudo mount -t tmpfs -o size="$sz"m tmpfs /mnt/ramdisk
