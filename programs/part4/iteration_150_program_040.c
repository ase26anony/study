/* main.c */
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/stat.h>

int execute_command(const char *cmd) {
    int status = system(cmd);
    if (status == -1) {
        perror("system() failed");
        return -1;
    } else if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

int ensure_directory(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) == -1) {
            perror("mkdir failed");
            return -1;
        }
    }
    return 0;
}

int main() {
    int ret = 0;
    
    /* Create dump directories if they don't exist */
    if (ensure_directory("./testdump") == -1 ||
        ensure_directory("./otherdump") == -1) {
        return EXIT_FAILURE;
    }
    
    /* Check if source files exist */
    if (access("test.c", F_OK) == -1 || access("test2.c", F_OK) == -1) {
        fprintf(stderr, "Source files test.c and/or test2.c not found\n");
        return EXIT_FAILURE;
    }
    
    printf("=== GCC Help (first 5 lines) ===\n");
    ret = execute_command("gcc --help 2>&1 | head -5");
    if (ret != 0) {
        fprintf(stderr, "gcc --help failed with code %d\n", ret);
    }
    
    printf("\n=== GCC Version ===\n");
    ret = execute_command("gcc --version");
    if (ret != 0) {
        fprintf(stderr, "gcc --version failed with code %d\n", ret);
    }
    
    printf("\n=== Compiling test.c with verbose output ===\n");
    ret = execute_command("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    if (ret != 0) {
        fprintf(stderr, "Compilation of test.c failed with code %d\n", ret);
    }
    
    printf("\n=== Compiling test2.c ===\n");
    ret = execute_command("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1");
    if (ret != 0) {
        fprintf(stderr, "Compilation of test2.c failed with code %d\n", ret);
    }
    
    /* Optional: Cleanup temporary files */
    printf("\n=== Cleaning up temporary files ===\n");
    system("rm -f test.i test.s test.o test2.i test2.s test2.o mytest* other*");
    system("rm -rf testdump otherdump");
    
    return EXIT_SUCCESS;
}

/* test.c */
int foo(void) { return 42; }

/* test2.c */
int bar(void) { return 43; }
