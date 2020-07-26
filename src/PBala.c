/* This file is part of PBala (http://github.com/oscarsaleta/PBala)
 *
 * Copyright (C) 2016  O. Saleta
 *
 * PBala is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*! \mainpage PVM general parallelizer for antz
 * \author Oscar Saleta Reig
 */

/*! \file PBala.c
 * \brief Main PVM program. Distributes executions of SDMP in antz
 * \author Oscar Saleta Reig
 */

#include "PBala_config.h"
#include "PBala_errcodes.h"
#include "PBala_lib.h"

#include <argp.h>
#include <dirent.h>
#include <pvm3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* Program version and bug email */
const char *argp_program_version = VERSION;
const char *argp_program_bug_address = "<osr@mat.uab.cat>";

/* Program documentation */
static char doc[] = "PBala -- PVM SPMD execution parallellizer.\n\tprogramflag "
                    "argument can be: 0 (Maple), 1 (C), 2 (Python), 3 (Pari), "
                    "4 (Sage), or 5 (Octave)";
/* Arguments we accept */
static char args_doc[] = "programflag programfile datafile nodefile outdir";

/* Options we understand */
static struct argp_option options[] = {
    {"kill", 'k', 0, 0,
     "Kill remainig PBala/PVM processes (WARNING: use at "
     "own risk! Use only if something goes wrong during an "
     "execution and PVM stops working and you have no other "
     "important processes running)"},
    {"max-mem-size", 'm', "MAX_MEM", 0, "Max memory size of a task (KB)"},
    {"maple-single-core", 's', 0, 0, "Force single core Maple"},
    {"create-errfiles", 'e', 0, 0, "Create stderr files"},
    {"create-memfiles", 103, 0, 0, "Create memory files"},
    {"create-slavefile", 104, 0, 0, "Create node file"},
    {"custom-process", 'c', "/path/to/exec", 0,
     "Specify a custom path for the executable program"},
    {0}};

/* Struct for communicating arguments to main */
struct arguments {
    char *args[5];
    int kill;
    long int max_mem_size;
    int maple_single_cpu, create_err, create_mem, create_slave;
    int custom_path;
    char program_path[BUFFER_SIZE];
};

/* Parse a single option */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;

    switch (key) {
    case 'k':
        arguments->kill = 1;
        break;
    case 'm':
        sscanf(arg, "%ld", &(arguments->max_mem_size));
        break;
    case 's':
        arguments->maple_single_cpu = 1;
        break;
    case 'e':
        arguments->create_err = 1;
        break;
    case 103:
        arguments->create_mem = 1;
        break;
    case 104:
        arguments->create_slave = 1;
        break;
    case 'c':
        arguments->custom_path = 1;
        sscanf(arg, "%s", arguments->program_path);
        break;

    case ARGP_KEY_ARG:
        if (state->arg_num >= 5)
            argp_usage(state);
        arguments->args[state->arg_num] = arg;
        break;

    case ARGP_KEY_END:
        if (state->arg_num < 5 && arguments->kill != 1)
            argp_usage(state);
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* argp parser */
static struct argp argp = {options, parse_opt, args_doc, doc};

/**
 * Main PVM function. Handles task creation and result gathering.
 * Call: ./PBala programFlag programFile dataFile nodeFile outDir [max_mem_size
 * (KB)] [maple_single_core]
 *
 * \param[in] argv[1] flag for program type (0=maple,1=C,2=python,3=pari,4=sage)
 * \param[in] argv[2] program file (maple library, c executable, etc)
 * \param[in] argv[3] data input file
 * \param[in] argv[4] nodes file (2 cols: node cpus)
 * \param[in] argv[5] output file directory
 * \param[in] argv[6] (optional) aprox max memory size of biggest execution in
 * KB
 * \param[in] argv[7] (optional) flag for single core execution (Maple only:
 * 0=no, 1=yes)
 *
 * \return 0 if successful
 */
int main(int argc, char *argv[]) {
    // Program options and arguments
    struct arguments arguments;
    arguments.kill = 0;
    arguments.max_mem_size = 0;
    arguments.maple_single_cpu = 0;
    arguments.create_err = 0;
    arguments.create_mem = 0;
    arguments.create_slave = 0;
    arguments.custom_path = 0;
    // PVM args
    int myparent, mytid;
    int itid;
    int work_code;
    // File names
    char inp_programFile[FNAME_SIZE];
    char inp_dataFile[FNAME_SIZE];
    char inp_nodes[FNAME_SIZE];
    char out_dir[FNAME_SIZE];
    char nodeInfoFileName[FNAME_SIZE];
    char out_file[FNAME_SIZE];
    char cwd[FNAME_SIZE];
    // Files
    FILE *nodeInfoFile = NULL;
    FILE *f_out = NULL;
    // Nodes variables
    char **nodes;
    int *nodeCores;
    int nNodes, maxConcurrentTasks;
    // tasks
    int nTasks, runningTasks = 0;
    task_ptr currentTask;
    int unfinished_tasks_present = 0;
    // Aux variables
    int i, j;
    char aux_str[BUFFER_SIZE];
    // Task variables
    int task_type;
    // Execution time variables
    double exec_time, total_time = 0;
    struct timespec tspec_before, tspec_after, tspec_result;

    clock_gettime(CLOCK_REALTIME, &tspec_before);

    /* MASTER CODE */

    /* set stderr as a line buffered output stream */
    setlinebuf(stderr);
    // setvbuf(stderr, NULL, _IOLBF, BUFFER_SIZE);

    /* Read command line arguments */
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    /* if kill option is found, we do that and exit */
    if (arguments.kill)
        return killPBala();

    if (sscanf(arguments.args[0], "%d", &task_type) != 1 ||
        sscanf(arguments.args[1], "%s", inp_programFile) != 1 ||
        sscanf(arguments.args[2], "%s", inp_dataFile) != 1 ||
        sscanf(arguments.args[3], "%s", inp_nodes) != 1 ||
        sscanf(arguments.args[4], "%s", out_dir) != 1) {
        fprintf(stderr, "%-20s - reading arguments\n", "[ERROR]");
        return E_ARGS;
    }

    // sanitize maple library if single cpu is required
    if (arguments.maple_single_cpu) {
        if (mapleSingleCPU(inp_programFile) != 0)
            return E_MPL;
    }

    // check if task type is correct
    if (task_type != 0 && task_type != 1 && task_type != 2 && task_type != 3 &&
        task_type != 4 && task_type != 5) {
        fprintf(stderr,
                "%-20s - Wrong task_type value (must be one of: "
                "0,1,2,3,4,5)\n",
                "[ERROR]");
        return E_WRONG_TASK;
    }

    // prepare node_info.txt file if desired
    if (arguments.create_slave) {
        sprintf(nodeInfoFileName, "%s/node_info.txt", out_dir);
        nodeInfoFile = fopen(nodeInfoFileName, "w");
        if (nodeInfoFile == NULL) {
            fprintf(
                stderr,
                "%-20s - Cannot create file %s, make sure the output folder %s "
                "exists\n",
                "[ERROR]", nodeInfoFileName, out_dir);
            return E_OUTDIR;
        }
        fprintf(nodeInfoFile, "# NODE CODENAMES\n");
    }

    /*
     * Read node configuration file
     */
    if ((nNodes = parseNodeFile(inp_nodes, &nodes, &nodeCores)) < 0) {
        printAbort();
        return E_NODEFILE;
    }

    /*
     * READ DATAFILE
     */
    // get tasknumber,args from file and store them in a linked list of tasks
    currentTask = NULL;
    if ((nTasks = getDataFromFile(inp_dataFile, &currentTask)) < 0) {
        printAbort();
        return E_DATAFILE;
    }

    /*
     * INITIALIZE PVMD
     */
    /* get current working directory */
    if (getcwd(cwd, FNAME_SIZE) == NULL) {
        fprintf(stderr, "%-20s - Cannot resolve current directory\n",
                "[ERROR]");
        return E_CWD;
    }

    /* create hostfile */
    FILE *hostfile = fopen("hostfile", "w");
    fprintf(hostfile, "* ep=%s wd=%s\n", cwd, cwd);
    for (i = 0; i < nNodes; i++)
        fprintf(hostfile, "%s\n", nodes[i]);
    fclose(hostfile);

    /* attempt PVM initialization */
    char *pvmd_argv[1] = {"hostfile"};
    int pvmd_argc = 1;
    int start_status;
    int start_tries = 0;
    while ((start_status = pvm_start_pvmd(pvmd_argc, pvmd_argv, 1)) ==
           PvmDupHost) {
        ++start_tries;
        pvm_halt();
        system("rm -f /tmp/pvm*");
        if (start_tries > 3)
            return E_PVM_DUP;
    }
    sprintf(out_file, "%s/pvm_log.txt", out_dir);
    if ((f_out = fopen(out_file, "w")) == NULL) {
        fprintf(stderr, "%-20s - Cannot create PVM log file %s\n", "[ERROR]",
                out_file);
        pvm_halt();
        return E_OUTFILE_OPEN;
    }
    pvm_catchout(f_out);
    // Error task id
    mytid = pvm_mytid();
    if (mytid < 0) {
        pvm_perror(argv[0]);
        pvm_halt();
        return E_PVM_MYTID;
    }
    // Error parent id
    myparent = pvm_parent();
    if (myparent < 0 && myparent != PvmNoParent) {
        pvm_perror(argv[0]);
        pvm_halt();
        return E_PVM_PARENT;
    }
    /***/

    // Max number of tasks running at once
    maxConcurrentTasks = 0;
    for (i = 0; i < nNodes; i++) {
        maxConcurrentTasks += nodeCores[i];
    }

    // Print execution info
    printf("\n\n == PRINCESS BALA v%s ==\n", VERSION);
    printf("System call: ");
    for (i = 0; i < argc; i++)
        printf("%s ", argv[i]);
    printf("\n\n");

    printf("%-20s - Will use executable %s\n", "[INFO]", inp_programFile);
    printf("%-20s - Will use datafile %s\n", "[INFO]", inp_dataFile);
    printf("%-20s - Will use nodefile %s\n", "[INFO]", inp_nodes);
    printf("%-20s - Results will be stored in %s\n\n", "[INFO]", out_dir);

    printf("%-20s - Will use nodes ", "[INFO]");
    for (i = 0; i < nNodes - 1; i++)
        printf("%s (%d), ", nodes[i], nodeCores[i]);
    printf("%s (%d)\n", nodes[nNodes - 1], nodeCores[nNodes - 1]);
    printf("%-20s - Will create %d tasks for %d slaves in %d nodes\n\n",
           "[INFO]", nTasks, maxConcurrentTasks, nNodes);

    // Spawn all the slaves
    printf("== INITIALISING PVM NODES ==\n");
    int slaveId[maxConcurrentTasks];
    itid = 0;
    int numt;
    int numnode = 0;
    for (i = 0; i < nNodes; i++) {
        for (j = 0; j < nodeCores[i]; j++) {
            if (access("PBala_task", F_OK) != -1) {
                numt = pvm_spawn("PBala_task", NULL, PvmTaskHost, nodes[i], 1,
                                 &slaveId[itid]);
            } else {
                char username[BUFFER_SIZE];
                sprintf(username, "%s", getlogin());
                char auxchar[BUFFER_SIZE];
                sprintf(auxchar, "/home/%s/bin/PBala_task", username);

                if (access(auxchar, F_OK) != -1) {
                    numt = pvm_spawn(auxchar, NULL, PvmTaskHost, nodes[i], 1,
                                     &slaveId[itid]);
                } else {
                    fprintf(stderr,
                            "%-20s - Cannot find executable PBala_task "
                            "in working directory or in %s. Make "
                            "sure you place it correctly.\n",
                            "[ERROR]", auxchar);
                    printAbort();
                    return E_NO_PBALA_TASK;
                }
            }
            if (numt != 1) {
                fprintf(stderr,
                        "%-20s - Code %d while creating task %4d in node %s\n",
                        "[ERROR]", numt, slaveId[itid], nodes[i]);
                fflush(stderr);
                pvm_perror(argv[0]);
                pvm_halt();
                return E_PVM_SPAWN;
            }
            // Send info to task
            pvm_initsend(PVM_ENCODING);
            pvm_pkint(&itid, 1, 1);
            pvm_pkint(&task_type, 1, 1);
            pvm_pklong(&(arguments.max_mem_size), 1, 1);
            pvm_pkint(&(arguments.create_err), 1, 1);
            pvm_pkint(&(arguments.create_mem), 1, 1);
            pvm_pkint(&(arguments.custom_path), 1, 1);
            if (arguments.custom_path)
                pvm_pkstr(arguments.program_path);
            pvm_send(slaveId[itid], MSG_GREETING);
            printf("%-20s - Initialised node %d\n", "[CREATE NODE]", itid);
            if (arguments.create_slave)
                fprintf(nodeInfoFile, "# Node %2d -> %s\n", numnode, nodes[i]);
            numnode++;

            itid++;
        }
    }
    printf("%-20s - All nodes created successfully\n\n", "[INFO]");

    if (arguments.create_slave)
        fprintf(nodeInfoFile, "\nNODE,TASK\n");

    printf("== SENDING WORK TO NODES ==\n");
    int status, taskNumber, tries;
    work_code = MSG_GREETING;
    while (currentTask != NULL || runningTasks != 0) {
        if (runningTasks != maxConcurrentTasks && currentTask != NULL) {
            // wait for a ready signal
            if (pvm_nrecv(-1, MSG_READY)) {
                pvm_upkint(&itid, 1, 1);
                // send a job if there is one
                if (currentTask != NULL) {
                    pvm_initsend(PVM_ENCODING);
                    pvm_pkint(&work_code, 1, 1);
                    pvm_pkint(&currentTask->number, 1, 1);
                    pvm_pkint(&currentTask->tries, 1, 1);
                    pvm_pkstr(inp_programFile);
                    pvm_pkstr(out_dir);
                    pvm_pkstr(currentTask->args);
                    // create file for pari execution if needed
                    if (task_type == 3) {
                        if (parifile(currentTask->number, currentTask->args,
                                     inp_programFile, out_dir) == -1)
                            return E_IO; // i/o error
                        printf("%-20s Creating auxiliary Pari script for task "
                               "%d\n",
                               "[CREATED SCRIPT]", currentTask->number);
                    } else if (task_type == 4) {
                        if (sagefile(currentTask->number, currentTask->args,
                                     inp_programFile, out_dir) == -1)
                            return E_IO; // i/o error
                        printf("%-20s Creating auxiliary Sage script for task "
                               "%d\n",
                               "[CREATED SCRIPT]", currentTask->number);
                    } else if (task_type == 5) {
                        if (octavefile(currentTask->number, currentTask->args,
                                       inp_programFile, out_dir) == -1)
                            return E_IO;
                        printf("%-20s Creating auxiliary Octave script for "
                               "task %d\n",
                               "[CREATED SCRIPT]", currentTask->number);
                    }

                    // send the job
                    pvm_send(slaveId[itid], MSG_WORK);
                    printf("%-20s - Sent task %3d for execution in slave %d\n",
                           "[TASK SENT]", currentTask->number, itid);
                    if (arguments.create_slave)
                        fprintf(nodeInfoFile, "%2d,%4d\n", i,
                                currentTask->number);

                    removeTask(&currentTask);
                    runningTasks++;
                }
            }
            // wait for a job result
            if (pvm_nrecv(-1, MSG_RESULT)) {
                pvm_upkint(&itid, 1, 1);
                pvm_upkint(&taskNumber, 1, 1);
                pvm_upkint(&tries, 1, 1);
                pvm_upkint(&status, 1, 1);
                pvm_upkstr(aux_str);
                // Check if response is error at forking
                if (status == ST_MEM_ERR) {
                    fprintf(stderr,
                            "%-20s - Could not execute task %d in slave %d "
                            "(out of memory)\n",
                            "[ERROR]", taskNumber, itid);
                    if (tries < MAX_TASK_TRIES)
                        addTask(&currentTask, taskNumber, aux_str, tries);
                    else {
                        addUnfinishedTask(inp_dataFile, taskNumber, aux_str);
                        unfinished_tasks_present = 1;
                    }
                } else if (status == ST_FORK_ERR) {
                    fprintf(stderr,
                            "%-20s - Could not fork process for task "
                            "%d in slave %d\n",
                            "[ERROR]", taskNumber, itid);
                    if (tries < MAX_TASK_TRIES)
                        addTask(&currentTask, taskNumber, aux_str, tries);
                    else {
                        addUnfinishedTask(inp_dataFile, taskNumber, aux_str);
                        unfinished_tasks_present = 1;
                    }
                } else {
                    pvm_upkdouble(&exec_time, 1, 1);
                    // Check if task was killed or completed
                    if (status == ST_TASK_KILLED) {
                        // no retry if task was killed (was killed for a
                        // reason!)
                        fprintf(stderr,
                                "%-20s - Task %4d was stopped or killed "
                                "after %14.9G seconds\n",
                                "[ERROR]", taskNumber, exec_time);
                        addUnfinishedTask(inp_dataFile, taskNumber, aux_str);
                        unfinished_tasks_present = 1;
                    } else {
                        printf("%-20s - Task %4d completed in %14.9G seconds\n",
                               "[TASK COMPLETED]", taskNumber, exec_time);
                        if (arguments.create_slave) {
                            fprintf(nodeInfoFile, "%2d,%4d\n", itid,
                                    taskNumber);
                        }
                    }
                    total_time += exec_time;
                    runningTasks--;
                }
            }
        } else {
            // wait for a job result
            if (pvm_recv(-1, MSG_RESULT)) {
                pvm_upkint(&itid, 1, 1);
                pvm_upkint(&taskNumber, 1, 1);
                pvm_upkint(&tries, 1, 1);
                pvm_upkint(&status, 1, 1);
                pvm_upkstr(aux_str);
                // Check if response is error at forking
                if (status == ST_MEM_ERR) {
                    fprintf(stderr,
                            "%-20s - Could not execute task %d in slave %d "
                            "(out of memory)\n",
                            "[ERROR]", taskNumber, itid);
                    if (tries < MAX_TASK_TRIES)
                        addTask(&currentTask, taskNumber, aux_str, tries);
                    else {
                        addUnfinishedTask(inp_dataFile, taskNumber, aux_str);
                        unfinished_tasks_present = 1;
                    }
                } else if (status == ST_FORK_ERR) {
                    fprintf(stderr,
                            "%-20s - Could not fork process for task "
                            "%d in slave %d\n",
                            "[ERROR]", taskNumber, itid);
                    if (tries < MAX_TASK_TRIES)
                        addTask(&currentTask, taskNumber, aux_str, tries);
                    else {
                        addUnfinishedTask(inp_dataFile, taskNumber, aux_str);
                        unfinished_tasks_present = 1;
                    }
                } else {
                    pvm_upkdouble(&exec_time, 1, 1);
                    // Check if task was killed or completed
                    if (status == ST_TASK_KILLED) {
                        // no retry if task was killed (was killed for a
                        // reason!)
                        fprintf(stderr,
                                "%-20s - Task %4d was stopped or killed "
                                "after %14.9G seconds\n",
                                "[ERROR]", taskNumber, exec_time);
                        addUnfinishedTask(inp_dataFile, taskNumber, aux_str);
                        unfinished_tasks_present = 1;
                    } else {
                        printf("%-20s - Task %4d completed in %14.9G seconds\n",
                               "[TASK COMPLETED]", taskNumber, exec_time);
                        if (arguments.create_slave) {
                            fprintf(nodeInfoFile, "%2d,%4d\n", itid,
                                    taskNumber);
                        }
                    }
                    total_time += exec_time;
                    runningTasks--;
                }
            }
        }
    }
    printf("%-20s - End of work, all unfinished jobs (if any) have been "
           "recorded\n\n",
           "[INFO]");

    // Shut down all the slaves
    printf("== SHUTTING DOWN ALL SLAVES ==\n");
    work_code = MSG_STOP;
    for (i = 0; i < maxConcurrentTasks; i++) {
        pvm_initsend(PVM_ENCODING);
        pvm_pkint(&work_code, 1, 1);
        pvm_send(slaveId[i], MSG_STOP);
        printf("%-20s - Shutting down slave %2d\n", "[INFO]", i);
    }
    printf("%-20s - All slaves have been successfully dismantled\n\n",
           "[INFO]");

    // Final message
    clock_gettime(CLOCK_REALTIME, &tspec_after);
    timespec_subtract(&tspec_result, &tspec_after, &tspec_before);

    printf("== END OF EXECUTION ==\n"
           " - Combined computing time: %14.5G seconds.\n"
           " - Total execution time:    %6ld.%9ld seconds.\n\n",
           total_time, (long int)tspec_result.tv_sec, tspec_result.tv_nsec);

    /*
     * CLEANUP
     */
    // free memory
    printf("%-20s - Freeing used memory\n", "[CLEANUP]");
    while (currentTask != NULL)
        removeTask(&currentTask);
    free(nodes);
    free(nodeCores);
    // close files
    fclose(f_out);
    if (arguments.create_slave)
        fclose(nodeInfoFile);
    // remove tmp program (if modified)
    if (arguments.maple_single_cpu) {
        printf("%-20s - Removing temporary Maple program\n", "[CLEANUP]");
        sprintf(aux_str, "[ ! -f %s.bak ] || mv %s.bak %s", inp_programFile,
                inp_programFile, inp_programFile);
        if (system(aux_str))
            fprintf(stderr,
                    "%-20s - Could not clean up Maple single CPU aux scripts\n",
                    "[ERROR]");
    }
    // remove tmp pari/sage/octave programs (if created)
    if (task_type == 3 || task_type == 4 || task_type == 5) {
        printf("%-20s - Removing PARI/Sage/Octave aux programs\n", "[CLEANUP]");
        DIR *dir;
        struct dirent *ent;
        dir = opendir(out_dir);
        while ((ent = readdir(dir))) {
            if (strstr(ent->d_name, "auxprog") != NULL) {
                sprintf(aux_str, "%s/%s", out_dir, ent->d_name);
                remove(aux_str);
            }
        }
        closedir(dir);
    }
    // warn that there are unfinished tasks
    if (unfinished_tasks_present == 1) {
        printf("%-20s - Unfinished tasks present, run the following "
               "command if you want to finish the execution:\n\n",
               "[WARNING]");
        for (i = 0; i < argc; i++) {
            if (strcmp(argv[i], inp_dataFile) == 0) {
                printf("unfinished_%s ", argv[i]);
            } else {
                printf("%s ", argv[i]);
            }
        }
        printf("\n\n");
    }

    pvm_catchout(0);
    pvm_halt();

    return 0;
}
