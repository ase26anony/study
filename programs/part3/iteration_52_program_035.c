#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_input.c"
#define TEMP_OBJECT_FILE "test_coverage_output.o"

/* Create a simple valid C source file */
static int create_test_source(void) {
    FILE *fp = fopen(TEMP_SOURCE_FILE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(fp, "/* Test file for GCC coverage */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    printf(\"Hello from coverage test!\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/* Execute GCC with given arguments using fork/exec */
static int run_gcc(const char *description, char *const argv[]) {
    printf("\n=== Running: %s ===\n", description);
    
    /* Print the command for traceability */
    fprintf(stderr, "Command: ");
    for (int i = 0; argv[i] != NULL; i++) {
        fprintf(stderr, "%s ", argv[i]);
    }
    fprintf(stderr, "\n");
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        execvp(argv[0], argv);
        /* If execvp returns, it failed */
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid failed");
            return -1;
        }
        
        if (WIFEXITED(status)) {
            printf("Exit status: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Terminated by signal: %d\n", WTERMSIG(status));
        }
        
        return 0;
    }
}

int main(void) {
    /* Clean up any previous temporary files */
    unlink(TEMP_SOURCE_FILE);
    unlink(TEMP_OBJECT_FILE);
    
    /* Create the test source file */
    if (create_test_source() < 0) {
        return EXIT_FAILURE;
    }
    
    /* Array to hold GCC command arguments */
    char *gcc_argv[20];
    int arg_index;
    
    /* Test 1: Basic compilation with dumpdir and save-temps */
    printf("\n--- Test 1: Basic with dumpdir and save-temps ---");
    arg_index = 0;
    gcc_argv[arg_index++] = "gcc";
    gcc_argv[arg_index++] = "-dumpdir";
    gcc_argv[arg_index++] = "./dump/";
    gcc_argv[arg_index++] = "-dumpbase";
    gcc_argv[arg_index++] = "myprog";
    gcc_argv[arg_index++] = "-dumpbase-ext";
    gcc_argv[arg_index++] = ".c";
    gcc_argv[arg_index++] = "-save-temps";
    gcc_argv[arg_index++] = "-c";
    gcc_argv[arg_index++] = TEMP_SOURCE_FILE;
    gcc_argv[arg_index++] = "-o";
    gcc_argv[arg_index++] = TEMP_OBJECT_FILE;
    gcc_argv[arg_index++] = NULL;
    
    if (run_gcc("Test 1: Basic dump options with save-temps", gcc_argv) < 0) {
        fprintf(stderr, "Test 1 failed\n");
    }
    
    /* Clean up object file for next test */
    unlink(TEMP_OBJECT_FILE);
    
    /* Test 2: Different combination - dumpbase and dumpbase-ext without dumpdir */
    printf("\n--- Test 2: dumpbase and dumpbase-ext without dumpdir ---");
    arg_index = 0;
    gcc_argv[arg_index++] = "gcc";
    gcc_argv[arg_index++] = "-dumpbase";
    gcc_argv[arg_index++] = "test2";
    gcc_argv[arg_index++] = "-dumpbase-ext";
    gcc_argv[arg_index++] = ".c";
    gcc_argv[arg_index++] = "-save-temps=cwd";
    gcc_argv[arg_index++] = "-c";
    gcc_argv[arg_index++] = TEMP_SOURCE_FILE;
    gcc_argv[arg_index++] = "-o";
    gcc_argv[arg_index++] = "test2_output.o";
    gcc_argv[arg_index++] = NULL;
    
    if (run_gcc("Test 2: dumpbase/ext with save-temps=cwd", gcc_argv) < 0) {
        fprintf(stderr, "Test 2 failed\n");
    }
    
    /* Clean up */
    unlink("test2_output.o");
    
    /* Test 3: All options combined with different output name */
    printf("\n--- Test 3: All options combined ---");
    arg_index = 0;
    gcc_argv[arg_index++] = "gcc";
    gcc_argv[arg_index++] = "-dumpdir";
    gcc_argv[arg_index++] = "dumpdir_test/";
    gcc_argv[arg_index++] = "-dumpbase";
    gcc_argv[arg_index++] = "all_options";
    gcc_argv[arg_index++] = "-dumpbase-ext";
    gcc_argv[arg_index++] = ".c";
    gcc_argv[arg_index++] = "-save-temps";
    gcc_argv[arg_index++] = "-c";
    gcc_argv[arg_index++] = TEMP_SOURCE_FILE;
    gcc_argv[arg_index++] = "-o";
    gcc_argv[arg_index++] = "all_options.o";
    gcc_argv[arg_index++] = NULL;
    
    if (run_gcc("Test 3: All dump options with save-temps", gcc_argv) < 0) {
        fprintf(stderr, "Test 3 failed\n");
    }
    
    /* Clean up */
    unlink("all_options.o");
    
    /* Test 4: Trigger cleanup after error exit */
    printf("\n--- Test 4: Error case to test cleanup ---");
    arg_index = 0;
    gcc_argv[arg_index++] = "gcc";
    gcc_argv[arg_index++] = "-dumpdir";
    gcc_argv[arg_index++] = "./error_dump/";
    gcc_argv[arg_index++] = "-dumpbase";
    gcc_argv[arg_index++] = "error_test";
    gcc_argv[arg_index++] = "-invalid-opt";  /* This will cause an error */
    gcc_argv[arg_index++] = "-c";
    gcc_argv[arg_index++] = TEMP_SOURCE_FILE;
    gcc_argv[arg_index++] = NULL;
    
    if (run_gcc("Test 4: Error case with dump options", gcc_argv) < 0) {
        fprintf(stderr, "Test 4 failed (expected due to invalid option)\n");
    }
    
    /* Test 5: Minimal case with just -o to affect outbase */
    printf("\n--- Test 5: Minimal case affecting outbase ---");
    arg_index = 0;
    gcc_argv[arg_index++] = "gcc";
    gcc_argv[arg_index++] = "-dumpbase";
    gcc_argv[arg_index++] = "minimal";
    gcc_argv[arg_index++] = "-c";
    gcc_argv[arg_index++] = TEMP_SOURCE_FILE;
    gcc_argv[arg_index++] = "-o";
    gcc_argv[arg_index++] = "custom_output_name.o";
    gcc_argv[arg_index++] = NULL;
    
    if (run_gcc("Test 5: Minimal with custom output name", gcc_argv) < 0) {
        fprintf(stderr, "Test 5 failed\n");
    }
    
    /* Clean up temporary files */
    unlink(TEMP_SOURCE_FILE);
    unlink("custom_output_name.o");
    
    /* Clean up any dump files that might have been created */
    system("rm -rf ./dump/ dumpdir_test/ ./error_dump/ 2>/dev/null");
    
    printf("\n=== All tests completed ===\n");
    return EXIT_SUCCESS;
}
