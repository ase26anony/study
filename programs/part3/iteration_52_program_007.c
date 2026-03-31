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
    printf("Command: ");
    for (int i = 0; argv[i] != NULL; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process: execute GCC */
        execvp(argv[0], argv);
        /* If execvp returns, it failed */
        perror("execvp failed");
        _exit(EXIT_FAILURE);
    } else {
        /* Parent process: wait for child */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Exit status: %d\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            printf("Terminated by signal: %d\n", WTERMSIG(status));
            return -1;
        }
    }
    
    return 0;
}

int main(void) {
    int ret = 0;
    
    /* Create test source file */
    if (create_test_source() != 0) {
        return EXIT_FAILURE;
    }
    
    printf("Created test source file: %s\n", TEMP_SOURCE_FILE);
    
    /* Test 1: Basic compilation with dump options */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump/",
            "-dumpbase", "myprog",
            "-dumpbase-ext", ".c",
            "-c", TEMP_SOURCE_FILE,
            "-o", TEMP_OUTPUT_FILE,
            NULL
        };
        run_gcc("Basic dump options", argv);
    }
    
    /* Test 2: With save-temps flag */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump2/",
            "-dumpbase", "myprog2",
            "-save-temps",
            "-c", TEMP_SOURCE_FILE,
            "-o", "test2.o",
            NULL
        };
        run_gcc("With save-temps", argv);
    }
    
    /* Test 3: With save-temps=cwd */
    {
        char *argv[] = {
            "gcc",
            "-dumpbase", "myprog3",
            "-dumpbase-ext", ".c",
            "-save-temps=cwd",
            "-c", TEMP_SOURCE_FILE,
            "-o", "test3.o",
            NULL
        };
        run_gcc("With save-temps=cwd (no dumpdir)", argv);
    }
    
    /* Test 4: All options combined */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump_all/",
            "-dumpbase", "myprog_all",
            "-dumpbase-ext", ".c",
            "-save-temps",
            "-c", TEMP_SOURCE_FILE,
            "-o", "test_all.o",
            NULL
        };
        run_gcc("All options combined", argv);
    }
    
    /* Test 5: Different dumpbase-ext */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump_ext/",
            "-dumpbase", "myprog_ext",
            "-dumpbase-ext", ".test",
            "-save-temps=cwd",
            "-c", TEMP_SOURCE_FILE,
            "-o", "test_ext.o",
            NULL
        };
        run_gcc("Different dumpbase-ext", argv);
    }
    
    /* Test 6: Without dumpdir (to test default behavior) */
    {
        char *argv[] = {
            "gcc",
            "-dumpbase", "myprog_nodir",
            "-dumpbase-ext", ".c",
            "-save-temps",
            "-c", TEMP_SOURCE_FILE,
            "-o", "test_nodir.o",
            NULL
        };
        run_gcc("Without dumpdir", argv);
    }
    
    /* Test 7: Error case with invalid option (should still trigger cleanup) */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump_err/",
            "-dumpbase", "myprog_err",
            "-invalid-opt",  /* This will cause an error */
            "-c", TEMP_SOURCE_FILE,
            "-o", "test_err.o",
            NULL
        };
        run_gcc("Error case with invalid option", argv);
    }
    
    /* Test 8: Multiple output files with different bases */
    {
        char *argv1[] = {
            "gcc",
            "-dumpdir", "./multi1/",
            "-dumpbase", "multi1",
            "-dumpbase-ext", ".c",
            "-save-temps",
            "-c", TEMP_SOURCE_FILE,
            "-o", "multi1.o",
            NULL
        };
        
        char *argv2[] = {
            "gcc",
            "-dumpdir", "./multi2/",
            "-dumpbase", "multi2",
            "-dumpbase-ext", ".c",
            "-save-temps=cwd",
            "-c", TEMP_SOURCE_FILE,
            "-o", "multi2.o",
            NULL
        };
        
        run_gcc("Multiple test 1", argv1);
        run_gcc("Multiple test 2", argv2);
    }
    
    /* Clean up temporary files */
    printf("\n=== Cleaning up ===\n");
    if (remove(TEMP_SOURCE_FILE) == 0) {
        printf("Removed %s\n", TEMP_SOURCE_FILE);
    } else {
        perror("Failed to remove source file");
    }
    
    /* Try to remove output files that might have been created */
    char *output_files[] = {
        TEMP_OUTPUT_FILE,
        "test2.o",
        "test3.o",
        "test_all.o",
        "test_ext.o",
        "test_nodir.o",
        "test_err.o",
        "multi1.o",
        "multi2.o",
        NULL
    };
    
    for (int i = 0; output_files[i] != NULL; i++) {
        if (remove(output_files[i]) == 0) {
            printf("Removed %s\n", output_files[i]);
        }
    }
    
    /* Also clean up dump directories if they exist */
    system("rm -rf ./dump/ ./dump2/ ./dump_all/ ./dump_ext/ ./dump_err/ ./multi1/ ./multi2/ 2>/dev/null");
    
    printf("\nTest program completed.\n");
    return ret;
}
