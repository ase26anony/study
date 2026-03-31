/* main.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    int status;
    
    /* Check if source files exist */
    if (access("test.c", F_OK) != 0) {
        fprintf(stderr, "Error: test.c not found\n");
        return 1;
    }
    
    if (access("test2.c", F_OK) != 0) {
        fprintf(stderr, "Error: test2.c not found\n");
        return 1;
    }
    
    printf("=== GCC Help (first 5 lines) ===\n");
    status = system("gcc --help 2>&1 | head -5");
    if (status == -1) {
        perror("system() failed");
    }
    
    printf("\n=== GCC Version ===\n");
    status = system("gcc --version");
    if (status == -1) {
        perror("system() failed");
    }
    
    printf("\n=== Compiling test.c with verbose output ===\n");
    status = system("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    if (status == -1) {
        perror("system() failed");
    }
    
    printf("\n=== Compiling test2.c ===\n");
    status = system("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1");
    if (status == -1) {
        perror("system() failed");
    }
    
    /* Optional: Cleanup */
    printf("\n=== Cleaning up dump directories ===\n");
    system("rm -rf ./testdump ./otherdump 2>/dev/null");
    
    return 0;
}
