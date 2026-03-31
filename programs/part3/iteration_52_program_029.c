#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_cleanup.c"
#define TEMP_OUTPUT_FILE "test_gcc_cleanup.o"

/* Create a simple valid C source file */
static int create_test_source(void) {
    FILE *fp = fopen(TEMP_SOURCE_FILE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(fp, "/* Test file for GCC cleanup coverage */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    printf(\"Hello from test program\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/* Execute GCC with specified arguments using fork/exec */
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
            printf("GCC exited with status: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("GCC terminated by signal: %d\n", WTERMSIG(status));
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
    
    printf("Created test source file: %s\n", TEMP_SOURCE_FILE);
    
    /* Define various GCC invocations to exercise different code paths */
    
    /* 1. Basic invocation with dumpdir and save-temps */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump/",
            "-save-temps",
            "-c", TEMP_SOURCE_FILE,
            "-o", TEMP_OUTPUT_FILE,
            NULL
        };
        run_gcc("Basic with -dumpdir and -save-temps", argv);
    }
    
    /* 2. With dumpbase and dumpbase-ext but no dumpdir */
    {
        char *argv[] = {
            "gcc",
            "-dumpbase", "myprog",
            "-dumpbase-ext", ".c",
            "-save-temps=cwd",
            "-c", TEMP_SOURCE_FILE,
            NULL
        };
        run_gcc("With -dumpbase and -dumpbase-ext", argv);
    }
    
    /* 3. Comprehensive test with all relevant options */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./comprehensive_dump/",
            "-dumpbase", "comprehensive",
            "-dumpbase-ext", ".test",
            "-save-temps",
            "-o", "comprehensive_output.o",
            "-c", TEMP_SOURCE_FILE,
            NULL
        };
        run_gcc("Comprehensive with all options", argv);
    }
    
    /* 4. Test with optimization flags and coverage */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./opt_dump/",
            "-O2",
            "-fprofile-arcs",
            "-ftest-coverage",
            "-save-temps",
            "-c", TEMP_SOURCE_FILE,
            "-o", "opt_coverage.o",
            NULL
        };
        run_gcc("With optimization and coverage flags", argv);
    }
    
    /* 5. Test with static analysis flags */
    {
        char *argv[] = {
            "gcc",
            "-dumpbase", "static_analysis",
            "-O2",
            "-Wall",
            "-Wextra",
            "-Werror",
            "-save-temps=cwd",
            "-c", TEMP_SOURCE_FILE,
            "-o", "static_analysis.o",
            NULL
        };
        run_gcc("With static analysis flags", argv);
    }
    
    /* 6. Test error case with invalid option (should still trigger cleanup) */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./error_dump/",
            "-dumpbase", "error_test",
            "-invalid-opt",  /* This should cause an error */
            "-c", TEMP_SOURCE_FILE,
            NULL
        };
        run_gcc("Error case with invalid option", argv);
    }
    
    /* 7. Test without -c flag (link step) */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./link_dump/",
            "-dumpbase", "link_test",
            "-save-temps",
            TEMP_SOURCE_FILE,
            "-o", "link_test_prog",
            NULL
        };
        run_gcc("Linking step with dump options", argv);
    }
    
    /* 8. Minimal invocation to ensure baseline coverage */
    {
        char *argv[] = {
            "gcc",
            "-dumpbase", "minimal",
            "-c", TEMP_SOURCE_FILE,
            NULL
        };
        run_gcc("Minimal dumpbase only", argv);
    }
    
    /* 9. Test with verbose flag (verbose_only_flag in uncovered lines) */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./verbose_dump/",
            "-v",  /* verbose flag */
            "-save-temps",
            "-c", TEMP_SOURCE_FILE,
            "-o", "verbose_output.o",
            NULL
        };
        run_gcc("With verbose flag", argv);
    }
    
    /* 10. Test with version flag (print_version in uncovered lines) */
    {
        char *argv[] = {
            "gcc",
            "-dumpbase", "version_test",
            "--version",  /* version flag */
            NULL
        };
        run_gcc("Version request with dumpbase", argv);
    }
    
    /* Clean up temporary files */
    printf("\n=== Cleaning up temporary files ===\n");
    
    if (unlink(TEMP_SOURCE_FILE) < 0 && errno != ENOENT) {
        perror("Failed to remove source file");
        ret = -1;
    } else {
        printf("Removed: %s\n", TEMP_SOURCE_FILE);
    }
    
    /* Try to remove other generated files */
    char *files_to_remove[] = {
        TEMP_OUTPUT_FILE,
        "comprehensive_output.o",
        "opt_coverage.o",
        "static_analysis.o",
        "link_test_prog",
        "verbose_output.o",
        "test_gcc_cleanup.i",
        "test_gcc_cleanup.s",
        "comprehensive.i",
        "comprehensive.s",
        "opt_coverage.gcno",
        NULL
    };
    
    for (int i = 0; files_to_remove[i] != NULL; i++) {
        if (unlink(files_to_remove[i]) < 0 && errno != ENOENT) {
            /* Ignore files that don't exist */
        }
    }
    
    /* Remove dump directories if they exist */
    system("rm -rf ./dump/ ./comprehensive_dump/ ./opt_dump/ ./error_dump/ ./link_dump/ ./verbose_dump/ 2>/dev/null");
    
    printf("\nTest program completed. Check GCC driver coverage for the target cleanup block.\n");
    
    return ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
