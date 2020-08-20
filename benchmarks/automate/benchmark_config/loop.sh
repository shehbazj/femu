source counting

num_scripts=${#PREP_SCRIPTS[@]}
last_elem=`expr $num_scripts - 1`

prev_param=0

CMD=()

for i in `seq 0 $last_elem`
do
	script_name=${PREP_SCRIPTS[$i]}
#	echo $script_name
	num_script_params=${PREP_ARG_NUM[$i]}
	last_script_param=`expr $prev_param + $num_script_params - 1`
	for j in `seq $prev_param $last_script_param`
	do
		TEST_ARG+=${PREP_ARGS[$j]}
	done
	prev_param=`expr $prev_param + $num_script_params`
	echo $script_name $TEST_ARG
	CMD+=($script_name $TEST_ARG)
done

echo ${CMD[@]}
