/* main.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

int execute_command(const char *cmd) {
    int status = system(cmd);
    if (status == -1) {
        perror("system() failed");
        return -1;
    } else if (WIFEXITED(status)) {
        printf("Command exited with status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    } else {
        printf("Command terminated abnormally\n\n");
        return -1;
    }
}

int ensure_directory(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) == -1) {
            perror("Failed to create directory");
            return -1;
        }
    }
    return 0;
}

int main() {
    int ret = 0;
    
    /* Check if source files exist */
    if (access("test.c", F_OK) == -1) {
        fprintf(stderr, "test.c not found\n");
        return 1;
    }
    
    if (access("test2.c", F_OK) == -1) {
        fprintf(stderr, "test2.c not found\n");
        return 1;
    }
    
    /* Create dump directories */
    if (ensure_directory("./testdump") == -1 ||
        ensure_directory("./otherdump") == -1) {
        return 1;
    }
    
    printf("=== Getting GCC help (first 5 lines) ===\n");
    ret |= execute_command("gcc --help 2>&1 | head -5");
    
    printf("=== Getting GCC version ===\n");
    ret |= execute_command("gcc --version");
    
    printf("=== Compiling test.c with verbose output ===\n");
    ret |= execute_command("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    
    printf("=== Compiling test2.c ===\n");
    ret |= execute_command("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1");
    
    return ret;
}
