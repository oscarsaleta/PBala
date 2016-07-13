# PBala (Princess Bala): DISTRIBUTED EXECUTION SOFTWARE FOR ANTZ

# Releases and changelog

- **v4.0.0**:
 - Revamped argument parsing

- **v3.0.2**:
 - Bugfix: if the program exited with error status after PVM was initialized, it did not halt the virtual machine. Now it does automatically, so no manual halt is needed.

- **v3.0.1**:
 - Changed procedure for cleaning auxiliary Pari/Sage scripts generated during execution.

- **v3.0.0**:
 - Added Sage support using `task_type=4`. Sage scripts must have extension *.sage*, *.py* or *.spyx* in order for Sage to execute them.

- **v2.0.1**:
 - Solved bug with executions that had less tasks than nodes (execution would not finish unless killed)

- **v2.0.0**:
 - **Automatic generation of unfinished tasks datafile** (i.e. a clone of datafile containing only the lines that correspond to tasks that were killed before they could finish execution). This means that the program can later be run again using this new data file as input instead of having to manually search for the necessary data.
 - Added more error checks and error cases when forking tasks.

- **v1.3.0**:
 - Added more info at the beginning of stderr (system call, executable, datafile, nodefile, output directory)
 - Added a version variable in antz_lib.h

- **v1.2.1**:
 - Solved a long standing segfault problem that triggered when the output_dir did not exist.
 - Added one more error code

- **v1.2.0**:
 - Added error codes (see bottom of this file) to better inform of what made your execution crash.

- **v1.1.0**:
 - Added ability to detect if a task was killed or stopped.
 - Changed format files, now there are 4 different files: task\*_stdout, task\*_stderr, task\*_mem and task\*_killed (this last one just for tasks that *have been killed*). This way all the files for the same task can be easily located.
 - Fixed a bug that caused nodefiles of a single node to stop the execution.
 
- **v1.0.0**: First release
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
 - **Pari/GP** scripts (although Pari should be also supported through the Pari C/C++ library or through gp2c by manually editing the output C code and compiling as a C program using the command provided by gp2c. The executable can then be run using the C module of this software.)
 - **Sage** scripts

This interface lets the user execute a same script/program over multiple input data in several CPUs located at the antz computing server. It sports memory management so nodes do not run out of RAM due to too many processes being started in the same node. It also reports resource usage data after execution.

## Compilation

The command `make` takes care.

## Documentation

Run `doxigen` and take a look at html/index.html for documentation

## Usage

**As of v4.0.0**: the program admits standard `--help` (`-?`), `--usage` and `--version` (`-V`) arguments.

Output from `./PBala --help`:
```
Usage: PBala [OPTION...] programflag programfile datafile nodefile outdir
PBala -- PVM SPMD execution parallellizer

  -e, --create-errfiles      Create stderr files
  -g, --create-memfiles      Create memory files
  -h, --create-slavefile     Create node file
  -m, --max-mem-size=MAX_MEM Max memory size of a task (KB)
  -s, --maple-single-core    Force single core Maple
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <osr@mat.uab.cat>.
```
Output from `./PBala --usage`:
```
Usage: PBala [-eghs?V] [-m MAX_MEM] [--create-errfiles] [--create-memfiles]
            [--create-slavefile] [--max-mem-size=MAX_MEM] [--maple-single-core]
            [--help] [--usage] [--version]
            programflag programfile datafile nodefile outdir
```

Mandatory arguments explained:
- `programflag`:
 - 0 = Maple
 - 1 = C
 - 2 = Python
 - 3 = Pari (as of v1.0.0)
 - 4 = Sage (as of v3.0.0)
- `programfile`: path to program file
- `datafile`: path to data file
 - Line format is "tasknumber,arg1,arg2,...,argN"
- `nodefile`: path to PVM node file
 - Line format is "nodename number_of_processes"
- `outdir`: path to output directory

Options explained:
- `-e, --create-errfiles`: Save stderr output for each execution in a task_stderr.txt file
- `-g, --create-memfiles`: Save memory info for each execution in a task_mem.txt file
- `-h, --create-slavefile`: Save a log of which task is given to which slave in node_info.txt
- `-m, --max-mem-size=MAX_MEM`: max amount of RAM (in KB) that a single execution can require
 - Remark: the default behaviour is not giving work to a slave unless more than 15% of the max memory is available
- `-s, --maple-single-core`: Force Maple to use a single core for its executions


**For versions previous to v4.0.0**:
`./PBala exec_type program datafile nodefile outputdir [memory] [maple_flag]`
 - *exec_type*:
  - 0 = Maple
  - 1 = C
  - 2 = Python
  - 3 = Pari (as of v1.0.0)
  - 4 = Sage (as of v3.0.0)
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
