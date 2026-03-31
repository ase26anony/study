/* main.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed with status %d: %s\n", status, cmd);
    }
    printf("\n---\n\n");
    return status;
}

int main() {
    int all_success = 0;
    
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
    
    /* Invoke gcc with help flag */
    all_success |= execute_command("gcc --help 2>&1 | head -5");
    
    /* Invoke gcc with version flag */
    all_success |= execute_command("gcc --version");
    
    /* Compile a simple file with save-temps and dumpdir flags */
    all_success |= execute_command("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    
    /* Compile another file with different dumpdir to trigger cleanup */
    all_success |= execute_command("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1");
    
    /* Optional: Clean up temporary files */
    if (all_success == 0) {
        printf("Cleaning up...\n");
        system("rm -f test.c test2.c mytest.* testdump/* otherdump/* 2>/dev/null");
        system("rmdir testdump otherdump 2>/dev/null");
    }
    
    return all_success;
}
