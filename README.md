[comment]: #  (This file is part of PBala (http://github.com/oscarsaleta/PBala))
[comment]: #  ( )
[comment]: #  (Copyright (C) 2016  O. Saleta)
[comment]: #  ( )
[comment]: #  (PBala is free software: you can redistribute it and/or modify)
[comment]: #  (it under the terms of the GNU Lesser General Public License as published)
[comment]: #  (by the Free Software Foundation, either version 3 of the License, or)
[comment]: #  ((at your option) any later version.)
[comment]: #  ( )
[comment]: #  (This program is distributed in the hope that it will be useful,)
[comment]: #  (but WITHOUT ANY WARRANTY; without even the implied warranty of)
[comment]: #  (MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the)
[comment]: #  (GNU Lesser General Public License for more details.)
[comment]: #  ( )
[comment]: #  (You should have received a copy of the GNU Lesser General Public License)
[comment]: #  (along with this program.  If not, see <http://www.gnu.org/licenses/>.)


# PBala (Princess Bala): DISTRIBUTED EXECUTION SOFTWARE FOR ANTZ

# Current version

Current version is 4.2.0. 

# Download and install
Go to the <a href="https://www.github.com/oscarsaleta/PBala/releases/latest">latest release</a> for download links and installation instructions.

# Examples
See <a href="Examples/">the examples directory</a>.

# Changelog
See <a href="CHANGELOG.md">CHANGELOG.md</a>.

# Explanation of the software

## Introduction

This C program uses PVM libraries in order to create a parallellization interface for

* **Maple** scripts
* **C** programs
* **Python** scripts
* **Pari/GP** scripts (although Pari should be also supported through the Pari C/C++ library or through gp2c by manually editing the output C code and compiling as a C program using the command provided by gp2c. The executable can then be run using the C module of this software.)
* **Sage** scripts

This interface lets the user execute a same script/program over multiple input data in several CPUs located at the antz computing server. It sports memory management so nodes do not run out of RAM due to too many processes being started in the same node. It also reports resource usage data after execution.

## Manual compilation and installation

### v4.1.2 and above (requires CMake 3.0 or higher)

* Clone this git repository: `git clone https://github.com/oscarsaleta/PBala.git` in a directory of your system,
* Change dir into *PBala*: `cd PBala`,
* Create a build directory: `mkdir build && cd build`,
* Run the CMake command: `cmake ..`,
* Compile the code with make: `make`, the executables *PBala* and *PBala_task* will be found in *build/src*.
* Optionally, perform a system wide install: `sudo make install`, which will install *PBala* and *PBala_task* into */usr/local/bin* and *PBala_config.h* into */usr/local/include*.


### Versions older than v4.1.0
If the older version of the source tree provides a Makefile, running `make` in a Terminal from that directory should take care of compilation. No automatic system installation is provided in the Makefile.


## Documentation

Run `doxigen` and take a look at *html/index.html* for documentation.

## Usage

### As of v4.0.0

The program admits standard `--help` (`-?`), `--usage` and `--version` (`-V`) arguments.

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

* `programflag`:
 - 0 = Maple
 - 1 = C
 - 2 = Python
 - 3 = Pari (as of v1.0.0)
 - 4 = Sage (as of v3.0.0)
* `programfile`: path to program file
* `datafile`: path to data file
 - Line format is "tasknumber,arg1,arg2,...,argN"
* `nodefile`: path to PVM node file
 - Line format is "nodename number_of_processes"
* `outdir`: path to output directory

Options explained:

- `-e, --create-errfiles`: Save stderr output for each execution in a task_stderr.txt file
- `-g, --create-memfiles`: Save memory info for each execution in a task_mem.txt file
- `-h, --create-slavefile`: Save a log of which task is given to which slave in node_info.txt
- `-m, --max-mem-size=MAX_MEM`: max amount of RAM (in KB) that a single execution can require
 - Remark: the default behaviour is not giving work to a slave unless more than 15% of the max memory is available
- `-s, --maple-single-core`: Force Maple to use a single core for its executions


### For versions previous to v4.0.0

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

  Pari and Sage are executed by creating auxiliary scripts where **taskId** and **taskArgs** are defined, so they could be directly used in the scripts just like in Maple.

### Procedure of execution

**As of v1.0.0**

There is no need of starting PVM using a hostfile (it is actually advised not to do so). Instead, execute the program and we will start PVM for you, using the information extracted from the nodefile.


**For versions before v1.0.0 [DEPRECATED]**

  - First, you will need to execute PVM and add all the nodes that you want to use. This is done easily with `pvm hostfile`, where *hostfile* is simply a file with all the names of the nodes we will use in it (exactly the same names that appear in *nodefile*, but without the 2nd column). From inside the PVM cmd, type `quit` to leave the application and have the PVM daemon running in the background.
  - Now compile the software (if you have not done so yet) with `make`, and your own program if needed. Also, `mkdir resultdir` if it does not exist.
  - Third, run the parallelization software with the command stated above (remember to use absolute paths). You can redirect *stderr* to a file appending `2> foo.txt` to the command and leave the process running in the background by appending `&`.
  - When the execution finishes and all the nodes are closed (this is done automatically) PVM leaves behind a bunch of sleeping task processes which should be cleaned up by shutting down the PVM daemon: `pvm` and `halt` from inside the PVM cmd to stop it.

## Memory reporting
  
 - *Maple*: Maple uses multithreading to parallelize the executions by default. This is good for performance but bad for resource management, because the PVM task loses control of the processes spawned. Therefore, the output files task_mem\*.txt don't show accurate values for resource usage of the program, because we can only track the parent Maple process, which doesn't do any work besides spawning and controlling its child processes. 
  - *Workaround*: we execute maple with the -t flag, so in \*\_err.txt (output error file) we can see the kernelopts line that reports memory usage and computation time directly from maple.
 - *C*, *Python*, *Pari* and *Sage*: as long as the program to be executed is sequential, the PVM task will be able to get resource usage from the execution (using C system call *getrusage()*) and print it to the task_mem\*.txt file.

## Error codes

*(New in version 1.2.0)* Error codes were added to the main executable *PBala*. Now if the program aborts you can simply check the error code in terminal (you also still can check the output file) to see what happened.

Existing codes:

* **PBala.c**
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
    - 22: invalid task number
    - 23: error when writing to a file (PARI/GP or Sage aux script)
    - 24: error when sanitizing Maple script for single CPU execution
    - 25: error when there is a duplicate PVM host and PBala fails to remove it
* **PBala_task.c**
    - 10: error when forking process
    - 11: task killed by system
