/*! \file testCprogram.c
 * \brief Test C program for pvm_test.c
 * \author Oscar Saleta Reig
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Main function. Simply print program arguments (4 arguments required)
 *
 * \return 0 if successful
 */
int main (int argc, char *argv[]) {
    int taskId;
    int arg1,arg2,arg3;

    /* Test if arguments are correctly read */
    if (argc != 5) {
        fprintf(stderr,"%s:: %d args given (4 required)\n",argv[0],argc-1);
        fprintf(stderr,"Given: ");
        for (int i=1; i<argc; i++)
            fprintf(stderr,"\"%s\" ",argv[i]);
        fprintf(stderr,"\n");
        return 1;
    } else if (sscanf(argv[1],"%d",&taskId)!=1) {
        fprintf(stderr,"%s:: taskId read error\n",argv[0]);
        return 1;
    } else if (sscanf(argv[2],"%d",&arg1)!=1) {
        fprintf(stderr,"%s:: arg1 read error\n",argv[0]);
        return 1;
    } else if (sscanf(argv[3],"%d",&arg2)!=1) {
        fprintf(stderr,"%s:: arg2 (%s) read error\n",argv[0],argv[3]);
        return 1;
    } else if (sscanf(argv[4],"%d",&arg3)!=1) {
        fprintf(stderr,"%s:: arg3 read error\n",argv[0]);
        return 1;
    }
    
    printf("args: %d,%d,%d\n",arg1,arg2,arg3);

    /* Test if memory usage is correctly measured */
    char *stack;
    stack = malloc(1000000000); // alloc 1GB of memory
    for (int i=0;i<1000000000;i++)
        stack[i]='0';
    sleep(20); // sleep 20s for the system to catch up
    free(stack);

    return 0;
}
