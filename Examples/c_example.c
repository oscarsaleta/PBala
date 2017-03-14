/* Job parallelizer in PVM for SPMD executions in computing cluster
 * URL: https://github.com/oscarsaleta/PVMantz
 *
 * Copyright (C) 2016  Oscar Saleta Reig
 *
 * PBala is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * PBala is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PBala.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>

int main(int argc, char *argv[])
{
    /* Assume we are using the following data file:
     *      33, 1.1251, 32.653, 2.221, 4
     * (this is just a single row). THe first number,
     * 33, is the task identificator, but IT IS PASSED
     * as an argument, so we need to take it into account.
     */

    /* Declare variables for storing the arguments */
    int taskId;
    double x1, x2, x3;
    int x4;

    /* Read command line arguments and make sure there
     * is no error (unexpected number of arguments, or
     * error reading them and storing them in the variables).
     * Take into account that argv[0] is the name of the program,
     * so we want 6 arguments and we start counting from 1.
     */
    if ( // check for 6 arguments
        argc != 6
        // store 1st argument as int in taskId
        || sscanf(argv[1], "%d", &taskId) != 1
        // store 2nd argument as double in x1
        || sscanf(argv[2], "%lf", &x1) != 1
        // store 3rd argument as double in x2
        || sscanf(argv[2], "%lf", &x2) != 1
        // store 4th argument as double in x3
        || sscanf(argv[2], "%lf", &x3) != 1
        // store 5th argument as int in x4
        || sscanf(argv[2], "%d", &x4) != 1) {
        // if some of these conditions are met, there is an error!
        fprintf(stderr, "Error: wrong arguments passed.\n");
        return -1;
    }

    /* If the execution got to this point, we can use the arguments
     * in our program. In this simple example, we will only print them.
     */
    printf("Arguments passed are\n", taskId, x1, x2, x3, x4);

    return 0;
}