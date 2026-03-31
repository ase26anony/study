/* main.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

int run_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed: %s\n", cmd);
    }
    return status;
}

int file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

int main() {
    /* Check if source files exist */
    if (!file_exists("test.c")) {
        fprintf(stderr, "Error: test.c not found\n");
        return 1;
    }
    
    if (!file_exists("test2.c")) {
        fprintf(stderr, "Error: test2.c not found\n");
        return 1;
    }
    
    /* Create dump directories if they don't exist */
    run_command("mkdir -p ./testdump ./otherdump");
    
    /* Invoke gcc with help flag */
    run_command("gcc --help 2>&1 | head -5");
    
    /* Invoke gcc with version flag */
    run_command("gcc --version");
    
    /* Compile a simple file with save-temps and dumpdir flags */
    run_command("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    
    /* Compile another file with different dumpdir */
    run_command("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1");
    
    /* Optional: Clean up temporary files */
    printf("\nTemporary files created:\n");
    run_command("find . -name '*.i' -o -name '*.s' -o -name '*.o' | xargs ls -la 2>/dev/null || true");
    
    /* Ask user if they want to clean up */
    printf("\nClean up temporary files? (y/n): ");
    char response;
    if (scanf(" %c", &response) == 1 && (response == 'y' || response == 'Y')) {
        run_command("rm -f *.i *.s *.o");
        run_command("rm -rf ./testdump ./otherdump");
    }
    
    return 0;
}
