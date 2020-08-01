if [[ $# -ne 1 ]]; then
	echo "./host_decompress.sh <outputfile>"
	exit
fi

gunzip -c $1 > infile
