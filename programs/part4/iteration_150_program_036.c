/* main.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int status;
    
    printf("=== GCC Help (first 5 lines) ===\n");
    status = system("gcc --help 2>&1 | head -5");
    
    printf("\n=== GCC Version ===\n");
    status = system("gcc --version");
    
    /* Check if test.c exists before compiling */
    printf("\n=== Compiling test.c with save-temps ===\n");
    if (access("test.c", F_OK) == 0) {
        status = system("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    } else {
        printf("test.c not found, creating it...\n");
        status = system("echo 'int foo(void) { return 42; }' > test.c");
        status = system("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    }
    
    printf("\n=== Compiling test2.c ===\n");
    if (access("test2.c", F_OK) == 0) {
        status = system("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1");
    } else {
        printf("test2.c not found, creating it...\n");
        status = system("echo 'int bar(void) { return 43; }' > test2.c");
        status = system("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1");
    }
    
    /* Clean up temporary files */
    printf("\n=== Cleaning up ===\n");
    system("rm -f test.c test2.c *.o *.i *.s 2>/dev/null");
    system("rm -rf testdump otherdump 2>/dev/null");
    
    return 0;
}
