# PBala (Princess Bala): DISTRIBUTED EXECUTION SOFTWARE FOR ANTZ

# Releases and changelog

- v1.3.0:
 - Added more info at the beginning of stderr (system call, executable, datafile, nodefile, output directory)
 - Added a version variable in antz_lib.h

- v1.2.1:
 - Solved a long standing segfault problem that triggered when the output_dir did not exist.
 - Added one more error code

- v1.2.0:
 - Added error codes (see bottom of this file) to better inform of what made your execution crash.

- v1.1.0:
 - Added ability to detect if a task was killed or stopped.
 - Changed format files, now there are 4 different files: task\*_stdout, task\*_stderr, task\*_mem and task\*_killed (this last one just for tasks that *have been killed*). This way all the files for the same task can be easily located.
 - Fixed a bug that caused nodefiles of a single node to stop the execution.
 
- v1.0.0: First release
 - New name: *Princess Bala* (because this software makes Antz work much harder).
 - Paths no longer need to be absolute.
 - PVM path is the current dir, so the executables work out of the box.
 - PVM automatically starts and stops, no need of prior startup or halt to clean task processes. Hostfile is automatically created from nodefile.
 - Pari *.gp* (not compiled) scripts can be executed using flag 3.

# Explanation of the software

## Introduction

This C program uses PVM libraries in order to create a parallellization interface for
 - **Maple** scripts
 - **C** programs
 - **Python** scripts
 - **Pari** scripts (although Pari should be also supported through the Pari C/C++ library or through gp2c by manually editing the output C code and compiling as a C program using the command provided by gp2c. The executable can then be run using the C module of this software.)

This interface lets the user execute a same script/program over multiple input data in several CPUs located at the antz computing server. It sports memory management so nodes do not run out of RAM due to too many processes being started in the same node. It also reports resource usage data after execution.

## Compilation

The command `make` takes care.

## Documentation

Run `doxigen` and take a look at html/index.html for documentation

## Usage

`./PBala exec_type program datafile nodefile outputdir [memory] [maple_flag]`
 - *exec_type*:
  - 0 = Maple
  - 1 = C
  - 2 = Python
  - 3 = Pari
 - *program*: path to program file
 - *datafile*: path to data file
  - Line format is "tasknumber,arg1,arg2,...,argN"
 - *nodefile*: path to PVM node file
  - Line format is "nodename numerofprocesses"
 - *outputdir*: path to output directory
 - *memory*: (optional) max amount of RAM (in KB) that a single execution can require
  - Remark: if this argument is not used, the program picks a 25% of max RAM threshold for assigning jobs (this is not optimal for nodes with much RAM such as a05 and a08, but it is safe and not too bad for the rest).
 - *maple_flag*:
  - 0 = multicore execution (default)
  - 1 = single core execution

### Conventions

  For Maple, we define 2 variables: **taskId** and **taskArgs**. taskId is an identifier for the task number that we are sending to the Maple script. taskArgs are the actual arguments that Maple has to use to do the computations. It is important to use these names because they are passed to Maple this way.

  For C and Python we use the argv arrays so make sure the program can read and use those variables (and perform the error checking because this software has no way of knowing if the data file is suitable for your program).

### Procedure of execution

  *As of v1.0.0, there is no need of starting PVM using a hostfile (it is actually advised not to do so). Instead, execute the program and we will start PVM for you, using the information extracted from the nodefile.*

  - First, you will need to execute PVM and add all the nodes that you want to use. This is done easily with `pvm hostfile`, where *hostfile* is simply a file with all the names of the nodes we will use in it (exactly the same names that appear in *nodefile*, but without the 2nd column). From inside the PVM cmd, type `quit` to leave the application and have the PVM daemon running in the background.
  - Now compile the software (if you have not done so yet) with `make`, and your own program if needed. Also, `mkdir resultdir` if it does not exist.
  - Third, run the parallelization software with the command stated above (remember to use absolute paths). You can redirect *stderr* to a file appending `2> foo.txt` to the command and leave the process running in the background by appending `&`.
  - When the execution finishes and all the nodes are closed (this is done automatically) PVM leaves behind a bunch of sleeping task processes which should be cleaned up by shutting down the PVM daemon: `pvm` and `halt` from inside the PVM cmd to stop it.

## Memory reporting
  
 - *Maple*: Maple uses multithreading to parallelize the executions by default. This is good for performance but bad for resource management, because the PVM task loses control of the processes spawned. Therefore, the output file mem\*.log doesn't show accurate values for resource usage of the program, because it can only track the parent Maple process, which doesn't do any work besides spawning and controlling its child processes. 
  - *Workaround*: we execute maple with the -t flag, so in \*\_err.txt (output error file) we can see the kernelopts line that reports memory usage and computation time directly from maple.
 - *C*, *Python* and *Pari*: as long as the program to be executed is sequential, the PVM task will be able to get resource usage from the execution (using C system call *getrusage()*) and print it to the mem\*.log file.

## Error codes

*(New in version 1.2.0)* Error codes were added to the main executable *PBala*. Now if the program aborts you can simply check the error code in terminal (you also still can check the output file) to see what happened.

Existing codes:

- 10: error with command line arguments passed to the program
- 11: error when counting number of lines of nodefile
- 12: error when trying to open nodefile
- 13: error when trying to read nodefile
- 14: error when getting the current working directory
- 15: error when asking pvmd for my TID (task identifier)
- 16: error when asking pvmd for the task parent
- 17: error when counting number of lines of datafile
- 18: error when creating output file
- 19: error when spawning pvm tasks
- 20: error when reading first column of datafile (must be task id)
- 21: error when creating out_dir/node_info.txt (maybe out_dir does not exist?)
