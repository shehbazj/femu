#./vm_ops.sh benchmark_config/compressionbm/cpulimit/compression_60.percent_nsc
#FILES=`ls benchmark_config/compressionbm/filesize/*`
FILES=`ls benchmark_config/compressionbm/cpulimit/*`
for f in $FILES;
do
	echo "F - $f"
#	echo $f
	./vm_ops.sh "$f"
done


#FILES=`ls benchmark_config/compressionbm/filesize/*`
FILES=`ls benchmark_config/compressionbm/filesize/*`
for f in $FILES;
do
	echo "F - $f"
#	echo $f
	./vm_ops.sh "$f"
done
