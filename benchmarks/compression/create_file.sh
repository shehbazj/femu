if [[ $# -ne 1 ]]; then
	echo "Usage: ./create_file.sh <infile in GB>"
	exit
fi

filesize=$1
ofname=$1.GB_raw

dd if=/dev/urandom of=$ofname bs=${filesize}M count=1000
