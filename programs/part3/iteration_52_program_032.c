#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_source.c"
#define TEMP_OUTPUT_FILE "test_coverage_output.o"

/* Create a simple valid C source file for compilation */
static int create_test_source(void) {
    FILE *fp = fopen(TEMP_SOURCE_FILE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(fp, "/* Test source file for GCC coverage */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    printf(\"Hello from coverage test!\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/* Clean up temporary files */
static void cleanup_files(void) {
    remove(TEMP_SOURCE_FILE);
    remove(TEMP_OUTPUT_FILE);
    /* Clean up any dump files that might have been created */
    system("rm -f ./dump/* 2>/dev/null");
    system("rmdir ./dump 2>/dev/null 2>&1");
}

/* Execute GCC with given arguments using fork/exec */
static int execute_gcc(const char *description, char *const argv[]) {
    fprintf(stderr, "\n=== Executing: %s ===\n", description);
    
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
        /* Child process: execute GCC */
        execvp("gcc", argv);
        /* If execvp returns, it failed */
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        /* Parent process: wait for child */
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid failed");
            return -1;
        }
        
        if (WIFEXITED(status)) {
            fprintf(stderr, "GCC exited with status: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "GCC terminated by signal: %d\n", WTERMSIG(status));
        }
        
        return 0;
    }
}

int main(void) {
    int ret = 0;
    
    /* Create the test source file */
    if (create_test_source() < 0) {
        return EXIT_FAILURE;
    }
    
    /* Ensure dump directory exists */
    system("mkdir -p ./dump 2>/dev/null");
    
    /* Test 1: Basic compilation with dumpdir and save-temps */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump/",
            "-dumpbase", "myprog",
            "-dumpbase-ext", ".c",
            "-save-temps",
            "-c", TEMP_SOURCE_FILE,
            "-o", TEMP_OUTPUT_FILE,
            NULL
        };
        execute_gcc("Test 1: Basic with dumpdir and save-temps", argv);
    }
    
    /* Test 2: Without dumpdir, but with dumpbase and dumpbase-ext */
    {
        char *argv[] = {
            "gcc",
            "-dumpbase", "test2",
            "-dumpbase-ext", ".c",
            "-save-temps=cwd",
            "-c", TEMP_SOURCE_FILE,
            "-o", "test2_output.o",
            NULL
        };
        execute_gcc("Test 2: Without dumpdir, with dumpbase and dumpbase-ext", argv);
        remove("test2_output.o");
    }
    
    /* Test 3: All options combined with different values */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump/alt/",
            "-dumpbase", "full_test",
            "-dumpbase-ext", ".c",
            "-save-temps=obj",
            "-c", TEMP_SOURCE_FILE,
            "-o", "full_test_output.o",
            NULL
        };
        execute_gcc("Test 3: All options with different values", argv);
        remove("full_test_output.o");
        system("rm -f ./dump/alt/* 2>/dev/null");
        system("rmdir ./dump/alt 2>/dev/null 2>&1");
    }
    
    /* Test 4: Minimal options to trigger basic cleanup */
    {
        char *argv[] = {
            "gcc",
            "-dumpbase", "minimal",
            "-c", TEMP_SOURCE_FILE,
            NULL
        };
        execute_gcc("Test 4: Minimal options", argv);
    }
    
    /* Test 5: With -o option to influence outbase */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump/",
            "-dumpbase", "outbase_test",
            "-o", "custom_output_name.o",
            "-c", TEMP_SOURCE_FILE,
            NULL
        };
        execute_gcc("Test 5: Testing outbase with -o option", argv);
        remove("custom_output_name.o");
    }
    
    /* Test 6: Error case - invalid option to test cleanup after error */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump/",
            "-dumpbase", "error_test",
            "-invalid-opt",  /* This should cause an error */
            "-c", TEMP_SOURCE_FILE,
            NULL
        };
        execute_gcc("Test 6: Error case with invalid option", argv);
    }
    
    /* Test 7: Multiple dump options with verbose flag */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump/final/",
            "-dumpbase", "verbose_test",
            "-dumpbase-ext", ".c",
            "-save-temps",
            "-v",  /* Verbose flag */
            "-c", TEMP_SOURCE_FILE,
            "-o", "verbose_output.o",
            NULL
        };
        execute_gcc("Test 7: With verbose flag", argv);
        remove("verbose_output.o");
    }
    
    /* Clean up temporary files */
    cleanup_files();
    
    fprintf(stderr, "\n=== All GCC invocations completed ===\n");
    fprintf(stderr, "Check coverage data for gcc.cc lines 11228-11250\n");
    
    return ret;
}
