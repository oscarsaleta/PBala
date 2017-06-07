/* This file is part of PBala (http://github.com/oscarsaleta/PBala)
 *
 * Copyright (C) 2016  O. Saleta
 *
 * PBala is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*! \mainpage PVM general parallelizer for antz
 * \author Oscar Saleta Reig
 */

/*! \file PBala.c
 * \brief Main PVM program. Distributes executions of SDMP in antz
 * \author Oscar Saleta Reig
 */

#include "PBala_config.h"
#include "PBala_errcodes.h"
#include "PBala_lib.h"

#include <argp.h>
#include <dirent.h>
#include <pvm3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* Program version and bug email */
const char *argp_program_version = VERSION;
const char *argp_program_bug_address = "<osr@mat.uab.cat>";

/* Program documentation */
static char doc[] =
    "PBala -- PVM SPMD execution parallellizer.\n\tprogramflag "
    "argument can be: 0 (Maple), 1 (C), 2 (Python), 3 (Pari), "
    "4 (Sage), or 5 (Octave)";
/* Arguments we accept */
static char args_doc[] = "programflag programfile datafile nodefile outdir";

/* Options we understand */
static struct argp_option options[] = {
    {"kill", 'k', 0, 0,
     "Kill remainig PBala/PVM processes (WARNING: use at "
     "own risk! Use only if something goes wrong during an "
     "execution and PVM stops working and you have no other "
     "important processes running)"},
    {"max-mem-size", 'm', "MAX_MEM", 0, "Max memory size of a task (KB)"},
    {"maple-single-core", 's', 0, 0, "Force single core Maple"},
    {"create-errfiles", 'e', 0, 0, "Create stderr files"},
    {"create-memfiles", 103, 0, 0, "Create memory files"},
    {"create-slavefile", 104, 0, 0, "Create node file"},
    {"custom-process", 'c', "/path/to/exec", 0,
     "Specify a custom path for the executable program"},
    {0}};

/* Struct for communicating arguments to main */
struct arguments {
  char *args[5];
  int kill;
  long int max_mem_size;
  int maple_single_cpu, create_err, create_mem, create_slave;
  int custom_path;
  char program_path[BUFFER_SIZE];
};

/* Parse a single option */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;

  switch (key) {
    case 'k':
      arguments->kill = 1;
      break;
    case 'm':
      sscanf(arg, "%ld", &(arguments->max_mem_size));
      break;
    case 's':
      arguments->maple_single_cpu = 1;
      break;
    case 'e':
      arguments->create_err = 1;
      break;
    case 103:
      arguments->create_mem = 1;
      break;
    case 104:
      arguments->create_slave = 1;
      break;
    case 'c':
      arguments->custom_path = 1;
      sscanf(arg, "%s", arguments->program_path);
      break;

    case ARGP_KEY_ARG:
      if (state->arg_num >= 5) argp_usage(state);
      arguments->args[state->arg_num] = arg;
      break;

    case ARGP_KEY_END:
      if (state->arg_num < 5 && arguments->kill != 1) argp_usage(state);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

/* argp parser */
static struct argp argp = {options, parse_opt, args_doc, doc};

/**
 * Main PVM function. Handles task creation and result gathering.
 * Call: ./PBala programFlag programFile dataFile nodeFile outDir [max_mem_size
 * (KB)] [maple_single_core]
 *
 * \param[in] argv[1] flag for program type (0=maple,1=C,2=python,3=pari,4=sage)
 * \param[in] argv[2] program file (maple library, c executable, etc)
 * \param[in] argv[3] data input file
 * \param[in] argv[4] nodes file (2 cols: node cpus)
 * \param[in] argv[5] output file directory
 * \param[in] argv[6] (optional) aprox max memory size of biggest execution in
 * KB
 * \param[in] argv[7] (optional) flag for single core execution (Maple only:
 * 0=no, 1=yes)
 *
 * \return 0 if successful
 */
int main(int argc, char *argv[]) {
  // Program options and arguments
  struct arguments arguments;
  arguments.kill = 0;
  arguments.max_mem_size = 0;
  arguments.maple_single_cpu = 0;
  arguments.create_err = 0;
  arguments.create_mem = 0;
  arguments.create_slave = 0;
  arguments.custom_path = 0;
  // PVM args
  int myparent, mytid, nTasks, taskNumber;
  int itid;
  int work_code;
  // File names
  char inp_programFile[FNAME_SIZE];
  char inp_dataFile[FNAME_SIZE];
  char inp_nodes[FNAME_SIZE];
  char out_dir[FNAME_SIZE];
  char nodeInfoFileName[FNAME_SIZE];
  char out_file[FNAME_SIZE];
  char cwd[FNAME_SIZE];
  // Files
  FILE *f_data = NULL;
  FILE *nodeInfoFile = NULL;
  FILE *f_out = NULL;
  // Nodes variables
  char **nodes;
  int *nodeCores;
  int nNodes, maxConcurrentTasks;
  // Aux variables
  char buffer[BUFFER_SIZE];
  int i, j, err;
  char aux_str[BUFFER_SIZE];
  size_t aux_size;
  // Task variables
  int task_type;
  // Execution time variables
  double exec_time, total_time;
  struct timespec tspec_before, tspec_after, tspec_result;
  double total_total_time = 0;

  clock_gettime(CLOCK_REALTIME, &tspec_before);

  /* MASTER CODE */

  /* set stderr as a line buffered output stream */
  setlinebuf(stderr);
  // setvbuf(stderr, NULL, _IOLBF, BUFFER_SIZE);

  /* Read command line arguments */
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  /* if kill option is found, we do that and exit */
  if (arguments.kill) return killPBala();

  if (sscanf(arguments.args[0], "%d", &task_type) != 1 ||
      sscanf(arguments.args[1], "%s", inp_programFile) != 1 ||
      sscanf(arguments.args[2], "%s", inp_dataFile) != 1 ||
      sscanf(arguments.args[3], "%s", inp_nodes) != 1 ||
      sscanf(arguments.args[4], "%s", out_dir) != 1) {
    fprintf(stderr, "[ERROR] - reading arguments\n");
    return E_ARGS;
  }

  // sanitize maple library if single cpu is required
  if (arguments.maple_single_cpu) {
    if (mapleSingleCPU(inp_programFile) != 0) return E_MPL;
  }

  // check if task type is correct
  if (task_type != 0 && task_type != 1 && task_type != 2 && task_type != 3 &&
      task_type != 4 && task_type != 5) {
    fprintf(stderr,
            "[ERROR] - wrong task_type value (must be one of: "
            "0,1,2,3,4,5)\n");
    return E_WRONG_TASK;
  }

  // prepare node_info.txt file if desired
  if (arguments.create_slave) {
    strcpy(nodeInfoFileName, out_dir);
    strcat(nodeInfoFileName, "/node_info.txt");
    nodeInfoFile = fopen(nodeInfoFileName, "w");
    if (nodeInfoFile == NULL) {
      fprintf(stderr,
              "[ERROR] - cannot create file %s, make sure the output folder %s "
              "exists\n",
              nodeInfoFileName, out_dir);
      return E_OUTDIR;
    }
    fprintf(nodeInfoFile, "# NODE CODENAMES\n");
  }

  /* Read node configuration file */
  // Get file length (number of nodes)
  if ((nNodes = getLineCount(inp_nodes)) == -1) {
    fprintf(stderr, "[ERROR] - cannot open file %s\n", inp_nodes);
    return E_NODE_LINES;
  }
  // Read node file
  if ((err = parseNodefile(inp_nodes, nNodes, &nodes, &nodeCores)) == 1) {
    fprintf(stderr, "[ERROR] - cannot open file %s\n", inp_nodes);
    return E_NODE_OPEN;
  } else if (err == 2) {
    fprintf(stderr, "[ERROR] - while reading node file %s\n", inp_nodes);
    return E_NODE_READ;
  }

  /* INITIALIZE PVMD */

  /* get current working directory */
  if (getcwd(cwd, FNAME_SIZE) == NULL) {
    fprintf(stderr, "[ERROR] - cannot resolve current directory\n");
    return E_CWD;
  }

  /* create hostfile */
  FILE *hostfile = fopen("hostfile", "w");
  fprintf(hostfile, "* ep=%s wd=%s\n", cwd, cwd);
  for (i = 0; i < nNodes; i++) fprintf(hostfile, "%s\n", nodes[i]);
  fclose(hostfile);

  /* attempt PVM initialization */
  char *pvmd_argv[1] = {"hostfile"};
  int pvmd_argc = 1;
  int start_status;
  int start_tries = 0;
  while ((start_status = pvm_start_pvmd(pvmd_argc, pvmd_argv, 1)) ==
         PvmDupHost) {
    ++start_tries;
    pvm_halt();
    system("rm -f /tmp/pvm*");
    if (start_tries > 3) return E_PVM_DUP;
  }
  sprintf(out_file, "%s/outfile.txt", out_dir);
  if ((f_out = fopen(out_file, "w")) == NULL) {
    fprintf(stderr, "[ERROR] - cannot open output file %s\n", out_file);
    pvm_halt();
    return E_OUTFILE_OPEN;
  }
  pvm_catchout(f_out);
  // Error task id
  mytid = pvm_mytid();
  if (mytid < 0) {
    pvm_perror(argv[0]);
    pvm_halt();
    return E_PVM_MYTID;
  }
  // Error parent id
  myparent = pvm_parent();
  if (myparent < 0 && myparent != PvmNoParent) {
    pvm_perror(argv[0]);
    pvm_halt();
    return E_PVM_PARENT;
  }
  /***/

  // Max number of tasks running at once
  maxConcurrentTasks = 0;
  for (i = 0; i < nNodes; i++) {
    maxConcurrentTasks += nodeCores[i];
  }

  // Read how many tasks we have to perform
  if ((nTasks = getLineCount(inp_dataFile)) == -1) {
    fprintf(stderr, "[ERROR] - cannot open data file %s\n", inp_dataFile);
    pvm_halt();
    return E_DATAFILE_LINES;
  }

  // Print execution info
  fprintf(stdout, "PRINCESS BALA v%s\n", VERSION);
  fprintf(stdout, "System call: ");
  for (i = 0; i < argc; i++) fprintf(stdout, "%s ", argv[i]);
  fprintf(stdout, "\n\n");

  fprintf(stdout, "[INFO] - will use executable %s\n", inp_programFile);
  fprintf(stdout, "[INFO] - will use datafile %s\n", inp_dataFile);
  fprintf(stdout, "[INFO] - will use nodefile %s\n", inp_nodes);
  fprintf(stdout, "[INFO] - results will be stored in %s\n\n", out_dir);

  fprintf(stdout, "[INFO] - will use nodes ");
  for (i = 0; i < nNodes - 1; i++)
    fprintf(stdout, "%s (%d), ", nodes[i], nodeCores[i]);
  fprintf(stdout, "%s (%d)\n", nodes[nNodes - 1], nodeCores[nNodes - 1]);
  fprintf(stdout, "[INFO] - will create %d tasks for %d slaves\n\n", nTasks,
          maxConcurrentTasks);

  // Spawn all the tasks
  int taskId[maxConcurrentTasks];
  itid = 0;
  int numt;
  int numnode = 0;
  for (i = 0; i < nNodes; i++) {
    for (j = 0; j < nodeCores[i]; j++) {
      numt = pvm_spawn("PBala_task", NULL, PvmTaskHost, nodes[i], 1,
                       &taskId[itid]);
      if (numt != 1) {
        fprintf(stderr, "[ERROR] - %d creating task %4d in node %s\n", numt,
                taskId[itid], nodes[i]);
        fflush(stderr);
        pvm_perror(argv[0]);
        pvm_halt();
        return E_PVM_SPAWN;
      }
      // Send info to task
      pvm_initsend(PVM_ENCODING);
      pvm_pkint(&itid, 1, 1);
      pvm_pkint(&task_type, 1, 1);
      pvm_pklong(&(arguments.max_mem_size), 1, 1);
      pvm_pkint(&(arguments.create_err), 1, 1);
      pvm_pkint(&(arguments.create_mem), 1, 1);
      pvm_pkint(&(arguments.custom_path), 1, 1);
      if (arguments.custom_path) pvm_pkstr(arguments.program_path);
      pvm_send(taskId[itid], MSG_GREETING);
      fprintf(stdout, "[CREATED_SLAVE] - created slave %d\n", itid);
      if (arguments.create_slave)
        fprintf(nodeInfoFile, "# Node %2d -> %s\n", numnode, nodes[i]);
      numnode++;
      // Do not create more tasks than necessary
      // if (numnode >= nTasks)
      //    break;
      // FIXME: si deixem això, podem crear esclaus inútils i ometre nodes
      // bons
      itid++;
    }
  }
  fprintf(stdout, "[INFO] - all slaves created successfully\n\n");

  // First batch of work sent at once (we will listen to answers later)
  f_data = fopen(inp_dataFile, "r");
  if (arguments.create_slave) fprintf(nodeInfoFile, "\nNODE,TASK\n");
  int firstBatchSize =
      nTasks < maxConcurrentTasks ? nTasks : maxConcurrentTasks;
  work_code = MSG_GREETING;
  for (i = 0; i < firstBatchSize; i++) {
    if (fgets(buffer, BUFFER_SIZE, f_data) != NULL) {
      if (sscanf(buffer, "%d", &taskNumber) != 1) {
        fprintf(stderr,
                "[ERROR] - first column of data file must be task id\n");
        pvm_halt();
        return E_DATAFILE_FIRSTCOL;
      }
      pvm_initsend(PVM_ENCODING);
      pvm_pkint(&work_code, 1, 1);
      pvm_pkint(&taskNumber, 1, 1);
      pvm_pkstr(inp_programFile);
      pvm_pkstr(out_dir);
      /* parse arguments (skip tasknumber) */
      sprintf(aux_str, "%d", taskNumber);
      aux_size = strlen(aux_str);
      buffer[strlen(buffer) - 1] = 0;
      // copy to aux_str the data line from after the first ","
      sprintf(aux_str, "%s", &buffer[aux_size + 1]);
      pvm_pkstr(aux_str);
      // create file for pari execution if needed
      if (task_type == 3) {
        if (parifile(taskNumber, aux_str, inp_programFile, out_dir) == -1)
          return E_IO;  // i/o error
        fprintf(
            stdout,
            "[CREATED_SCRIPT] - creating auxiliary Pari script for task %d\n",
            taskNumber);
      } else if (task_type == 4) {
        if (sagefile(taskNumber, aux_str, inp_programFile, out_dir) == -1)
          return E_IO;  // i/o error
        fprintf(
            stdout,
            "[CREATED_SCRIPT] - creating auxiliary Sage script for task %d\n",
            taskNumber);
      } else if (task_type == 5) {
        if (octavefile(taskNumber, aux_str, inp_programFile, out_dir) == -1)
          return E_IO;
        fprintf(
            stdout,
            "[CREATED_SCRIPT] - creating auxiliary Octave script for task %d\n",
            taskNumber);
      }

      // send the job
      pvm_send(taskId[i], MSG_WORK);
      fprintf(stdout, "[TASK_SENT] - sent task %4d for execution\n",
              taskNumber);
      if (arguments.create_slave)
        fprintf(nodeInfoFile, "%2d,%4d\n", i, taskNumber);
    }
  }
  fprintf(stdout, "[INFO] - first batch of work sent\n\n");

  // Close nodeInfoFile so it updates in file system
  if (arguments.create_slave) fclose(nodeInfoFile);
  // Keep assigning work to nodes if needed
  int status, unfinished_tasks_present = 0;
  FILE *unfinishedTasks;
  char unfinishedTasks_name[FNAME_SIZE];
  sprintf(unfinishedTasks_name, "%s/unfinished_tasks.txt", out_dir);
  unfinishedTasks = fopen(unfinishedTasks_name, "w");
  fclose(unfinishedTasks);
  if (nTasks > maxConcurrentTasks) {
    for (j = maxConcurrentTasks; j < nTasks; j++) {
      // Receive answer from slaves
      pvm_recv(-1, MSG_RESULT);
      pvm_upkint(&itid, 1, 1);
      pvm_upkint(&taskNumber, 1, 1);
      pvm_upkint(&status, 1, 1);
      pvm_upkstr(aux_str);
      // Check if response is error at forking
      if (status == ST_MEM_ERR) {
        fprintf(stderr,
                "[ERROR] - could not execute task %d in "
                "slave %d (out of memory)\n",
                taskNumber, itid);
        unfinishedTasks = fopen(unfinishedTasks_name, "a");
        fprintf(unfinishedTasks, "%d,%s\n", taskNumber, aux_str);
        fclose(unfinishedTasks);
        unfinished_tasks_present = 1;
      } else if (status == ST_FORK_ERR) {
        fprintf(stderr,
                "[ERROR] - could not fork process for task "
                "%d in slave %d\n",
                taskNumber, itid);
        unfinishedTasks = fopen(unfinishedTasks_name, "a");
        fprintf(unfinishedTasks, "%d,%s\n", taskNumber, aux_str);
        fclose(unfinishedTasks);
        unfinished_tasks_present = 1;
      } else {
        pvm_upkdouble(&exec_time, 1, 1);
        // Check if task was killed or completed
        if (status == ST_TASK_KILLED) {
          fprintf(stderr, "[ERROR] - task %4d was stopped or killed\n",
                  taskNumber);
          unfinishedTasks = fopen(unfinishedTasks_name, "a");
          fprintf(unfinishedTasks, "%d,%s\n", taskNumber, aux_str);
          fclose(unfinishedTasks);
          unfinished_tasks_present = 1;
        } else
          fprintf(stdout,
                  "[TASK_COMPLETED] - task %4d completed "
                  "in %14.9G seconds\n",
                  taskNumber, exec_time);
      }
      // Assign more work until we're done
      if (fgets(buffer, BUFFER_SIZE, f_data) != NULL) {
        // Open nodeInfoFile for appending work
        if (arguments.create_slave) nodeInfoFile = fopen(nodeInfoFileName, "a");
        if (sscanf(buffer, "%d", &taskNumber) != 1) {
          fprintf(stderr,
                  "[ERROR] - first column of data file must be task id\n");
          pvm_halt();
          return E_DATAFILE_FIRSTCOL;
        }
        pvm_initsend(PVM_ENCODING);
        pvm_pkint(&work_code, 1, 1);
        pvm_pkint(&taskNumber, 1, 1);
        pvm_pkstr(inp_programFile);
        pvm_pkstr(out_dir);
        sprintf(aux_str, "%d", taskNumber);
        aux_size = strlen(aux_str);
        buffer[strlen(buffer) - 1] = 0;
        sprintf(aux_str, "%s", &buffer[aux_size + 1]);
        pvm_pkstr(aux_str);
        // create file for pari execution if needed
        if (task_type == 3) {
          if (parifile(taskNumber, aux_str, inp_programFile, out_dir) == -1)
            return E_IO;  // i/o error
          fprintf(
              stdout,
              "[CREATED_SCRIPT] - creating auxiliary Pari script for task %d\n",
              taskNumber);
        } else if (task_type == 4) {
          if (sagefile(taskNumber, aux_str, inp_programFile, out_dir) == -1)
            return E_IO;  // i/o error
          fprintf(
              stdout,
              "[CREATED_SCRIPT] - creating auxiliary Sage script for task %d\n",
              taskNumber);
        } else if (task_type == 5) {
          if (octavefile(taskNumber, aux_str, inp_programFile, out_dir) == -1)
            return E_IO;
          fprintf(stdout,
                  "[CREATED_SCRIPT] - creating auxiliary Octave script for "
                  "task %d\n",
                  taskNumber);
        }
        // send the job
        pvm_send(taskId[itid], MSG_WORK);
        fprintf(stdout, "[TASK_SENT] - sent task %3d for execution\n",
                taskNumber);
        if (arguments.create_slave) {
          fprintf(nodeInfoFile, "%2d,%4d\n", itid, taskNumber);
          fclose(nodeInfoFile);
        }
      }
      // If user wants to force finish, keep going with unfinishedtasks
      /*if (arguments.force_finish) {
          FILE *f_data2 = fopen(unfinishedTasks_name,"r");
          if (fgets(buffer, BUFFER_SIZE, f_data2) != NULL) {
              // Open nodeInfoFile for appending work
              if (arguments.create_slave)
                  nodeInfoFile = fopen(nodeInfoFileName, "a");
              if (sscanf(buffer, "%d", &taskNumber) != 1) {
                  fprintf(stderr,
                          "[ERROR] - first column of data file "
                          "must be task id\n",
                          argv[0]);
                  pvm_halt();
                  return E_DATAFILE_FIRSTCOL;
              }
              pvm_initsend(PVM_ENCODING);
              pvm_pkint(&work_code, 1, 1);
              pvm_pkint(&taskNumber, 1, 1);
              pvm_pkstr(inp_programFile);
              pvm_pkstr(out_dir);
              sprintf(aux_str, "%d", taskNumber);
              aux_size = strlen(aux_str);
              buffer[strlen(buffer) - 1] = 0;
              sprintf(aux_str, "%s", &buffer[aux_size + 1]);
              pvm_pkstr(aux_str);
              // create file for pari execution if needed
              if (task_type == 3) {
                  if (parifile(taskNumber, aux_str, inp_programFile,
                               out_dir) == -1)
                      return E_IO; // i/o error
                  fprintf(stdout,
                          "[CREATED_SCRIPT] - creating auxiliary "
                          "Pari script for task %d\n",
                           taskNumber);
              } else if (task_type == 4) {
                  if (sagefile(taskNumber, aux_str, inp_programFile,
                               out_dir) == -1)
                      return E_IO; // i/o error
                  fprintf(stdout,
                          "[CREATED_SCRIPT] - creating auxiliary "
                          "Sage script for task %d\n",
                           taskNumber);
              } else if (task_type == 5) {
                  if (octavefile(taskNumber, aux_str, inp_programFile,
                                 out_dir) == -1)
                      return E_IO;
                  fprintf(stdout,
                          "[CREATED_SCRIPT] - creating auxiliary "
                          "Octave script for task %d\n",
                           taskNumber);
              }
              // send the job
              pvm_send(taskId[itid], MSG_WORK);
              fprintf(stdout,
                      "[TASK_SENT] - sent task %3d for execution\n",
                       taskNumber);
              if (arguments.create_slave) {
                  fprintf(nodeInfoFile, "%2d,%4d\n", itid, taskNumber);
                  fclose(nodeInfoFile);
              }
          }
      }*/
    }
  }

  // Listen to answers from slaves that keep working
  work_code = MSG_STOP;
  for (i = 0; i < firstBatchSize; i++) {
    // Receive answer from slaves
    pvm_recv(-1, MSG_RESULT);
    pvm_upkint(&itid, 1, 1);
    pvm_upkint(&taskNumber, 1, 1);
    pvm_upkint(&status, 1, 1);
    pvm_upkstr(aux_str);
    // Check if response is error at forking
    if (status == ST_MEM_ERR) {
      fprintf(stderr,
              "[ERROR] - could not execute task %d in "
              "slave %d (out of memory)\n",
              taskNumber, itid);
      unfinishedTasks = fopen(unfinishedTasks_name, "a");
      fprintf(unfinishedTasks, "%d,%s\n", taskNumber, aux_str);
      fclose(unfinishedTasks);
      unfinished_tasks_present = 1;
    } else if (status == ST_FORK_ERR) {
      fprintf(stderr,
              "[ERROR] - could not fork process for task %d in slave %d\n",
              taskNumber, itid);
      unfinishedTasks = fopen(unfinishedTasks_name, "a");
      fprintf(unfinishedTasks, "%d,%s\n", taskNumber, aux_str);
      fclose(unfinishedTasks);
      unfinished_tasks_present = 1;
    } else {
      pvm_upkdouble(&exec_time, 1, 1);
      // Check if task was killed or completed
      if (status == ST_TASK_KILLED) {
        fprintf(stderr, "[ERROR] - task %4d was stopped or killed\n",
                taskNumber);
        unfinishedTasks = fopen(unfinishedTasks_name, "a");
        fprintf(unfinishedTasks, "%d,%s\n", taskNumber, aux_str);
        fclose(unfinishedTasks);
        unfinished_tasks_present = 1;
      } else
        fprintf(stdout,
                "[TASK_COMPLETED] - task %4d completed in "
                "%14.9G seconds\n",
                taskNumber, exec_time);
    }
    pvm_upkdouble(&total_time, 1, 1);
    // Shut down slave
    pvm_initsend(PVM_ENCODING);
    pvm_pkint(&work_code, 1, 1);
    pvm_send(taskId[itid], MSG_STOP);
    fprintf(stdout,
            "[INFO] - shutting down slave %2d (total execution "
            "time: %13.5G seconds)\n",
            itid, total_time);
    total_total_time += total_time;
  }

  // Final message
  clock_gettime(CLOCK_REALTIME, &tspec_after);
  timespec_subtract(&tspec_result, &tspec_after, &tspec_before);

  fprintf(stdout,
          "\n== END OF EXECUTION ==\n"
          " - Combined computing time: %14.5G seconds.\n"
          " - Total execution time:    %6ld.%9ld seconds.\n",
          total_total_time, (long int)tspec_result.tv_sec,
          tspec_result.tv_nsec);

  free(nodes);
  free(nodeCores);
  fclose(f_data);
  fclose(f_out);

  // remove tmp program (if modified)
  if (arguments.maple_single_cpu) {
    sprintf(aux_str, "[ ! -f %s.bak ] || mv %s.bak %s", inp_programFile,
            inp_programFile, inp_programFile);
    if (system(aux_str))
      fprintf(stderr,
              "[ERROR] - could not clean up Maple single CPU "
              "aux scripts\n");
  }
  // remove tmp pari/sage/octave programs (if created)
  if (task_type == 3 || task_type == 4 || task_type == 5) {
    DIR *dir;
    struct dirent *ent;
    dir = opendir(out_dir);
    while ((ent = readdir(dir))) {
      if (strstr(ent->d_name, "auxprog") != NULL) {
        sprintf(aux_str, "%s/%s", out_dir, ent->d_name);
        remove(aux_str);
      }
    }
    closedir(dir);
  }
  // remove unfinished_tasks.txt file if empty
  if (!unfinished_tasks_present) {
    sprintf(aux_str, "rm %s", unfinishedTasks_name);
    if (system(aux_str))
      fprintf(stderr,
              "[ERROR] - could not clean up empty unfinished tasks file\n");
  }

  pvm_catchout(0);
  pvm_halt();

  return 0;
}
