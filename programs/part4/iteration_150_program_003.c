/* main.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status == -1) {
        perror("system() failed");
        return -1;
    } else if (WIFEXITED(status)) {
        printf("Command exited with status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    return -1;
}

int file_exists(const char *filename) {
    struct stat st;
    return stat(filename, &st) == 0;
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
    system("mkdir -p ./testdump ./otherdump");
    
    /* Invoke gcc with help flag */
    execute_command("gcc --help 2>&1 | head -5");
    
    /* Invoke gcc with version flag */
    execute_command("gcc --version");
    
    /* Compile a simple file with save-temps and dumpdir flags */
    execute_command("gcc -save-temps -dumpdir ./testdump/ -dumpbase mytest -c test.c 2>&1 | tail -10");
    
    /* Compile another file with different dumpdir */
    execute_command("gcc -dumpdir ./otherdump/ -dumpbase other -c test2.c 2>&1");
    
    /* Optional: Clean up temporary files */
    printf("\nTemporary files created:\n");
    system("find . -name '*.i' -o -name '*.s' -o -name '*.o' -o -name '*.ii' 2>/dev/null");
    
    /* Ask user if they want to clean up */
    printf("\nClean up temporary files? (y/n): ");
    char response = getchar();
    if (response == 'y' || response == 'Y') {
        system("rm -f *.i *.s *.o *.ii");
        system("rm -rf ./testdump ./otherdump");
        printf("Temporary files cleaned up.\n");
    }
    
    return 0;
}
