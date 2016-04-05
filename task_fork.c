/*! \file task_fork.c 
 * \brief PVM task. Adapts to different programs, forks execution to track mem usage
 * \author Oscar Saleta Reig
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <pvm3.h>
#include "antz_lib.h"

/**
 * Main task function.
 *
 * \return 0 if successful
 */
int main(int argc, char *argv[]) {
    int myparent,taskNumber;
    int me,work_code;
    char inp_programFile[FNAME_SIZE];
    char out_dir[FNAME_SIZE];
    char arguments[BUFFER_SIZE];
    int task_type; //0:maple,1:C,2:python
    long int max_task_size;
    int i;

    myparent = pvm_parent();
    // Be greeted by master
    pvm_recv(myparent,MSG_GREETING);
    pvm_upkint(&me,1,1);
    pvm_upkint(&task_type,1,1);
    pvm_upklong(&max_task_size,1,1);

    /* Perform generic check or use specific size info?
     *  memcheck_flag = 0 means generic check
     *  memcheck_flag = 1 means specific info
     */
    int memcheck_flag = max_task_size > 0 ? 1 : 0;

    // Work work work
    while (1) {
        /* Race condition. Mitigated by executing few CPUs on each node
         * Explanation: 2 tasks could check memory simultaneously and
         *  both conclude that there is enough because they see the same
         *  output, but maybe there is not enough memory for 2 tasks.
         */
        if (memcheck(memcheck_flag,max_task_size) == 1) {
            sleep(60); // arbitrary number that could be much lower
            continue;
        }
        // Receive inputs
        pvm_recv(myparent,MSG_WORK);
        pvm_upkint(&work_code,1,1);
        if (work_code == MSG_STOP) // if master tells task to shutdown
            break;
        pvm_upkint(&taskNumber,1,1);
        pvm_upkstr(inp_programFile);
        pvm_upkstr(out_dir);
        pvm_upkstr(arguments); // string of comma-separated arguments read from datafile

        /* Fork one process that will do the execution
         * the "parent task" will only wait for this process to end
         * and then report resource usage via getrusage()
         */
        pid_t pid = fork();
        // If fork fails, notify master and exit
        if (pid<0) {
            fprintf(stderr,"ERROR - task %d could not spawn execution process\n",taskNumber);
            int state=1;
            pvm_initsend(PVM_ENCODING);
            pvm_pkint(&me,1,1);
            pvm_pkint(&taskNumber,1,1);
            pvm_pkint(&state,1,1);
            pvm_send(myparent,MSG_RESULT);
            pvm_exit();
            exit(1);
        }
        //Child code (work done here)
        if (pid == 0) {
            char output_file[BUFFER_SIZE];
            // Move stdout to output_file
            sprintf(output_file,"%s/%d_out.txt",out_dir,taskNumber);
            int fd = open(output_file,O_WRONLY|O_CREAT,0666);
            dup2(fd,1);
            close(fd);
            // Move stderr to output_file
            sprintf(output_file,"%s/%d_err.txt",out_dir,taskNumber);
            fd = open(output_file,O_WRONLY|O_CREAT,0666);
            dup2(fd,2);
            close(fd);

            /*
             * GENERATE EXECUTION OF PROGRAM
             */
            /* MAPLE */
            if (task_type == 0) {
                // Variables for holding command line arguments passed to execlp
                /*char arg0[BUFFER_SIZE],arg1[BUFFER_SIZE],arg2[BUFFER_SIZE],
                    arg3[BUFFER_SIZE];
                // Very simple because we can pass arguments directly with commas
                sprintf(arg0,"maple");
                sprintf(arg1,"-tc \"taskId:=%d\"",taskNumber);
                sprintf(arg2,"-c \"X:=[%s]\"",arguments);
                sprintf(arg3,"%s",inp_programFile);

                execlp(arg0,arg0,arg1,arg2,arg3,NULL);*/

                char **args;
                args = (char**)malloc(6*sizeof(char*));
                for (i=0;i<6;i++)
                    args[i] = malloc(BUFFER_SIZE);
                sprintf(args[0],"maple");
                sprintf(args[1],"-tc 'taskId:=%d'",taskNumber);
                sprintf(args[2],"-c 'X:=[%s]'",arguments);
                sprintf(args[3],">&2");
                sprintf(args[4],"%s/%d_err.txt",out_dir,taskNumber);
                args[5] = NULL;
                for (i=0;i<6;i++)
                    fprintf(stderr,"'%s' ",args[i]);
                fprintf(stderr,"\n");
                int err = execvp(args[0],args);
                perror("ERROR:: child Maple process");
                exit(err);

            } else {
                // Preparations for C or Python execution 
                char *arguments_cpy;
                arguments_cpy=malloc(strlen(arguments));
                strcpy(arguments_cpy,arguments);
                // This counts how many commas there are in arguments, giving the number
                // of arguments passed to the program
                for(i=0;arguments_cpy[i];arguments_cpy[i]==','?i++:*arguments_cpy++);
                int nargs = i+1; // i = number of commas
                int nargs_tot; // will be the total number of arguments (nargs+program name+etc)
                char *token; // used for tokenizing arguments
                char **args; // this is the NULL-terminated array of strings

                /* C */
                if (task_type == 1) {
                    /* In this case it's necessary to parse the arguments string
                     * breaking it into tokens and then arranging the args of the
                     * system call in a NULL-terminated array of strings which we
                     * pass to execvp
                     */
                    // Tokenizing breaks the original string so we make a copy
                    strcpy(arguments_cpy,arguments);
                    // Args in system call are (program tasknum arguments), so 2+nargs
                    nargs_tot = 2+nargs;
                    args = (char**)malloc((nargs_tot+1)*sizeof(char*));
                    args[nargs_tot]=NULL; // NULL-termination of args
                    for (i=0;i<nargs_tot;i++)
                        args[i] = malloc(BUFFER_SIZE);
                    // Two first command line arguments
                    strcpy(args[0],inp_programFile);
                    sprintf(args[1],"%d",taskNumber);
                    // Tokenize arguments_cpy
                    token = strtok(arguments_cpy,",");
                    // Copy the token to its place in args
                    for (i=2;i<nargs_tot;i++) {
                        strcpy(args[i],token);
                        token = strtok(NULL,",");
                    }

                    execvp(args[0],args);
                }
                /* PYTHON */
                else if (task_type == 2) {
                    // Same as in C, but adding "python" as first argument
                    // Tokenizing breaks the original string so we make a copy
                    strcpy(arguments_cpy,arguments);
                    // Args in system call are (program tasknum arguments), so 2+nargs
                    nargs_tot = 3+nargs;
                    args = (char**)malloc((nargs_tot+1)*sizeof(char*));
                    args[nargs_tot]=NULL; // NULL-termination of args
                    for (i=0;i<nargs_tot;i++)
                        args[i] = malloc(BUFFER_SIZE);
                    // Two first command line arguments
                    strcpy(args[0],"python");
                    strcpy(args[1],inp_programFile);
                    sprintf(args[2],"%d",taskNumber);
                    // Tokenize arguments_cpy
                    token = strtok(arguments_cpy,",");
                    // Copy the token to its place in args
                    for (i=3;i<nargs_tot;i++) {
                        strcpy(args[i],token);
                        token = strtok(NULL,",");
                    }

                    execvp(args[0],args);
                }
            }
        }

        siginfo_t infop;
        waitid(P_PID,pid,&infop,WEXITED);
        struct rusage usage;
        getrusage(RUSAGE_CHILDREN,&usage);
        prtusage(pid,taskNumber,out_dir,usage);

        // Send response to master
        int state=0;
        pvm_initsend(PVM_ENCODING);
        pvm_pkint(&me,1,1);
        pvm_pkint(&taskNumber,1,1);
        pvm_pkint(&state,1,1);
        pvm_send(myparent,MSG_RESULT);
    }
    pvm_exit();
    return 0;
}
