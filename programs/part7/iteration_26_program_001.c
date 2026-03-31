#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

/* Create a minimal C program to generate a .gcda file */
const char *test_program = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Test program for coverage\\n\");\n"
"    return 0;\n"
"}\n";

/* Execute a command and wait for completion */
int execute_command(char *const args[]) {
    pid_t pid = fork();
    
    if (pid == 0) {
        /* Child process */
        execvp(args[0], args);
        /* If execvp returns, there was an error */
        fprintf(stderr, "Failed to execute %s: %s\n", args[0], strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    } else {
        /* Fork failed */
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
        return -1;
    }
}

/* Execute via system() call */
int execute_system(const char *cmd) {
    int ret = system(cmd);
    if (ret == -1) {
        fprintf(stderr, "system() failed for: %s\n", cmd);
    }
    return ret;
}

int main() {
    int i;
    int ret;
    
    printf("=== Testing gcov-dump flag parsing ===\n\n");
    
    /* First, create a dummy .gcda file for testing */
    printf("1. Creating dummy .gcda file...\n");
    
    /* Write test program */
    FILE *fp = fopen("test_coverage.c", "w");
    if (!fp) {
        perror("Failed to create test_coverage.c");
        return 1;
    }
    fprintf(fp, "%s", test_program);
    fclose(fp);
    
    /* Compile with coverage flags */
    char *compile_args[] = {"gcc", "-fprofile-arcs", "-ftest-coverage", 
                           "test_coverage.c", "-o", "test_coverage", NULL};
    ret = execute_command(compile_args);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed\n");
        /* Continue anyway - some tests don't need the file */
    } else {
        /* Run the program to generate .gcda */
        char *run_args[] = {"./test_coverage", NULL};
        execute_command(run_args);
    }
    
    /* Test 1: Individual flag cases (using execvp for precise control) */
    printf("\n2. Testing individual flag cases:\n");
    
    char *test_cases[][4] = {
        {"gcov-dump", "-h", NULL},                     /* Help flag */
        {"gcov-dump", "-v", NULL},                     /* Version flag */
        {"gcov-dump", "-l", NULL},                     /* Contents dump flag */
        {"gcov-dump", "-p", NULL},                     /* Positions dump flag */
        {"gcov-dump", "-r", NULL},                     /* Raw dump flag */
        {"gcov-dump", "-s", NULL},                     /* Stable dump flag */
        {"gcov-dump", "-x", NULL},                     /* Invalid flag (triggers default case) */
        {NULL}  /* Sentinel */
    };
    
    for (i = 0; test_cases[i][0] != NULL; i++) {
        printf("\n  Testing: ");
        for (int j = 0; test_cases[i][j] != NULL; j++) {
            printf("%s ", test_cases[i][j]);
        }
        printf("\n");
        
        ret = execute_command(test_cases[i]);
        printf("    Exit status: %d\n", ret);
    }
    
    /* Test 2: No arguments (triggers default behavior) */
    printf("\n3. Testing no arguments:\n");
    char *no_args[] = {"gcov-dump", NULL};
    ret = execute_command(no_args);
    printf("  Exit status: %d\n", ret);
    
    /* Test 3: Combined valid flags in single command (using system for variety) */
    printf("\n4. Testing combined flags (using system()):\n");
    
    const char *combined_tests[] = {
        "gcov-dump -l -p",
        "gcov-dump -r -s -v",
        "gcov-dump -h -l",      /* -h might cause early exit, but still test */
        "gcov-dump -p -p",      /* Repeated flag */
        "gcov-dump -l -p -r -s", /* All flags together */
        NULL
    };
    
    for (i = 0; combined_tests[i] != NULL; i++) {
        printf("\n  Testing: %s\n", combined_tests[i]);
        ret = execute_system(combined_tests[i]);
        printf("    Return: %d\n", ret);
    }
    
    /* Test 4: Different syntactic styles */
    printf("\n5. Testing different syntactic styles:\n");
    
    /* Combined short options */
    printf("\n  Testing combined short options (-lp):\n");
    char *combined_short[] = {"gcov-dump", "-lp", NULL};
    execute_command(combined_short);
    
    /* With positional arguments */
    printf("\n  Testing with .gcda file argument:\n");
    char *with_file[] = {"gcov-dump", "-l", "test_coverage.gcda", NULL};
    execute_command(with_file);
    
    /* With -- delimiter */
    printf("\n  Testing with -- delimiter:\n");
    char *with_delim[] = {"gcov-dump", "-l", "--", "test_coverage.gcda", NULL};
    execute_command(with_delim);
    
    /* Test 5: Environment variable simulation */
    printf("\n6. Testing with environment variables:\n");
    
    /* Set environment variable if gcov-dump reads it */
    setenv("GCOV_DUMP_OPTIONS", "-l", 1);
    
    char *env_test[] = {"gcov-dump", NULL};
    printf("  With GCOV_DUMP_OPTIONS=-l:\n");
    execute_command(env_test);
    
    /* Test with different env var */
    setenv("GCOV_DUMP_OPTIONS", "-v -p", 1);
    printf("\n  With GCOV_DUMP_OPTIONS='-v -p':\n");
    execute_command(env_test);
    
    unsetenv("GCOV_DUMP_OPTIONS");
    
    /* Test 6: Error stream redirection test */
    printf("\n7. Testing error stream capture (invalid flag):\n");
    
    /* Use popen to capture stderr */
    FILE *pipe = popen("gcov-dump -x 2>&1", "r");
    if (pipe) {
        char buffer[256];
        printf("  Captured output:\n    ");
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            printf("%s", buffer);
        }
        pclose(pipe);
    }
    
    /* Test 7: Multiple invalid flags */
    printf("\n8. Testing multiple invalid flags:\n");
    char *multi_invalid[] = {"gcov-dump", "-x", "-y", "-z", NULL};
    execute_command(multi_invalid);
    
    /* Test 8: Mixed valid and invalid flags */
    printf("\n9. Testing mixed valid and invalid flags:\n");
    char *mixed[] = {"gcov-dump", "-l", "-x", "-p", NULL};
    execute_command(mixed);
    
    /* Test 9: Long arguments (if supported) */
    printf("\n10. Testing with file that doesn't exist:\n");
    char *nonexistent[] = {"gcov-dump", "-l", "nonexistent.gcda", NULL};
    execute_command(nonexistent);
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    remove("test_coverage.c");
    remove("test_coverage");
    remove("test_coverage.gcda");
    remove("test_coverage.gcno");
    
    printf("\nAll tests completed.\n");
    return 0;
}
