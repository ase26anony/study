/* main.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed with status %d: %s\n", status, cmd);
    }
    return status;
}

int file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

int main() {
    int all_ok = 1;
    
    /* Check if source files exist */
    if (!file_exists("test.c")) {
        fprintf(stderr, "Error: test.c not found\n");
        all_ok = 0;
    }
    
    if (!file_exists("test2.c")) {
        fprintf(stderr, "Error: test2.c not found\n");
        all_ok = 0;
    }
    
    if (!all_ok) {
        return EXIT_FAILURE;
    }
    
    /* Create dump directories if they don't exist */
    system("mkdir -p ./testdump ./otherdump");
    
    /* Invoke gcc with help flag */
    if (execute_command("gcc --help 2>&1 | head -5") != 0) {
        fprintf(stderr, "gcc --help failed\n");
    }
    
    printf("\n---\n\n");
    
    /* Invoke gcc with version flag */
    if (execute_command("gcc --version") != 0) {
        fprintf(stderr, "gcc --version failed\n");
    }
    
    printf("\n---\n\n");
    
    /* Compile a simple file with save-temps and dumpdir flags */
    if (execute_command("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10") != 0) {
        fprintf(stderr, "Compilation of test.c failed\n");
    }
    
    printf("\n---\n\n");
    
    /* Compile another file with different dumpdir */
    if (execute_command("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1") != 0) {
        fprintf(stderr, "Compilation of test2.c failed\n");
    }
    
    /* Optional: Clean up temporary files */
    printf("\nCleaning up temporary files...\n");
    system("rm -f *.i *.s *.o mytest* other*");
    system("rm -rf ./testdump ./otherdump 2>/dev/null");
    
    return EXIT_SUCCESS;
}

/* test.c */
int foo(void) { return 42; }

/* test2.c */
int bar(void) { return 43; }
