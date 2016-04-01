/*! \file antz_lib.c 
 * \brief File with generic functions and constant definitions
 * \author Oscar Saleta Reig
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <pvm3.h>
#include "antz_lib.h"

/**
 * Count how many lines a textfile has
 *
 * \param[in] *prName name of program that calls the function
 * \param[in] *fileName name of file to be counted
 *
 * \return int lineCount
 */
int getLineCount(char *prName, char *fileName) {
    FILE *f_aux;
    char str_tmp[BUFFER_SIZE];
    int lineCount;

    sprintf(str_tmp,"wc -l %s",fileName);
    f_aux = popen(str_tmp,"r");
    if (f_aux == NULL) {
        fprintf(stderr,"%s:: ERROR - cannot open file %s\n",prName,fileName);
        return 1;
    }
    fgets(str_tmp,1024,f_aux);
    sscanf(str_tmp,"%d",&lineCount);
    fclose(f_aux);

    return lineCount;
}


/**
 * Memory check for tasks. If no guess is given, limit to 25% of max RAM
 *
 * \param[in] flag Tells the function if there is a guess by the user
 * \param[in] max_task_size If flag!=0 use this as constraint to memory
 *
 * \returns 0 if there is enough memory, 1 if there is not enough memory
 */
int memcheck(int flag, long int max_task_size) {
    FILE *f;
    char buffer[1024];
    char *token;
    int maxmem,freemem;

    f = fopen("/proc/meminfo","r");
    fgets(buffer,1024,f); // reads total memory
    token = strtok(buffer," ");
    token = strtok(NULL," ");
    sscanf(token,"%d",&maxmem);
    fgets(buffer,1024,f); // reads free memory
    token = strtok(buffer," ");
    token = strtok(NULL," ");
    sscanf(token,"%d",&freemem);
    fclose(f);
    if (flag==0) {
        if (freemem < 0.25*maxmem)
            return 1;
    } else {
        if (freemem < max_task_size)
            return 1;
    }
    return 0;
}


/**
 * Print usage information to output file
 *
 * \param[in] pid Proccess identifier (within system)
 * \param[in] taskNumber Task identifier (within our program)
 * \param[in] usage Struct that contains all the resource usage information
 * \param[in] out_dir Output file name
 *
 * \returns 0 if successful
 */
int prtusage(int pid, int taskNumber, char *out_dir, struct rusage usage) {
    FILE *memlog;
    char memlogfilename[FNAME_SIZE];
    
    sprintf(memlogfilename,"%s/mem%d.log",out_dir,taskNumber);
    memlog = fopen(memlogfilename,"w");
    fprintf(memlog,"TASK %d RESOURCE USAGE (PID %d)\n",taskNumber,pid);
    fprintf(memlog,"----------------------\n");
    fprintf(memlog,"User CPU time used:               %20.10g\n",
                usage.ru_utime.tv_sec+usage.ru_utime.tv_usec/1e6);
    fprintf(memlog,"System CPU time used:             %20.10g\n",
                usage.ru_stime.tv_sec+usage.ru_stime.tv_usec/1e6);
    fprintf(memlog,"Maximum resident set size (KB):   %20ld\n",
                usage.ru_maxrss);
    fprintf(memlog,"Integral shared memory size (KB): %20ld\n",
                usage.ru_ixrss);
    fprintf(memlog,"Integral unshared data size (KB)  %20ld\n",
                usage.ru_idrss);
    fprintf(memlog,"Integral shared stack size (KB):  %20ld\n",
                usage.ru_isrss);
    fprintf(memlog,"Page reclaims (soft page faults): %20ld\n",
                usage.ru_minflt);
    fprintf(memlog,"Page faults (hard page faults):   %20ld\n",
                usage.ru_majflt);
    fprintf(memlog,"Swaps:                            %20ld\n",
                usage.ru_nswap);
    fprintf(memlog,"Block input operations:           %20ld\n",
                usage.ru_inblock);
    fprintf(memlog,"block output operations:          %20ld\n",
                usage.ru_oublock);
    fprintf(memlog,"IPC messages sent:                %20ld\n",
                usage.ru_msgsnd);
    fprintf(memlog,"IPC messages received:            %20ld\n",
                usage.ru_msgrcv);
    fprintf(memlog,"Signals received:                 %20ld\n",
                usage.ru_nsignals);
    fprintf(memlog,"Voluntary context switches:       %20ld\n",
                usage.ru_nvcsw);
    fprintf(memlog,"Involuntary context switches:     %20ld\n",
                usage.ru_nivcsw);
    fclose(memlog);

    return 0;
}
