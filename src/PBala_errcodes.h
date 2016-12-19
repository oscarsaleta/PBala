/* Job parallelizer in PVM for SPMD executions in computing cluster
 * URL: https://github.com/oscarsaleta/PVMantz
 *
 * Copyright (C) 2016  Oscar Saleta Reig
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

#ifndef PBALA_ERRCODES_H
#define PBALA_ERRCODES_H
/*! \file PBala_errcodes.h
 * \brief Header for error codes in PBala
 * \author Oscar Saleta Reig
 */

/* ERROR CODES FOR PBala.c MAIN RETURN VALUES */
#define E_ARGS 10
#define E_NODE_LINES 11
#define E_NODE_OPEN 12
#define E_NODE_READ 13
#define E_CWD 14
#define E_PVM_MYTID 15
#define E_PVM_PARENT 16
#define E_DATAFILE_LINES 17
#define E_OUTFILE_OPEN 18
#define E_PVM_SPAWN 19
#define E_DATAFILE_FIRSTCOL 20
#define E_OUTDIR 21
#define E_WRONG_TASK 22
#define E_IO 23
#define E_MPL 24

/* ERROR CODES FOR PBala_task.c SLAVE RETURN STATUS */
#define ST_FORK_ERR 10
#define ST_TASK_KILLED 11

#endif /* PBALA_ERRCODES_H */
