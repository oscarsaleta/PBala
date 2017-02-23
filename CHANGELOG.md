# Releases and changelog
* **v4.1.3**:
    - Solved an issue that prevented the user from select Maple executions.
* **v4.1.2**:
    - Changed build system to *CMake* (build instructions in <a href="README.md">readme</a>).
    - Added error check for Maple single CPU file preparation.
    - Added parentheses in two conditional expressions to avoid compilation warnings.
    - Removed unused variable.
* **v4.1.1**:
    - Added error checks in some unchecked `fgets`, `sscanf` and `fopen` calls.
    - Some source files renamed from `antz_*` to `PBala_*`.
* **v4.1.0**:
    - Added *autotools* support (`configure && make && make install`).
    - Solved issue <a href="https://github.com/oscarsaleta/PBala/issues/1">#1</a>.
* **v4.0.0**:
    - Revamped argument parsing (see the whole new *Usage* section).
    - Changed default safety memory threshold from 25% of total RAM to 15% of total RAM (user caution advised).
    - Added flags for creating \*_stderr.txt files, \*_mem.txt files, and node_info.txt file. Default behaviour now is *not* to create them unless specified with the optional flags.
* **v3.0.2**:
    - Bugfix: if the program exited with error status after PVM was initialized, it did not halt the virtual machine. Now it does automatically, so no manual halt is needed.
* **v3.0.1**:
    - Changed procedure for cleaning auxiliary Pari/Sage scripts generated during execution.
* **v3.0.0**:
    - Added Sage support using `task_type=4`. Sage scripts must have extension *.sage*, *.py* or *.spyx* in order for Sage to execute them.
* **v2.0.1**:
    - Solved bug with executions that had less tasks than nodes (execution would not finish unless killed)
* **v2.0.0**:
 * **Automatic generation of unfinished tasks datafile** (i.e. a clone of datafile containing only the lines that correspond to tasks that were killed before they could finish execution). This means that the program can later be run again using this new data file as input instead of having to manually search for the necessary data.
    - Added more error checks and error cases when forking tasks.
* **v1.3.0**:
    - Added more info at the beginning of stderr (system call, executable, datafile, nodefile, output directory)
    - Added a version variable in antz_lib.h
* **v1.2.1**:
    - Solved a long standing segfault problem that triggered when the output_dir did not exist.
    - Added one more error code
* **v1.2.0**:
    - Added error codes (see bottom of this file) to better inform of what made your execution crash.
* **v1.1.0**:
    - Added ability to detect if a task was killed or stopped.
    - Changed format files, now there are 4 different files: task\*_stdout, task\*_stderr, task\*_mem and task\*_killed (this last one just for tasks that *have been killed*). This way all the files for the same task can be easily located.
    - Fixed a bug that caused nodefiles of a single node to stop the execution. 
* **v1.0.0**: First release
    - New name: *Princess Bala* (because this software makes Antz work much harder).
    - Paths no longer need to be absolute.
    - PVM path is the current dir, so the executables work out of the box.
    - PVM automatically starts and stops, no need of prior startup or halt to clean task processes. Hostfile is automatically created from nodefile.
    - Pari *.gp* (not compiled) scripts can be executed using flag 3.
