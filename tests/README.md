From: https://portal.xsede.org/sdsc-comet

The standard compute nodes consist of Intel Xeon E5-2680v3 (formerly codenamed
Haswell) processors, 128 GB DDR4 DRAM (64 GB per socket), and 320 GB of SSD
local scratch memory. The GPU nodes contain four NVIDIA GPUs each. The large
memory nodes contain 1.5 TB of DRAM and four Haswell processors each. The
network topology is 56 Gbps FDR InfiniBand with rack-level full bisection
bandwidth and 4:1 oversubscription cross-rack bandwidth. Comet has 7 petabytes
of 200 GB/second performance storage and 6 petabytes of 100 GB/second durable
storage. It also has dedicated gateway hosting nodes and a Virtual Machine
repository. External connectivity to Internet2 and ESNet is 100 Gbps.


Jobs can be submitted with: `sbatch jobscriptfile`

Check status with `squeue -u user`

Compile with:

    module load boost/1.55.0
    module load intel/2015.2.164
    module load mvapich2_ib