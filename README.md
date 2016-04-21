# PARALLELLIZATION AND DISTRIBUTED EXECUTION SOFTWARE FOR ANTZ

This C program uses PVM libraries in order to create a parallellization interface for
 - **Maple** scripts
 - **C** programs
 - **Python** scripts
 - **Pari**: Pari is supported through gp2c by manually editing the output C code and compiling as a C program using the command provided by gp2c. The executable can then be run using the C module of this software.

This interface lets the user execute a same script/program over multiple input data in several CPUs located at the antz computing server. It sports memory management so nodes do not run out of RAM due to too many processes being started in the same node. It also reports resource usage data after execution.

**Compilation:** Makefile takes care.

**Documentation:** run $doxigen and take a look at html/index.html for documentation

**Usage:** ./pvm\_test exec\_type program datafile nodefile outputdir [memory] [maple\_flag]
 - *exec_type*:
  - 0 = Maple
  - 1 = C
  - 2 = Python
 - *program*: absolute path to program file
 - *datafile*: absolute path to data file
  - Line format is "tasknumber,arg1,arg2,...,argN"
 - *nodefile*: absolute path to PVM node file
  - Line format is "nodename numerofprocesses"
 - *outputdir*: absolute path to output directory
 - *memory*: (optional) max amount of RAM (in KB) that a single execution can require
  - Remark: if this argument is not used, the program picks a 25% of max RAM threshold for assigning jobs (this is not optimal for nodes with much RAM such as a05 and a08, but it is safe and not too bad for the rest).
 - *maple_flag*:
  - 0 = multicore execution (default)
  - 1 = single core execution

**Memory reporting:**
 - *Maple*: Maple uses multithreading to parallelize the executions by default. This is good for performance but bad for resource management, because the PVM task loses control of the processes spawned. Therefore, the output file mem\*.log doesn't show accurate values for resource usage of the program, because it can only track the parent Maple process, which doesn't do any work besides spawning and controlling its child processes. *Workaround*: we execute maple with the -t flag, so in \*\_err.txt (output error file) we can see the kernelopts line that reports memory usage and computation time directly from maple.
 - *C* and *Python*: as long as the program to be executed is sequential, the PVM task will be able to get resource usage from the execution (using C system call *getrusage()*) and print it to the mem\*.log file.
