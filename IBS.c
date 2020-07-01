/*
//
// 
*/
#include <sys/types.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <err.h>
#include <sys/poll.h>
#include <sched.h>
#include <time.h>


#include "perf_util.h"

#define N_MAX 10
#define PTRACE_DSCR 44

	typedef struct 
	{
	  char *events;
	  int delay;
	  int pinned;
	  int group;
	  int verbose;
	} options_t;

	typedef struct 
	{
		pid_t pid;
		int benchmark;
		int n_fins;
		int status;
		int core;
		int dscr;
		int current_counter;
		uint64_t counters [45];

		perf_event_desc_t *fds;
		int num_fds;
		cpu_set_t mask;

		//Contadores del ciclo 
		uint64_t a_ciclos;
		uint64_t a_instrucciones;
		uint64_t a_LLC_LOAD_MISSES;
		uint64_t a_PM_MEM_PREF;

		uint64_t totes_ciclos;
		uint64_t tot_instrucciones;
		uint64_t totes_instruccions;
		uint64_t totes_LLC_LOAD_MISSES;
		uint64_t totes_PM_MEM_PREF;
		//Métricas apagado 
		double bw_off;
		double ipc_off;
		//Métricas u7p2
		double bw_u7p7;
		double ipc_u7p7;
		double PU;


		int off_done;	//Intervalo OFF acabado
		int u7p7_done;	//Intervalo U7P2 acabado

		int q_actual;	//0->off; 1->def; 2->u1p2; 3->u7p2

		int q_sel; //Configuración seleccionada::0->off; 1->def; 2->u1p2; 3->u7p2
		double cSel;
		int quantums_sel; //Quantums con la configuración seleccionada
		int finalizado;	//Ha finalizado las instrucciones
		int muestreo;	//Esta en proceso de muestreo
		int selected;
		int mid;	//ID local dentro de la cola de nodos

	} node;

	typedef struct 
	{

		double current_bw;	//Consumo bw del sistema
		int quantums_sel; //Quantums con la configuración seleccionada
		double max_bw;	//Maximo bw consumido admitido (Antes de tomar acciones)
		double max_c;		//Umbral del compromiso
		double p2bThreshold;
        int phase;
		int app_sel;

		double last_bw;
		double last_ipc;
		double current_ipc;

		int last_affected;


	} global_stats;
	
	typedef struct 
	{

		int dscr_s; //Identificador de la seleccion
		double ipc_s;	//IPC config
		double c_s;		//C config


	} config_list;
	
	
node queue [N_MAX];

config_list listConfig [4];
int listConfigN = 0;

static options_t options;
global_stats g_stats;

int completed_proc;
int fin_ejecucion = 0;
int quantums = 0;
int N;
int selec;
int carga = -1;
int print_res_final;
int print_quantums;
int vervose;

int delayMuestreo;
int delaySeleccion;

double bw_system;

uint64_t pmu_counters[7];
char *events[7];

int boost = 0; // Activo en V4

//**********************//
//	BENCHMARKS	//
//**********************//
char *benchmarks[][200] = {
	//* SPEC CPU 2006 *//
	
	// 0 -> perlbench
	{NULL, NULL, NULL},
	// 1 -> bzip2
	{"/home/malursem/working_dir/spec_bin/bzip2.ppc64", "/home/malursem/working_dir/CPU2006/401.bzip2/data/all/input/input.combined", "200", NULL},
	// 2 -> gcc
	{"/home/malursem/working_dir/spec_bin/gcc.ppc64", "/home/malursem/working_dir/CPU2006/403.gcc/data/ref/input/scilab.i", "-o", "scilab.s", NULL},
	// 3 -> mcf
	{"/home/malursem/working_dir/spec_bin/mcf.ppc64", "/home/malursem/working_dir/CPU2006/429.mcf/data/ref/input/inp.in", NULL},
	// 4 -> gobmk
	{"/home/malursem/working_dir/spec_bin/gobmk.ppc64", "--quiet", "--mode", "gtp", NULL},
	// 5 -> hmmer
	{"/home/malursem/working_dir/spec_bin/hmmer.ppc64", "--fixed", "0", "--mean", "500", "--num", "500000", "--sd", "350", "--seed", "0", "/home/malursem/working_dir/CPU2006/456.hmmer/data/ref/input/retro.hmm", NULL},
	// 6 -> sjeng
	{"/home/malursem/working_dir/spec_bin/sjeng.ppc64", "/home/malursem/working_dir/CPU2006/458.sjeng/data/ref/input/ref.txt", NULL},
	// 7 -> libquantum
	{"/home/malursem/working_dir/spec_bin/libquantum.ppc64", "1397", "8", NULL},
	// 8 -> h264ref
	{"/home/malursem/working_dir/spec_bin/h264ref.ppc64", "-d", "/home/malursem/working_dir/CPU2006/464.h264ref/data/ref/input/foreman_ref_encoder_baseline.cfg", NULL},
	// 9 -> omnetpp
	{"/home/malursem/working_dir/spec_bin/omnetpp.ppc64", "/home/malursem/working_dir/CPU2006/471.omnetpp/data/ref/input/omnetpp.ini", NULL},
	// 10 -> astar
	{"/home/malursem/working_dir/spec_bin/astar.ppc64", "/home/malursem/working_dir/CPU2006/473.astar/data/ref/input/BigLakes2048.cfg", NULL},
	// 11 -> xalancbmk
	{"/home/malursem/working_dir/spec_bin/Xalan.ppc64", "-v", "/home/malursem/working_dir/CPU2006/483.xalancbmk/data/ref/input/t5.xml", "/home/malursem/working_dir/CPU2006/483.xalancbmk/data/ref/input/xalanc.xsl", NULL},
	// 12 -> bwaves
	{"/home/malursem/working_dir/spec_bin/bwaves.ppc64", NULL},
	// 13 -> gamess
	{"/home/malursem/working_dir/spec_bin/gamess.ppc64", NULL},
	// 14 -> milc
	{"/home/malursem/working_dir/spec_bin/milc.ppc64", NULL},
	// 15 -> zeusmp
	{"/home/malursem/working_dir/spec_bin/zeusmp.ppc64", NULL},
	// 16 -> gromacs
	{"/home/malursem/working_dir/spec_bin/gromacs.ppc64", "-silent", "-deffnm", "/home/malursem/working_dir/CPU2006/435.gromacs/data/ref/input/gromacs", "-nice", "0", NULL},
	// 17 -> cactusADM
	{"/home/malursem/working_dir/spec_bin/cactusADM.ppc64", "/home/malursem/working_dir/CPU2006/436.cactusADM/data/ref/input/benchADM.par", NULL},
	// 18 -> leslie3d
	{"/home/malursem/working_dir/spec_bin/leslie3d.ppc64", NULL},
	// 19 -> namd
	{"/home/malursem/working_dir/spec_bin/namd.ppc64", "--input", "/home/malursem/working_dir/CPU2006/444.namd/data/all/input/namd.input", "--iterations", "38", "--output", "namd.out", NULL},
	// 20 -> /*/*/*/*/*/*/ microbench /*/*/*/*/*/*/ 
	{"/home/malursem/working_dir/microbenchArray160MBinf", "100", "0", "1024", "0"},
	// 21 -> soplex
	{"/home/malursem/working_dir/spec_bin/soplex.ppc64", "-s1", "-e","-m45000", "/home/malursem/working_dir/CPU2006/450.soplex/data/ref/input/pds-50.mps", NULL},
	//22 -> povray
	{"/home/malursem/working_dir/spec_bin/povray.ppc64", "/home/malursem/working_dir/CPU2006/453.povray/data/ref/input/SPEC-benchmark-ref.ini", NULL},
	// 23 -> GemsFDTD
	{"/home/malursem/working_dir/spec_bin/GemsFDTD.ppc64", NULL},
	// 24 -> lbm
	{"/home/malursem/working_dir/spec_bin/lbm.ppc64", "300", "reference.dat", "0", "1", "/home/malursem/working_dir/CPU2006/470.lbm/data/ref/input/100_100_130_ldc.of", NULL},
	// 25 -> tonto
	{"/home/malursem/working_dir/spec_bin/tonto.ppc64", NULL},
	// 26 -> calculix
	{"/home/malursem/working_dir/spec_bin/calculix.ppc64", "-i", "/home/malursem/working_dir/CPU2006/454.calculix/data/ref/input/hyperviscoplastic", NULL},
	
	// 27
	{NULL, NULL, NULL},
	// 28
	{NULL, NULL, NULL},
	// 29
	{NULL, NULL, NULL},

    // SPEC CPU2017
    
    // 30 -> perlbench_r checkspam
    {"/home/malursem/working_dir_test/spec_bin2017/perlbench_r.ppc64", "-I/home/malursem/working_dir_test/CPU2017/500.perlbench_r/lib", "/home/malursem/working_dir_test/CPU2017/500.perlbench_r/checkspam.pl", "2500", "5", "25", "11", "150", "1", "1", "1", "1", NULL},
    // 31 -> perlbench_r diffmail
    {"/home/malursem/working_dir_test/spec_bin2017/perlbench_r.ppc64", "-I/home/malursem/working_dir_test/CPU2017/500.perlbench_r/lib", "/home/malursem/working_dir_test/CPU2017/500.perlbench_r/diffmail.pl", "4", "800", "10", "17", "19", "300", NULL},
    // 32 -> perlbench_r splitmail
    {"/home/malursem/working_dir_test/spec_bin2017/perlbench_r.ppc64", "-I/home/malursem/working_dir_test/CPU2017/500.perlbench_r/lib", "/home/malursem/working_dir_test/CPU2017/500.perlbench_r/splitmail.pl", "6400", "12", "26", "16", "100", "0", NULL},
    // 33 -> gcc_r
    {"/home/malursem/working_dir_test/spec_bin2017/gcc_r.ppc64", "/home/malursem/working_dir_test/CPU2017/502.gcc_r/gcc-smaller.c", "-O3", "-fipa-pta", "-o", "gcc-smaller.opts-O3_-fipa-pta.s", NULL},
    // 34 -> bwaves_r   
    {"/home/malursem/working_dir_test/spec_bin2017/bwaves_r.ppc64","bwaves1", NULL},
    // 35 -> mcf_r
    {"/home/malursem/working_dir_test/spec_bin2017/mcf_r.ppc64","/home/malursem/working_dir_test/CPU2017/505.mcf_r/inp.in",NULL},
    // 36 -> cactuBSSN_r
    {"/home/malursem/working_dir_test/spec_bin2017/cactuBSSN_r.ppc64","/home/malursem/working_dir_test/CPU2017/507.cactuBSSN_r/spec_ref.par",NULL} ,
    // 37 -> namd_r
    {"/home/malursem/working_dir_test/spec_bin2017/namd_r.ppc64","--input","/home/malursem/working_dir_test/CPU2017/508.namd_r/apoa1.input","--output","/home/malursem/working_dir_test/CPU2017/508.namd_r/apoa1.ref.output","--iterations","65",NULL} , 
    // 38 -> parest_r
    {"/home/malursem/working_dir_test/spec_bin2017/parest_r.ppc64","/home/malursem/working_dir_test/CPU2017/510.parest_r/ref.prm",NULL},
    // 39 -> povray_r
    {"/home/malursem/working_dir_test/spec_bin2017/povray_r.ppc64","/home/malursem/working_dir_test/CPU2017/511.povray_r/SPEC-benchmark-ref.ini",NULL},
    // 40 -> lbm_r
    {"/home/malursem/working_dir_test/spec_bin2017/lbm_r.ppc64","300","/home/malursem/working_dir_test/CPU2017/519.lbm_r/reference.dat","0","0","/home/malursem/working_dir_test/CPU2017/519.lbm_r/100_100_130_ldc.of",NULL}  ,  
    // 41 -> omnetpp_r
    {"/home/malursem/working_dir_test/spec_bin2017/omnetpp_r.ppc64", "-c", "General", "-r", "0" ,NULL},
    // 42 -> wrf_r
    {"wrf_r.ppc64", NULL},
    // 43 -> xalancbmk_r
    {"/home/malursem/working_dir_test/spec_bin2017/xalancbmk_r.ppc64","-v","/home/malursem/working_dir_test/CPU2017/523.xalancbmk_r/t5.xml","/home/malursem/working_dir_test/CPU2017/523.xalancbmk_r/xalanc.xsl",NULL},
    // 44 -> x264_r a
    {"/home/malursem/working_dir_test/spec_bin2017/x264_r.ppc64","--pass","1","--stats","x264_stats.log","--bitrate","1000","-o", "BuckBunny_New.264","/home/malursem/working_dir_test/CPU2017/525.x264_r/BuckBunny.yuv","1280x720",NULL},
    // 45 -> x264_r b
    {"/home/malursem/working_dir_test/spec_bin2017/x264_r.ppc64","--seek","500","--dumpyuv","200","--frames","1250","-o", "BuckBunny_New.264","/home/malursem/working_dir_test/CPU2017/525.x264_r/BuckBunny.yuv","1280x720",NULL},
    // 46 -> blender_r //NO COMPILA 
    {"","",NULL},
    // 47 -> cam4_r
    {"/home/malursem/working_dir_test/spec_bin2017/cam4_r.ppc64", NULL},
    // 48 -> deepsjeng_r
    {"/home/malursem/working_dir_test/spec_bin2017/deepsjeng_r.ppc64","/home/malursem/working_dir_test/CPU2017/531.deepsjeng_r/ref.txt",NULL},
    // 49 -> imagick_r
    {"/home/malursem/working_dir_test/spec_bin2017/imagick_r.ppc64","-limit", "disk", "0", "/home/malursem/working_dir_test/CPU2017/538.imagick_r/refrate_input.tga", "-edge", "41" ,"-resample", "181%", "-emboss", "31", "-colorspace", "YUV", "-mean-shift", "19x19+15%", "-resize", "30%", "/home/malursem/working_dir_test/CPU2017/538.imagick_r/refrate_output.tga" , NULL},
    // 50 -> leela_r
    {"/home/malursem/working_dir_test/spec_bin2017/leela_r.ppc64","/home/malursem/working_dir_test/CPU2017/541.leela_r/ref.sgf",NULL},
    // 51 -> nab_r
    {"/home/malursem/working_dir_test/spec_bin2017/nab_r.ppc64","1am0","1122214447","122",NULL},
    // 52 -> exchange2_r
    {"/home/malursem/working_dir_test/spec_bin2017/exchange2_r.ppc64","6",NULL},
    // 53 -> fotonik3d_r
    {"/home/malursem/working_dir_test/spec_bin2017/fotonik3d_r.ppc64", NULL},
    // 54 -> roms_r 
    {"/home/malursem/working_dir_test/spec_bin2017/roms_r.ppc64", NULL},
    // 55 -> xz_r 1
    {"/home/malursem/working_dir_test/spec_bin2017/xz_r.ppc64", "/home/malursem/working_dir_test/CPU2017/557.xz_r/cld.tar.xz", "160", "19cf30ae51eddcbefda78dd06014b4b96281456e078ca7c13e1c0c9e6aaea8dff3efb4ad6b0456697718cede6bd5454852652806a657bb56e07d61128434b474", "59796407", "61004416", "6" , NULL},
    // 56 -> xz_r 2
    {"/home/malursem/working_dir_test/spec_bin2017/xz_r.ppc64", "/home/malursem/working_dir_test/CPU2017/557.xz_r/cpu2006docs.tar.xz", "250", "055ce243071129412e9dd0b3b69a21654033a9b723d874b2015c774fac1553d9713be561ca86f74e4f16f22e664fc17a79f30caa5ad2c04fbc447549c2810fae", "23047774", "23513385", "6e", NULL },
	// 57 -> xz_r 3
    {"/home/malursem/working_dir_test/spec_bin2017/xz_r.ppc64", "/home/malursem/working_dir_test/CPU2017/557.xz_r/input.combined.xz", "250", "a841f68f38572a49d86226b7ff5baeb31bd19dc637a922a972b2e6d1257a890f6a544ecab967c313e370478c74f760eb229d4eef8a8d2836d233d3e9dd1430bf", "40401484", "41217675", "7", NULL },
    
};

//**********************//
//	BENCH.NAME			//
//**********************//
char *benchNames [] = {
	"perlBench","bzip2","gcc","mcf","gobmk","hmmer","sjeng","libquantum","h264ref",			//0--8
	"omnetpp","astar","xalancbmk","bwaves","gamess","milc","zeusmp","gromacs","cactusADM",	//9--17
	"leslie3d","namd","microbench","soplex","povray","gemsFDTD","lbm","tonto","calculix",	//18--26
	"null","null","null","perlbench_s","gcc_r","mcf_s","omnetpp_s","xalancbmk_s","x264_s",	//27--35
	"deepsjeng_r","leela_s","exchange2_s","xz_r","bwaves_r","cactuBSSN_r","lbm_r","wrf_s",	//36--43
	"pop2_s","imagick_r","nab_s","fotonik3d_r","roms_r","namd_r","parest_r","povray_r"		//44--51
};


//**********************//
//	INSTR.NUM			//
//**********************//
unsigned long int instruccions_totals [] = {
	0,840131507263,822621600783,292265817828,0,//5
	1164738680687,919433791907,721005141365,1203951466751,//4
	302819732424,552098634277,660603927234,618382088655,1336910527525,//5
	303687578711,754432510857,696299380258,1268432131579,740595255778,//5
	747512744101,76892779107,557115387111,937452576720,567031395377,//5
	667572934143,0,0,0,0,0,0,//7
	828238392009,451861723968,0,590452090165,1271012525589,913683744736,//6
	795774154617,0,533921317594,0,266179365272,631298979050,0,//7
	0,1680277884536,0,0,0,1050168030954,863114955579,946239859963//8
};
//**********************//
//	APPxWORKLOAD	//
//**********************//
//Numero de aplicaciones por mezcla
int nmezclas [] = { // Numero de cargas que contiene la mezcla
	1,	// 0	NO MODIFICAR aplicacion seleccionada en solitario.
	4,	// 1	NO MODIFICAR Benchmark y aplicacion seleccionada.

	// Cargas de 6 aplicaciones
	6,	// 2
	6,	// 3
	6,	// 4 --
	6,	// 5
	6,	// 6
	6,	// 7
	6,	// 8
	6,	// 9
	6,	// 10
	6,	// 11
	6,	// 12
	6,	// 13
	6,	// 14 --
	6,	// 15 --
	6,	// 16
	6,	// 17
	6,	// 18
	6,	// 19 --
	6,	// 20
	6,	// 21 --
	6,	// 22
	6,	// 23

	// Cargas de 8 aplicaciones
	8,	// 24 --
	8,	// 25 --
	8,	// 26
	8,	// 27
	8,	// 28
	8,	// 29
	8,	// 30
	8,	// 31
	8,	// 32
	8,	// 33
	8,	// 34 --
	8,	// 35 --
	8,	// 36 --
	8,	// 37
	8,	// 36
	8,	// 39

	// Cargas de 10 aplicaciones
	10,	// 40 carga estudio1 (26)
	10,	// 41 carga estudio2 (27)
	10,	// 42
	10,	// 43
	10,	// 44
	10,	// 45
	10,	// 46
	10,	// 47
	10,	// 48 --
	10,	// 49
	10,	// 50 --
	10,	// 51
	10,	// 52
	10,	// 53
	10,	// 54
	10	// 55
};


//**********************//
//	WORKLOAD	//
//**********************//
//Aplicaciones dentro de cada mezcla
int mezclas [][12] = { // Cargas a ejecutar
	{-1},// 0	NO MODIFICAR aplicacion seleccionada en solitario.
	{-1,20,20,20},// 1	NO MODIFICAR Benchmark y aplicacion seleccionada.
	
	// Cargas de 6 aplicaciones
	{8, 7, 6, 8, 11, 14},// 2
	{6, 23, 7, 1, 8, 23},// 3
	{2, 31, 3, 32, 7, 34},// 4
	{19, 10, 1, 23, 10, 22},// 5 
	{10, 9, 5, 18, 22, 2},// 6
	{22, 5, 7, 18, 11, 8},// 7
	{12, 2, 10, 15, 2, 2},// 8
	{21, 19, 11, 7, 16, 9},// 9
	{21, 2, 2, 21, 6, 9},// 10
	{19, 6, 8, 10, 21, 7},// 11
	{8, 11, 15, 18, 14, 14},// 12
	{35, 9, 36, 11, 41, 12},// 13
	{14, 37, 15, 39, 17, 41},// 14
	{2, 9, 24, 21, 18, 49},// 15
	{3, 14, 23, 16, 1, 15},// 16
	{35, 23, 35, 39, 12, 21},// 17
	{19, 8, 15, 1, 9, 22},// 18
	{17, 11, 9, 14, 18, 15},// 19
	{15, 11, 19, 12, 18, 14},// 20
	{21, 39, 11, 7, 37, 9},// 21
	{36, 7, 6, 42, 11, 14},// 22
	{3, 11, 15, 18, 14, 14},// 23

	// Cargas de 8 aplicaciones
    {11, 34, 21, 18, 9, 12, 51, 45},// 24
	{3, 14, 23, 50, 36, 15, 50, 49},// 25
	{45, 32, 50, 50, 23, 23, 15, 9},// 26
	{42, 51, 51, 50, 15, 21, 7, 7},// 27
	{51, 51, 32, 50, 24, 14, 12, 9},// 28
	{51, 51, 51, 32, 11, 3, 2, 14},// 29
	{50, 50, 50, 36, 3, 12, 12, 11},// 30
	{50, 32, 32, 51, 21, 23, 9, 3},// 31
	{9, 15, 36, 11, 3, 10, 18, 11},// 32
	{7, 14, 9, 51, 7, 6, 7, 15},// 33
	{21, 18, 18, 18, 18, 18, 18, 18},// 34
	{37, 3, 11, 23, 51, 7, 14, 11},// 35
	{15, 7, 15, 36, 42, 7, 15, 3},// 36
	{23, 11, 2, 16, 21, 10, 14, 7},// 37
	{2, 14, 21, 21, 19, 2, 23, 18},// 38
	{6, 18, 3, 19, 15, 18, 13, 11},// 39

	// Cargas de 10 aplicaciones
	{14, 12, 23, 2, 15, 3, 15, 10, 11, 10},// 40 carga estudio1 (26)
	{11, 3, 19, 7, 21, 6, 23, 15, 10, 15},// 41 carga estudio2 (27)
	{50, 51, 50, 36, 36, 6, 11, 23, 7, 24},// 42
	{51, 32, 32, 50, 50, 6, 15, 9, 3, 17},// 43
	{32, 51, 36, 36, 36, 18, 2, 2, 6, 23},// 44
	{32, 51, 32, 50, 51, 9, 12, 9, 23, 9},// 45
	{36, 36, 51, 32, 51, 12, 17, 12, 11, 23},// 46
	{45, 36, 45, 32, 36, 9, 9, 6, 21, 2},// 47
	{11, 3, 37, 7, 21, 6, 23, 15, 11, 15 },// 48
	{15, 2, 9, 9, 9, 7, 51, 18, 2, 36 },// 49
	{18, 7, 35, 9, 7, 11, 14, 36, 11, 42},// 50
	{9, 31, 36, 7, 32, 21, 42, 35, 34, 35},// 51
	{36, 11, 34, 35, 42, 11, 7, 9, 6, 37},// 52
	{7, 8, 7, 3, 9, 19, 10, 13, 3, 23},// 53
	{22, 23, 9, 19, 5, 3, 19, 13, 9, 5},// 54
	{8, 8, 18, 3, 13, 3, 21, 1, 19, 15} // 55
};



int def = 0; // 0-> DO NOTHING; 1->ALL PREFETCH DEF.
int off = 0; // 0-> DO NOTHING; 1->ALL PREFETCH OFF


//**************************************//
//	PREFETCH CONFIGURATION CHANGER	//
//**************************************//
static int do_dscr_pid(int dscr_state, pid_t pid)
{
	int rc;

	rc = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
	if (rc) {
		//fprintf(stdout, "Could not attach to process %d to %s the DSCR value\n%s\n", pid, (dscr_state ? "set" : "get"),strerror(errno));
		return rc;
	}

	wait(NULL);

	//if (dscr_state) {
		rc = ptrace(PTRACE_POKEUSER, pid, PTRACE_DSCR << 3, dscr_state);
		if (rc) {
			//fprintf(stdout, "Could not set the DSCR value for pid %d\n%s\n", pid, strerror(errno));
			ptrace(PTRACE_DETACH, pid, NULL, NULL);
			return rc;
		}
	//}

	rc = ptrace(PTRACE_PEEKUSER, pid, PTRACE_DSCR << 3, NULL);
	if (errno) {
		//fprintf(stderr, "Could not get the DSCR value for pid "
			//"%d\n%s\n", pid, strerror(errno));
		rc = -1;
	} else {
		printf("DSCR for pid %d is %d\n", pid, rc);
	}

	ptrace(PTRACE_DETACH, pid, NULL, NULL);
	return rc;
}




static void desactivateWorstPU()
{
	double worstPU = 10; // Theorical max is 1
	int worstID = -1;
	int i;

	for (i=0; i<N; i++) 
	{
		if (queue[i].selected == 0)
		{
			if (queue[i].PU < worstPU)
			{
				worstPU = queue[i].PU;
				worstID = i;
			}

		}
	}

	queue[worstID].selected = 1;
	queue[worstID].dscr = 1;
	g_stats.app_sel = worstID;
	printf("%d Tiene el peor PU.\n", worstID);
	do_dscr_pid(queue[worstID].dscr, queue[worstID].pid);

}


//**************************//
//	PRINT QUANTUM VALUES	//
//**************************//
static void print_quantum_values(){
	
	int i;
	double ipcMeanAux = 0.0;
	
	
	for (i=0; i<N; i++) 
	{
		fprintf(stdout, "QuantumCounters:\t");

		fprintf(stdout, "%s\t", benchNames[queue[i].benchmark] );
		fprintf(stdout, "%d\t" , queue[i].mid);
		
		fprintf(stdout, "%"PRIu64"\t", queue[i].a_ciclos );
		fprintf(stdout, "%"PRIu64"\t", queue[i].a_instrucciones);
		fprintf(stdout, "%"PRIu64"\t", queue[i].a_LLC_LOAD_MISSES );
		fprintf(stdout, "%"PRIu64"\t", queue[i].a_PM_MEM_PREF );
		
		fprintf(stdout, "%lf\t" , queue[i].ipc_off);
		fprintf(stdout, "%lf\t" , 0.0);
		fprintf(stdout, "%lf\t" , 0.0);
		fprintf(stdout, "%lf\t" , queue[i].ipc_u7p7);
		
		fprintf(stdout, "%lf\t" , queue[i].bw_off);
		fprintf(stdout, "%lf\t" , 0.0);
		fprintf(stdout, "%lf\t" , 0.0);
		fprintf(stdout, "%lf\t" , queue[i].bw_u7p7);
		
		fprintf(stdout, "%lf\t" , 0.0);
		fprintf(stdout, "%lf\t" , 0.0);
		fprintf(stdout, "%lf\t" , queue[i].PU);
		
		fprintf(stdout, "%d\t" , queue[i].dscr);
		fprintf(stdout, "%d-%d-%d-%d\t" , queue[i].off_done,2 ,2 ,queue[i].u7p7_done);
		
		fprintf(stdout, "%d\t" , queue[i].finalizado);
		fprintf(stdout, "%d\t" , quantums);
		fprintf(stdout, "%d" , options.delay);
		fprintf(stdout, "\n");
		
		ipcMeanAux += ((double)queue[i].a_instrucciones/(double)queue[i].a_ciclos);
	}
	
	ipcMeanAux = ipcMeanAux/N;
	
	//fprintf(stdout, "SystemBW=%lf\tSystemIPC=%lf\tSystemC=%lf\n",g_stats.current_bw, ipcMeanAux, g_stats.max_c);
	fprintf(stdout, "QuantumSystemInfo:\t%lf\t%lf\n",g_stats.current_bw, ipcMeanAux); // , g_stats.max_c);

}





//**********************//
//	GET COUNTS	//
//**********************//
static void get_countsv2(node *aux){

	ssize_t ret;
	int i;

	for(i=0; i < aux->num_fds; i++) {
		uint64_t val;

		ret = read(aux->fds[i].fd, aux->fds[i].values, sizeof(aux->fds[i].values));
		if (ret < (ssize_t)sizeof(aux->fds[i].values)) {
			fprintf(stderr,"FALLA ALGO.\n");
			if (ret == -1)
				fprintf(stderr, "cannot read values event %s", aux->fds[i].name);
			else
				fprintf(stderr,"could not read event%d", i);
		}
    

		val = aux->fds[i].values[0] - aux->fds[i].prev_values[0];
		aux->fds[i].prev_values[0] = aux->fds[i].values[0];
		aux->counters[i] += val;
		if(i==0){
			//Ciclos
			aux->a_ciclos = val;
			if(!(node->finished)){//como se quiere almacenar solo el target instructions hay que poner este if para que deje de guardar los resultados del core que termina.
				aux->totes_ciclos += val;
			}
		}else if(i==1){
			//Instrucciones
			aux->a_instrucciones = val;
			aux->tot_instrucciones += val;
			if(!(node->finished)){//como se quiere almacenar solo el target instructions hay que poner este if para que deje de guardar los resultados del core que termina.
				aux->totes_instruccions += val;
			}
		}else if(i==3){
			//LLC-Load-Misses
			aux->a_LLC_LOAD_MISSES = val;
			if(!(node->finished)){//como se quiere almacenar solo el target instructions hay que poner este if para que deje de guardar los resultados del core que termina.
				aux->totes_LLC_LOAD_MISSES += val;
			}
		}else if(i==4){
			//PM_MEM_PREF
			aux->a_PM_MEM_PREF = val;
			if(!(node->finished)){//como se quiere almacenar solo el target instructions hay que poner este if para que deje de guardar los resultados del core que termina.
				aux->totes_PM_MEM_PREF += val;
			}
		}
	}
	
}




//**********************//
//	MEASURE				//
//**********************//
int measure() {
	int i, ret;

	// Libera los procesos (SIGCONT-Contiua un proceso pausado)
	for (i=0; i<N; i++) 
	{
		if (queue[i].pid > 0) 
		{
			kill(queue[i].pid, 18);
		}
	}
	// Revisa que todos hayan realizado la operacion
	for (i=0; i<N; i++) 
	{
		waitpid(queue[i].pid, &(queue[i].status), WCONTINUED);
		if (WIFEXITED(queue[i].status)) 
		{
			//fprintf(stderr, "ERROR: command process %d_%d exited too early with status %d\n", queue[i].benchmark, queue[i].pid, WEXITSTATUS(queue[i].status));
		}
	}
	// Ejecutamos durante un quantum
	usleep(options.delay*1000);
	
	
	// Pausamos los procesos (SIGSTOP-Pausamos un proceso)
	ret = 0;
	for (i=0; i<N; i++) 
	{
		if (queue[i].pid > 0) 
		{
			kill(queue[i].pid, 19); 
			//waitpid(aux->pid, &(aux->status), WUNTRACED);
		}
	}
	// Revisar que no ha muerto ningun proceso 
	for (i=0; i<N; i++) 
	{
		waitpid(queue[i].pid, &(queue[i].status), WUNTRACED);
		if (WIFEXITED(queue[i].status)) 
		{
			//fprintf(stderr, "Process %d_%d finished with status %d\n", queue[i].mid, queue[i].pid, WEXITSTATUS(queue[i].status));
			ret++;
			queue[i].pid = -1;
		}
	}
	//fprintf(stderr, "antes get counts\n");
	// Coger contadores
	for (i=0; i<N; i++) 
	{
		get_countsv2(&(queue[i]));
		//fprintf(stderr, "gc\n");
	}
	//fprintf(stderr, "despues getcounts\n");
	double bw_aux = 0.0;
	for (i=0; i<N; i++) 
	{
		bw_aux += (double) ((((double)queue[i].a_LLC_LOAD_MISSES+(double)queue[i].a_PM_MEM_PREF)*3690)/(double)queue[i].a_ciclos); //Revisar inicialización
					
	}
	//fprintf(stderr, "1\n");
	//print(current bw= g_stats.current_bw)
	g_stats.last_bw = g_stats.current_bw;
	g_stats.current_bw = bw_aux;
	
	double ipc_aux = 0.0;
	for (i=0; i<N; i++) 
	{
		ipc_aux += (double) ((double)queue[i].a_instrucciones/(double)queue[i].a_ciclos); //Revisar inicialización
					
	}
	
	g_stats.last_ipc = g_stats.current_ipc;
	g_stats.current_ipc = ipc_aux;
	
	
	// Si esta el scheduler activo
	if(!def && !off){
		
		print_quantum_values();
        switch (g_stats.phase)
        {
        case 0:
			printf("FASE 0.\n");
			// Collect data from all app
			for (i=0; i<N; i++) 
			{
				queue[i].ipc_u7p7 =  ((double)queue[i].a_instrucciones/(double)queue[i].a_ciclos);
				queue[i].bw_u7p7 = (double) ((((double)queue[i].a_LLC_LOAD_MISSES+(double)queue[i].a_PM_MEM_PREF)*3690)/(double)queue[i].a_ciclos);
			}
			

			// Set prefetch off for the first app
			queue[0].dscr = 1;
			g_stats.app_sel = 0;
			do_dscr_pid(queue[0].dscr,queue[0].pid);
			
			// Change phase
			g_stats.phase = 1;

			break;

		case 1:

			printf("FASE 1.\n");
			queue[g_stats.app_sel].ipc_off =  ((double)queue[g_stats.app_sel].a_instrucciones/(double)queue[g_stats.app_sel].a_ciclos);
			queue[g_stats.app_sel].bw_off = (double) ((((double)queue[g_stats.app_sel].a_LLC_LOAD_MISSES+(double)queue[g_stats.app_sel].a_PM_MEM_PREF)*3690)/(double)queue[g_stats.app_sel].a_ciclos);
			
			queue[g_stats.app_sel].PU = (double) ( (double)queue[g_stats.app_sel].ipc_u7p7 / (double)queue[g_stats.app_sel].bw_u7p7 )/( (double)queue[g_stats.app_sel].ipc_off / (double)queue[g_stats.app_sel].bw_off );
			
			

			queue[g_stats.app_sel].dscr = 0;
			do_dscr_pid(queue[g_stats.app_sel].dscr,queue[g_stats.app_sel].pid);

			g_stats.app_sel++;

			if (g_stats.app_sel >= N)
			{
				g_stats.app_sel = -1;
				g_stats.phase = 2;
			}
			else
			{
				queue[g_stats.app_sel].dscr = 1;
				do_dscr_pid(queue[g_stats.app_sel].dscr,queue[g_stats.app_sel].pid);
			}
			
			break;

		case 2:

			printf("FASE 2.\n");
			
			if ( g_stats.current_bw > (0.9*185.0) )
			{
				printf("SOBREPASADO EL LIMITE F2\n");
				int still = 0;
				for (i=0; i<N; i++) 
				{
					if (queue[i].selected == 0)
					{
						still = 1;
						break;
					}

				}
				if (still)
				{
					printf("AUN HAY ALGUNA CON PREF ACTIVO.\n");
					//DESACTIVATE PREFERCH FOR THE WORST PU
					desactivateWorstPU();
					g_stats.phase = 3;
				}else
				{
					printf("YA SE HAN DESACTIVADO TODOS, ESPERAR.\n");
					options.delay = delaySeleccion;
					g_stats.phase = 4;

				}

			}else
			{

				options.delay = delayMuestreo;
				g_stats.phase = 4;

			}

			break;

		case 3:
			
			printf("FASE 3.\n");
			
			if (g_stats.current_ipc < g_stats.last_ipc)
			{
				printf("NO SE HA MEJORADO EL IPC, REACTIVAMOS.\n");
				
				queue[g_stats.app_sel].dscr = 0;
				do_dscr_pid(queue[g_stats.app_sel].dscr,queue[g_stats.app_sel].pid);
				g_stats.app_sel = -1;
			}

			g_stats.phase = 2;

            break;

		case 4:

			printf("FASE 4.\n");
			
			g_stats.phase = 0;
			options.delay = delayMuestreo;

			for (i=0; i<N; i++) 
			{
				queue[i].selected = 0;
				queue[i].dscr = 0;
				do_dscr_pid(queue[i].dscr,queue[i].pid);
			}
			break;
			
        
        default:
			printf("FASE DEF.\n");
            break;
        }
        
		

		for (i=0; i<N; i++) 
		{
			
			queue[i].a_LLC_LOAD_MISSES = 0;
			queue[i].a_PM_MEM_PREF = 0;
			queue[i].a_ciclos = 0;
			queue[i].a_instrucciones = 0;
		}
		

	}
	//FIN SCHED
   
	if(off || def){
		print_quantum_values();
		for (i=0; i<N; i++) 
		{
			queue[i].a_LLC_LOAD_MISSES = 0;
			queue[i].a_PM_MEM_PREF = 0;
			queue[i].a_ciclos = 0;
			queue[i].a_instrucciones = 0;
		}
	}
  return ret;
  
}



//**********************//
//	START PROCES.	//
//**********************//
int llansar_proces (node *node) {
	FILE *fitxer;
	pid_t pid;
	
	pid = fork();
	switch (pid) {
		case -1: //Error
			fprintf(stderr, "Error al crear el hilo (hijo).\n");
			exit(-3);

		case 0: // Hijo
		//Descriptores para loss que tienen la entrada por la entrada estandar
			switch(node->benchmark) {

					case 4:
						close(0);
						fitxer = fopen("/home/malursem/working_dir/CPU2006/445.gobmk/data/ref/input/13x13.tst", "r");
						if (fitxer == NULL) {
							printf("Error. No se ha podido abrir el fichero arb.tst.\n");
							return -1;
						}
						break;

					case 13:
						close(0);
						fitxer = fopen("/home/malursem/working_dir/CPU2006/416.gamess/data/ref/input/h2ocu2+.gradient.config", "r");
						if (fitxer == NULL) {
							printf("Error. No se ha podido abrir el fichero h2ocu2+.energy.config.\n");
							return -1;
						}
						break;

					case 14:
						close(0);
						fitxer = fopen("/home/malursem/working_dir/CPU2006/433.milc/data/ref/input/su3imp.in", "r");
						if (fitxer == NULL) {
							printf("Error. No se ha podido abrir el fichero su3imp.in.\n");
							return -1;
						}
						break;

					case 18:
						close(0);
						fitxer = fopen("/home/malursem/working_dir/CPU2006/437.leslie3d/data/ref/input/leslie3d.in", "r");
						if (fitxer == NULL) {
							printf("Error. No se ha podido abrir el fichero leslie3d.in.\n");
							return -1;
						}
						break;

					case 22:
						close(2);
						fitxer = fopen("/home/malursem/working_dir/povray.sal", "w");
						if (fitxer == NULL) {
							printf("Error. No se ha podido abrir el fichero povray.sal\n");
							return -1;
						}
						break;
				}
		execv(benchmarks[node->benchmark][0], benchmarks[node->benchmark]);
		fprintf(stderr, "ERROR EN EL EXEC de %d.\n",node->benchmark);
		exit (-2);

	default:  //Padre


		usleep(100);

		//Paramos el proceso
		kill (pid, 19);
		
		// Se mira que no haya fallado
		waitpid(pid, &(node->status), WUNTRACED);
		if (WIFEXITED(node->status)) {
			fprintf(stderr, "ERROR: command process %d exited too early with status %d\n", pid, WEXITSTATUS(node->status));
			return -2;
		}

		// Se asigna el pid
		node->pid = pid;
		
		if (sched_setaffinity(node->pid, sizeof(node->mask), &node->mask) != 0) {
			fprintf(stderr, "Sched_setaffinity error: %d.\n", errno);
			exit(1);
		}
		if(def){
			do_dscr_pid(0,node->pid);
		}else if(off){
			do_dscr_pid(1,node->pid);
		}else{
			do_dscr_pid(node->dscr,node->pid);
		}
		
		return 1;
  }
}


//**********************//
//	START EVENTS	//
//**********************//
void iniciar_events(node *node) {
	int i, ret;

	// Configurem els events
	ret = perf_setup_list_events(options.events, &(node->fds), &(node->num_fds));
	if (ret || (node->num_fds == 0)) {
		exit (1);
	}

	node->fds[0].fd = -1;
  
	for (i=0; i<node->num_fds; i++) {
		node->fds[i].hw.disabled = 0;  /* start immediately */
		/* request timing information necessary for scaling counts */
		node->fds[i].hw.read_format = PERF_FORMAT_SCALE;
		node->fds[i].hw.pinned = !i && options.pinned;
		node->fds[i].fd = perf_event_open(&node->fds[i].hw, node->pid, -1, (options.group? node->fds[i].fd : -1), 0);
		if (node->fds[i].fd == -1) {
			errx(1, "cannot attach event %s", node->fds[i].name);
		}
	}
 
}


//**********************//
//	END EVENTS	//
//**********************//
void finalitzar_events (node *node) {
  int i;

  // Llibera els descriptors
  for(i=0; i < node->num_fds; i++) {
    close(node->fds[i].fd);
  }

  // Llibera els comptadors
  perf_free_fds(node->fds, node->num_fds);
  node->fds = NULL;
}


//**********************//
//	START COUNTERS	//
//**********************//
void iniciar_contadors (node *node) {
  int i;

  node->current_counter = 0;

  for (i=0; i<45; i++) {
    node->counters[i] = 0;
  }
}


//**********************//
//	USAGE		//
//**********************//
static void usage(void) {
  printf("usage: [PROGRAMA] \n\t -d duracionQuantumNormal -m duracionQuantumMuestreo \n\t -A Workload {-O (off)} {-D (def)} \n\t {-S strideMB -N nopMB} \n\t {-Q (quantum_counters)} {-F (final_counters)} \n\t {-v (Vervose bw/quantum)} \n\t {-c seleccion umbral compromiso, por def 0.25} \n\t {-b seleccion máximo bw}\n");
}



//*******************//
//*******************//
//*		MAIN		*//
//*******************//
//*******************//
int main(int argc, char **argv) {
	int c, i, ret;
	print_res_final = 0;
	print_quantums = 0;
	vervose = 0;


	options.delay = 0;
	delayMuestreo = 0;
	delaySeleccion = 0;

    g_stats.last_bw = 0;
	g_stats.current_bw = 0;
	g_stats.last_ipc = 0;
	g_stats.current_ipc = 0;

	g_stats.quantums_sel = 0;
	/*CAMBIAR PARA DIFERENTE COMPORTAMIENTO*/
	g_stats.max_bw = 140.0;	//Máximo bw soportado por el sistema antes de tomar acciones
	g_stats.max_c = 0.25;	//Máximo 'c' soportado por el sistema para seleccionar una configuración "umbral compromiso"
	g_stats.p2bThreshold = 0.3;
    g_stats.phase = 0;
	g_stats.app_sel = -1;
	g_stats.last_affected = -1;

	N = -1;
	selec = -1;

	for (i=0; i<N_MAX; i++) 
	{
		queue[i].benchmark = -1;
		queue[i].n_fins = 0;
		queue[i].pid = -1;
		queue[i].core = -1;

		queue[i].a_LLC_LOAD_MISSES = 0;
		queue[i].a_PM_MEM_PREF = 0;
		queue[i].a_ciclos = 0;
		queue[i].a_instrucciones = 0;

		queue[i].tot_instrucciones = 0;
		queue[i].totes_instruccions = 0;
		queue[i].totes_LLC_LOAD_MISSES = 0;
		queue[i].totes_PM_MEM_PREF = 0;
		queue[i].totes_ciclos = 0;

		queue[i].bw_off = 0;
		queue[i].bw_u7p7 = 0;

		queue[i].ipc_off= 0;
		queue[i].ipc_u7p7 = 0;

		queue[i].off_done = 0;
		queue[i].u7p7_done = 0;

        queue[i].selected = 0;

		queue[i].q_actual=0;
		queue[i].cSel = 0.0;
		queue[i].q_sel = -1;
		queue[i].quantums_sel = 0;
		queue[i].muestreo = 1;
		queue[i].finalizado = 0;
		queue[i].mid = i;
		queue[i].selected = 0;
	}

while ((c=getopt(argc, argv,"hgvPODQFA:d:S:N:c:b:m:B:")) != -1) {
	switch(c) {
		case 'P':
			options.pinned = 1;
			break;
		case 'B':
			g_stats.p2bThreshold = atof(optarg);
		case 'g':
			options.group = 1;
			break;
		case 'd':
			delaySeleccion = atoi(optarg);
			break;
		case 'm':
			delayMuestreo = atoi(optarg);
			break;
		case 'h':
			usage();
			exit(0);
		case 'c':
			g_stats.max_c = atof(optarg);
			break;
		case 'b':
			g_stats.max_bw = atof(optarg);
			break;
		case 'A':
			carga = atoi(optarg);
			N = nmezclas[carga]; // N -> Num. of proc. in workload
			//Seleccionar cores predefinidos*/
			queue[0].core = 0;
			queue[1].core = 8;
			queue[2].core = 16;
			queue[3].core = 24;
			queue[4].core = 32;
			queue[5].core = 40;
			queue[6].core = 48;
			queue[7].core = 56;
			queue[8].core = 64;
			queue[9].core = 72;
			break;
		case 'O':
			off = 1; 
			break;
		case 'D':
			def = 1;
			break;
		case 'S':
			//Stride
			benchmarks[20][3] = optarg;
			break;
		case 'N':
			//Nop
			benchmarks[20][2] = optarg;
			break;
		case 'Q':
			print_quantums = 1;
			break;
		case 'F':
			print_res_final = 1;
			break;
		case 'v':
			vervose = 1;
			break;
		default:
			errx(1, "unknown error");
	}
}

	options.events = strdup("cycles,instructions,LLC-LOADS,LLC-LOAD-MISSES,PM_MEM_PREF");
	

	if (delayMuestreo < 1)
	{
		delayMuestreo = 1;
	}
	
	if (delaySeleccion < 1)
	{
		delaySeleccion = 100;
	}
	
	options.delay = delayMuestreo;
	// if (options.delay < 1) 
	// {
		
		// options.delay = 200;
		
	// }

	if (carga < 0)
	{
		fprintf(stderr, "Error: Falta por seleccionar la carga.\n");
		return -1;
	}
  
	//Asignar primer valor del prefetcher
	if(def)
	{	//Por defecto siempre
		for (i=0; i<N; i++) 
		{
			queue[i].dscr = 0;
		}
	}else if(off)
	{	//Off siempre
		for (i=0; i<N; i++) 
		{
			queue[i].dscr = 1;
		}
	}else
	{	//Empezamos con U7P7
		for (i=0; i<N; i++) 
		{
			queue[i].dscr = 0;
		}
	}
	//Asignación de la carga
	for (i=0; i<N; i++) 
	{
		queue[i].benchmark = mezclas[carga][i];
	}
	//Comprobar que todos tienen el benchmark y el núcleo asigando
	for (i=0; i<N; i++) {
		if (queue[i].benchmark < 0) 
		{
			fprintf(stderr, "Error: Falta algun proceso por asignar benchmark.\n");
			return -1;
		}
		if (queue[i].core < 0) 
		{
			fprintf(stderr, "Error: Falta algun core por asignar.\n");
			return -1;
		}
	}
	//Init. Counters
	for (i=0; i<N; i++) 
	{
		iniciar_contadors (&(queue[i])); 	
	}
	//Core set
	for (i=0; i<N; i++) 
	{
		CPU_ZERO(&(queue[i].mask));
		CPU_SET(queue[i].core, &(queue[i].mask));
	}
	//Init. libpfm
	if (pfm_initialize() != PFM_SUCCESS) 
	{
		errx(1, "libpfm initialization failed\n");
	}
	//Create process.
	for(i=0; i<N; i++) 
	{
		llansar_proces(&(queue[i]));
		iniciar_events(&(queue[i]));
	}
	fprintf(stdout, "###################################################################################################");
	fprintf(stdout, "\n");
	fprintf(stdout, "QuantumCount:\tName\tID\tq_ciclos\tq_instr\tq_llc_loads\tq_pm_memPref\tipc_off\tipc_def\tipc_u1p2\tipc_u7p2\tbw_off\tbw_def\tbw_u1p2\tbw_u7p2\tc_def\tc_u1p2\tc_u7p2\tdscr\tmuestreo\tfinalizado\tquantums\tduracionQuantum");
	fprintf(stdout, "QuantumSysnfo:\tSysBW\tSysIPC\n");
	fprintf(stdout, "Workload:\t%d\nNumeroBenchs:\t%d\nBenchmarks:\t",carga,N);
	fprintf(stdout, "MaxC:\t%lf\tMaxBW:%lf\n",g_stats.max_c,g_stats.max_bw);
	for (i=0; i<N; i++)
	{ 
		fprintf(stdout, "%s(%d)\t", benchNames[queue[i].benchmark], queue[i].mid );
	}
	
	fprintf(stdout, "\n");
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	fprintf(stdout,"InicioEjecucion:\t%d-%d-%d\t%d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	fprintf(stdout, "\n");
	fprintf(stdout, "###################################################################################################\n");
	

	do 
	{
		
		//fprintf(stderr, "antes measure\n");
		ret = measure();
		//fprintf(stderr, "despues measure\n");
		if (ret) 
		{
			//fprintf(stderr, "Dentro RET\n");
			for (i=0; i<N; i++) 
			{
				if (queue[i].pid == -1) 
				{
					//fprintf(stderr, "Dentro RET2\n");
					get_countsv2(&(queue[i]));
					//fprintf(stderr, "Dentro RET3\n");
					finalitzar_events(&(queue[i]));
					//fprintf(stderr, "Dentro RET4\n");
					//fprintf(stderr, "TotAPP=%20"PRIu64"TotObjetivo=%20"PRIu64"\n",queue[i].tot_instrucciones,instruccions_totals[queue[i].benchmark]);
					if(queue[i].tot_instrucciones < instruccions_totals[queue[i].benchmark])
					{
						//fprintf(stderr, "ALGUN PROCESO HA ACABADO ANTES DE FINALIZAR LAS INSTRUCCIONES %d.\n",queue[i].mid);
						llansar_proces (&(queue[i]));
						iniciar_events (&(queue[i]));
					}
						
				}
			}
		}
		//fprintf(stderr, "RET.\n");
		for (i=0; i<N; i++) 
		{
			if(queue[i].tot_instrucciones >= instruccions_totals[queue[i].benchmark])
			{	
				//fprintf(stderr, "ALGUN PROCESO HA FINALIZADO LAS INSTRUCCIONES %d.\n",queue[i].mid);
				if(!queue[i].finalizado)
				{
					//fprintf(stderr, "FINALIZADO POR PRIMERA VEZ %d.\n",queue[i].mid);
					fin_ejecucion++;
					queue[i].finalizado = 1;
				}
				
				if (queue[i].pid > 0) 
				{
					kill(queue[i].pid, 9);
					queue[i].pid = -1;
					finalitzar_events(&(queue[i]));
					
				}
				
				queue[i].tot_instrucciones = 0;
				
				llansar_proces (&(queue[i]));
				iniciar_events (&(queue[i]));
				//fprintf(stderr, "ALGUN PROCESO SE HA REINICIADO %d.\n",queue[i].mid);
			}
		}
		
		
		
		quantums++;
	} while(fin_ejecucion < N);


	// Matar cualquier proceso que quede pendiente
	for (i=0; i<N; i++) 
	{
		if (queue[i].pid > 0) 
		{
			kill(queue[i].pid, 9);
			finalitzar_events(&(queue[i]));
		}
	}

	/* free libpfm resources cleanly */
	pfm_terminate();


  	// Imprimim els resultats
  	fprintf(stderr, "-\tBenchmark\tinstrucciones\tciclos\tLLC_LOAD_MISSES\tPM_MEM_PREF\n");
	for (c=0; c<N; c++) 
	{
		fprintf(stderr, "Final Counter:\t%s\t%ld\t%ld\t%ld\t%ld\n", benchNames[queue[c].benchmark], queue[c].totes_instruccions, queue[c].totes_ciclos, queue[c].totes_LLC_LOAD_MISSES, queue[c].totes_PM_MEM_PREF);
	}
	
	t = time(NULL);
	tm = *localtime(&t);
	fprintf(stdout,"FinEjecucion:\t%d-%d-%d\t%d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	
  return 0;
}
