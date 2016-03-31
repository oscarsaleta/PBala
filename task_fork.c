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
	 * memcheck_flag = 0 means generic check
	 * memcheck_flag = 1 means specific info
	 */
	int memcheck_flag = max_task_size > 0 ? 1 : 0;

	// Work work work
	while (1) {
		// Race condition. Mitigated by executing few CPUs on each node
		if (memcheck(memcheck_flag,max_task_size) == 1) {
			sleep(60);
			continue;
		}
		// Receive inputs
		pvm_recv(myparent,MSG_WORK);
		pvm_upkint(&work_code,1,1);
		if (work_code == MSG_STOP)
			break;
		pvm_upkint(&taskNumber,1,1);
		pvm_upkstr(inp_programFile);
		pvm_upkstr(out_dir);
		pvm_upkstr(arguments);


		pid_t pid = fork();
		if (pid<0) {
			fprintf(stderr,"ERROR - task %d could not spawn memory monitor\n",taskNumber);
			return 1;
		}
		//Child code (work done here)
		if (pid == 0) {
			char arg0[BUFFER_SIZE],arg1[BUFFER_SIZE],arg2[BUFFER_SIZE],
				arg3[BUFFER_SIZE];
			char output_file[BUFFER_SIZE];
			sprintf(output_file,"%s/%d_out.txt",out_dir,taskNumber);

			// Move stdout to output_file
			int fd = open(output_file,O_WRONLY|O_CREAT,0666);
			dup2(fd,1);
			close(fd);

			// Generate execution
			char *replaced;
			size_t arglen = strlen(arguments);
			switch (task_type) {
				case 0:
					sprintf(arg0,"maple");
					sprintf(arg1,"-qc \"taskId:=%d\"",taskNumber);
					sprintf(arg2,"-c \"X:=[%s]\"",arguments);
					sprintf(arg3,"%s",inp_programFile);

					execlp(arg0,arg0,arg1,arg2,arg3,NULL);
					break;
				case 1:
					// TODO: parse arguments (no haurien de venir separats per coma)
					
					replaced = (char*)malloc(arglen);
					for (i=0; i<arglen; i++) {
						if (arguments[i]==',')
							replaced[i]=' ';
						else
							replaced[i]=arguments[i];
					}
					sprintf(arg0,"%s",inp_programFile);
					sprintf(arg1,"%d",taskNumber);
					sprintf(arg2,"%s",replaced);

					execlp(arg0,arg0,arg1,arg2,NULL);
					break;
				case 2:
					sprintf(arg0,"python");
					sprintf(arg1,"%s",inp_programFile);
					sprintf(arg2,"%d",taskNumber);
					sprintf(arg3,"%s",arguments);
					
					execlp(arg0,arg0,arg1,arg2,arg3,NULL);
					break;
				default:
					return 1;
			}
		}

		int state;
		/*id_t wpid;
		do {
			fprintf(stderr,"Waiting for %d...\n",pid);
			wpid = waitpid(pid,&state,WEXITED | WNOHANG);
			if (wpid == -1) {
				perror("waitpid");
				exit(EXIT_FAILURE);
			}
			char memcmd[BUFFER_SIZE];
			sprintf(memcmd,"cat /proc/%d/status | grep VmPeak > %s/mem%d.log",pid,out_dir,taskNumber);
			system(memcmd);
			sleep(30);
		} while (!WIFEXITED(state) && !WIFSIGNALED(state));*/

		siginfo_t infop;
		/*do {
			fprintf(stderr,"Waiting for %d...\n",pid);
			infop.si_pid = 0;
			state = waitid(P_PID,pid,&infop,WEXITED | WNOWAIT | WNOHANG);
			if (state == -1) {
				perror("waitpid");
				exit(EXIT_FAILURE);
			}
			char memcmd[BUFFER_SIZE];
			sprintf(memcmd,"cat /proc/%d/status | grep VmPeak >> %s/mem%d.log",pid,out_dir,taskNumber);
			system(memcmd);
			sleep(5);
		} while (infop.si_pid == 0);
		waitid(P_PID,pid,&infop,WEXITED | WNOHANG);*/
		waitid(P_PID,pid,&infop,WEXITED);
		struct rusage usage;
		getrusage(RUSAGE_CHILDREN,&usage);

		FILE *memlog;
		char memlogfilename[FNAME_SIZE];
		sprintf(memlogfilename,"%s/mem%d.log",out_dir,taskNumber);
		memlog = fopen(memlogfilename,"w");
		fprintf(memlog,"TASK %d RESOURCE USAGE\n",taskNumber);
		fprintf(memlog,"----------------------\n");
		fprintf(memlog,"User CPU time used:               %20.10g\n",
			usage.ru_utime.tv_sec+usage.ru_utime.tv_usec/1e6);
		fprintf(memlog,"System CPU time used:             %20.10g\n",
			usage.ru_stime.tv_sec+usage.ru_stime.tv_usec/1e6);
		fprintf(memlog,"Maximum resident set size:        %20ld\n",
			usage.ru_maxrss);
		fprintf(memlog,"Integral shared memory size:      %20ld\n",
			usage.ru_ixrss);
		fprintf(memlog,"Integral unshared data size:      %20ld\n",
			usage.ru_idrss);
		fprintf(memlog,"Integral shared stack size:       %20ld\n",
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

		// Send response to master
		state=0;
		pvm_initsend(PVM_ENCODING);
		pvm_pkint(&me,1,1);
		pvm_pkint(&taskNumber,1,1);
		pvm_pkint(&state,1,1);
		pvm_send(myparent,MSG_RESULT);
	}
	pvm_exit();
	return 0;
}