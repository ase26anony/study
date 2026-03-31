/* main.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed with status %d\n", status);
    }
    return status;
}

int ensure_directory(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) != 0) {
            perror("Failed to create directory");
            return -1;
        }
    }
    return 0;
}

int main() {
    /* Create source files if they don't exist */
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
    
    /* Ensure dump directories exist */
    ensure_directory("./testdump");
    ensure_directory("./otherdump");
    
    /* Invoke gcc with help flag */
    execute_command("gcc --help 2>&1 | head -5");
    
    /* Invoke gcc with version flag */
    execute_command("gcc --version");
    
    /* Compile a simple file with save-temps and dumpdir flags */
    execute_command("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    
    /* Compile another file with different dumpdir to trigger cleanup */
    execute_command("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1");
    
    /* Clean up temporary files */
    printf("\nCleaning up...\n");
    system("rm -f test.c test2.c mytest.* other.* 2>/dev/null");
    system("rm -rf testdump otherdump 2>/dev/null");
    
    return 0;
}
