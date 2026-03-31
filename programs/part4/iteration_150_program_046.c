/* main.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int execute_command(const char *cmd) {
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed: %s\n", cmd);
        return -1;
    }
    return 0;
}

int main() {
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
    
    printf("=== GCC Help (first 5 lines) ===\n");
    execute_command("gcc --help 2>&1 | head -5");
    
    printf("\n=== GCC Version ===\n");
    execute_command("gcc --version");
    
    printf("\n=== Compiling test.c with verbose output ===\n");
    execute_command("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    
    printf("\n=== Compiling test2.c ===\n");
    execute_command("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1");
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    execute_command("rm -f test.c test2.c *.o *.i *.s testdump/* otherdump/* 2>/dev/null");
    execute_command("rmdir testdump otherdump 2>/dev/null");
    
    return 0;
}
