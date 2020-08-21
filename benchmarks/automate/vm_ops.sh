# This is an automated script to launch a VM
# and run one testcase within the VM, using the
# benchmark configuration file provided that contains
# more details about the type of benchmark to be
# run and associated parameters.

if [[ $# -ne 1 ]]; then
	echo "./vm_ops.sh benchmark_config_file"
	exit
fi

config_file=$1
source $config_file

if test -z "$TESTSCRIPT"; then
	echo "No TestScript Specified, please specify benchmark exec to run"
	exit 
fi

if test -z "$PARAMS"; then
	echo "Warning: No parameters for testscript $PARAMS"
	exit 
fi

get_prereq()
{
		i=$1
		script_name=${PREP_SCRIPTS[$i]}
		prep_arg_start=${PREP_ARG_START[$i]}
		prep_arg_end=${PREP_ARG_END[$i]}
		for j in `seq $prep_arg_start $prep_arg_end`
		do
			TEST_ARG+=${PREP_ARGS[$j]}
			TEST_ARG+=" "
		done
		echo $script_name $TEST_ARG
}

shutdown_vm()
{
	# ssh and run shutdown
	echo "Kill CPULIMIT if exists"
	sudo kill -9 `pgrep cpulimit` 2> /dev/null

	echo "Shut down VM"
	ssh -p 8080 -t vm@localhost "sudo shutdown now"
}

FEMU=$HOME/femu

OUTDIR=$BMNAME.`date +%s`

GUEST_DIR=autobm/$OUTDIR/

if ! [[ -d hostdir/$OUTDIR ]]; then
	mkdir -p hostdir/$OUTDIR
fi

# launch the VM
echo "launch the VM"

sudo kill -9 `pgrep cpulimit` 2> /dev/null
sudo kill -9 `pgrep qemu` 2> /dev/null

$FEMU/iscos_scripts/run-auto.sh $config_file > bootlog &

echo "Wait for VM to Boot"
sleep 30

# wait and copy path to VM
echo "Copy guest scripts from $BM to VM"

echo "create guest directory"
ssh -p 8080 -t vm@localhost "mkdir -p autobm/$OUTDIR/guest/$BMNAME"

echo "dir creation $?"

echo "copy bm to guest directory"
scp -P 8080 -r $FEMU/benchmarks/$BM/* vm@localhost:autobm/$OUTDIR/guest/$BMNAME
scp -P 8080 -r $FEMU/benchmarks/stream_common vm@localhost:autobm/$OUTDIR/guest/

echo "copy $?"

echo "Copy complete"

# set cpulimit if required
echo "set cpulimit if required"

QEMU_PID=`pgrep qemu | head -1`

echo "QEMU PID is $QEMU_PID"

sudo cpulimit -p $QEMU_PID -l $CPU_LIMIT -b

echo "Build Benchmark"

ssh -p 8080 -t vm@localhost "make -C autobm/$OUTDIR/guest/$BMNAME >> autobm/$OUTDIR/guest/$BMNAME/guest_log"

# ssh and run a benchmark

num_scripts=${#PREP_SCRIPTS[@]}

if [ $num_scripts -ne 0 ]; then
	echo "num scripts = $num_scripts"
	echo "Run prereq Benchmark"
	last_elem=`expr $num_scripts - 1`
	prev_param=0
	for i in `seq 0 $last_elem`
	do
		SCRIPTNAME=$(get_prereq $i)
		echo "RUN PREQ $SCRIPTNAME"
		ssh -p 8080 -t vm@localhost "sudo ./autobm/$OUTDIR/guest/$BMNAME/$SCRIPTNAME >> autobm/$OUTDIR/guest/$BMNAME/guest_log"
	done
fi

echo "Run test Benchmark"

ssh -p 8080 -t vm@localhost "sudo ./autobm/$OUTDIR/guest/$BMNAME/$TESTSCRIPT ${PARAMS[@]} >> autobm/$OUTDIR/guest/$BMNAME/guest_log"

echo "Copy results into outdir"

scp -P 8080 vm@localhost:autobm/$OUTDIR/guest/$BMNAME/guest_log hostdir/$OUTDIR/

echo "Kill CPULimits App if required"

sudo kill -9 `pgrep cpulimit` 2> /dev/null

echo "Shutdown VM"
shutdown_vm


echo "Kill instance and wait for 10 seconds"
sudo kill -9 `pgrep qemu` 2> /dev/null

sleep 10
echo "Check for qemu instance"
QEMU_PID=`pgrep qemu`

if [[ $QEMU_PID ]]; then
	echo "QEMU $QEMU_PID still active!"
	exit 1
fi
