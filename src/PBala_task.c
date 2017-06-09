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

/*! \file PBala_task.c
 * \brief PVM task. Adapts to different programs, forks execution to track mem
 * usage
 * \author Oscar Saleta Reig
 */
#include "PBala_config.h"
#include "PBala_errcodes.h"
#include "PBala_lib.h"

#include <fcntl.h>
#include <pvm3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * Main task function.
 *
 * \return 0 if successful
 */
int main(int argc, char *argv[])
{
    int myparent, taskNumber, tries; // myparent is the master
    int me;                          // me is the PVM id of this child
    int work_code; // work_code is a flag that tells the child what to do
    char inp_programFile[FNAME_SIZE]; // name of the program
    char out_dir[FNAME_SIZE];         // name of output directory
    char arguments[BUFFER_SIZE]; // string of arguments as read from data file
    int task_type;               // 0:maple, 1:C, 2:python
    long int max_task_size; // if given, max size in KB of a spawned process
    int flag_err;           // 0=no err files, 1=yes err files
    int flag_mem;           // 0=no mem files, 1=yes mem files
    int flag_custom_path;   // 0=no custom path, 1=custom path provided
    char custom_path[BUFFER_SIZE];
    char *custom_path_ptr = NULL;
    struct timespec tspec_before, tspec_after, tspec_result;
    double difft, totalt = 0;
    long int sec, nsec;
    int state;
    int mcheck; // error status of memory file check

    myparent = pvm_parent();

    // Be greeted by master
    pvm_recv(myparent, MSG_GREETING);
    pvm_upkint(&me, 1, 1);
    pvm_upkint(&task_type, 1, 1);
    pvm_upklong(&max_task_size, 1, 1);
    pvm_upkint(&flag_err, 1, 1);
    pvm_upkint(&flag_mem, 1, 1);
    pvm_upkint(&flag_custom_path, 1, 1);
    if (flag_custom_path) {
        pvm_upkstr(custom_path);
        custom_path_ptr = &custom_path[0];
    }

    /* Perform generic check or use specific size info?
     *  memcheck_flag = 0 means generic check
     *  memcheck_flag = 1 means specific info
     */
    int memcheck_flag = max_task_size > 0 ? 1 : 0;

    // Work work work work work
    while (1) {
        /* Race condition. Mitigated by executing few CPUs on each node
         * Explanation: 2 tasks could check memory simultaneously and
         *  both conclude that there is enough because they see the same
         *  output, but maybe there is not enough memory for 2 tasks.
         */
        if ((mcheck = memcheck(memcheck_flag, max_task_size)) != 0) {
            /* if memcheck fails, return to master to try another node */
            sleep(1);
            continue;
        }

        // send ready message
        pvm_initsend(PVM_ENCODING);
        pvm_pkint(&me, 1, 1);
        pvm_send(myparent, MSG_READY);

        // Receive inputs
        pvm_recv(myparent, MSG_WORK);
        pvm_upkint(&work_code, 1, 1);
        if (work_code == MSG_STOP) // if master tells task to shutdown
            break;
        pvm_upkint(&taskNumber, 1, 1);
        pvm_upkint(&tries, 1, 1);
        pvm_upkstr(inp_programFile);
        pvm_upkstr(out_dir);
        pvm_upkstr(arguments); // string of comma-separated arguments read from
                               // datafile
        tries++;
        clock_gettime(CLOCK_REALTIME, &tspec_before);

        /* Fork one process that will do the execution
         * the "parent task" will only wait for this process to end
         * and then report resource usage via getrusage()
         */
        pid_t pid = fork();
        // If fork fails, notify master and exit
        if (pid < 0) {
            fprintf(stderr,
                    "ERROR - task %d could not spawn execution process\n",
                    taskNumber);
            int state = ST_FORK_ERR;
            pvm_initsend(PVM_ENCODING);
            pvm_pkint(&me, 1, 1);
            pvm_pkint(&taskNumber, 1, 1);
            pvm_pkint(&tries, 1, 1);
            pvm_pkint(&state, 1, 1);
            pvm_pkstr(arguments);
            pvm_pkdouble(&totalt, 1, 1);
            pvm_send(myparent, MSG_RESULT);
            continue;
        }
        // Child code (work done here)
        if (pid == 0) {
            char output_file[BUFFER_SIZE];

            // Move stdout to taskNumber_out.txt
            sprintf(output_file, "%s/task%d_stdout.txt", out_dir, taskNumber);
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            dup2(fd, 1);
            close(fd);

            // Move stderr to taskNumber_err.txt
            if (flag_err) {
                sprintf(output_file, "%s/task%d_stderr.txt", out_dir,
                        taskNumber);
                fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                dup2(fd, 2);
                close(fd);
            }

            /*
             * GENERATE EXECUTION OF PROGRAM
             */
            if (task_type == 0) {
                /* MAPLE */
                mapleProcess(taskNumber, inp_programFile, arguments,
                             custom_path_ptr);
                perror("ERROR:: child Maple process");
            } else if (task_type == 1) {
                /* C */
                cProcess(taskNumber, inp_programFile, arguments,
                         custom_path_ptr);
                perror("ERROR:: child C process");
            } else if (task_type == 2) {
                /* PYTHON */
                pythonProcess(taskNumber, inp_programFile, arguments,
                              custom_path_ptr);
                perror("ERROR:: child Python process");
            } else if (task_type == 3) {
                /* PARI/GP */
                pariProcess(taskNumber, out_dir, custom_path_ptr);
                perror("ERROR:: child PARI process");
            } else if (task_type == 4) {
                /* SAGE */
                sageProcess(taskNumber, out_dir, custom_path_ptr);
                perror("ERROR:: child Sage process");
            } else if (task_type == 5) {
                /* OCTAVE */
                octaveProcess(taskNumber, out_dir, custom_path_ptr);
                perror("ERROR:: child Octave process");
            }
        }

        /* Attempt at measuring memory usage for the child process */
        // Stores information about the child execution
        siginfo_t infop;
        // Wait for the execution to end
        waitid(P_PID, pid, &infop, WEXITED);

        // Computation time
        clock_gettime(CLOCK_REALTIME, &tspec_after);
        timespec_subtract(&tspec_result, &tspec_after, &tspec_before);
        sec = (long int)tspec_result.tv_sec;
        nsec = tspec_result.tv_nsec;

        difft = sec + nsec * 1e-9;
        totalt += difft;
        if (infop.si_code == CLD_KILLED || infop.si_code == CLD_DUMPED) {
            prterror(pid, taskNumber, out_dir,
                     difft); // this could fail silently
            state = ST_TASK_KILLED;
        } else if (flag_mem) {
            struct rusage
                usage; // Stores information about the child's resource usage
            getrusage(RUSAGE_CHILDREN, &usage); // Get child resource usage
            prtusage(pid, taskNumber, out_dir,
                     usage); // Print resource usage to file
            state = 0;
        }

        // Send response to master
        pvm_initsend(PVM_ENCODING);
        pvm_pkint(&me, 1, 1);
        pvm_pkint(&taskNumber, 1, 1);
        pvm_pkint(&tries, 1, 1);
        pvm_pkint(&state, 1, 1);
        pvm_pkstr(arguments);
        pvm_pkdouble(&difft, 1, 1);
        pvm_pkdouble(&totalt, 1, 1);
        pvm_send(myparent, MSG_RESULT);
    }

    // Dismantle slave
    pvm_exit();
    exit(0);
}
