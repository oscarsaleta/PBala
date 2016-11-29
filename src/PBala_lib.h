/* Job parallelizer in PVM for SPMD executions in computing cluster
 * URL: https://github.com/oscarsaleta/PVMantz
 *
 * Copyright (C) 2016  Oscar Saleta Reig
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PBALA_LIB_H
#define PBALA_LIB_H
/*! \file PBala_lib.h 
 * \brief Header for antz_lib.c
 * \author Oscar Saleta Reig
 */

#include <sys/resource.h>

#define PVM_ENCODING PvmDataRaw ///< Little Endian encoding
#define MAX_NODE_LENGTH 6 ///< Max length of node names (a0X)
#define FNAME_SIZE 150 ///< Max length of filename (including path)
#define BUFFER_SIZE 2048 ///< Max size of buffer for reading files
#define MSG_GREETING 1 ///< Flag for initializing task
#define MSG_WORK 2 ///< Flag for telling task to do work
#define MSG_RESULT 3 ///< Flag that indicates that message contains results
#define MSG_STOP 4 ///< Flag for stopping a task


int getLineCount(char *inp_dataFile);
int parseNodefile(char *nodefile, int nNodes, char ***nodes, int **nodeCores);
int memcheck(int flag, long int max_task_size);
int prtusage(int pid, int taskNumber, char *out_dir, struct rusage usage);
int parifile(int taskId, char *args, char *programfile, char *directory);
int sagefile(int taskId, char *args, char *programfile, char *directory);
int prterror (int pid, int taskNumber, char *out_dir, double time);

#endif /* PBALA_LIB_H */
