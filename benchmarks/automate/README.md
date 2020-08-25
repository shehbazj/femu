- Setup

Install cpulimit on host
Configure guest VM with passwordless access by editing visudo
```
sudo visudo
vm ALL=(ALL)	NOPASSWD:ALL
```

- Helper Scripts

`vm_ops.sh`

Script that copies and runs benchmarks within the FEMU VM.
The arguments to the script are picked up from a configuration
file specified in `benchmark_config` folder

`benchmark_config`
Folder containing parameters to benchmark various VM configurations
Copy template to a benchmark name file
run `./vm_ops.sh <benchmark_config/bmname`

`hostdir`
Output directory containing benchmark.timestamp folder containing
logs of the benchmark that was run.

``
