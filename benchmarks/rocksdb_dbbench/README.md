This folder contains the following scripts:

`fillrandom.sh` Minimal `db_bench` code that triggers compactions by filling rocksdb with random key sequences.
`read_while_writing.sh` 

`compaction_type.sh` kCompactionStyleNone - try running `db_bench` with this example. compaction should not run in this case. this type of workload will be used when compaction is run externally.


