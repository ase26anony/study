/**
 * gcov-dump_parser_test.c
 * 
 * Comprehensive test of gcov-dump command-line flag parsing logic.
 * Tests all uncovered switch cases in gcov-dump.cc lines 111-130.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/**
 * Creates a minimal valid .gcda file for testing.
 * Uses gcc with coverage flags to generate a real .gcda file.
 */
static int create_dummy_gcda(void) {
    FILE *fp;
    int status;
    
    /* Create a trivial C source file */
    fp = fopen("dummy_test.c", "w");
    if (!fp) {
        perror("Failed to create dummy_test.c");
        return -1;
    }
    fprintf(fp, "int main() { return 0; }\n");
    fclose(fp);
    
    /* Compile with coverage flags */
    status = system("gcc -fprofile-arcs -ftest-coverage -o dummy_test dummy_test.c 2>/dev/null");
    if (status != 0) {
        fprintf(stderr, "Warning: Failed to compile dummy_test.c with coverage flags\n");
        fprintf(stderr, "Some file-based tests may fail\n");
        return -1;
    }
    
    /* Run the program to generate .gcda file */
    status = system("./dummy_test 2>/dev/null");
    if (status != 0) {
        fprintf(stderr, "Warning: Failed to run dummy_test\n");
        return -1;
    }
    
    return 0;
}

/**
 * Clean up test files
 */
static void cleanup_test_files(void) {
    system("rm -f dummy_test.c dummy_test dummy_test.gcda dummy_test.gcno 2>/dev/null");
}

/**
 * Execute gcov-dump using execvp for precise argument control
 */
static int exec_gcov_dump(const char *args[], const char *description) {
    pid_t pid;
    int status;
    
    printf("\n=== Testing: %s ===\n", description);
    printf("Command: ");
    for (int i = 0; args[i] != NULL; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
    
    pid = fork();
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", (char * const *)args);
        /* If execvp returns, it failed */
        fprintf(stderr, "Failed to execute gcov-dump: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        /* Parent process */
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("Exit status: %d\n", WEXITSTATUS(status));
        } else {
            printf("Process terminated abnormally\n");
        }
        return status;
    } else {
        /* Fork failed */
        perror("fork failed");
        return -1;
    }
}

/**
 * Execute gcov-dump using system() for shell interpretation tests
 */
static void system_gcov_dump(const char *cmd, const char *description) {
    printf("\n=== Testing (system): %s ===\n", description);
    printf("Command: %s\n", cmd);
    
    int status = system(cmd);
    if (status == -1) {
        printf("system() call failed\n");
    } else if (WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    }
}

int main(void) {
    /* Test command argument arrays */
    const char *test_cases[][5] = {
        /* Individual flag tests - direct switch cases */
        {"gcov-dump", "-h", NULL},                          /* case 'h': print_usage() */
        {"gcov-dump", "-v", NULL},                          /* case 'v': print_version() */
        {"gcov-dump", "-l", NULL},                          /* case 'l': flag_dump_contents = 1 */
        {"gcov-dump", "-p", NULL},                          /* case 'p': flag_dump_positions = 1 */
        {"gcov-dump", "-r", NULL},                          /* case 'r': flag_dump_raw = 1 */
        {"gcov-dump", "-s", NULL},                          /* case 's': flag_dump_stable = 1 */
        {"gcov-dump", "-x", NULL},                          /* default: unknown flag 'x' */
        
        /* Combination of valid flags */
        {"gcov-dump", "-l", "-p", NULL},                    /* -l -p combination */
        {"gcov-dump", "-r", "-s", "-v", NULL},              /* -r -s -v combination */
        {"gcov-dump", "-h", "-l", NULL},                    /* -h with other flag (may exit early) */
        
        /* Repeated flags */
        {"gcov-dump", "-p", "-p", NULL},                    /* -p -p repetition */
        {"gcov-dump", "-l", "-l", "-l", NULL},              /* Triple -l repetition */
        
        /* With positional arguments (gcov files) */
        {"gcov-dump", "-l", "dummy_test.gcda", NULL},       /* -l with file */
        {"gcov-dump", "-p", "-r", "dummy_test.gcda", NULL}, /* Multiple flags with file */
        
        /* Edge cases */
        {"gcov-dump", NULL},                                /* No arguments */
        {"gcov-dump", "--", "-l", NULL},                    /* -- delimiter with flag after */
        {"gcov-dump", "-l", "--", "dummy_test.gcda", NULL}, /* -- delimiter separating flags and file */
    };
    
    const char *test_descriptions[] = {
        "Help flag (-h)",
        "Version flag (-v)",
        "Contents dump flag (-l)",
        "Positions dump flag (-p)",
        "Raw dump flag (-r)",
        "Stable dump flag (-s)",
        "Invalid flag (-x) - triggers default case",
        "Combination: -l -p",
        "Combination: -r -s -v",
        "Combination: -h -l (early exit test)",
        "Repeated flag: -p -p",
        "Repeated flag: -l -l -l",
        "With file argument: -l dummy_test.gcda",
        "Multiple flags with file: -p -r dummy_test.gcda",
        "No arguments",
        "Delimiter test: -- -l",
        "Delimiter with file: -l -- dummy_test.gcda",
    };
    
    /* Set environment variable if gcov-dump uses it */
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    
    /* Create dummy .gcda file for file-based tests */
    if (create_dummy_gcda() != 0) {
        fprintf(stderr, "Note: Some file-based tests may fail without dummy .gcda file\n");
    }
    
    printf("========================================\n");
    printf("Testing gcov-dump flag parsing logic\n");
    printf("Target: Uncovered lines 111-130 in gcov-dump.cc\n");
    printf("========================================\n");
    
    /* Test using execvp for precise argument passing */
    for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        exec_gcov_dump(test_cases[i], test_descriptions[i]);
    }
    
    /* Test using system() for shell interpretation */
    printf("\n\n========================================\n");
    printf("Additional tests using system() calls\n");
    printf("========================================\n");
    
    /* Test combined short options syntax (if supported by getopt) */
    system_gcov_dump("gcov-dump -lp", "Combined short options: -lp");
    system_gcov_dump("gcov-dump -lps", "Combined short options: -lps");
    
    /* Test with shell redirection */
    system_gcov_dump("gcov-dump -h 2>&1", "Help with stderr redirect");
    system_gcov_dump("gcov-dump -x 2>&1", "Invalid flag with stderr redirect (capture 'unknown flag')");
    
    /* Test with environment variable */
    system_gcov_dump("GCOV_DUMP_OPTIONS='-l' gcov-dump -p 2>&1", "With GCOV_DUMP_OPTIONS env var");
    
    /* Test error cases */
    system_gcov_dump("gcov-dump -", "Single dash (invalid)");
    system_gcov_dump("gcov-dump --help", "Long option --help (if supported)");
    
    /* Test with non-existent file */
    system_gcov_dump("gcov-dump -l non_existent.gcda 2>&1", "With non-existent file");
    
    /* Clean up */
    cleanup_test_files();
    
    printf("\n========================================\n");
    printf("All tests completed\n");
    printf("========================================\n");
    
    return 0;
}
