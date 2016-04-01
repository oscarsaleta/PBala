/*! \mainpage PVM general parallellizer for antz
 * \author Oscar Saleta Reig
 */

/*! \file pvm_test.c 
 * \brief Main PVM program. Distributes executions of SDMP in antz
 * \author Oscar Saleta Reig
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pvm3.h>
#include "antz_lib.h"


/**
 * Main PVM function. Handles task creation and result gathering.
 * Call: ./pvm_test programFlag programFile dataFile nodeFile outDir [maxMemSize (KB)]
 *
 * \param[in] argv[1] flag for program type (0=maple,1=C,2=python,3=pari)
 * \param[in] argv[2] program file (maple library, c executable, etc)
 * \param[in] argv[3] data input file
 * \param[in] argv[4] nodes file (2 cols: node cpus)
 * \param[in] argv[5] output file directory
 * \param[in] argv[6] (optional) aprox max memory size of biggest execution in KB
 *
 * \return 0 if successful
 */
int main (int argc, char *argv[]) {
    // PVM args
    int myparent, mytid, nTasks, taskNumber;
    int itid;
    int work_code;
    // Files
    char inp_programFile[FNAME_SIZE];
    char inp_dataFile[FNAME_SIZE];
    FILE *f_data;
    char inp_nodes[FNAME_SIZE];
    FILE *f_nodes;
    char out_dir[FNAME_SIZE];
    char logfilename[FNAME_SIZE];
    FILE *logfile;
    char out_file[FNAME_SIZE];
    FILE *f_out;
    // Nodes variables
    char **nodes;
    int *nodeCores;
    int nNodes,maxConcurrentTasks;
    // Aux variables
    char buffer[BUFFER_SIZE];
    int i,j;
    char aux_char[BUFFER_SIZE];
    size_t aux_size;
    // Task variables
    int task_type;
    long int maxMemSize=0;

    // Error task id
    mytid = pvm_mytid();
    if (mytid<0) {
        pvm_perror(argv[0]);
        return -1;
    }
    // Error parent id
    myparent = pvm_parent();
    if (myparent<0 && myparent != PvmNoParent) {
        pvm_perror(argv[0]);
        pvm_exit();
        return -1;
    }

    /* MASTER CODE */

    /* Read command line arguments */
    if (argc < 6
        || sscanf(argv[1],"%d",&task_type)!=1
        || sscanf(argv[2],"%s",inp_programFile)!=1
        || sscanf(argv[3],"%s",inp_dataFile)!=1
        || sscanf(argv[4],"%s",inp_nodes)!=1
        || sscanf(argv[5],"%s",out_dir)!=1
        || ( argc == 7 && sscanf(argv[6],"%ld",&maxMemSize)!=1 )
        || argc > 7
        ) {
        fprintf(stderr,"%s:: programFile dataFile nodeFile outDir [maxMemSize (KB)]\n",argv[0]);
        pvm_exit();
        return 1;
    }
    
    strcpy(logfilename,out_dir);
    strcat(logfilename,"/node_info.log");
    logfile = fopen(logfilename,"w");
    fprintf(logfile,"# NODE CODENAMES\n");


    
    /* Read node configuration file */
    // Get file length (number of nodes)
    nNodes = getLineCount(argv[0],inp_nodes);
    // Open nodes file
    f_nodes = fopen(inp_nodes,"r");
    if (f_nodes == NULL) {
        fprintf(stderr,"%s:: ERROR - invalid node file %s\n",argv[0],inp_nodes);
        return -1;
    }
    // Allocate memory for node and cpu array
    nodes = (char**)malloc(nNodes*sizeof(char*));
    for (i=0; i<nNodes; i++)
        nodes[i] = (char*)malloc(MAX_NODE_LENGTH*sizeof(char));
    nodeCores = (int*)malloc(nNodes*sizeof(int));
    // Rewind and read again to store nodes and cores
    for (i=0; i<nNodes; i++) {
        if (fscanf(f_nodes,"%s %d",nodes[i],&nodeCores[i]) != 2) {
            fprintf(stderr,"%s:: ERROR - while reading node file %s\n",argv[0],inp_nodes);
            return -1;
        }
    }
    // Close the nodes file
    fclose(f_nodes);

    // Max number of tasks running at once
    maxConcurrentTasks = 0;
    for (i=0; i<nNodes; i++) {
        maxConcurrentTasks += nodeCores[i];
    }
    
    // Read how many tasks we have to perform
    nTasks = getLineCount(argv[0],inp_dataFile);

    fprintf(stderr,"%s:: INFO - will use nodes ",argv[0]);
    for (i=0; i<nNodes-1; i++)
        fprintf(stderr,"%s (%d), ",nodes[i],nodeCores[i]);
    fprintf(stderr,"%s (%d)\n",nodes[nNodes-1],nodeCores[nNodes-1]);
    fprintf(stderr,"%s:: INFO - will create %d tasks\n",argv[0],nTasks);

    
    sprintf(out_file,"%s/outfile.txt",out_dir);
    if ((f_out = fopen(out_file,"w")) == NULL) {
        fprintf(stderr,"%s:: ERROR - cannot open output file %s\n",argv[0],out_file);
        return 1;
    }
    
    pvm_catchout(f_out);



    // Spawn all the tasks
    int taskId[maxConcurrentTasks];
    itid = 0;
    int numt;
    int numnode=0;
    for (i=0; i<nNodes; i++) {
        for (j=0; j<nodeCores[i]; j++) {
            numt = pvm_spawn("task",NULL,PvmTaskHost,nodes[i],1,&taskId[itid]);
            if (numt != 1) {
                fprintf(stderr,"%s:: ERROR - %d creating task %d in node %s\n",
                    argv[0],numt,taskId[itid],nodes[i]);
                pvm_perror(argv[0]);
                return 1;
            }
            // Send info to task
            pvm_initsend(PVM_ENCODING);
            pvm_pkint(&itid,1,1);
            pvm_pkint(&task_type,1,1);
            pvm_pklong(&maxMemSize,1,1);
            /* msgtag=1 used for greeting slave */
            pvm_send(taskId[itid],MSG_GREETING);
            fprintf(stderr,"%s:: INFO - created slave %d\n",argv[0],itid);
            fprintf(logfile,"# Node %d -> %s\n",numnode,nodes[i]);
            numnode++;
            itid++;
        }
    }
    // First batch of work sent at once (we will listen to answers later)
    f_data = fopen(inp_dataFile,"r");
    fprintf(logfile,"\nNODE,TASK\n");
    int firstBatchSize = nTasks < maxConcurrentTasks ? nTasks : maxConcurrentTasks;
    work_code = MSG_GREETING;
    for (i=0; i<firstBatchSize; i++) {
        if (fgets(buffer,BUFFER_SIZE,f_data)!=NULL) {
            if (sscanf(buffer,"%d",&taskNumber)!=1) {
                fprintf(stderr,"%s:: ERROR - first column of data file must be task id\n",argv[0]);
                return 1;
            }
            pvm_initsend(PVM_ENCODING);
            pvm_pkint(&work_code,1,1);
            pvm_pkint(&taskNumber,1,1);
            pvm_pkstr(inp_programFile);
            pvm_pkstr(out_dir);
            /* parse arguments (skip tasknumber) */
            sprintf(aux_char,"%d",taskNumber);
            aux_size = strlen(aux_char);
            buffer[strlen(buffer)-1]=0;
            // copy to aux_char the data line from after the first ","
            sprintf(aux_char,"%s",&buffer[aux_size+1]); 
            pvm_pkstr(aux_char);
            pvm_send(taskId[i],MSG_WORK);
            fprintf(stderr,"%s:: INFO - sent task %d for execution\n",argv[0],taskNumber);
            fprintf(logfile,"%d,%d\n",i,taskNumber);
        }
    }
    // Close logfile so it updates in file system
    fclose(logfile);
    // Keep assigning work to nodes if needed
    int status;
    if (nTasks > maxConcurrentTasks) {
        for (j=maxConcurrentTasks; j<nTasks; j++) {
            // Receive answer from slaves
            pvm_recv(-1,MSG_RESULT);
            pvm_upkint(&itid,1,1);
            pvm_upkint(&taskNumber,1,1);
            pvm_upkint(&status,1,1);
            if (status != 0) {
                fprintf(stderr,"%s:: ERROR - task %d failed at execution\n",argv[0],taskNumber);
                pvm_perror(argv[0]);
                return 1;
            }
            fprintf(stderr,"%s:: INFO - task %d completed\n",argv[0],taskNumber);
            
            // Assign more work until we're done
            if (fgets(buffer,BUFFER_SIZE,f_data)!=NULL) {
                // Open logfile for appending work
                logfile = fopen(logfilename,"a");
                if (sscanf(buffer,"%d",&taskNumber)!=1) {
                    fprintf(stderr,"%s:: ERROR - first column of data file must be task id\n",argv[0]);
                    return 1;
                }
                pvm_initsend(PVM_ENCODING);
                pvm_pkint(&work_code,1,1);
                pvm_pkint(&taskNumber,1,1);
                pvm_pkstr(inp_programFile);
                pvm_pkstr(out_dir);
                sprintf(aux_char,"%d",taskNumber);
                aux_size = strlen(aux_char);
                buffer[strlen(buffer)-1]=0;
                sprintf(aux_char,"%s",&buffer[aux_size+1]);
                pvm_pkstr(aux_char);
                pvm_send(taskId[itid],MSG_WORK);
                fprintf(stderr,"%s:: INFO - sent task %d for execution\n",argv[0],taskNumber);
                fprintf(logfile,"%d,%d\n",itid,taskNumber);
                fclose(logfile);
            }
        }
    }
    // Listen to answers from slaves that keep working
    work_code = MSG_STOP;
    for (i=0; i<maxConcurrentTasks; i++) {
        // Receive response
        pvm_recv(-1,MSG_RESULT);
        pvm_upkint(&itid,1,1);
        pvm_upkint(&taskNumber,1,1);
        pvm_upkint(&status,1,1);
        if (status != 0) {
            fprintf(stderr,"%s:: ERROR - task %d failed at execution\n",argv[0],itid);
            pvm_perror(argv[0]);
            return 1;
        }
        fprintf(stderr,"%s:: INFO - task %d completed\n",argv[0],taskNumber);
        // Shut down node
        pvm_initsend(PVM_ENCODING);
        pvm_pkint(&work_code,1,1);
        pvm_send(taskId[itid],MSG_STOP);
        fprintf(stderr,"%s:: INFO - shutting down slave %d\n",argv[0],itid);
    }

    free(nodes);
    free(nodeCores);
    fclose(f_data);
    fclose(f_out);
    pvm_catchout(0);
    

    pvm_exit();

    return 0;
}