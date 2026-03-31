/* main.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int execute_command(const char *cmd) {
    int status = system(cmd);
    if (status == -1) {
        perror("system() failed");
        return -1;
    } else if (WIFEXITED(status)) {
        printf("Command exited with status %d\n", WEXITSTATUS(status));
    } else {
        printf("Command terminated abnormally\n");
    }
    return status;
}

int main() {
    int ret = 0;
    
    /* Check if source files exist */
    if (access("test.c", F_OK) != 0 || access("test2.c", F_OK) != 0) {
        fprintf(stderr, "Error: test.c or test2.c not found\n");
        return 1;
    }
    
    /* Create directories for dump files */
    system("mkdir -p ./testdump ./otherdump");
    
    /* Invoke gcc with help flag */
    printf("\n=== GCC Help (first 5 lines) ===\n");
    ret |= execute_command("gcc --help 2>&1 | head -5");
    
    /* Invoke gcc with version flag */
    printf("\n=== GCC Version ===\n");
    ret |= execute_command("gcc --version");
    
    /* Compile a simple file with save-temps and dumpdir flags */
    printf("\n=== Compiling test.c with verbose output ===\n");
    ret |= execute_command("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    
    /* Compile another file with different dumpdir */
    printf("\n=== Compiling test2.c ===\n");
    ret |= execute_command("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1");
    
    /* Optional: Clean up temporary files */
    printf("\n=== Temporary files created ===\n");
    system("find . -name '*.i' -o -name '*.s' -o -name '*.o' -o -name '*.dump*' | sort");
    
    printf("\n=== Cleanup? Run: rm -rf testdump otherdump *.i *.s *.o ===\n");
    
    return ret;
}

/* test.c */
int foo(void) { return 42; }

/* test2.c */
int bar(void) { return 43; }
