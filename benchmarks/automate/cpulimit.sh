FILES=`ls benchmark_config/compressionbm/cpulimit/*`
for f in $FILES;
do
	echo "F - $f"
#	echo $f
	./vm_ops.sh "$f"
done
