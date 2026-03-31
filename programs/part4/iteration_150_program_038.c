/* main.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

int run_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    
    if (status == -1) {
        fprintf(stderr, "Failed to execute command: %s\n", strerror(errno));
        return -1;
    } else if (WIFEXITED(status)) {
        printf("Command exited with status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    } else {
        fprintf(stderr, "Command terminated abnormally\n\n");
        return -1;
    }
}

int main() {
    int ret = 0;
    
    /* Check if source files exist */
    if (access("test.c", F_OK) != 0) {
        fprintf(stderr, "Error: test.c not found\n");
        return 1;
    }
    
    if (access("test2.c", F_OK) != 0) {
        fprintf(stderr, "Error: test2.c not found\n");
        return 1;
    }
    
    /* Create directories for dump files */
    system("mkdir -p ./testdump ./otherdump");
    
    /* Invoke gcc with help flag */
    if (run_command("gcc --help 2>&1 | head -5") != 0) {
        fprintf(stderr, "Warning: gcc --help failed\n");
    }
    
    /* Invoke gcc with version flag */
    if (run_command("gcc --version") != 0) {
        fprintf(stderr, "Warning: gcc --version failed\n");
    }
    
    /* Compile a simple file with save-temps and dumpdir flags */
    if (run_command("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10") != 0) {
        fprintf(stderr, "Warning: Compilation of test.c failed\n");
        ret = 1;
    }
    
    /* Compile another file with different dumpdir */
    if (run_command("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1") != 0) {
        fprintf(stderr, "Warning: Compilation of test2.c failed\n");
        ret = 1;
    }
    
    /* Optional: Clean up temporary files */
    printf("Temporary files created in ./testdump and ./otherdump\n");
    printf("To clean up, run: rm -rf ./testdump ./otherdump *.o\n");
    
    return ret;
}
