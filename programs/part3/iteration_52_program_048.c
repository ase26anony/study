#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_coverage.c"
#define TEMP_OUTPUT_FILE "test_gcc_coverage.o"

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
    fprintf(fp, "    printf(\"Hello from test program\\n\");\n");
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
    /* Create test source file */
    if (create_test_source() < 0) {
        return EXIT_FAILURE;
    }
    
    /* Array to hold GCC command arguments */
    char *gcc_argv[20];
    int arg_count;
    
    /* Test 1: Basic compilation with dump options */
    printf("\n--- Test 1: Basic dump options ---");
    arg_count = 0;
    gcc_argv[arg_count++] = "gcc";
    gcc_argv[arg_count++] = "-dumpdir";
    gcc_argv[arg_count++] = "./dump/";
    gcc_argv[arg_count++] = "-dumpbase";
    gcc_argv[arg_count++] = "myprog";
    gcc_argv[arg_count++] = "-dumpbase-ext";
    gcc_argv[arg_count++] = ".c";
    gcc_argv[arg_count++] = "-c";
    gcc_argv[arg_count++] = TEMP_SOURCE_FILE;
    gcc_argv[arg_count++] = "-o";
    gcc_argv[arg_count++] = TEMP_OUTPUT_FILE;
    gcc_argv[arg_count] = NULL;
    
    run_gcc("Basic dump options", gcc_argv);
    
    /* Test 2: With save-temps and different dumpdir */
    printf("\n--- Test 2: With save-temps ---");
    arg_count = 0;
    gcc_argv[arg_count++] = "gcc";
    gcc_argv[arg_count++] = "-dumpdir";
    gcc_argv[arg_count++] = "temp_dump/";
    gcc_argv[arg_count++] = "-dumpbase";
    gcc_argv[arg_count++] = "test_save";
    gcc_argv[arg_count++] = "-dumpbase-ext";
    gcc_argv[arg_count++] = ".c";
    gcc_argv[arg_count++] = "-save-temps";
    gcc_argv[arg_count++] = "-c";
    gcc_argv[arg_count++] = TEMP_SOURCE_FILE;
    gcc_argv[arg_count++] = "-o";
    gcc_argv[arg_count++] = "test_save.o";
    gcc_argv[arg_count] = NULL;
    
    run_gcc("With save-temps", gcc_argv);
    
    /* Test 3: With save-temps=cwd and no dumpdir */
    printf("\n--- Test 3: save-temps=cwd without dumpdir ---");
    arg_count = 0;
    gcc_argv[arg_count++] = "gcc";
    gcc_argv[arg_count++] = "-dumpbase";
    gcc_argv[arg_count++] = "test_cwd";
    gcc_argv[arg_count++] = "-dumpbase-ext";
    gcc_argv[arg_count++] = ".c";
    gcc_argv[arg_count++] = "-save-temps=cwd";
    gcc_argv[arg_count++] = "-c";
    gcc_argv[arg_count++] = TEMP_SOURCE_FILE;
    gcc_argv[arg_count++] = "-o";
    gcc_argv[arg_count++] = "test_cwd.o";
    gcc_argv[arg_count] = NULL;
    
    run_gcc("save-temps=cwd without dumpdir", gcc_argv);
    
    /* Test 4: All options combined */
    printf("\n--- Test 4: All options combined ---");
    arg_count = 0;
    gcc_argv[arg_count++] = "gcc";
    gcc_argv[arg_count++] = "-dumpdir";
    gcc_argv[arg_count++] = "full_dump/";
    gcc_argv[arg_count++] = "-dumpbase";
    gcc_argv[arg_count++] = "full_test";
    gcc_argv[arg_count++] = "-dumpbase-ext";
    gcc_argv[arg_count++] = ".c";
    gcc_argv[arg_count++] = "-save-temps";
    gcc_argv[arg_count++] = "-c";
    gcc_argv[arg_count++] = TEMP_SOURCE_FILE;
    gcc_argv[arg_count++] = "-o";
    gcc_argv[arg_count++] = "full_test_output.o";
    gcc_argv[arg_count] = NULL;
    
    run_gcc("All options combined", gcc_argv);
    
    /* Test 5: Invalid option to test cleanup after error */
    printf("\n--- Test 5: Invalid option (testing error cleanup) ---");
    arg_count = 0;
    gcc_argv[arg_count++] = "gcc";
    gcc_argv[arg_count++] = "-dumpdir";
    gcc_argv[arg_count++] = "error_dump/";
    gcc_argv[arg_count++] = "-dumpbase";
    gcc_argv[arg_count++] = "error_test";
    gcc_argv[arg_count++] = "-invalid-opt";
    gcc_argv[arg_count++] = "-c";
    gcc_argv[arg_count++] = TEMP_SOURCE_FILE;
    gcc_argv[arg_count] = NULL;
    
    run_gcc("Invalid option test", gcc_argv);
    
    /* Test 6: Different output base name */
    printf("\n--- Test 6: Different output base ---");
    arg_count = 0;
    gcc_argv[arg_count++] = "gcc";
    gcc_argv[arg_count++] = "-dumpdir";
    gcc_argv[arg_count++] = "outbase_dump/";
    gcc_argv[arg_count++] = "-dumpbase";
    gcc_argv[arg_count++] = "custom_base";
    gcc_argv[arg_count++] = "-dumpbase-ext";
    gcc_argv[arg_count++] = ".c";
    gcc_argv[arg_count++] = "-save-temps";
    gcc_argv[arg_count++] = "-c";
    gcc_argv[arg_count++] = TEMP_SOURCE_FILE;
    gcc_argv[arg_count++] = "-o";
    gcc_argv[arg_count++] = "custom_output_name.o";
    gcc_argv[arg_count] = NULL;
    
    run_gcc("Different output base", gcc_argv);
    
    /* Clean up temporary files */
    printf("\n--- Cleaning up temporary files ---\n");
    remove(TEMP_SOURCE_FILE);
    remove(TEMP_OUTPUT_FILE);
    remove("test_save.o");
    remove("test_cwd.o");
    remove("full_test_output.o");
    remove("custom_output_name.o");
    
    /* Also clean up any .i, .s files created by -save-temps */
    remove("test_gcc_coverage.i");
    remove("test_gcc_coverage.s");
    remove("myprog.i");
    remove("myprog.s");
    remove("test_save.i");
    remove("test_save.s");
    remove("test_cwd.i");
    remove("test_cwd.s");
    remove("full_test.i");
    remove("full_test.s");
    remove("custom_base.i");
    remove("custom_base.s");
    
    printf("\nAll tests completed. Check GCC driver coverage.\n");
    return 0;
}
