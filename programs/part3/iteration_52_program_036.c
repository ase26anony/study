#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_source.c"
#define TEMP_OUTPUT_FILE "test_coverage_output.o"

/* Create a simple valid C source file */
static int create_test_source(void) {
    FILE *f = fopen(TEMP_SOURCE_FILE, "w");
    if (!f) {
        perror("Failed to create test source file");
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
    /* Clean up any dump files that might have been created */
    system("rm -rf ./dump 2>/dev/null");
    system("rm -f *.i *.s *.o 2>/dev/null");
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
    }
    
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

int main(void) {
    /* Create the test source file */
    if (create_test_source() < 0) {
        return EXIT_FAILURE;
    }
    
    /* Set up argument arrays for different GCC invocations */
    
    /* 1. Invocation with -dumpdir and -save-temps */
    char *gcc_args1[] = {
        "gcc",
        "-dumpdir", "./dump/",
        "-dumpbase", "myprog",
        "-dumpbase-ext", ".c",
        "-save-temps",
        "-c", TEMP_SOURCE_FILE,
        "-o", TEMP_OUTPUT_FILE,
        NULL
    };
    
    /* 2. Invocation with -dumpbase and -dumpbase-ext but no -dumpdir */
    char *gcc_args2[] = {
        "gcc",
        "-dumpbase", "testdump",
        "-dumpbase-ext", ".c",
        "-save-temps=cwd",
        "-c", TEMP_SOURCE_FILE,
        "-o", "different_output.o",
        NULL
    };
    
    /* 3. Invocation with all options */
    char *gcc_args3[] = {
        "gcc",
        "-dumpdir", "./dump_all/",
        "-dumpbase", "fullcoverage",
        "-dumpbase-ext", ".c",
        "-save-temps",
        "-O2",
        "-c", TEMP_SOURCE_FILE,
        "-o", "final_output.o",
        NULL
    };
    
    /* 4. Invocation with minimal options to test basic cleanup */
    char *gcc_args4[] = {
        "gcc",
        "-dumpbase", "minimal",
        "-c", TEMP_SOURCE_FILE,
        NULL
    };
    
    /* 5. Invocation with invalid option to test cleanup after error */
    char *gcc_args5[] = {
        "gcc",
        "-dumpdir", "./error_dump/",
        "-dumpbase", "error",
        "-invalid-opt",  /* This should cause an error */
        "-c", TEMP_SOURCE_FILE,
        NULL
    };
    
    /* 6. Invocation with -o option to influence outbase */
    char *gcc_args6[] = {
        "gcc",
        "-dumpdir", "./outbase_test/",
        "-dumpbase", "outbasetest",
        "-save-temps=obj",
        "-c", TEMP_SOURCE_FILE,
        "-o", "custom_name.o",
        NULL
    };
    
    /* Run all GCC invocations */
    run_gcc("Test 1: -dumpdir, -dumpbase, -dumpbase-ext, -save-temps, -o", gcc_args1);
    sleep(1);  /* Small delay to ensure file system operations complete */
    
    run_gcc("Test 2: -dumpbase, -dumpbase-ext, -save-temps=cwd, no -dumpdir", gcc_args2);
    sleep(1);
    
    run_gcc("Test 3: All options with optimization", gcc_args3);
    sleep(1);
    
    run_gcc("Test 4: Minimal -dumpbase only", gcc_args4);
    sleep(1);
    
    run_gcc("Test 5: With invalid option (error case)", gcc_args5);
    sleep(1);
    
    run_gcc("Test 6: Testing outbase with custom output name", gcc_args6);
    
    /* Clean up temporary files */
    cleanup_files();
    
    printf("\n=== All GCC invocations completed ===\n");
    printf("The cleanup logic in gcc.cc (lines 11228-11250) should have been triggered.\n");
    
    return EXIT_SUCCESS;
}
