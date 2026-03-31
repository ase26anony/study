/* main.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int status;
    
    /* Check if source files exist */
    if (access("test.c", F_OK) == -1) {
        fprintf(stderr, "Error: test.c not found\n");
        return 1;
    }
    
    if (access("test2.c", F_OK) == -1) {
        fprintf(stderr, "Error: test2.c not found\n");
        return 1;
    }
    
    printf("=== GCC Help (first 5 lines) ===\n");
    status = system("gcc --help 2>&1 | head -5");
    
    printf("\n=== GCC Version ===\n");
    status = system("gcc --version");
    
    printf("\n=== Compiling test.c with verbose output ===\n");
    status = system("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    
    printf("\n=== Compiling test2.c ===\n");
    status = system("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1");
    
    /* Check compilation results */
    if (WIFEXITED(status)) {
        printf("\nCompilation completed with exit code: %d\n", WEXITSTATUS(status));
    }
    
    return 0;
}

/* test.c */
int foo(void) { return 42; }

/* test2.c */
int bar(void) { return 43; }
