#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Create a minimal C program to generate a .gcda file */
const char *test_program = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Test program for gcov\\n\");\n"
"    return 0;\n"
"}\n";

/* Function to create a dummy .gcda file */
int create_dummy_gcda(void) {
    FILE *fp;
    int status;
    
    /* Write test program */
    fp = fopen("test_gcov.c", "w");
    if (!fp) {
        perror("Failed to create test_gcov.c");
        return -1;
    }
    fprintf(fp, "%s", test_program);
    fclose(fp);
    
    /* Compile with coverage flags */
    status = system("gcc -fprofile-arcs -ftest-coverage -O0 test_gcov.c -o test_gcov");
    if (status != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return -1;
    }
    
    /* Run to generate .gcda file */
    status = system("./test_gcov");
    if (status != 0) {
        fprintf(stderr, "Failed to run test program\n");
        return -1;
    }
    
    return 0;
}

/* Execute gcov-dump using execvp */
void execute_gcov_dump_execvp(const char *args[]) {
    pid_t pid;
    int status;
    
    pid = fork();
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", (char *const *)args);
        /* If execvp returns, it failed */
        fprintf(stderr, "Failed to execute gcov-dump: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        /* Parent process */
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("gcov-dump exited with status %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("gcov-dump terminated by signal %d\n", WTERMSIG(status));
        }
    } else {
        perror("fork failed");
    }
}

/* Execute gcov-dump using system() */
void execute_gcov_dump_system(const char *cmd) {
    int status = system(cmd);
    printf("system() call returned: %d\n", status);
}

int main(void) {
    /* Test cases for individual flags (uncovered lines) */
    const char *test_cases[][4] = {
        /* Individual flags from uncovered switch cases */
        {"gcov-dump", "-h", NULL},                     /* Help flag */
        {"gcov-dump", "-v", NULL},                     /* Version flag */
        {"gcov-dump", "-l", NULL},                     /* Contents dump flag */
        {"gcov-dump", "-p", NULL},                     /* Positions dump flag */
        {"gcov-dump", "-r", NULL},                     /* Raw dump flag */
        {"gcov-dump", "-s", NULL},                     /* Stable dump flag */
        {"gcov-dump", "-x", NULL},                     /* Invalid flag (triggers default case) */
        
        /* Combination of valid flags */
        {"gcov-dump", "-l", "-p", NULL},               /* Two flags separate */
        {"gcov-dump", "-r", "-s", "-v", NULL},         /* Three flags */
        {"gcov-dump", "-h", "-l", NULL},               /* Help with another flag */
        
        /* Repeated flags */
        {"gcov-dump", "-p", "-p", NULL},               /* Same flag twice */
        {"gcov-dump", "-l", "-l", "-l", NULL},         /* Same flag three times */
        
        /* No arguments */
        {"gcov-dump", NULL},                           /* No flags */
        
        /* End marker */
        {NULL, NULL, NULL}
    };
    
    /* Additional test cases for system() calls */
    const char *system_tests[] = {
        "gcov-dump -lp",                               /* Combined short options */
        "gcov-dump -l -p",                             /* Separate (same as execvp test) */
        "gcov-dump -h -l",                             /* Help with another flag */
        "gcov-dump -x",                                /* Invalid flag */
        "gcov-dump",                                   /* No arguments */
        NULL
    };
    
    int i, j;
    
    printf("=== Creating dummy .gcda file ===\n");
    if (create_dummy_gcda() != 0) {
        fprintf(stderr, "Warning: Could not create dummy .gcda file\n");
    }
    
    printf("\n=== Testing with execvp (precise argument control) ===\n");
    
    /* Test with file arguments */
    const char *file_tests[][5] = {
        {"gcov-dump", "-l", "test_gcov.gcda", NULL},   /* With positional file argument */
        {"gcov-dump", "-p", "test_gcov.gcda", NULL},
        {"gcov-dump", "-l", "-p", "test_gcov.gcda", NULL},
        {"gcov-dump", "-l", "--", "test_gcov.gcda", NULL}, /* With -- delimiter */
        {NULL, NULL, NULL, NULL}
    };
    
    /* Execute file-based tests first */
    for (i = 0; file_tests[i][0] != NULL; i++) {
        printf("\nTest %d: ", i + 1);
        for (j = 0; file_tests[i][j] != NULL; j++) {
            printf("%s ", file_tests[i][j]);
        }
        printf("\n");
        execute_gcov_dump_execvp(file_tests[i]);
    }
    
    /* Execute basic flag tests */
    for (i = 0; test_cases[i][0] != NULL; i++) {
        printf("\nTest %d: ", i + 1 + (int)(sizeof(file_tests)/sizeof(file_tests[0]) - 1));
        for (j = 0; test_cases[i][j] != NULL; j++) {
            printf("%s ", test_cases[i][j]);
        }
        printf("\n");
        execute_gcov_dump_execvp(test_cases[i]);
    }
    
    printf("\n=== Testing with system() calls ===\n");
    
    /* Set environment variable to test if gcov-dump uses it */
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    
    for (i = 0; system_tests[i] != NULL; i++) {
        printf("\nSystem test %d: %s\n", i + 1, system_tests[i]);
        execute_gcov_dump_system(system_tests[i]);
    }
    
    /* Test with environment variable unset */
    unsetenv("GCOV_DUMP_OPTIONS");
    printf("\nWith GCOV_DUMP_OPTIONS unset:\n");
    execute_gcov_dump_system("gcov-dump -v");
    
    /* Test invalid flag with error redirection */
    printf("\n=== Testing error output redirection ===\n");
    printf("Testing invalid flag with stderr capture:\n");
    system("gcov-dump -x 2>&1");
    
    /* Test valid flag combination with output redirection */
    printf("\nTesting valid flags with output redirection:\n");
    system("gcov-dump -l -p 2>&1 | head -20");
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    system("rm -f test_gcov test_gcov.c test_gcov.gcda test_gcov.gcno test_gcov.o");
    
    return 0;
}
