FILES=`ls benchmark_config/compressionbm/filesize/*`
for f in $FILES;
do
	echo "F - $f"
#	echo $f
	./vm_ops.sh "$f"
done
