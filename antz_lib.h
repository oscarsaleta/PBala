/*! \file antz_lib.h 
 * \brief Header for antz_lib.c
 * \author Oscar Saleta Reig
 */

#define PVM_ENCODING PvmDataRaw ///< Little Endian encoding
#define MAX_NODE_LENGTH 6 ///< Max length of node names (a0X)
#define FNAME_SIZE 150 ///< Max length of filename (including path)
#define BUFFER_SIZE 2048 ///< Max size of buffer for reading files
#define MSG_GREETING 1 ///< Flag for initializing task
#define MSG_WORK 2 ///< Flag for telling task to do work
#define MSG_RESULT 3 ///< Flag that indicates that message contains results
#define MSG_STOP 4 ///< Flag for stopping a task

int getLineCount(char *prName, char *inp_dataFile);
int memcheck(int flag, long int max_task_size);
int prtusage(int pid, int taskNumber, char *out_dir, struct rusage usage);
void parifile(int taskId, char *args, char *programfile, char *directory);
