/* main.c */
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

int main() {
    int status;
    
    /* Check if source files exist */
    if (access("test.c", F_OK) == -1 || access("test2.c", F_OK) == -1) {
        fprintf(stderr, "Error: test.c or test2.c not found\n");
        return 1;
    }
    
    /* Create dump directories */
    system("mkdir -p ./testdump ./otherdump");
    
    printf("=== GCC Help (first 5 lines) ===\n");
    status = system("gcc --help 2>&1 | head -5");
    
    printf("\n=== GCC Version ===\n");
    status = system("gcc --version");
    
    printf("\n=== Compiling test.c with save-temps ===\n");
    status = system("gcc -save-temps -dumpdir ./testdump/ -dumpbase mytest -c test.c 2>&1 | tail -10");
    
    printf("\n=== Compiling test2.c with different dumpdir ===\n");
    status = system("gcc -dumpdir ./otherdump/ -dumpbase other -c test2.c 2>&1");
    
    /* Cleanup demonstration */
    printf("\n=== Cleaning up dump directories ===\n");
    status = system("rm -rf ./testdump ./otherdump *.o *.i *.s");
    
    return 0;
}
