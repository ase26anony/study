#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_input.c"
#define TEMP_OUTPUT_FILE "test_coverage_output.o"

/* Create a simple valid C source file for compilation */
static int create_test_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
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

/* Execute GCC with specific options using fork/exec */
static int run_gcc_with_options(const char *options) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process - execute GCC */
        fprintf(stderr, "Executing: gcc %s\n", options);
        
        /* Parse the options string into arguments */
        char *argv[64];
        int argc = 0;
        char *options_copy = strdup(options);
        char *token = strtok(options_copy, " ");
        
        argv[argc++] = "gcc";
        
        while (token != NULL && argc < 63) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;
        
        execvp("gcc", argv);
        
        /* If execvp returns, it failed */
        perror("execvp failed");
        free(options_copy);
        exit(EXIT_FAILURE);
    } else {
        /* Parent process - wait for child */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            fprintf(stderr, "GCC exited with status: %d\n\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else {
            fprintf(stderr, "GCC terminated abnormally\n\n");
            return -1;
        }
    }
}

int main(void) {
    int overall_status = 0;
    
    /* Step 1: Create a temporary valid C source file */
    if (create_test_source(TEMP_SOURCE_FILE) != 0) {
        return EXIT_FAILURE;
    }
    
    fprintf(stderr, "=== Starting GCC coverage test ===\n\n");
    
    /* Step 2: Multiple distinct GCC invocations with varying options */
    
    /* Test 1: Basic compilation to trigger initialization */
    fprintf(stderr, "Test 1: Basic compilation\n");
    overall_status |= run_gcc_with_options("-c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 2: With dumpdir and save-temps (triggers dumpdir allocation) */
    fprintf(stderr, "Test 2: With -dumpdir and -save-temps\n");
    overall_status |= run_gcc_with_options(
        "-dumpdir ./dumpdir_test/ "
        "-save-temps=cwd "
        "-c " TEMP_SOURCE_FILE " "
        "-o " TEMP_OUTPUT_FILE
    );
    
    /* Test 3: With dumpbase and dumpbase-ext (no dumpdir) */
    fprintf(stderr, "Test 3: With -dumpbase and -dumpbase-ext\n");
    overall_status |= run_gcc_with_options(
        "-dumpbase my_test_program "
        "-dumpbase-ext .c "
        "-c " TEMP_SOURCE_FILE " "
        "-o " TEMP_OUTPUT_FILE
    );
    
    /* Test 4: With all options combined */
    fprintf(stderr, "Test 4: All options combined\n");
    overall_status |= run_gcc_with_options(
        "-dumpdir ./full_dump/ "
        "-dumpbase full_coverage_test "
        "-dumpbase-ext .c "
        "-save-temps "
        "-c " TEMP_SOURCE_FILE " "
        "-o custom_output_name.o"
    );
    
    /* Test 5: With outbase (implied by -o with different name) */
    fprintf(stderr, "Test 5: Different output name for outbase\n");
    overall_status |= run_gcc_with_options(
        "-dumpdir ./outbase_test/ "
        "-dumpbase outbase_test "
        "-save-temps=cwd "
        "-c " TEMP_SOURCE_FILE " "
        "-o different_output.o"
    );
    
    /* Test 6: Error case - invalid option to test cleanup after error */
    fprintf(stderr, "Test 6: Invalid option (testing cleanup after error)\n");
    overall_status |= run_gcc_with_options(
        "-dumpdir ./error_test/ "
        "-dumpbase error_test "
        "-invalid-option "
        "-c " TEMP_SOURCE_FILE
    );
    
    /* Test 7: Verbose mode to potentially trigger verbose_only_flag */
    fprintf(stderr, "Test 7: With verbose flag\n");
    overall_status |= run_gcc_with_options(
        "-v "
        "-dumpdir ./verbose_dump/ "
        "-save-temps "
        "-c " TEMP_SOURCE_FILE " "
        "-o " TEMP_OUTPUT_FILE
    );
    
    /* Test 8: Version printing (triggers print_version) */
    fprintf(stderr, "Test 8: Version request\n");
    overall_status |= run_gcc_with_options("--version");
    
    /* Test 9: Help request (triggers print_help_list) */
    fprintf(stderr, "Test 9: Help request\n");
    overall_status |= run_gcc_with_options("--help");
    
    /* Test 10: Complex combination with optimization flags */
    fprintf(stderr, "Test 10: Complex combination with optimization\n");
    overall_status |= run_gcc_with_options(
        "-dumpdir ./opt_dump/ "
        "-dumpbase optimized "
        "-dumpbase-ext .c "
        "-save-temps "
        "-O2 "
        "-c " TEMP_SOURCE_FILE " "
        "-o optimized_output.o"
    );
    
    fprintf(stderr, "=== Cleaning up temporary files ===\n");
    
    /* Clean up temporary files */
    if (remove(TEMP_SOURCE_FILE) != 0) {
        perror("Failed to remove source file");
    }
    if (remove(TEMP_OUTPUT_FILE) != 0 && errno != ENOENT) {
        perror("Failed to remove output file");
    }
    if (remove("custom_output_name.o") != 0 && errno != ENOENT) {
        perror("Failed to remove custom output file");
    }
    if (remove("different_output.o") != 0 && errno != ENOENT) {
        perror("Failed to remove different output file");
    }
    if (remove("optimized_output.o") != 0 && errno != ENOENT) {
        perror("Failed to remove optimized output file");
    }
    
    /* Clean up potential temporary files from save-temps */
    char *temp_files[] = {
        "test_coverage_input.i",
        "test_coverage_input.s",
        "my_test_program.*",
        "full_coverage_test.*",
        "outbase_test.*",
        "error_test.*",
        "optimized.*",
        NULL
    };
    
    for (int i = 0; temp_files[i] != NULL; i++) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "rm -f %s 2>/dev/null", temp_files[i]);
        system(cmd);
    }
    
    fprintf(stderr, "\n=== GCC coverage test completed ===\n");
    fprintf(stderr, "Overall status: %d\n", overall_status);
    
    return overall_status == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
