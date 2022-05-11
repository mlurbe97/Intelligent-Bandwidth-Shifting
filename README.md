# Intelligent Bandwidth Shifting

## Implementation author

* Manel Lurbe Sempere (malursem@gap.upv.es)

## Original documentation from the authors

[- Increasing Multicore System Efficiency through Intelligent Bandwidth Shifting.](https://ieeexplore.ieee.org/document/7056020)

## Compile and run

- Tested on IBM POWER 8 with Ubuntu 18.04.

- The code is ready to run SPEC CPU Benchmarks, install them and change the path to the benchmarks in the scheduler code before compile.

- Use [libperf](https://github.com/mlurbe97/Intelligent-Bandwidth-Shifting/blob/master/doc/lib) library for C to compile the scheduler [IBS.c](https://github.com/mlurbe97/Intelligent-Bandwidth-Shifting/blob/master/src/IBS.c).

- To run the scheduler just launch this script as super user -> [launch_ibs](https://github.com/mlurbe97/Intelligent-Bandwidth-Shifting/blob/master/doc/launch_scripts/launch_ibs).