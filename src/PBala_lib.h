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

#ifndef PBALA_LIB_H
#define PBALA_LIB_H
/*! \file PBala_lib.h
 * \brief Header for antz_lib.c
 * \author Oscar Saleta Reig
 */

#include "stdio.h"
#include <sys/resource.h>
#include <sys/time.h>

#define PVM_ENCODING PvmDataRaw ///< Little Endian encoding
#define MAX_NODE_LENGTH 6       ///< Max length of node names (a0X)
#define MAX_TASK_TRIES 3        ///< Max number of tries per task
#define FNAME_SIZE 150          ///< Max length of filename (including path)
#define BUFFER_SIZE 2048        ///< Max size of buffer for reading files
#define MSG_GREETING 1          ///< Flag for initializing task
#define MSG_WORK 2              ///< Flag for telling task to do work
#define MSG_RESULT 3 ///< Flag that indicates that message contains results
#define MSG_STOP 4   ///< Flag for stopping a task
/**
 * Flag that indicates that message is a ready sign for master
 */
#define MSG_READY 5

typedef struct task_ {
    char args[BUFFER_SIZE];
    int number;
    int tries;
    struct task_ *next;
} task, *task_ptr;

/**
 * Prepare Maple scripts for single CPU executions
 *
 * @param fname name of input file
 * @return      0 if successful
 */
int mapleSingleCPU(char *fname);
/**
 * Count how many lines a textfile has
 *
 * @param fname name of file to be counted
 * @return      int lineCount, -1 if an error occurred
 */
int getLineCount(char *fname);
/**
 * Parse the node file and return an array of strings containing the names
 * of the nodes, and an array of ints containing their number of CPUs
 *
 * @param nodefile  name of node file
 * @param nodes     pointer to array of strings of node names
 * @param nodeCores pointer to array of ints of CPUs per node
 * @return          number of nodes, -1 if error
 */
int parseNodeFile(char *nodefile, char ***nodes, int **nodeCores);
/**
 * Memory check for tasks. If no guess is given, limit to 25% of max RAM
 *
 * @param flag          Tells the function if there is a guess by the user
 * @param max_task_size If flag!=0 use this as constraint to memory
 * @return              0 if there is enough memory, 1 if there is not enough
 *                      memory, -1 if an error occurred
 */
int memcheck(int flag, long int max_task_size);
/**
 * Subtract the two timespec structs
 *
 * @param result struct timespec where the substraction is stored
 * @param x      struct timespec X
 * @param y      struct timespec Y
 * @return       1 if the difference is negative, otherwise 0
 */
int timespec_subtract(struct timespec *result, struct timespec *x,
                      struct timespec *y);
/**
 * Print usage information to output file
 *
 * @param pid        Proccess identifier (within system)
 * @param taskNumber Task identifier (within our program)
 * @param usage      Struct that contains all the resource usage information
 * @param out_dir    Output file name
 * @return           0 if successful
 */
int prtusage(int pid, int taskNumber, char *out_dir, struct rusage usage);
/**
 * Creates an auxiliary program for PARI execution
 *
 * @param taskId      Task number
 * @param args        Arguments string separated by commas
 * @param programfile path to PARI script
 * @param directory   path to results directory
 * @return            0 if successful, -1 if error occurred
 */
int parifile(int taskId, char *args, char *programfile, char *directory);
/**
 * Creates an auxiliary program for Sage execution
 *
 * @param taskId      Task number
 * @param args        Arguments string separated by commas
 * @param programfile path to script
 * @param directory   path to results directory
 * @return            0 if successful
 */
int sagefile(int taskId, char *args, char *programfile, char *directory);
/**
 * Creates an auxiliary program for Octave execution
 *
 * @param taskId      Task number
 * @param args        Arguments string separated by commas
 * @param programfile Path to script
 * @param directory   Path to the results directory
 * @return            0 if successful
 */
int octavefile(int taskId, char *args, char *programfile, char *directory);
/**
 * Informs that a process has been killed or stopped
 * this function runs instead of prtusage()
 *
 * @param pid        Proccess identifier (within system)
 * @param taskNumber Task identifier (within our program)
 * @param out_dir    Output file name
 * @return           0 if successful, -1 if file error
 */
int prterror(int pid, int taskNumber, char *out_dir, double time);
/**
 * Kill every PBala related process from every antz node
 * @return 0
 */
int killPBala(void);
/**
 * Generate system command for Maple execution and run it
 *
 * @param  taskNumber task number
 * @param  inputFile  name of input file
 * @param  arguments  string of comma separated arguments
 * @param  customPath custom path to executable (if applies)
 * @return            output of execvp of the generated string
 */
int mapleProcess(int taskNumber, char *inputFile, char *arguments,
                 char *customPath);
/**
 * Generate system command for C execution and run it
 *
 * @param  taskNumber task number
 * @param  inputFile  name of input file
 * @param  arguments  string of comma separated arguments
 * @param  customPath custom path to executable (if applies)
 * @return            output of execvp of the generated string
 */
int cProcess(int taskNumber, char *inputFile, char *arguments,
             char *customPath);
/**
 * Generate system command for Python execution and run it
 *
 * @param  taskNumber task number
 * @param  inputFile  name of input file
 * @param  arguments  string of comma separated arguments
 * @param  customPath custom path to executable (if applies)
 * @return            output of execvp of the generated string
 */
int pythonProcess(int taskNumber, char *inputFile, char *arguments,
                  char *customPath);
/**
 * Generate system command for PARI/GP execution and run it
 *
 * It also creates the auxiliary PARI scripts
 *
 * @param  taskNumber task number
 * @param  outdir     output directory name
 * @param  customPath custom path to executable (if applies)
 * @return            output of execvp of the generated string
 */
int pariProcess(int taskNumber, char *outdir, char *customPath);
/**
 * Generate system command for Sage execution and run it
 *
 * It also creates the auxiliary Sage scripts
 *
 * @param  taskNumber task number
 * @param  outdir     output directory name
 * @param  customPath custom path to executable (if applies)
 * @return            output of execvp of the generated string
 */
int sageProcess(int taskNumber, char *outdir, char *customPath);
/**
 * Generate system command for Octave execution and run it
 *
 * It also creates the auxiliary Octave scripts
 *
 * @param  taskNumber task number
 * @param  outdir     output directory name
 * @param  customPath custom path to executable (if applies)
 * @return            output of execvp of the generated string
 */
int octaveProcess(int taskNumber, char *outdir, char *customPath);
/**
 * Get tasks from file and create a linked list
 *
 * We avoid additional file I/O by storing all the tasks in a linked list. This
 * also enables us to remove (pop) or add (push) tasks on runtime, in case tasks
 * are completed or fail.
 *
 * @param  filename    name of input data file
 * @param  currentTask pointer to the head of the task linked list
 * @return             number of tasks recorded
 */
int getDataFromFile(char *filename, task_ptr *currentTask);
/**
 * Print an abort message in case a fatal error occurred
 */
void printAbort(void);
/**
 * Add (push) task to the linked list
 *
 * The task added is the new current task
 *
 * @param currentTask pointer to head of linked list
 * @param tasknumber  task number
 * @param taskargs    string of comma separated arguments
 * @param tries       number of tries done for this task
 */
void addTask(task_ptr *currentTask, int tasknumber, char *taskargs, int tries);
/**
 * Pop a task from the linked list
 *
 * The new current task is currentTask->next
 *
 * @param currentTask pointer to head of linked list
 */
void removeTask(task_ptr *currentTask);
/**
 * Create a new task
 *
 * Allocate memory for a new task, and assign tasknumber, args and next task
 * pointer (next can be NULL)
 *
 * @param  number task number
 * @param  args   task arguments
 * @param  tries  number of tries performed for this task
 * @param  next   next task in linked list
 * @return        the created task
 */
task_ptr newTask(int number, char *args, int tries, task_ptr next);
/**
 * Print all tasks (for debugging purposes)
 *
 * Goes through the whole linked list and prints the task number and args of
 * each element
 *
 * @param currentTask current task head
 */
void printTasks(task_ptr currentTask);
/**
 * Write an unfinished task to a file
 *
 * If execution of a task failed and cannot be retried, we write the task in the
 * same format as the input file in a separate file, so the user can later
 * execute PBala with that file as input to finish all computations. The name of
 * the file is the name of the input filename with the suffix "unfinished_"
 *
 * @param fname      input file name
 * @param tasknumber task number
 * @param args       task arguments
 */
void addUnfinishedTask(char *fname, int tasknumber, char *args);

#endif /* PBALA_LIB_H */
