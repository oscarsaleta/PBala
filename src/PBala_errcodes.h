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

#ifndef PBALA_ERRCODES_H
#define PBALA_ERRCODES_H
/*! \file PBala_errcodes.h
 * \brief Header for error codes in PBala
 * \author Oscar Saleta Reig
 */

/* ERROR CODES FOR PBala.c MAIN RETURN VALUES */
//FIXME: arreglar numeros
#define E_ARGS 10
#define E_NODEFILE 11
#define E_CWD 12
#define E_PVM_MYTID 13
#define E_PVM_PARENT 14
#define E_DATAFILE 15
#define E_OUTFILE_OPEN 16
#define E_PVM_SPAWN 17
#define E_OUTDIR 18
#define E_WRONG_TASK 19
#define E_IO 20
#define E_MPL 21
#define E_PVM_DUP 22
#define E_NO_PBALA_TASK 23

/* ERROR CODES FOR PBala_task.c SLAVE RETURN STATUS */
#define ST_READY 10
#define ST_FORK_ERR 11
#define ST_TASK_KILLED 12
#define ST_MEM_ERR 13

#endif /* PBALA_ERRCODES_H */
