/* main.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

int file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

int main() {
    int status;
    
    /* Check if source files exist */
    if (!file_exists("test.c")) {
        fprintf(stderr, "Error: test.c not found\n");
        return 1;
    }
    
    if (!file_exists("test2.c")) {
        fprintf(stderr, "Error: test2.c not found\n");
        return 1;
    }
    
    /* Create dump directories if they don't exist */
    system("mkdir -p ./testdump ./otherdump 2>/dev/null");
    
    /* Invoke gcc commands with better error checking */
    printf("=== GCC Help (first 5 lines) ===\n");
    status = system("gcc --help 2>&1 | head -5");
    if (status != 0) {
        fprintf(stderr, "Warning: gcc --help failed\n");
    }
    
    printf("\n=== GCC Version ===\n");
    status = system("gcc --version");
    if (status != 0) {
        fprintf(stderr, "Error: gcc not found or failed\n");
        return 1;
    }
    
    printf("\n=== Compiling test.c with verbose output ===\n");
    status = system("gcc -save-temps -dumpdir ./testdump/ -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    
    printf("\n=== Compiling test2.c ===\n");
    status = system("gcc -dumpdir ./otherdump/ -dumpbase other -c test2.c 2>&1");
    
    /* Optional: Clean up temporary files */
    printf("\n=== Temporary files created ===\n");
    system("find . -name '*.i' -o -name '*.s' -o -name '*.o' 2>/dev/null | head -10");
    
    printf("\n=== Done ===\n");
    
    return 0;
}
