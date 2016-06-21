/* Job parallelizer in PVM for SPMD executions in antz computing server
 * URL: https://github.com/oscarsaleta/PVMantz
 *
 * Copyright (C) 2016  Dept. Matematiques UAB
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


/*! \mainpage PVM general parallelizer for antz
 * \author Oscar Saleta Reig
 */

/*! \file PBala.c 
 * \brief Main PVM program. Distributes executions of SDMP in antz
 * \author Oscar Saleta Reig
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pvm3.h>
#include "antz_lib.h"
#include "antz_errcodes.h"


/**
 * Main PVM function. Handles task creation and result gathering.
 * Call: ./PBala programFlag programFile dataFile nodeFile outDir [maxMemSize (KB)] [maple_single_core]
 *
 * \param[in] argv[1] flag for program type (0=maple,1=C,2=python,3=pari,4=sage)
 * \param[in] argv[2] program file (maple library, c executable, etc)
 * \param[in] argv[3] data input file
 * \param[in] argv[4] nodes file (2 cols: node cpus)
 * \param[in] argv[5] output file directory
 * \param[in] argv[6] (optional) aprox max memory size of biggest execution in KB
 * \param[in] argv[7] (optional) flag for single core execution (Maple only: 0=no, 1=yes)
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
    char out_dir[FNAME_SIZE];
    char logfilename[FNAME_SIZE];
    FILE *logfile;
    char out_file[FNAME_SIZE];
    FILE *f_out;
    char cwd[FNAME_SIZE];
    // Nodes variables
    //int hostinfos;
    char **nodes;
    int *nodeCores;
    int nNodes,maxConcurrentTasks;
    // Aux variables
    char buffer[BUFFER_SIZE];
    int i,j,err;
    char aux_str[BUFFER_SIZE];
    size_t aux_size;
    // Task variables
    int task_type;
    int maple_single_cpu=0;
    long int maxMemSize=0;
    // Execution time variables
    double exec_time,total_time;
    double total_total_time=0;
    time_t initt,endt;
    double difft;

    time(&initt);


    /* MASTER CODE */
    setvbuf(stderr,NULL,_IOLBF,BUFFER_SIZE);

    /* Read command line arguments */
    if (argc < 6
        || sscanf(argv[1],"%d",&task_type)!=1
        || sscanf(argv[2],"%s",inp_programFile)!=1
        || sscanf(argv[3],"%s",inp_dataFile)!=1
        || sscanf(argv[4],"%s",inp_nodes)!=1
        || sscanf(argv[5],"%s",out_dir)!=1
        || ( argc > 6 && sscanf(argv[6],"%ld",&maxMemSize)!=1 )
        || ( argc > 7 && sscanf(argv[7],"%d",&maple_single_cpu)!=1 )
        || argc > 8
        ) {
        fprintf(stderr,"%s:: exec_flag program_file data_file node_file out_dir [max_mem_size (KB)] [maple_single_cpu]\n",argv[0]);
        return E_ARGS;
    }
    // sanitize maple library if single cpu is required
    if (maple_single_cpu) {
        sprintf(aux_str,"grep -q -F 'kernelopts(numcpus=1)' %s || (sed '1ikernelopts(numcpus=1);' %s > %s_tmp && mv %s %s.bak && mv %s_tmp %s)",
                inp_programFile,inp_programFile,inp_programFile,inp_programFile,inp_programFile,inp_programFile,inp_programFile);
        system(aux_str);
    }

    // check if task type is correct
    if(task_type!=1
            && task_type!=2
            && task_type!=3
            && task_type!=4) {
        fprintf(stderr,"%s:: ERROR - wrong task_type value (must be one of: 1,2,3,4)\n",argv[0]);
        return E_WRONG_TASK;
    }


    // prepare node_info.log file
    strcpy(logfilename,out_dir);
    strcat(logfilename,"/node_info.log");
    logfile = fopen(logfilename,"w");
    if (logfile==NULL) {
        fprintf(stderr,"%s:: ERROR - cannot create file %s, make sure the output folder %s exists\n",argv[0],logfilename,out_dir);
        return E_OUTDIR;
    }
    fprintf(logfile,"# NODE CODENAMES\n");

    /* Read node configuration file */
    // Get file length (number of nodes)
    if ((nNodes = getLineCount(inp_nodes))==-1) {
        fprintf(stderr,"%s:: ERROR - cannot open file %s\n",argv[0],inp_nodes);
        return E_NODE_LINES;
    }
    // Read node file
    if ((err=parseNodefile(inp_nodes,nNodes,&nodes,&nodeCores)) == 1) {
        fprintf(stderr,"%s:: ERROR - cannot open file %s\n",argv[0],inp_nodes);
        return E_NODE_OPEN;
    } else if (err==2) {
        fprintf(stderr,"%s:: ERROR - while reading node file %s\n",argv[0],inp_nodes);
        return E_NODE_READ;
    }

    /* INITIALIZE PVMD */
    if (getcwd(cwd,FNAME_SIZE)==NULL) {
        fprintf(stderr,"%s:: ERROR - cannot resolve current directory\n",argv[0]);
        return E_CWD;
    }
    sprintf(aux_str,"echo '* ep=%s wd=%s' > hostfile",cwd,cwd);
    system(aux_str);
    for (i=0; i<nNodes; i++) {
        sprintf(aux_str,"echo '%s' >> hostfile",nodes[i]);
        system(aux_str);
    }
    char *pvmd_argv[1] = {"hostfile"};
    int pvmd_argc = 1;
    pvm_start_pvmd(pvmd_argc,pvmd_argv,1);
    sprintf(out_file,"%s/outfile.txt",out_dir);
    if ((f_out = fopen(out_file,"w")) == NULL) {
        fprintf(stderr,"%s:: ERROR - cannot open output file %s\n",argv[0],out_file);
        return E_OUTFILE_OPEN;
    }
    pvm_catchout(f_out);
    //pvm_addhosts(nodes,nNodes,&hostinfos);
    // Error task id
    mytid = pvm_mytid();
    if (mytid<0) {
        pvm_perror(argv[0]);
        return E_PVM_MYTID;
    }
    // Error parent id
    myparent = pvm_parent();
    if (myparent<0 && myparent != PvmNoParent) {
        pvm_perror(argv[0]);
        pvm_exit();
        return E_PVM_PARENT;
    }
    /***/

    // Max number of tasks running at once
    maxConcurrentTasks = 0;
    for (i=0; i<nNodes; i++) {
        maxConcurrentTasks += nodeCores[i];
    }
    
    // Read how many tasks we have to perform
    if((nTasks = getLineCount(inp_dataFile))==-1) {
        fprintf(stderr,"%s:: cannot open data file %s\n",argv[0],inp_dataFile);
        return E_DATAFILE_LINES;
    }

    // Print execution info
    fprintf(stderr,"PRINCESS BALA v%s\n",VERSION);
    fprintf(stderr,"System call: ");
    for (i=0;i<argc;i++)
        fprintf(stderr,"%s ",argv[i]);
    fprintf(stderr,"\n\n");

    fprintf(stderr,"%s:: INFO - will use executable %s\n",argv[0],inp_programFile);
    fprintf(stderr,"%s:: INFO - will use datafile %s\n",argv[0],inp_dataFile);
    fprintf(stderr,"%s:: INFO - will use nodefile %s\n",argv[0],inp_nodes);
    fprintf(stderr,"%s:: INFO - results will be stored in %s\n\n",argv[0],out_dir);


    fprintf(stderr,"%s:: INFO - will use nodes ",argv[0]);
    for (i=0; i<nNodes-1; i++)
        fprintf(stderr,"%s (%d), ",nodes[i],nodeCores[i]);
    fprintf(stderr,"%s (%d)\n",nodes[nNodes-1],nodeCores[nNodes-1]);
    fprintf(stderr,"%s:: INFO - will create %d tasks for %d slaves\n\n",argv[0],nTasks,nNodes);

    
    

    // Spawn all the tasks
    int taskId[maxConcurrentTasks];
    itid = 0;
    int numt;
    int numnode=0;
    for (i=0; i<nNodes; i++) {
        for (j=0; j<nodeCores[i]; j++) {
            numt = pvm_spawn("task",NULL,PvmTaskHost,nodes[i],1,&taskId[itid]);
            if (numt != 1) {
                fprintf(stderr,"%s:: ERROR - %d creating task %4d in node %s\n",
                    argv[0],numt,taskId[itid],nodes[i]);
                fflush(stderr);
                pvm_perror(argv[0]);
                return E_PVM_SPAWN;
            }
            // Send info to task
            pvm_initsend(PVM_ENCODING);
            pvm_pkint(&itid,1,1);
            pvm_pkint(&task_type,1,1);
            pvm_pklong(&maxMemSize,1,1);
            pvm_send(taskId[itid],MSG_GREETING);
            fprintf(stderr,"%s:: CREATED_SLAVE - created slave %d\n",argv[0],itid);
            fprintf(logfile,"# Node %2d -> %s\n",numnode,nodes[i]);
            numnode++;
            // Do not create more tasks than necessary
            if (numnode >= nTasks)
                break;
            itid++;
        }
    }
    fprintf(stderr,"%s:: INFO - all slaves created successfully\n\n",argv[0]);

    // First batch of work sent at once (we will listen to answers later)
    f_data = fopen(inp_dataFile,"r");
    fprintf(logfile,"\nNODE,TASK\n");
    int firstBatchSize = nTasks < maxConcurrentTasks ? nTasks : maxConcurrentTasks;
    work_code = MSG_GREETING;
    for (i=0; i<firstBatchSize; i++) {
        if (fgets(buffer,BUFFER_SIZE,f_data)!=NULL) {
            if (sscanf(buffer,"%d",&taskNumber)!=1) {
                fprintf(stderr,"%s:: ERROR - first column of data file must be task id\n",argv[0]);
                return E_DATAFILE_FIRSTCOL;
            }
            pvm_initsend(PVM_ENCODING);
            pvm_pkint(&work_code,1,1);
            pvm_pkint(&taskNumber,1,1);
            pvm_pkstr(inp_programFile);
            pvm_pkstr(out_dir);
            /* parse arguments (skip tasknumber) */
            sprintf(aux_str,"%d",taskNumber);
            aux_size = strlen(aux_str);
            buffer[strlen(buffer)-1]=0;
            // copy to aux_str the data line from after the first ","
            sprintf(aux_str,"%s",&buffer[aux_size+1]); 
            pvm_pkstr(aux_str);
            // create file for pari execution if needed
            if (task_type == 3) {
                parifile(taskNumber,aux_str,inp_programFile,out_dir);
                fprintf(stderr,"%s:: CREATED_SCRIPT - creating auxiliary Pari script for task %d\n",argv[0],taskNumber);
            } else if (task_type == 4) {
                sagefile(taskNumber,aux_str,inp_programFile,out_dir);
                fprintf(stderr,"%s:: CREATED_SCRIPT - creating auxiliary Sage script for task %d\n",argv[0],taskNumber);
            }

            // send the job
            pvm_send(taskId[i],MSG_WORK);
            fprintf(stderr,"%s:: TASK_SENT - sent task %4d for execution\n",argv[0],taskNumber);
            fprintf(logfile,"%2d,%4d\n",i,taskNumber);
        }
    }
    fprintf(stderr,"%s:: INFO - first batch of work sent\n\n",argv[0]);


    // Close logfile so it updates in file system
    fclose(logfile);
    // Keep assigning work to nodes if needed
    int status;
    FILE *unfinishedTasks;
    char unfinishedTasks_name[FNAME_SIZE];
    sprintf(unfinishedTasks_name,"%s/unfinished_tasks.txt",out_dir);
    unfinishedTasks=fopen(unfinishedTasks_name,"w");
    fclose(unfinishedTasks);
    if (nTasks > maxConcurrentTasks) {
        for (j=maxConcurrentTasks; j<nTasks; j++) {
            // Receive answer from slaves
            pvm_recv(-1,MSG_RESULT);
            pvm_upkint(&itid,1,1);
            pvm_upkint(&taskNumber,1,1);
            pvm_upkint(&status,1,1);
            pvm_upkstr(aux_str);
            // Check if response is error at forking
            if (status == ST_FORK_ERR) {
                fprintf(stderr,"%s:: ERROR - could not fork process for task %d at slave %d\n",
                        argv[0],taskNumber,itid);
                unfinishedTasks = fopen(unfinishedTasks_name,"a");
                fprintf(unfinishedTasks,"%d,%s\n",taskNumber,aux_str);
                fclose(unfinishedTasks);
            } else {
                pvm_upkdouble(&exec_time,1,1);
                // Check if task was killed or completed
                if (status == ST_TASK_KILLED) {
                    fprintf(stderr,"%s:: ERROR - task %4d was stopped or killed\n",argv[0],taskNumber);
                    unfinishedTasks = fopen(unfinishedTasks_name,"a");
                    fprintf(unfinishedTasks,"%d,%s\n",taskNumber,aux_str);
                    fclose(unfinishedTasks);
                } else
                    fprintf(stderr,"%s:: TASK_COMPLETED - task %4d completed in %10.5G seconds\n",argv[0],taskNumber,exec_time);
            }
            // Assign more work until we're done
            if (fgets(buffer,BUFFER_SIZE,f_data)!=NULL) {
                // Open logfile for appending work
                logfile = fopen(logfilename,"a");
                if (sscanf(buffer,"%d",&taskNumber)!=1) {
                    fprintf(stderr,"%s:: ERROR - first column of data file must be task id\n",argv[0]);
                    return E_DATAFILE_FIRSTCOL;
                }
                pvm_initsend(PVM_ENCODING);
                pvm_pkint(&work_code,1,1);
                pvm_pkint(&taskNumber,1,1);
                pvm_pkstr(inp_programFile);
                pvm_pkstr(out_dir);
                sprintf(aux_str,"%d",taskNumber);
                aux_size = strlen(aux_str);
                buffer[strlen(buffer)-1]=0;
                sprintf(aux_str,"%s",&buffer[aux_size+1]);
                pvm_pkstr(aux_str);
                // create file for pari execution if needed
                if (task_type == 3) {
                    parifile(taskNumber,aux_str,inp_programFile,out_dir);
                    fprintf(stderr,"%s:: CREATED_SCRIPT - creating auxiliary Pari script for task %d\n",argv[0],taskNumber);
                } else if (task_type == 4) {
                    sagefile(taskNumber,aux_str,inp_programFile,out_dir);
                    fprintf(stderr,"%s:: CREATED_SCRIPT - creating auxiliary Sage script for task %d\n",argv[0],taskNumber);
                }
                // send the job
                pvm_send(taskId[itid],MSG_WORK);
                fprintf(stderr,"%s:: TASK_SENT - sent task %3d for execution\n",argv[0],taskNumber);
                fprintf(logfile,"%2d,%4d\n",itid,taskNumber);
                fclose(logfile);
            }
        }
    }

    // Listen to answers from slaves that keep working
    work_code = MSG_STOP;
    for (i=0; i<firstBatchSize; i++) {
        // Receive answer from slaves
        pvm_recv(-1,MSG_RESULT);
        pvm_upkint(&itid,1,1);
        pvm_upkint(&taskNumber,1,1);
        pvm_upkint(&status,1,1);
        pvm_upkstr(aux_str);
        // Check if response is error at forking
        if (status == ST_FORK_ERR) {
            fprintf(stderr,"%s:: ERROR - could not fork process for task %d at slave %d\n",
                    argv[0],taskNumber,itid);
            unfinishedTasks = fopen(unfinishedTasks_name,"a");
            fprintf(unfinishedTasks,"%d,%s\n",taskNumber,aux_str);
            fclose(unfinishedTasks);
        } else {
            pvm_upkdouble(&exec_time,1,1);
            // Check if task was killed or completed
            if (status == ST_TASK_KILLED) {
                fprintf(stderr,"%s:: ERROR - task %4d was stopped or killed\n",argv[0],taskNumber);
                unfinishedTasks = fopen(unfinishedTasks_name,"a");
                fprintf(unfinishedTasks,"%d,%s\n",taskNumber,aux_str);
                fclose(unfinishedTasks);
            } else
                fprintf(stderr,"%s:: TASK_COMPLETED - task %4d completed in %10.5G seconds\n",argv[0],taskNumber,exec_time);
        }
        pvm_upkdouble(&total_time,1,1);
        // Shut down slave
        pvm_initsend(PVM_ENCODING);
        pvm_pkint(&work_code,1,1);
        pvm_send(taskId[itid],MSG_STOP);
        fprintf(stderr,"%s:: INFO - shutting down slave %2d (total execution time: %13.5G seconds)\n",argv[0],itid,total_time);
        total_total_time += total_time;
    }

    // Final message
    time(&endt);
    difft = difftime(endt,initt);
    fprintf(stderr,"\n%s:: END OF EXECUTION.\nCombined computing time: %14.5G seconds.\nTotal execution time:    %14.5G seconds.\n",argv[0],total_total_time,difft);

    free(nodes);
    free(nodeCores);
    fclose(f_data);
    fclose(f_out);
    
    
    // remove tmp program (if modified)
    if (maple_single_cpu) {
        sprintf(aux_str,"[ ! -f %s.bak ] || mv %s.bak %s",inp_programFile,inp_programFile,inp_programFile);
        system(aux_str);
    }
    // remove tmp pari programs (if created)
    if (task_type == 3 || task_type == 4) {
        sprintf(aux_str,"rm %s/auxprog-*",out_dir);
        system(aux_str);
    }

    pvm_catchout(0);
    pvm_halt();

    return 0;
}
