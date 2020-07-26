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

#include "PBala_lib.h"
#include "PBala_errcodes.h"

#include <fcntl.h>
#include <pvm3.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int mapleSingleCPU(char *fname) {
    char aux_str[BUFFER_SIZE];
    sprintf(aux_str,
            "grep -q -F 'kernelopts(numcpus=1)' %s || (sed "
            "'1ikernelopts(numcpus=1);' %s > %s_tmp && mv %s "
            "%s.bak && mv %s_tmp %s)",
            fname, fname, fname, fname, fname, fname, fname);
    if (system(aux_str) == -1)
        return -1;
    return 0;
}

int getLineCount(char *fileName) {
    FILE *f_aux;
    char str_tmp[BUFFER_SIZE];
    // char *str_aux;
    int lineCount;

    sprintf(str_tmp, "wc -l %s", fileName);
    f_aux = popen(str_tmp, "r");
    if (f_aux == NULL)
        return -1;
    if (fgets(str_tmp, 1024, f_aux) == NULL)
        return -1;
    sscanf(str_tmp, "%d", &lineCount);
    fclose(f_aux);

    return lineCount;
}

int parseNodeFile(char *nodefile, char ***nodes, int **nodeCores) {
    int i, nNodes;
    FILE *f_nodes;
    // count number of nodes
    if ((nNodes = getLineCount(nodefile)) < 0) {
        fprintf(stderr, "%-20s - cannot open file %s\n", "[ERROR]", nodefile);
        return -1;
    }
    // Open node file
    f_nodes = fopen(nodefile, "r");
    if (f_nodes == NULL) {
        fprintf(stderr, "%-20s - cannot open file %s\n", "[ERROR]", nodefile);
        return -1;
    }
    // Allocate memory for node and cpu array
    *nodeCores = (int *)malloc(nNodes * sizeof(int));
    *nodes = (char **)malloc(nNodes * sizeof(char *));
    for (i = 0; i < nNodes; i++)
        (*nodes)[i] = (char *)malloc(MAX_NODE_LENGTH * sizeof(char));
    // Read node file and store the info
    for (i = 0; i < nNodes; i++) {
        if (fscanf(f_nodes, "%s %d", (*nodes)[i], &(*nodeCores)[i]) != 2) {
            fprintf(stderr, "%-20s - while reading node file %s\n", "[ERROR]",
                    nodefile);
            return -1;
        }
    }
    fclose(f_nodes);
    return nNodes;
}

int memcheck(int flag, long int max_task_size) {
    FILE *f;
    char buffer[1024];
    char *token;

    long int maxmem, freemem;

    f = fopen("/proc/meminfo", "r");
    if (f == NULL)
        return -1;
    if (fgets(buffer, 1024, f) == NULL) // reads total memory
        return -1;
    token = strtok(buffer, " ");
    token = strtok(NULL, " ");
    if (sscanf(token, "%ld", &maxmem) != 1)
        return -1;
    if (fgets(buffer, 1024, f) == NULL) // reads free memory
        return -1;
    token = strtok(buffer, " ");
    token = strtok(NULL, " ");
    if (sscanf(token, "%ld", &freemem) != 1)
        return -1;
    fclose(f);
    if (flag == 0) {
        if (freemem < 0.15 * maxmem)
            return 1;
    } else {
        if (freemem < max_task_size)
            return 1;
    }
    return 0;
}

int timespec_subtract(struct timespec *result, struct timespec *x,
                      struct timespec *y) {
    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_nsec < y->tv_nsec) {
        int nsec = (y->tv_nsec - x->tv_nsec) / 1000000000 + 1;
        y->tv_nsec -= 1000000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_nsec - y->tv_nsec > 1000000000) {
        int nsec = (x->tv_nsec - y->tv_nsec) / 1000000000;
        y->tv_nsec += 1000000000 * nsec;
        y->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
       tv_nsec is certainly positive. */
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_nsec = x->tv_nsec - y->tv_nsec;

    /* Return 1 if result is negative. */
    return x->tv_sec < y->tv_sec;
}

int prtusage(int pid, int taskNumber, char *out_dir, struct rusage usage) {
    FILE *memlog;
    char memlogfilename[FNAME_SIZE];

    sprintf(memlogfilename, "%s/task%d_mem.txt", out_dir, taskNumber);
    memlog = fopen(memlogfilename, "w");
    fprintf(memlog, "TASK %d RESOURCE USAGE (PID %d)\n", taskNumber, pid);
    fprintf(memlog, "----------------------\n");
    fprintf(memlog, "User CPU time used:               %20.10g\n",
            usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6);
    fprintf(memlog, "System CPU time used:             %20.10g\n",
            usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6);
    fprintf(memlog, "Maximum resident set size (KB):   %20ld\n",
            usage.ru_maxrss);
    fprintf(memlog, "Integral shared memory size (KB): %20ld\n",
            usage.ru_ixrss);
    fprintf(memlog, "Integral unshared data size (KB)  %20ld\n",
            usage.ru_idrss);
    fprintf(memlog, "Integral shared stack size (KB):  %20ld\n",
            usage.ru_isrss);
    fprintf(memlog, "Page reclaims (soft page faults): %20ld\n",
            usage.ru_minflt);
    fprintf(memlog, "Page faults (hard page faults):   %20ld\n",
            usage.ru_majflt);
    fprintf(memlog, "Swaps:                            %20ld\n",
            usage.ru_nswap);
    fprintf(memlog, "Block input operations:           %20ld\n",
            usage.ru_inblock);
    fprintf(memlog, "block output operations:          %20ld\n",
            usage.ru_oublock);
    fprintf(memlog, "IPC messages sent:                %20ld\n",
            usage.ru_msgsnd);
    fprintf(memlog, "IPC messages received:            %20ld\n",
            usage.ru_msgrcv);
    fprintf(memlog, "Signals received:                 %20ld\n",
            usage.ru_nsignals);
    fprintf(memlog, "Voluntary context switches:       %20ld\n",
            usage.ru_nvcsw);
    fprintf(memlog, "Involuntary context switches:     %20ld\n",
            usage.ru_nivcsw);
    fclose(memlog);

    return 0;
}

int parifile(int taskId, char *args, char *programfile, char *directory) {
    FILE *f;
    char aux[FNAME_SIZE];

    sprintf(aux, "%s/auxprog-%d.gp", directory, taskId);
    f = fopen(aux, "w");
    if (f == NULL)
        return -1;
    fprintf(f, "taskId=%d;\n", taskId);
    fprintf(f, "taskArgs=[%s];\n", args);
    fprintf(f, "\\r %s\n", programfile);
    fprintf(f, "\\q\n");

    fclose(f);
    return 0;
}

int sagefile(int taskId, char *args, char *programfile, char *directory) {
    FILE *f;
    char aux[FNAME_SIZE];

    sprintf(aux, "%s/auxprog-%d.sage", directory, taskId);
    f = fopen(aux, "w");
    if (f == NULL)
        return -1;
    fprintf(f, "taskId=%d;\n", taskId);
    fprintf(f, "taskArgs=[%s];\n", args);
    fprintf(f, "load('%s')\n", programfile);

    fclose(f);
    return 0;
}

int octavefile(int taskId, char *args, char *programfile, char *directory) {
    FILE *f;
    char aux[FNAME_SIZE];

    sprintf(aux, "%s/auxprog-%d.m", directory, taskId);
    f = fopen(aux, "w");
    if (f == NULL)
        return -1;
    fprintf(f, "#! /usr/bin/octave -qf\n");
    fprintf(f, "taskId = %d;\n", taskId);
    fprintf(f, "taskArgs = {%s};\n", args);
    fprintf(f, "source (\"%s\");\n", programfile);

    fclose(f);
    return 0;
}

int prterror(int pid, int taskNumber, char *out_dir, double time) {
    FILE *memlog;
    char memlogfname[FNAME_SIZE];

    sprintf(memlogfname, "%s/task%d_killed.log", out_dir, taskNumber);
    memlog = fopen(memlogfname, "w");
    if (memlog == NULL)
        return -1;
    fprintf(memlog, "TASK %d (PID %d) ERROR REPORT\n", taskNumber, pid);
    fprintf(memlog, "----------------------\n");
    fprintf(memlog, "Task was killed or stopped after %10.5G seconds.\n", time);

    fclose(memlog);
    return 0;
}

#define NNODES 8
int killPBala(void) {
    char nodes[NNODES][4] = {"a01", "a02", "a03", "a04",
                             "a05", "a06", "a07", "a08"};

    char processes[7][15] = {"PBala_task", "pvmd", "mserver", "python",
                             "gp",         "sage", "bash"};

    int i, j;
    char buffer[300];
    for (i = 0; i < NNODES; i++) {
        fprintf(stdout, "\n=== Cleaning PBala related processes from %s...\n",
                nodes[i]);
        /* kill processes from the list in every node */
        for (j = 0; j < 7; j++) {
            sprintf(buffer, "ssh %s killall %s", nodes[i], processes[j]);
            system(buffer);
        }
        /* remove pvmd.uid files from /tmp */
        sprintf(buffer, "ssh %s rm -f /tmp/pvm*", nodes[i]);
        system(buffer);
    }

    return 0;
}
#undef NNODES

int mapleProcess(int taskNumber, char *inputFile, char *arguments,
                 char *customPath) {
    int i; // for loops
    // NULL-terminated array of strings for calling the Maple script
    char **args;
    // 0: maple, 1: taskid, 2: X, 3: input, 4: NULL
    int nargs = 4;
    args = (char **)malloc((nargs + 1) * sizeof(char *));
    // Do not malloc for NULL
    for (i = 0; i < nargs; i++)
        args[i] = malloc(BUFFER_SIZE);
    // Fill up the array with strings
    if (customPath != NULL)
        sprintf(args[0], "%s", customPath);
    else
        sprintf(args[0], "maple");
    sprintf(args[1], "-tc \"taskId:=%d\"", taskNumber);
    sprintf(args[2], "-c \"taskArgs:=[%s]\"", arguments);
    sprintf(args[3], "%s", inputFile);
    args[4] = NULL;

    return execvp(args[0], args);
}

int cProcess(int taskNumber, char *inputFile, char *arguments,
             char *customPath) {
    int i; // for loops
    // Preparations for C or Python execution
    char *arguments_cpy;
    arguments_cpy = malloc(strlen(arguments));
    strcpy(arguments_cpy, arguments);
    // This counts how many commas there are in arguments, giving
    // the number
    // of arguments passed to the program
    for (i = 0; arguments_cpy[i];
         arguments_cpy[i] == ',' ? i++ : *arguments_cpy++)
        ;
    int nargs = i + 1; // i = number of commas
    int nargs_tot;     // will be the total number of arguments
                       // (nargs+program name+etc)
    char *token;       // used for tokenizing arguments
    char **args;       // this is the NULL-terminated array of strings

    /* In this case it's necessary to parse the arguments string
     * breaking it into tokens and then arranging the args of the
     * system call in a NULL-terminated array of strings which we
     * pass to execvp
     */
    // Tokenizing breaks the original string so we make a copy
    strcpy(arguments_cpy, arguments);
    // Args in system call are (program tasknum arguments), so
    // 2+nargs
    nargs_tot = 2 + nargs;
    args = (char **)malloc((nargs_tot + 1) * sizeof(char *));
    args[nargs_tot] = NULL; // NULL-termination of args
    for (i = 0; i < nargs_tot; i++)
        args[i] = malloc(BUFFER_SIZE);
    // Two first command line arguments
    // strcpy(args[0],inp_programFile);
    sprintf(args[0], "./%s", inputFile);
    sprintf(args[1], "%d", taskNumber);
    // Tokenize arguments_cpy
    token = strtok(arguments_cpy, ",");
    // Copy the token to its place in args
    for (i = 2; i < nargs_tot; i++) {
        strcpy(args[i], token);
        token = strtok(NULL, ",");
    }
    return execvp(args[0], args);
}

int pythonProcess(int taskNumber, char *inputFile, char *arguments,
                  char *customPath) {
    int i; // for loops
    /* Preparations for C or Python execution */
    char *arguments_cpy;
    arguments_cpy = malloc(strlen(arguments));
    strcpy(arguments_cpy, arguments);
    // This counts how many commas there are in arguments, giving
    // the number
    // of arguments passed to the program
    for (i = 0; arguments_cpy[i];
         arguments_cpy[i] == ',' ? i++ : *arguments_cpy++)
        ;
    int nargs = i + 1; // i = number of commas
    int nargs_tot;     // will be the total number of arguments
                       // (nargs+program name+etc)
    char *token;       // used for tokenizing arguments
    char **args;       // this is the NULL-terminated array of strings

    // Same as in C, but adding "python" as first argument
    // Tokenizing breaks the original string so we make a copy
    strcpy(arguments_cpy, arguments);
    // args: (0)-python (1)-program (2)-tasknumber
    // (3..nargs+3)-arguments
    nargs_tot = 3 + nargs;
    args = (char **)malloc((nargs_tot + 1) * sizeof(char *));
    args[nargs_tot] = NULL; // NULL-termination of args
    for (i = 0; i < nargs_tot; i++)
        args[i] = malloc(BUFFER_SIZE);
    // Two first command line arguments
    strcpy(args[0], "python");
    strcpy(args[1], inputFile);
    sprintf(args[2], "%d", taskNumber);
    // Tokenize arguments_cpy
    token = strtok(arguments_cpy, ",");
    // Copy the token to its place in args
    for (i = 3; i < nargs_tot; i++) {
        strcpy(args[i], token);
        token = strtok(NULL, ",");
    }
    return execvp(args[0], args);
}

int pariProcess(int taskNumber, char *outdir, char *customPath) {
    int i;
    // NULL-terminated array of strings
    char **args;
    char filename[FNAME_SIZE];
    sprintf(filename, "%s/auxprog-%d.gp", outdir, taskNumber);
    // 0: gp, 1: -f, 2: -s400G, 3: file, 4: NULL
    int nargs = 4;
    args = (char **)malloc((nargs + 1) * sizeof(char *));
    // Do not malloc for NULL
    for (i = 0; i < nargs; i++)
        args[i] = malloc(BUFFER_SIZE);
    // Fill up the array with strings
    sprintf(args[0], "gp");
    sprintf(args[1], "-f");
    sprintf(args[2], "-s400G");
    sprintf(args[3], "%s", filename);
    args[4] = NULL;

    return execvp(args[0], args);
}

int sageProcess(int taskNumber, char *outdir, char *customPath) {
    int i;
    // NULL-terminated array of strings
    char **args;
    char filename[FNAME_SIZE];
    sprintf(filename, "%s/auxprog-%d.sage", outdir, taskNumber);
    // 0: sage, 1: file, 2: NULL
    int nargs = 2;
    args = (char **)malloc((nargs + 1) * sizeof(char *));
    // Do not malloc for NULL
    for (i = 0; i < nargs; i++)
        args[i] = malloc(BUFFER_SIZE);
    // Fill up the array with strings
    sprintf(args[0], "sage");
    sprintf(args[1], "%s", filename);
    args[2] = NULL;

    return execvp(args[0], args);
}

int octaveProcess(int taskNumber, char *outdir, char *customPath) {
    int i;
    // NULL-terminated array of strings
    char **args;
    char filename[FNAME_SIZE];
    sprintf(filename, "%s/auxprog-%d.m", outdir, taskNumber);
    // 0: octave, 1: -qf, 2: file, 3: NULL
    int nargs = 3;
    args = (char **)malloc((nargs + 1) * sizeof(char *));
    // Do not malloc for NULL
    for (i = 0; i < nargs; i++)
        args[i] = malloc(BUFFER_SIZE);
    // Fill up the array with strings
    sprintf(args[0], "octave");
    sprintf(args[1], "-qf");
    sprintf(args[2], "%s", filename);
    args[3] = NULL;

    return execvp(args[0], args);
}

int getDataFromFile(char *filename, task_ptr *currentTask) {
    int i, nTasks;
    FILE *f;
    int tasknumber;
    char arguments[BUFFER_SIZE];

    // get argument count
    if ((nTasks = getLineCount(filename)) < 0)
        return -1;

    // fill arguments array with data from file
    f = fopen(filename, "r");
    if (f == NULL) {
        fprintf(stderr, "%-20s - cannot open file %s\n", "[ERROR]", filename);
        return -1;
    }

    for (i = 0; i < nTasks; i++) {
        if (fscanf(f, "%d,%s\n", &tasknumber, arguments) != 2) {
            fprintf(stderr, "%-20s - cannot read line %d in file %s\n",
                    "[ERROR]", i, filename);
            return -1;
        }
        addTask(currentTask, tasknumber, arguments, 0);
    }

    fclose(f);
    return nTasks;
}

void printAbort(void) { fprintf(stderr, "\n== EXECUTION ABORTED ==\n"); }

task_ptr newTask(int number, char *args, int tries, task_ptr next) {
    task_ptr new_task = (task_ptr)malloc(sizeof(task));
    strcpy(new_task->args, args);
    new_task->number = number;
    new_task->next = next;
    new_task->tries = tries;
    return new_task;
}

void addTask(task_ptr *currentTask, int tasknumber, char *taskargs, int tries) {
    task_ptr new_task = newTask(tasknumber, taskargs, tries, *currentTask);
    *currentTask = new_task;
}

void removeTask(task_ptr *currentTask) {
    if (*currentTask == NULL)
        return;
    task_ptr aux = *currentTask;
    *currentTask = (*currentTask)->next;
    free(aux);
}

void printTasks(task_ptr currentTask) {
    task_ptr t = currentTask;
    fprintf(stdout, "\nPRINTING TASKS\n");

    while (t != NULL) {
        fprintf(stdout, "task:%d,%s\n", t->number, t->args);
        t = t->next;
    }
}

void addUnfinishedTask(char *fname, int tasknumber, char *args) {
    char fname2[FNAME_SIZE];
    int fd;
    FILE *f;
    sprintf(fname2, "unfinished_%s", fname);
    fd = open(fname2, O_WRONLY | O_APPEND | O_CREAT, 0644);
    f = fdopen(fd, "a");
    fprintf(f, "%d,%s\n", tasknumber, args);
    fclose(f);
    close(fd);
}