/*! \file antz_lib.c 
 * \brief File with generic functions and constant definitions
 * \author Oscar Saleta Reig
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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