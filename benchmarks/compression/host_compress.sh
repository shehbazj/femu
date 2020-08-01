if [[ $# -ne 1 ]]; then
	echo "./host_compress.sh <infile>"
	exit
fi

gzip -c $1 > /dev/nvme0n1
