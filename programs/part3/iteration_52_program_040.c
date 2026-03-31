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
        perror("fopen");
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

/* Execute GCC with the given arguments using fork/exec */
static int run_gcc(const char **argv) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        execvp("gcc", (char * const *)argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    
    /* Parent process */
    int status;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return -1;
    }
    
    if (WIFEXITED(status)) {
        fprintf(stderr, "GCC exited with status %d\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        fprintf(stderr, "GCC terminated by signal %d\n", WTERMSIG(status));
        return -1;
    }
    
    return 0;
}

/* Build and execute a GCC command with specific options */
static int execute_gcc_test(const char *description, const char *source_file, 
                           const char *output_file, const char *dumpdir,
                           const char *dumpbase, const char *dumpbase_ext,
                           int save_temps, int use_o_option) {
    
    fprintf(stderr, "\n=== %s ===\n", description);
    
    /* Maximum arguments for safety */
    const char *argv[20];
    int argc = 0;
    
    argv[argc++] = "gcc";
    
    /* Add dump options if provided */
    if (dumpdir) {
        argv[argc++] = "-dumpdir";
        argv[argc++] = dumpdir;
    }
    
    if (dumpbase) {
        argv[argc++] = "-dumpbase";
        argv[argc++] = dumpbase;
    }
    
    if (dumpbase_ext) {
        argv[argc++] = "-dumpbase-ext";
        argv[argc++] = dumpbase_ext;
    }
    
    /* Add save-temps option */
    if (save_temps) {
        argv[argc++] = "-save-temps";
    }
    
    /* Add compilation mode */
    argv[argc++] = "-c";
    argv[argc++] = source_file;
    
    /* Add output option if requested */
    if (use_o_option && output_file) {
        argv[argc++] = "-o";
        argv[argc++] = output_file;
    }
    
    /* Add optimization and debug flags for the test program itself */
    argv[argc++] = "-O0";
    argv[argc++] = "-g";
    
    /* Terminate argument list */
    argv[argc] = NULL;
    
    /* Print the command for traceability */
    fprintf(stderr, "Executing: ");
    for (int i = 0; i < argc; i++) {
        fprintf(stderr, "%s ", argv[i]);
    }
    fprintf(stderr, "\n");
    
    return run_gcc(argv);
}

int main(void) {
    int overall_status = 0;
    
    /* Step 1: Create temporary source file */
    if (create_test_source(TEMP_SOURCE_FILE) < 0) {
        fprintf(stderr, "Failed to create test source file\n");
        return EXIT_FAILURE;
    }
    
    fprintf(stderr, "Created test source file: %s\n", TEMP_SOURCE_FILE);
    
    /* Step 2: Execute multiple GCC invocations with different option combinations */
    
    /* Test 1: With dumpdir and save-temps */
    if (execute_gcc_test("Test 1: dumpdir + save-temps", 
                        TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE,
                        "./dump/", NULL, NULL, 1, 1) != 0) {
        fprintf(stderr, "Test 1 completed (may have warnings)\n");
    }
    
    /* Test 2: With dumpbase and dumpbase-ext but no dumpdir */
    if (execute_gcc_test("Test 2: dumpbase + dumpbase-ext", 
                        TEMP_SOURCE_FILE, "output2.o",
                        NULL, "myprog", ".c", 0, 1) != 0) {
        fprintf(stderr, "Test 2 completed (may have warnings)\n");
    }
    
    /* Test 3: All options combined */
    if (execute_gcc_test("Test 3: All dump options + save-temps", 
                        TEMP_SOURCE_FILE, "output3.o",
                        "./full_dump/", "fullprog", ".ext",
                        1, 1) != 0) {
        fprintf(stderr, "Test 3 completed (may have warnings)\n");
    }
    
    /* Test 4: Different dumpdir format (trailing slash) */
    if (execute_gcc_test("Test 4: dumpdir with trailing dash", 
                        TEMP_SOURCE_FILE, "output4.o",
                        "./dash-", "dashbase", NULL, 1, 1) != 0) {
        fprintf(stderr, "Test 4 completed (may have warnings)\n");
    }
    
    /* Test 5: Without -o option to test default outbase behavior */
    if (execute_gcc_test("Test 5: No -o option", 
                        TEMP_SOURCE_FILE, NULL,
                        "./no_o/", "nobase", ".c", 0, 0) != 0) {
        fprintf(stderr, "Test 5 completed (may have warnings)\n");
    }
    
    /* Test 6: Trigger error exit to test cleanup after error */
    fprintf(stderr, "\n=== Test 6: Invalid option to test error cleanup ===\n");
    {
        const char *argv[] = {
            "gcc", "-invalid-opt", "-c", TEMP_SOURCE_FILE, "-o", "error.o", NULL
        };
        fprintf(stderr, "Executing: ");
        for (int i = 0; argv[i]; i++) {
            fprintf(stderr, "%s ", argv[i]);
        }
        fprintf(stderr, "\n");
        run_gcc(argv);
        fprintf(stderr, "Expected error - testing cleanup after error exit\n");
    }
    
    /* Step 3: Clean up temporary files */
    fprintf(stderr, "\nCleaning up temporary files...\n");
    
    if (unlink(TEMP_SOURCE_FILE) < 0 && errno != ENOENT) {
        perror("unlink source file");
    } else {
        fprintf(stderr, "Removed: %s\n", TEMP_SOURCE_FILE);
    }
    
    /* Clean up potential output files */
    const char *output_files[] = {
        TEMP_OUTPUT_FILE, "output2.o", "output3.o", "output4.o", "error.o",
        "test_coverage_input.i", "test_coverage_input.s",
        "myprog.i", "myprog.s", "fullprog.i", "fullprog.s",
        "dashbase.i", "dashbase.s", "nobase.i", "nobase.s",
        NULL
    };
    
    for (int i = 0; output_files[i]; i++) {
        if (unlink(output_files[i]) == 0) {
            fprintf(stderr, "Removed: %s\n", output_files[i]);
        }
    }
    
    /* Clean up dump directories if created */
    system("rm -rf dump full_dump no_o dash- 2>/dev/null");
    
    fprintf(stderr, "\nAll tests completed. Check GCC driver coverage.\n");
    return overall_status;
}
