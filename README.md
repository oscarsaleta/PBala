<!--
    This file is part of PBala (http://github.com/oscarsaleta/PBala)

    Copyright (C) 2016  O. Saleta

    PBala is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
-->


# PBala (Princess Bala): DISTRIBUTED EXECUTION SOFTWARE FOR ANTZ

# Current version

Current version is 6.0.1. 

# Download and install
Go to the [latest release](https://www.github.com/oscarsaleta/PBala/releases/latest "Latest release") for download links.

To install PBala, simply extract the contents of one of the attached downloads in the release page and copy the executables `PBala` and `PBala_task` to the directory where you want to work (you can ignore the `include` directory).

Also, make sure you have these lines in your `.bashrc` (for _sh_ users):
```
export PVM_ROOT=/usr/lib/pvm3
export PVM_DPATH=$PVM_ROOT/lib/pvmd
```
or this in your `.cshrc` (for _csh_ users):
```
setenv PVM_ROOT /usr/lib/pvm3
```

# Examples
See [the examples directory](Examples "Examples") the examples directory.

# Changelog
See [the changelog](CHANGELOG.md "CHANGELOG.md").

# Explanation of the software

## Introduction

This C program uses PVM libraries in order to create a parallellization interface for

* **Maple** scripts
* **C** programs
* **Python** scripts
* **Pari/GP** scripts (although Pari should be also supported through the Pari C/C++ library or through gp2c by manually editing the output C code and compiling as a C program using the command provided by gp2c. The executable can then be run using the C module of this software.)
* **Sage** scripts
* **Octave** scripts

This interface lets the user execute a same script/program over multiple input data in several CPUs located at the antz computing server. It sports memory management so nodes do not run out of RAM due to too many processes being started in the same node. It also reports resource usage data after execution.

## Manual compilation and installation

### Dependencies
* [Parallel Virtual Machine (PVM)](http://www.netlib.org/pvm3/ "PVM homepage").
* C compiler
* CMake (version 2.6 or higher)

### Instructions

* Clone this git repository: `git clone https://github.com/oscarsaleta/PBala.git` in a directory of your system,
* Change dir into *PBala*: `cd PBala`,
* Create a build directory: `mkdir build && cd build`,
* Run the CMake command: `cmake ..`,
* Compile the code with `make`, the executables *PBala* and *PBala_task* will be found in *build/src*.
* Optionally, perform a system wide install: `sudo make install`, which will install *PBala* and *PBala_task* into */usr/local/bin* and *PBala_config.h* into */usr/local/include* (this requires root permissions).


## Documentation

Run `doxigen` and take a look at *html/index.html* for documentation.

## Usage

The program admits standard `--help` (`-?`), `--usage` and `--version` (`-V`) arguments.

Output from `./PBala --help`:

```
Usage: PBala [OPTION...] programflag programfile datafile nodefile outdir
PBala -- PVM SPMD execution parallellizer.
	programflag argument can be: 0 (Maple), 1 (C), 2 (Python), 3 (Pari), 4 (Sage),
or 5 (Octave)

  -c, --custom-process=/path/to/exec
                             Specify a custom path for the executable program
  -e, --create-errfiles      Create stderr files
  -g, --create-memfiles      Create memory files
  -h, --create-slavefile     Create node file
  -k, --kill                 Kill remainig PBala/PVM processes (WARNING: use at
                             own risk! Use only if something goes wrong during
                             an execution and PVM stops working and you have no
                             other important processes running)
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
Usage: PBala [-ceghs?V] [-m MAX_MEM] [--create-errfiles] [--create-memfiles]
            [--create-slavefile] [--max-mem-size=MAX_MEM] [--maple-single-core]
            [--help] [--usage] [--version]
            programflag programfile datafile nodefile outdir
```

Mandatory arguments explained:

* `programflag`:
    + 0 = Maple
    + 1 = C
    + 2 = Python
    + 3 = Pari
    + 4 = Sage
    + 5 = Octave
* `programfile`: path to program file
* `datafile`: path to data file
    + Line format is "tasknumber,arg1,arg2,...,argN"
* `nodefile`: path to PVM node file
    + Line format is "nodename number_of_processes"
* `outdir`: path to output directory

Options explained:

- `-c, --custom-process=/path/to/exec`: Specify a custom path for the Maple/Python/etc executable to use
- `-e, --create-errfiles`: Save stderr output for each execution in a task_stderr.txt file
- `-g, --create-memfiles`: Save memory info for each execution in a task_mem.txt file
- `-h, --create-slavefile`: Save a log of which task is given to which slave in node_info.txt
- `-m, --max-mem-size=MAX_MEM`: max amount of RAM (in KB) that a single execution can require
    + Remark: the default behaviour is not giving work to a slave unless more than 15% of the max memory is available
- `-s, --maple-single-core`: Force Maple to use a single core for its executions



### Conventions

 - Computer Algebra Systems (CAS): **Maple**, **PARI** and **Sage**:
    + For Maple, we define 2 variables: `taskId` and `taskArgs`. taskId is an identifier for the task number that we are sending to the Maple script. `taskArgs` are the actual arguments that Maple has to use to do the computations. It is important to use these names because they are passed to Maple this way.
    + PARI and Sage are executed by creating auxiliary scripts where `taskId` and `taskArgs` are defined, so they could be directly used in the scripts just like in Maple.
- Programming languages: **C**, **Python**:
    + For C and Python we use the `argv` arrays so make sure the program can read and use those variables (and perform the error checking because this software has no way of knowing if the data file is suitable for your program).
- Octave:
    + We define the `taskArgs` list as a _cell array_, so you need to use the proper indexing and accessors to use the stored values (see [the Examples section](Examples/README.md#octave "Octave section in Examples") for more information).

### Procedure of execution

There is no need of starting PVM using a hostfile (it is actually advised not to do so). Instead, execute the program and we will start PVM for you, using the information extracted from the nodefile.

## Memory reporting
  
- *Maple*: Maple uses multithreading to parallelize the executions by default. This is good for performance but bad for resource management, because the PVM task loses control of the processes spawned. Therefore, the output files task_mem\*.txt don't show accurate values for resource usage of the program, because we can only track the parent Maple process, which doesn't do any work besides spawning and controlling its child processes. 
    + *Workaround*: we execute Maple with the `-t` flag, so in \*\_err.txt (output error file) we can see the `kernelopts` line that reports memory usage and computation time directly from Maple.
- *C*, *Python*, *Pari* and *Sage*: as long as the program to be executed is sequential, the PVM task will be able to get resource usage from the execution (using C system call `getrusage()`) and print it to the task_mem\*.txt file.

## Error codes

If *PBala* aborts during execution you can check the return value in the command line to know which kind of error happened. You can also check the output file for more information.

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
