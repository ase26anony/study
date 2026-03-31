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

int file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

int main() {
    int all_ok = 1;
    
    /* Create test files if they don't exist */
    if (!file_exists("test.c")) {
        FILE *f = fopen("test.c", "w");
        if (f) {
            fprintf(f, "int foo(void) { return 42; }\n");
            fclose(f);
        }
    }
    
    if (!file_exists("test2.c")) {
        FILE *f = fopen("test2.c", "w");
        if (f) {
            fprintf(f, "int bar(void) { return 43; }\n");
            fclose(f);
        }
    }
    
    /* Create dump directories */
    system("mkdir -p ./testdump ./otherdump");
    
    /* Invoke gcc with help flag */
    if (execute_command("gcc --help 2>&1 | head -5") != 0) {
        fprintf(stderr, "gcc not found or failed\n");
        all_ok = 0;
    }
    
    /* Invoke gcc with version flag */
    execute_command("gcc --version");
    
    /* Compile a simple file with save-temps and dumpdir flags */
    if (execute_command("gcc -save-temps -dumpdir ./testdump/ -dumpbase mytest -c test.c -v 2>&1 | tail -10") != 0) {
        all_ok = 0;
    }
    
    /* Compile another file with different dumpdir to trigger cleanup */
    if (execute_command("gcc -dumpdir ./otherdump/ -dumpbase other -c test2.c 2>&1") != 0) {
        all_ok = 0;
    }
    
    /* Optional: Clean up temporary files */
    printf("\nTemporary files created:\n");
    system("find . -name '*.i' -o -name '*.s' -o -name '*.o' 2>/dev/null | sort");
    
    /* Optional cleanup - uncomment if desired */
    /*
    printf("\nCleaning up...\n");
    system("rm -f test.o test2.o mytest.* other.*");
    system("rm -rf testdump otherdump");
    */
    
    return all_ok ? 0 : 1;
}
