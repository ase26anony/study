#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_input.c"
#define TEMP_OUTPUT_FILE "test_coverage_output.o"

/* Create a simple valid C source file */
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

/* Execute GCC with given arguments using fork/exec */
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
    } else {
        /* Parent process */
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            return -1;
        }
        
        if (WIFEXITED(status)) {
            fprintf(stderr, "GCC exited with status %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "GCC terminated by signal %d\n", WTERMSIG(status));
        }
        
        return status;
    }
}

/* Build command line and execute GCC */
static int execute_gcc_test(const char *description, const char *extra_args) {
    fprintf(stderr, "\n=== %s ===\n", description);
    
    /* Build the command line */
    char cmd[1024];
    int len = snprintf(cmd, sizeof(cmd), 
                      "gcc -c %s %s -o %s 2>&1",
                      TEMP_SOURCE_FILE, extra_args, TEMP_OUTPUT_FILE);
    
    if (len >= (int)sizeof(cmd)) {
        fprintf(stderr, "Command too long\n");
        return -1;
    }
    
    fprintf(stderr, "Executing: %s\n", cmd);
    
    /* Execute using system() as requested */
    int status = system(cmd);
    if (status == -1) {
        perror("system");
        return -1;
    }
    
    return status;
}

int main(void) {
    int overall_status = 0;
    
    /* Create test source file */
    if (create_test_source(TEMP_SOURCE_FILE) < 0) {
        fprintf(stderr, "Failed to create test source file\n");
        return EXIT_FAILURE;
    }
    
    fprintf(stderr, "Created test source file: %s\n", TEMP_SOURCE_FILE);
    
    /* Test 1: Basic compilation with dump options */
    fprintf(stderr, "\n--- Test 1: Basic dump options ---\n");
    const char *test1_args[] = {
        "gcc",
        "-dumpdir", "./dumpdir_test/",
        "-dumpbase", "myprog",
        "-dumpbase-ext", ".c",
        "-c",
        TEMP_SOURCE_FILE,
        "-o", "test1_output.o",
        NULL
    };
    
    fprintf(stderr, "Executing: ");
    for (int i = 0; test1_args[i]; i++) {
        fprintf(stderr, "%s ", test1_args[i]);
    }
    fprintf(stderr, "\n");
    
    if (run_gcc(test1_args) != 0) {
        fprintf(stderr, "Test 1 completed (status may be non-zero)\n");
    }
    
    /* Test 2: Save temps with dumpbase variations */
    int status2 = execute_gcc_test(
        "Test 2: Save temps with dumpbase",
        "-save-temps -dumpbase myprog2 -dumpbase-ext .c"
    );
    if (status2 != 0) {
        fprintf(stderr, "Test 2 completed with status %d\n", status2);
    }
    
    /* Test 3: All options combined */
    int status3 = execute_gcc_test(
        "Test 3: All dump options with save-temps",
        "-dumpdir ./dump_all/ -dumpbase fulltest -dumpbase-ext .c "
        "-save-temps=cwd -fdump-tree-all -fdump-rtl-all"
    );
    if (status3 != 0) {
        fprintf(stderr, "Test 3 completed with status %d\n", status3);
    }
    
    /* Test 4: Different output base with dumpdir */
    int status4 = execute_gcc_test(
        "Test 4: Custom output and dumpdir",
        "-dumpdir ./varied_dump/ -dumpbase varied -o custom_output.o "
        "-save-temps"
    );
    if (status4 != 0) {
        fprintf(stderr, "Test 4 completed with status %d\n", status4);
    }
    
    /* Test 5: No dumpdir, only dumpbase and extension */
    int status5 = execute_gcc_test(
        "Test 5: Only dumpbase and dumpbase-ext",
        "-dumpbase nodir -dumpbase-ext .c -save-temps=cwd"
    );
    if (status5 != 0) {
        fprintf(stderr, "Test 5 completed with status %d\n", status5);
    }
    
    /* Test 6: Error case - invalid option (should still trigger cleanup) */
    fprintf(stderr, "\n--- Test 6: Error case with invalid option ---\n");
    const char *test6_args[] = {
        "gcc",
        "-dumpdir", "./error_dump/",
        "-dumpbase", "error",
        "-dumpbase-ext", ".c",
        "-invalid-option-that-does-not-exist",  /* This will cause error */
        "-c",
        TEMP_SOURCE_FILE,
        "-o", "test6_output.o",
        NULL
    };
    
    fprintf(stderr, "Executing (expecting error): ");
    for (int i = 0; test6_args[i]; i++) {
        fprintf(stderr, "%s ", test6_args[i]);
    }
    fprintf(stderr, "\n");
    
    if (run_gcc(test6_args) != 0) {
        fprintf(stderr, "Test 6 completed with error (as expected)\n");
    }
    
    /* Test 7: Using -o to influence outbase */
    fprintf(stderr, "\n--- Test 7: Explicit output name for outbase ---\n");
    const char *test7_args[] = {
        "gcc",
        "-dumpdir", "./outbase_test/",
        "-dumpbase", "outbase_prog",
        "-dumpbase-ext", ".c",
        "-save-temps",
        "-c",
        TEMP_SOURCE_FILE,
        "-o", "explicit_outbase_name.o",  /* This should set outbase */
        NULL
    };
    
    fprintf(stderr, "Executing: ");
    for (int i = 0; test7_args[i]; i++) {
        fprintf(stderr, "%s ", test7_args[i]);
    }
    fprintf(stderr, "\n");
    
    if (run_gcc(test7_args) != 0) {
        fprintf(stderr, "Test 7 completed\n");
    }
    
    /* Cleanup temporary files */
    fprintf(stderr, "\n--- Cleaning up temporary files ---\n");
    
    if (unlink(TEMP_SOURCE_FILE) == 0) {
        fprintf(stderr, "Removed: %s\n", TEMP_SOURCE_FILE);
    } else {
        perror("unlink source");
    }
    
    /* Try to remove any generated output files */
    const char *output_files[] = {
        "test1_output.o",
        TEMP_OUTPUT_FILE,
        "custom_output.o",
        "test6_output.o",
        "explicit_outbase_name.o",
        NULL
    };
    
    for (int i = 0; output_files[i]; i++) {
        if (unlink(output_files[i]) == 0) {
            fprintf(stderr, "Removed: %s\n", output_files[i]);
        }
    }
    
    /* Also clean up any dump directories created */
    system("rm -rf ./dumpdir_test/ ./dump_all/ ./varied_dump/ ./error_dump/ ./outbase_test/ 2>/dev/null");
    
    /* Clean up save-temps files */
    const char *temp_files[] = {
        "test_coverage_input.i",
        "test_coverage_input.s",
        "myprog2.i",
        "myprog2.s",
        "fulltest.i",
        "fulltest.s",
        "varied.i",
        "varied.s",
        "nodir.i",
        "nodir.s",
        "outbase_prog.i",
        "outbase_prog.s",
        NULL
    };
    
    for (int i = 0; temp_files[i]; i++) {
        if (unlink(temp_files[i]) == 0) {
            fprintf(stderr, "Removed: %s\n", temp_files[i]);
        }
    }
    
    fprintf(stderr, "\nAll tests completed. The GCC driver cleanup logic should have been triggered.\n");
    
    return overall_status;
}
