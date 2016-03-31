#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pvm3.h>
#include "antz_lib.h"


int main(int argc, char *argv[]) {
	int myparent,taskNumber;
	int me,status,work_code;
	char command[BUFFER_SIZE];
	char inp_programFile[FNAME_SIZE];
	char out_dir[FNAME_SIZE];
	char arguments[BUFFER_SIZE];
	int task_type; //0:maple,1:C,2:python
	long int max_task_size;

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
		while (memcheck(memcheck_flag,max_task_size) == 1) {
			sleep(10);
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

		// Generate execution
		switch (task_type) {
			case 0:
				sprintf(command,"maple -qc 'taskId:=%d' -c 'X:=[%s]' %s > %s/%d_out.txt",
					taskNumber,arguments,inp_programFile,out_dir,taskNumber);
				break;
			case 1:
				sprintf(command,"%s %d %s/%d_out.txt %s",
					inp_programFile,taskNumber,out_dir,taskNumber,arguments);
				break;
			case 2:
				sprintf(command,"python %s %d %s/%d_out.txt %s",
					inp_programFile,taskNumber,out_dir,taskNumber,arguments);
				break;
			default:
				fprintf(stderr,"ERROR - task not available\n");
				pvm_exit();
				return 1;
		}
		system(command);

		// Send response to master
		status = 0;
		pvm_initsend(PVM_ENCODING);
		pvm_pkint(&me,1,1);
		pvm_pkint(&taskNumber,1,1);
		pvm_pkint(&status,1,1);
		pvm_send(myparent,MSG_RESULT);
	}
	pvm_exit();
	return 0;
}