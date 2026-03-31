/* main.c */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
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
    execute_command("mkdir -p ./testdump");
    execute_command("mkdir -p ./otherdump");
    
    /* Invoke gcc with help flag */
    execute_command("gcc --help 2>&1 | head -5");
    
    /* Invoke gcc with version flag */
    execute_command("gcc --version");
    
    /* Compile a simple file with save-temps and dumpdir flags */
    execute_command("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    
    /* Compile another file with different dumpdir to trigger cleanup */
    execute_command("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1");
    
    /* Optional: Clean up temporary files */
    printf("\nTemporary files created:\n");
    execute_command("find ./testdump ./otherdump -type f -name '*.i' -o -name '*.s' -o -name '*.o' 2>/dev/null");
    
    /* Ask user if they want to clean up */
    printf("\nClean up temporary files? (y/n): ");
    char response;
    scanf("%c", &response);
    if (response == 'y' || response == 'Y') {
        execute_command("rm -rf ./testdump ./otherdump");
        execute_command("rm -f test.o test2.o mytest.* other.*");
    }
    
    return 0;
}
