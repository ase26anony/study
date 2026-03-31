/* main.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

int run_command(const char *cmd) {
    int status = system(cmd);
    if (status == -1) {
        fprintf(stderr, "Failed to execute command: %s\n", strerror(errno));
        return -1;
    } else if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

int main() {
    int ret = 0;
    
    /* Check if source files exist */
    if (access("test.c", F_OK) != 0 || access("test2.c", F_OK) != 0) {
        fprintf(stderr, "Error: test.c or test2.c not found\n");
        return 1;
    }
    
    /* Create dump directories if they don't exist */
    system("mkdir -p ./testdump ./otherdump 2>/dev/null");
    
    printf("=== GCC Help (first 5 lines) ===\n");
    ret = run_command("gcc --help 2>&1 | head -5");
    if (ret != 0) {
        fprintf(stderr, "Warning: gcc --help failed\n");
    }
    
    printf("\n=== GCC Version ===\n");
    ret = run_command("gcc --version");
    if (ret != 0) {
        fprintf(stderr, "Warning: gcc --version failed\n");
    }
    
    printf("\n=== Compiling test.c with verbose output ===\n");
    ret = run_command("gcc -save-temps -dumpdir ./testdump/ -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    if (ret != 0) {
        fprintf(stderr, "Warning: Compilation of test.c failed\n");
    }
    
    printf("\n=== Compiling test2.c ===\n");
    ret = run_command("gcc -dumpdir ./otherdump/ -dumpbase other -c test2.c 2>&1");
    if (ret != 0) {
        fprintf(stderr, "Warning: Compilation of test2.c failed\n");
    }
    
    /* Optional: Clean up temporary files */
    printf("\n=== Temporary files created ===\n");
    system("find . -name '*.i' -o -name '*.s' -o -name '*.o' -o -name '*.dump' 2>/dev/null | head -10");
    
    return 0;
}
