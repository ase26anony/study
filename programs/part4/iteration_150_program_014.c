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

int main() {
    int all_ok = 1;
    
    /* Create necessary directories */
    mkdir("./testdump", 0755);
    mkdir("./otherdump", 0755);
    
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
    if (execute_command("gcc --help 2>&1 | head -5") != 0) all_ok = 0;
    
    /* Invoke gcc with version flag */
    if (execute_command("gcc --version") != 0) all_ok = 0;
    
    /* Compile a simple file with save-temps and dumpdir flags */
    if (execute_command("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10") != 0) all_ok = 0;
    
    /* Compile another file with different dumpdir to trigger cleanup */
    if (execute_command("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1") != 0) all_ok = 0;
    
    /* Cleanup temporary files */
    printf("\nCleaning up temporary files...\n");
    system("rm -f test.c test2.c mytest.* other.*");
    system("rm -rf testdump otherdump");
    
    return all_ok ? 0 : 1;
}
