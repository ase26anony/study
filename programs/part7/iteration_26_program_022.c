#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Create a minimal valid .gcda file for testing */
void create_dummy_gcda(void) {
    /* Create a simple C program that will generate coverage data */
    FILE *fp = fopen("dummy.c", "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        exit(1);
    }
    fprintf(fp, "int main() { return 0; }\n");
    fclose(fp);
    
    /* Compile with coverage flags */
    if (system("gcc -fprofile-arcs -ftest-coverage -O0 -o dummy dummy.c 2>/dev/null") != 0) {
        fprintf(stderr, "Warning: Failed to compile dummy program\n");
        return;
    }
    
    /* Run to generate .gcda file */
    system("./dummy 2>/dev/null");
}

/* Execute gcov-dump with given arguments using execvp */
void test_gcov_dump_execvp(const char *args[]) {
    pid_t pid = fork();
    
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", (char *const *)args);
        /* If execvp returns, it failed */
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
void test_gcov_dump_system(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(status));
}

int main(void) {
    printf("=== Testing gcov-dump flag parsing ===\n\n");
    
    /* Create dummy .gcda file for file-based tests */
    create_dummy_gcda();
    
    /* Set environment variable if supported */
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    
    /* Test 1: Individual flags (execvp method) */
    printf("\n--- Test 1: Individual flags (execvp) ---\n");
    
    const char *test_cases[][4] = {
        {"gcov-dump", "-h", NULL},
        {"gcov-dump", "-v", NULL},
        {"gcov-dump", "-l", NULL},
        {"gcov-dump", "-p", NULL},
        {"gcov-dump", "-r", NULL},
        {"gcov-dump", "-s", NULL},
        {"gcov-dump", "-x", NULL},  /* Invalid flag for default case */
    };
    
    for (int i = 0; i < 7; i++) {
        printf("\nTest case %d: ", i + 1);
        for (int j = 0; test_cases[i][j] != NULL; j++) {
            printf("%s ", test_cases[i][j]);
        }
        printf("\n");
        test_gcov_dump_execvp(test_cases[i]);
    }
    
    /* Test 2: Flag combinations (system method) */
    printf("\n--- Test 2: Flag combinations (system) ---\n");
    
    const char *combinations[] = {
        "gcov-dump -l -p",
        "gcov-dump -r -s -v",
        "gcov-dump -h -l",           /* -h may cause early exit */
        "gcov-dump -p -p",           /* Repeated flag */
        "gcov-dump -l -p -r -s",     /* All valid flags */
        "gcov-dump -lp",             /* Combined short options */
        "gcov-dump -lprs",           /* All combined */
        NULL
    };
    
    for (int i = 0; combinations[i] != NULL; i++) {
        test_gcov_dump_system(combinations[i]);
    }
    
    /* Test 3: Flags with positional arguments */
    printf("\n--- Test 3: Flags with positional arguments ---\n");
    
    const char *file_tests[][5] = {
        {"gcov-dump", "-l", "dummy.gcda", NULL},
        {"gcov-dump", "-p", "dummy.gcda", NULL},
        {"gcov-dump", "-l", "-p", "dummy.gcda", NULL},
        {"gcov-dump", "-l", "--", "dummy.gcda", NULL},
        {"gcov-dump", "--", "-l", "dummy.gcda", NULL},  /* -l treated as filename */
        NULL
    };
    
    for (int i = 0; file_tests[i][0] != NULL; i++) {
        printf("\nFile test %d: ", i + 1);
        for (int j = 0; file_tests[i][j] != NULL; j++) {
            printf("%s ", file_tests[i][j]);
        }
        printf("\n");
        test_gcov_dump_execvp(file_tests[i]);
    }
    
    /* Test 4: Special cases (system method) */
    printf("\n--- Test 4: Special cases ---\n");
    
    const char *special_tests[] = {
        "gcov-dump",                    /* No arguments */
        "gcov-dump --help",             /* Long help option if supported */
        "gcov-dump --version",          /* Long version option if supported */
        "gcov-dump -l dummy.gcda 2>&1", /* Capture stderr */
        "gcov-dump -x 2>&1",            /* Capture unknown flag error */
        NULL
    };
    
    for (int i = 0; special_tests[i] != NULL; i++) {
        test_gcov_dump_system(special_tests[i]);
    }
    
    /* Test 5: Environment variable tests */
    printf("\n--- Test 5: Environment variable tests ---\n");
    
    /* Test with different environment settings */
    setenv("GCOV_DUMP_OPTIONS", "-l -p", 1);
    test_gcov_dump_system("gcov-dump dummy.gcda");
    
    unsetenv("GCOV_DUMP_OPTIONS");
    test_gcov_dump_system("gcov-dump -v");
    
    /* Test 6: Edge cases with invalid combinations */
    printf("\n--- Test 6: Edge cases ---\n");
    
    const char *edge_cases[][4] = {
        {"gcov-dump", "-", NULL},           /* Just a dash */
        {"gcov-dump", "--", NULL},          /* Just double dash */
        {"gcov-dump", "- ", NULL},          /* Flag with space */
        {"gcov-dump", "-lpq", NULL},        /* Combined with invalid */
        NULL
    };
    
    for (int i = 0; edge_cases[i][0] != NULL; i++) {
        printf("\nEdge case %d: ", i + 1);
        for (int j = 0; edge_cases[i][j] != NULL; j++) {
            printf("%s ", edge_cases[i][j]);
        }
        printf("\n");
        test_gcov_dump_execvp(edge_cases[i]);
    }
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    system("rm -f dummy dummy.c dummy.gcda dummy.gcno 2>/dev/null");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
