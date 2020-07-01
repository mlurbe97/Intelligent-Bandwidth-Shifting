#!/usr/bin/python
# -*- coding: utf-8 -*-

benchmarks = [1,2,3,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,32,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51]##23
benchmarksNames = ["bzip2","gcc","mcf","hmmer","sjeng","libquantum","h264ref","omnetpp","astar","xalancbmk","bwaves","gamess",
"milc","zeusmp","gromacs","cactusADM","leslie3d","namd","microbenchArray160MBinf","soplex","povray","GemsFDTD","lbm",
"tonto","calculix","null","null","null","perlbench_s","gcc_r","mcf_r","omnetpp_s","xalancbmk_s","x264_s","deepsjeng_r",
"leela_s","exchange2_s","xz_r","bwaves_r","cactuBSSN_r","lbm_r","wrf_s","pop2_s","imagick_r","nab_s","fotonik3d_r",
"roms_r","namd_r","parest_r","povray_r"]

file = open("benchmark_names.txt","w")

for h in range(50):
    file.write("Benchmark: "+benchmarksNames[h]+"\tNumber: "+str(benchmarks[h])+"\n")