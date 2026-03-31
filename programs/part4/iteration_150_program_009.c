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
    printf("\n---\n\n");
    return status;
}

int ensure_directory(const char *dir) {
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        if (mkdir(dir, 0755) == -1) {
            perror("mkdir failed");
            return -1;
        }
    }
    return 0;
}

int main() {
    /* Ensure source files exist */
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
    
    /* Create dump directories if they don't exist */
    ensure_directory("./testdump");
    ensure_directory("./otherdump");
    
    /* Invoke gcc with help flag */
    execute_command("gcc --help 2>&1 | head -5");
    
    /* Invoke gcc with version flag */
    execute_command("gcc --version");
    
    /* Compile a simple file with save-temps and dumpdir flags */
    execute_command("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    
    /* Compile another file with different dumpdir */
    execute_command("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1");
    
    /* Optional: Cleanup temporary files */
    printf("Temporary files created:\n");
    system("ls -la *.i *.s *.o 2>/dev/null || echo 'No temporary files found'");
    
    return 0;
}
