/* main.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

int run_command(const char *cmd) {
    printf("Running: %s\n", cmd);
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

void cleanup_temp_files(void) {
    // Clean up temporary files created by gcc
    system("rm -f testdump.* otherdump.* mytest.* other.* 2>/dev/null");
    system("rm -f test.i test.s test.o test2.i test2.s test2.o 2>/dev/null");
}

int main() {
    int ret = 0;
    
    // Check if source files exist
    if (!file_exists("test.c")) {
        fprintf(stderr, "Error: test.c not found\n");
        return 1;
    }
    
    if (!file_exists("test2.c")) {
        fprintf(stderr, "Error: test2.c not found\n");
        return 1;
    }
    
    // Register cleanup function
    atexit(cleanup_temp_files);
    
    /* Invoke gcc with help flag */
    if (run_command("gcc --help 2>&1 | head -5") != 0) {
        fprintf(stderr, "Warning: gcc help command failed\n");
    }
    
    /* Invoke gcc with version flag */
    if (run_command("gcc --version") != 0) {
        fprintf(stderr, "Warning: gcc version command failed\n");
    }
    
    /* Compile a simple file with save-temps and dumpdir flags */
    printf("Compiling test.c with save-temps...\n");
    if (run_command("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10") != 0) {
        fprintf(stderr, "Warning: test.c compilation failed\n");
        ret = 1;
    }
    
    /* Compile another file with different dumpdir to trigger cleanup */
    printf("Compiling test2.c...\n");
    if (run_command("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1") != 0) {
        fprintf(stderr, "Warning: test2.c compilation failed\n");
        ret = 1;
    }
    
    return ret;
}
