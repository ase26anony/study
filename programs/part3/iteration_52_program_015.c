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
    /* Create the test source file */
    if (create_test_source() < 0) {
        return EXIT_FAILURE;
    }
    
    /* Array of GCC invocations with different option combinations */
    
    /* 1. Basic invocation with dumpdir and save-temps */
    char *argv1[] = {
        "gcc",
        "-dumpdir", "./dump/",
        "-save-temps",
        "-c", TEMP_SOURCE_FILE,
        "-o", TEMP_OUTPUT_FILE,
        NULL
    };
    
    /* 2. With dumpbase and dumpbase-ext but no dumpdir */
    char *argv2[] = {
        "gcc",
        "-dumpbase", "myprog",
        "-dumpbase-ext", ".c",
        "-save-temps=cwd",
        "-c", TEMP_SOURCE_FILE,
        "-o", "test2.o",
        NULL
    };
    
    /* 3. With all options combined */
    char *argv3[] = {
        "gcc",
        "-dumpdir", "./full_dump/",
        "-dumpbase", "full_prog",
        "-dumpbase-ext", ".c",
        "-save-temps",
        "-c", TEMP_SOURCE_FILE,
        "-o", "test3.o",
        NULL
    };
    
    /* 4. With different dumpdir format (trailing slash) */
    char *argv4[] = {
        "gcc",
        "-dumpdir", "./dumpdir-",  /* Tests dumpdir_trailing_dash_added */
        "-dumpbase", "dash_test",
        "-save-temps=obj",
        "-c", TEMP_SOURCE_FILE,
        "-o", "test4.o",
        NULL
    };
    
    /* 5. Minimal invocation to test default paths */
    char *argv5[] = {
        "gcc",
        "-c", TEMP_SOURCE_FILE,
        NULL
    };
    
    /* 6. Invocation with invalid option to test cleanup after error */
    char *argv6[] = {
        "gcc",
        "-dumpdir", "./error_dump/",
        "-dumpbase", "error_test",
        "-invalid-opt",  /* This should cause an error */
        "-c", TEMP_SOURCE_FILE,
        NULL
    };
    
    /* 7. Another combination with outbase influenced by -o */
    char *argv7[] = {
        "gcc",
        "-dumpdir", "./final/",
        "-dumpbase", "final_base",
        "-dumpbase-ext", ".final",
        "-save-temps",
        "-c", TEMP_SOURCE_FILE,
        "-o", "custom_output_name.o",  /* Influences outbase */
        NULL
    };
    
    /* Execute all GCC invocations */
    run_gcc("Basic with dumpdir and save-temps", argv1);
    run_gcc("dumpbase and dumpbase-ext without dumpdir", argv2);
    run_gcc("All options combined", argv3);
    run_gcc("dumpdir with trailing dash", argv4);
    run_gcc("Minimal compilation", argv5);
    run_gcc("With invalid option (error case)", argv6);
    run_gcc("Custom output name influencing outbase", argv7);
    
    /* Clean up generated files */
    printf("\n=== Cleaning up temporary files ===\n");
    unlink(TEMP_SOURCE_FILE);
    unlink(TEMP_OUTPUT_FILE);
    unlink("test2.o");
    unlink("test3.o");
    unlink("test4.o");
    unlink("custom_output_name.o");
    
    /* Also clean up save-temps files if they exist */
    unlink("test_gcc_coverage.i");
    unlink("test_gcc_coverage.s");
    unlink("myprog.i");
    unlink("myprog.s");
    unlink("full_prog.i");
    unlink("full_prog.s");
    unlink("dash_test.i");
    unlink("dash_test.s");
    unlink("final_base.i");
    unlink("final_base.s");
    unlink("error_test.i");
    unlink("error_test.s");
    
    /* Clean up dump directories if created */
    system("rm -rf ./dump ./full_dump ./dumpdir- ./error_dump ./final 2>/dev/null");
    
    printf("\nTest program completed successfully.\n");
    return EXIT_SUCCESS;
}
