echo "See Logs at"

echo "tail -f /tmp/rocksdbtest-1000/dbbench/LOG | grep -i Compacting"
echo "Press key to continue"
read x

echo "FIRST WE RUN THE RANDOM FILL BENCHMARK"

./fillrandom.sh

echo "NOW WE RUN read While Writing using existing database"

$HOME/rocksdb/db_bench --benchmarks=readwhilewriting \
--disable_auto_compactions=0 \
--db=$HOME/rocksdb/testdb \
--num=10000000 \
--num_levels=4 \
--level0_file_num_compaction_trigger=4 \
--max_background_compactions=4 \
--max_background_flushes=0 \
--compaction_style=3 \
--use_existing_db
#--level0_stop_writes_trigger=6 \
#--level0_slowdown_writes_trigger=4 \
