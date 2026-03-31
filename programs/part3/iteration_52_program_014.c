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
    FILE *f = fopen(TEMP_SOURCE_FILE, "w");
    if (!f) {
        fprintf(stderr, "Failed to create source file: %s\n", strerror(errno));
        return -1;
    }
    
    fprintf(f, "/* Test source for GCC coverage */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main(void) {\n");
    fprintf(f, "    printf(\"Hello from coverage test!\\n\");\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
    return 0;
}

/* Clean up temporary files */
static void cleanup_files(void) {
    unlink(TEMP_SOURCE_FILE);
    unlink(TEMP_OUTPUT_FILE);
    
    /* Also clean up any dump files that might have been created */
    system("rm -f ./dump/* 2>/dev/null");
    system("rmdir ./dump 2>/dev/null 2>&1");
}

/* Execute GCC with given arguments using fork/exec */
static int execute_gcc(const char *description, char *const argv[]) {
    fprintf(stderr, "\n=== %s ===\n", description);
    fprintf(stderr, "Command: ");
    for (int i = 0; argv[i] != NULL; i++) {
        fprintf(stderr, "%s ", argv[i]);
    }
    fprintf(stderr, "\n");
    
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "fork failed: %s\n", strerror(errno));
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        execvp("gcc", argv);
        /* If execvp returns, it failed */
        fprintf(stderr, "execvp failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            fprintf(stderr, "Exit status: %d\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "Terminated by signal: %d\n", WTERMSIG(status));
            return -1;
        }
    }
    return 0;
}

int main(void) {
    int ret = 0;
    
    /* Create test source file */
    if (create_test_source() < 0) {
        return EXIT_FAILURE;
    }
    
    /* Create dump directory if it doesn't exist */
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
    
    /* Test 2: With dumpbase and dumpbase-ext but no dumpdir */
    {
        char *argv[] = {
            "gcc",
            "-dumpbase", "testdump",
            "-dumpbase-ext", ".c",
            "-save-temps=cwd",
            "-c", TEMP_SOURCE_FILE,
            "-o", "output2.o",
            NULL
        };
        execute_gcc("Test 2: dumpbase/ext with save-temps=cwd", argv);
        unlink("output2.o");
    }
    
    /* Test 3: All options combined */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump/",
            "-dumpbase", "fulltest",
            "-dumpbase-ext", ".c",
            "-save-temps=obj",
            "-c", TEMP_SOURCE_FILE,
            "-o", "full_output.o",
            NULL
        };
        execute_gcc("Test 3: All options with save-temps=obj", argv);
        unlink("full_output.o");
    }
    
    /* Test 4: Different dumpdir format (trailing dash) */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump-",  /* Trailing dash to test dumpdir_trailing_dash_added */
            "-dumpbase", "dash_test",
            "-save-temps",
            "-c", TEMP_SOURCE_FILE,
            "-o", "dash_output.o",
            NULL
        };
        execute_gcc("Test 4: Dumpdir with trailing dash", argv);
        unlink("dash_output.o");
    }
    
    /* Test 5: Minimal options to trigger basic cleanup */
    {
        char *argv[] = {
            "gcc",
            "-c", TEMP_SOURCE_FILE,
            "-o", "minimal.o",
            NULL
        };
        execute_gcc("Test 5: Minimal compilation", argv);
        unlink("minimal.o");
    }
    
    /* Test 6: Error case with invalid option (should still trigger cleanup) */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump/",
            "-dumpbase", "error_test",
            "-invalid-opt",  /* This will cause an error */
            "-c", TEMP_SOURCE_FILE,
            NULL
        };
        execute_gcc("Test 6: Error case with invalid option", argv);
    }
    
    /* Test 7: With outbase influenced by -o option */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump/",
            "-dumpbase", "outbase_test",
            "-dumpbase-ext", ".c",
            "-save-temps",
            "-c", TEMP_SOURCE_FILE,
            "-o", "custom_name.o",  /* Non-default output name */
            NULL
        };
        execute_gcc("Test 7: Custom output name for outbase", argv);
        unlink("custom_name.o");
    }
    
    /* Test 8: Multiple dump options without save-temps */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump/",
            "-dumpbase", "no_save",
            "-dumpbase-ext", ".c",
            "-c", TEMP_SOURCE_FILE,
            "-o", "no_save.o",
            NULL
        };
        execute_gcc("Test 8: Dump options without save-temps", argv);
        unlink("no_save.o");
    }
    
    /* Clean up temporary files */
    cleanup_files();
    
    fprintf(stderr, "\n=== All tests completed ===\n");
    
    return ret;
}
