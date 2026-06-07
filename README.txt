*************************************************************
		     CACHE SIMULATOR
*************************************************************
Here is a basic description of the project files:

Libraries:
uthash.h:
cache.h:

>Caching Algorithm Functions<
LRU.c
LFU-k.c
LFU.c
FF.c
LRFU.c
LIRS.c
_2Q.c
ARC.c
 






*************************************************************
		      	EXECUTION
*************************************************************
Here are some instructions for running the cache simulator 
project.

>Makefile<
-Use the make command for building all executables
-Use the make clean to clear all .o files

>run_experiments.sh<
*if all necessary files in the directory=
Executates the get_wss program to get distinct object count.
Runs main parallel execution for ALL 3 INPUT STREAMS and 
ALL 8 ALGORITHMS. Prints results.txt report during execution.
(Because execution takes around 10 minutes and is quite taxing 
the result.txt report is included).