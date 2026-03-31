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
        printf("Command exited with status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    } else {
        printf("Command terminated abnormally\n\n");
        return -1;
    }
}

int main() {
    printf("=== Testing gcc --help ===\n");
    execute_command("gcc --help 2>&1 | head -5");
    
    printf("=== Testing gcc --version ===\n");
    execute_command("gcc --version");
    
    /* Create test files if they don't exist */
    FILE *fp = fopen("test.c", "w");
    if (fp) {
        fprintf(fp, "int foo(void) { return 42; }\n");
        fclose(fp);
    }
    
    fp = fopen("test2.c", "w");
    if (fp) {
        fprintf(fp, "int bar(void) { return 43; }\n");
        fclose(fp);
    }
    
    /* Create dump directories */
    system("mkdir -p ./testdump ./otherdump");
    
    printf("=== Compiling test.c with save-temps ===\n");
    execute_command("gcc -save-temps -dumpdir ./testdump/ -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    
    printf("=== Compiling test2.c ===\n");
    execute_command("gcc -dumpdir ./otherdump/ -dumpbase other -c test2.c 2>&1");
    
    /* Optional cleanup */
    printf("=== Cleaning up ===\n");
    execute_command("rm -f test.c test2.c testdump/* otherdump/* *.o *.i *.s 2>/dev/null");
    execute_command("rmdir testdump otherdump 2>/dev/null");
    
    return 0;
}
