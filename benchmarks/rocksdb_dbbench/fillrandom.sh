echo "See Logs at"

echo "tail -f /tmp/rocksdbtest-1000/dbbench/LOG | grep -i Compacting"
echo "Press key to continue"
read x

$HOME/rocksdb/db_bench --benchmarks=fillrandom \
--db=$HOME/rocksdb/testdb \
--disable_auto_compactions=0 \
--num=10000000 \
--num_levels=6 \
--level0_file_num_compaction_trigger=4 \
--max_background_compactions=4 \
--max_background_flushes=0 \
--level0_slowdown_writes_trigger=4 \
--level0_stop_writes_trigger=6
