# PARALLELLIZATION AND DISTRIBUTED EXECUTION SOFTWARE FOR ANTZ

This C program uses PVM libraries in order to create a parallellization interface for
 - Maple scripts
 - C programs
 - Python scripts
 - (Pari support coming soon)

This interface lets the user execute a same script/program over multiple input data in several CPUs located at the antz computing server. It sports memory management so nodes do not run out of RAM due to too many processes being started in the same node. It also reports resource usage data after execution (work in progres...).

Compilation: Makefile takes care.

Documentation: run $doxigen and take a look at html/index.html for documentation

Usage: ./pvm_test flag program datafile nodefile outputdir [memory]
 - Flag:
  - 0 = Maple
  - 1 = C
  - 2 = Python
 - program: absolute path to program file
 - datafile: absolute path to data file
  - Line format is "tasknumber,arg1,arg2,...,argN"
 - nodefile: absolute path to PVM node file
  - Line format is "nodename numerofprocesses"
 - outputdir: absolute path to output directory
  - Remark: do not end the path with "/" (eg /home/user/data/out/ should be /home/user/data/out instead).
 - memory: (optional) max amount of RAM (in KB) that a single execution can require
  - Remark: if this argument is not used, the program picks a 25% of max RAM threshold for assigning jobs (this is not optimal for nodes with much RAM)
