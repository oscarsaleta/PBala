#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pvm3.h>
#include "antz_lib.h"


int memcheck(int flag, int max_task_size);

int main(int argc, char *argv[]) {
	int myparent,taskNumber;
	int me,status,work_code;
	char command[BUFFER_SIZE];
	char inp_programFile[FNAME_SIZE];
	char out_dir[FNAME_SIZE];
	char arguments[BUFFER_SIZE];
	int max_task_size;

	myparent = pvm_parent();
	// Be greeted by master
	pvm_recv(myparent,MSG_GREETING);
	pvm_upkint(&me,1,1);
	pvm_upkint(&max_task_size,1,1);

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
		sprintf(command,"python %s %d %s/%d.txt %s",
			inp_programFile,taskNumber,out_dir,taskNumber,arguments);
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

/* Memory check:
 * 	returns 0 if there is enough memory (more than half the maximum amount)
 * 	returns 1 if there is not enough memory
 */
int memcheck(int flag, int max_task_size) {
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