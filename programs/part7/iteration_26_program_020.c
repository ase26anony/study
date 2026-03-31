#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Create a minimal valid .gcda file for testing */
void create_dummy_gcda(void) {
    /* Create a simple C source file */
    FILE *src = fopen("dummy.c", "w");
    if (src) {
        fprintf(src, "int main() { return 0; }\n");
        fclose(src);
    }
    
    /* Compile with coverage flags */
    system("gcc -fprofile-arcs -ftest-coverage -o dummy dummy.c 2>/dev/null");
    
    /* Run to generate .gcda file */
    system("./dummy 2>/dev/null");
    
    /* Clean up intermediate files */
    remove("dummy.c");
    remove("dummy");
}

/* Execute gcov-dump using execvp */
void test_with_execvp(const char *args[]) {
    pid_t pid = fork();
    
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", (char *const *)args);
        /* If execvp fails */
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        printf("Exit status: %d\n", WEXITSTATUS(status));
    } else {
        perror("fork failed");
    }
}

/* Execute gcov-dump using system() */
void test_with_system(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    printf("System call returned: %d\n\n", status);
}

int main(void) {
    printf("=== Testing gcov-dump flag parsing ===\n\n");
    
    /* Create dummy .gcda file for file-based tests */
    create_dummy_gcda();
    
    /* Test individual flags (execvp method) */
    printf("--- Testing individual flags with execvp ---\n");
    
    const char *test_cases[][4] = {
        {"gcov-dump", "-h", NULL},
        {"gcov-dump", "-v", NULL},
        {"gcov-dump", "-l", NULL},
        {"gcov-dump", "-p", NULL},
        {"gcov-dump", "-r", NULL},
        {"gcov-dump", "-s", NULL},
        {"gcov-dump", "-x", NULL},  /* Invalid flag for default case */
        {NULL}
    };
    
    for (int i = 0; test_cases[i][0] != NULL; i++) {
        printf("Test %d: ", i + 1);
        for (int j = 0; test_cases[i][j] != NULL; j++) {
            printf("%s ", test_cases[i][j]);
        }
        printf("\n");
        test_with_execvp(test_cases[i]);
        printf("\n");
    }
    
    /* Test flag combinations (system method) */
    printf("--- Testing flag combinations with system() ---\n");
    
    const char *combination_tests[] = {
        "gcov-dump -l -p",
        "gcov-dump -r -s -v",
        "gcov-dump -h -l",  /* -h may cause early exit */
        "gcov-dump -p -p",  /* Repeated flag */
        "gcov-dump -lp",    /* Combined short options */
        "gcov-dump -l -p -r -s",  /* All valid flags */
        NULL
    };
    
    for (int i = 0; combination_tests[i] != NULL; i++) {
        test_with_system(combination_tests[i]);
    }
    
    /* Test with positional arguments (gcov files) */
    printf("--- Testing with file arguments ---\n");
    
    const char *file_tests[] = {
        "gcov-dump -l dummy.gcda",
        "gcov-dump -p dummy.gcda",
        "gcov-dump -l -p dummy.gcda",
        "gcov-dump -l -- dummy.gcda",  /* With -- delimiter */
        "gcov-dump -- -l dummy.gcda",  /* Invalid: -l after -- */
        NULL
    };
    
    for (int i = 0; file_tests[i] != NULL; i++) {
        test_with_system(file_tests[i]);
    }
    
    /* Test edge cases */
    printf("--- Testing edge cases ---\n");
    
    const char *edge_tests[] = {
        "gcov-dump",  /* No arguments */
        "gcov-dump --help",  /* Long option (if supported) */
        "gcov-dump -",  /* Just dash */
        "gcov-dump --",  /* Just double dash */
        "gcov-dump -l -p -x -r",  /* Mix valid and invalid */
        NULL
    };
    
    for (int i = 0; edge_tests[i] != NULL; i++) {
        test_with_system(edge_tests[i]);
    }
    
    /* Test with environment variables */
    printf("--- Testing with environment variables ---\n");
    
    /* Set environment variable if gcov-dump reads it */
    setenv("GCOV_DUMP_OPTIONS", "-l -p", 1);
    test_with_system("gcov-dump -v");
    
    setenv("GCOV_DUMP_OPTIONS", "invalid", 1);
    test_with_system("gcov-dump -h");
    
    unsetenv("GCOV_DUMP_OPTIONS");
    
    /* Test output redirection for error capture */
    printf("--- Testing error output redirection ---\n");
    
    const char *redirect_tests[] = {
        "gcov-dump -x 2>&1",  /* Capture stderr for invalid flag */
        "gcov-dump -l > /dev/null 2>&1",  /* Redirect all output */
        "gcov-dump -y 2>error.txt",  /* Save error to file */
        NULL
    };
    
    for (int i = 0; redirect_tests[i] != NULL; i++) {
        test_with_system(redirect_tests[i]);
    }
    
    /* Clean up */
    remove("dummy.gcda");
    remove("dummy.gcno");
    
    printf("=== All tests completed ===\n");
    return 0;
}
