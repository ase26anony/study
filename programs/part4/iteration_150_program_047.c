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
        fprintf(stderr, "Command failed with status %d\n", status);
    }
    return status;
}

int ensure_directory(const char *dir) {
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        if (mkdir(dir, 0755) != 0) {
            perror("Failed to create directory");
            return -1;
        }
    }
    return 0;
}

int main() {
    /* Create dump directories if they don't exist */
    if (ensure_directory("./testdump") != 0 ||
        ensure_directory("./otherdump") != 0) {
        return EXIT_FAILURE;
    }
    
    /* Check if source files exist */
    struct stat st;
    if (stat("test.c", &st) != 0) {
        fprintf(stderr, "test.c not found\n");
        return EXIT_FAILURE;
    }
    if (stat("test2.c", &st) != 0) {
        fprintf(stderr, "test2.c not found\n");
        return EXIT_FAILURE;
    }
    
    /* Invoke gcc with help flag */
    execute_command("gcc --help 2>&1 | head -5");
    
    /* Invoke gcc with version flag */
    execute_command("gcc --version");
    
    /* Compile a simple file with save-temps and dumpdir flags */
    execute_command("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    
    /* Compile another file with different dumpdir */
    execute_command("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1");
    
    /* Optional: Clean up temporary files */
    /* execute_command("rm -f test.i test.s test.o test2.o"); */
    
    return EXIT_SUCCESS;
}
