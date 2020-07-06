# Intelligent Bandwidth Shifting

## Implementation author

* Manel Lurbe Sempere (malursem@inf.upv.es)

## Original documentation from the authors

[- Increasing Multicore System Efficiency through Intelligent Bandwidth Shifting.](https://ieeexplore.ieee.org/document/7056020)

## Compile and run

- The code is ready to run SPEC CPU Benchmarks, install them and change the path to the benchmarks in the scheduler code before compile.

- Use [libperf](https://github.com/mlurbe97/Intelligent-Bandwidth-Shifting/blob/master/doc/lib) library for C to compile the scheduler [IBS.c](https://github.com/mlurbe97/Intelligent-Bandwidth-Shifting/blob/master/IBS/IBS.c).

- To run the scheduler just launch this script as super user -> [launch_ibs](https://github.com/mlurbe97/Intelligent-Bandwidth-Shifting/blob/master/doc/launch_scripts/launch_ibs).

## License

IBS implementation
Copyright (C) 2020  Manel Lurbe Sempere <malursem@inf.upv.es>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.