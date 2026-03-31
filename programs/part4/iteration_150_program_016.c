/* main.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

int execute_command(const char *cmd) {
    int status = system(cmd);
    if (status == -1) {
        perror("system");
        return -1;
    } else if (WIFEXITED(status)) {
        printf("Command exited with status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    } else {
        printf("Command terminated abnormally\n\n");
        return -1;
    }
}

int ensure_directory(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) == -1) {
            perror("mkdir");
            return -1;
        }
    }
    return 0;
}

int main() {
    int ret = 0;
    
    /* Create dump directories if they don't exist */
    if (ensure_directory("./testdump") != 0 ||
        ensure_directory("./otherdump") != 0) {
        fprintf(stderr, "Failed to create dump directories\n");
        return EXIT_FAILURE;
    }
    
    /* Create test source files if they don't exist */
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
    
    printf("=== Testing gcc --help (first 5 lines) ===\n");
    ret |= execute_command("gcc --help 2>&1 | head -5");
    
    printf("=== Testing gcc --version ===\n");
    ret |= execute_command("gcc --version");
    
    printf("=== Compiling test.c with save-temps ===\n");
    ret |= execute_command("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    
    printf("=== Compiling test2.c with different dumpdir ===\n");
    ret |= execute_command("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1");
    
    /* Cleanup */
    printf("=== Cleaning up generated files ===\n");
    system("rm -f test.c test2.c testdump/* otherdump/* 2>/dev/null");
    
    return ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
