BASEDIR=filesize

rm -rf $BASEDIR
mkdir -p $BASEDIR
# generate NSC benchmark

for FILESIZE in `seq 2 2 10`
do
echo "FILESIZE=$FILESIZE
BMNAME=compression.$FILESIZE.G_nsc
BM=compression
CPU=4
MEMORY=10G
DEVSIZE=10240
COMPUTEMODE=1
FLASH_READ_LATENCY=0
FLASH_WRITE_LATENCY=10000
CPU_LIMIT=50
#PREP_SCRIPTS=( disable_readahead.sh )
#PREP_ARG_START=( 0 )
#PREP_ARG_END=( 0 )
#PREP_ARGS=( )
TESTSCRIPT=stream_compression
PARAMS=( $FILESIZE )" > $BASEDIR/compression_$FILESIZE.G_nsc
done


# generate Host benchmark

for FILESIZE in `seq 2 2 10`
do
echo "FILESIZE=$FILESIZE
BMNAME=compression.$FILESIZE.G_host
BM=compression
CPU=4
MEMORY=10G
DEVSIZE=10240
COMPUTEMODE=0
FLASH_READ_LATENCY=0
FLASH_WRITE_LATENCY=10000
CPU_LIMIT=50
#PREP_SCRIPTS=( disable_readahead.sh )
#PREP_ARG_START=( 0 )
#PREP_ARG_END=( 0 )
#PREP_ARGS=( )
TESTSCRIPT=host_fstream_compression
PARAMS=( $FILESIZE )" > $BASEDIR/compression_$FILESIZE.G_host
done
