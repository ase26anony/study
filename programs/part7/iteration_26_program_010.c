/**
 * gcov-dump_parser_test.c
 * 
 * Comprehensive test program for gcov-dump command-line flag parsing.
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
 * Creates a minimal dummy .gcda file for testing.
 * This generates a valid gcov data file header.
 */
static int create_dummy_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create dummy .gcda file");
        return -1;
    }
    
    /* Write minimal gcov data file magic and version */
    unsigned int magic = 0x67636461; /* "gcda" in little-endian */
    unsigned int version = 0x4020000; /* GCC 4.2 format */
    unsigned int stamp = 0x12345678;
    
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    fwrite(&stamp, sizeof(stamp), 1, fp);
    
    /* Write zero records to indicate end of file */
    unsigned int tag = 0;
    unsigned int length = 0;
    fwrite(&tag, sizeof(tag), 1, fp);
    fwrite(&length, sizeof(length), 1, fp);
    
    fclose(fp);
    return 0;
}

/**
 * Compiles a trivial C program with coverage flags to generate real .gcda files.
 */
static int compile_coverage_test(void) {
    const char *source = 
        "int main() { return 0; }\n";
    
    FILE *fp = fopen("trivial.c", "w");
    if (!fp) {
        perror("Failed to create trivial.c");
        return -1;
    }
    fputs(source, fp);
    fclose(fp);
    
    /* Compile with coverage flags */
    int result = system("gcc -fprofile-arcs -ftest-coverage -O0 -o trivial trivial.c");
    if (result != 0) {
        fprintf(stderr, "Failed to compile trivial.c with coverage flags\n");
        return -1;
    }
    
    /* Run the program to generate .gcda file */
    result = system("./trivial");
    if (result != 0) {
        fprintf(stderr, "Failed to run trivial program\n");
        return -1;
    }
    
    return 0;
}

/**
 * Executes gcov-dump with given arguments using execvp.
 * Returns child process exit status.
 */
static int exec_gcov_dump(const char *args[]) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", (char *const *)args);
        
        /* If execvp returns, it failed */
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    }
}

/**
 * Executes gcov-dump using system() call.
 */
static void system_gcov_dump(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int result = system(cmd);
    printf("Exit status: %d\n\n", result);
}

int main(void) {
    printf("=== Testing gcov-dump flag parsing logic ===\n\n");
    
    /* Create test files */
    if (create_dummy_gcda("dummy.gcda") != 0) {
        fprintf(stderr, "Warning: Using placeholder for file tests\n");
    }
    
    /* Try to compile real coverage test if possible */
    compile_coverage_test();
    
    /* Test 1: Individual flag cases using execvp */
    printf("--- Test 1: Individual flags (execvp) ---\n");
    
    const char *individual_tests[][4] = {
        {"gcov-dump", "-h", NULL},                     /* Help flag */
        {"gcov-dump", "-v", NULL},                     /* Version flag */
        {"gcov-dump", "-l", NULL},                     /* Contents dump flag */
        {"gcov-dump", "-p", NULL},                     /* Positions dump flag */
        {"gcov-dump", "-r", NULL},                     /* Raw dump flag */
        {"gcov-dump", "-s", NULL},                     /* Stable dump flag */
        {"gcov-dump", "-x", NULL},                     /* Invalid flag (triggers default case) */
    };
    
    for (size_t i = 0; i < sizeof(individual_tests)/sizeof(individual_tests[0]); i++) {
        printf("Test %zu: ", i + 1);
        for (int j = 0; individual_tests[i][j] != NULL; j++) {
            printf("%s ", individual_tests[i][j]);
        }
        printf("\n");
        
        int status = exec_gcov_dump(individual_tests[i]);
        printf("Exit status: %d\n\n", status);
    }
    
    /* Test 2: Flag combinations using execvp */
    printf("--- Test 2: Flag combinations (execvp) ---\n");
    
    const char *combination_tests[][5] = {
        {"gcov-dump", "-l", "-p", NULL},               /* Two flags */
        {"gcov-dump", "-r", "-s", "-v", NULL},         /* Three flags */
        {"gcov-dump", "-p", "-p", NULL},               /* Repeated flag */
        {"gcov-dump", "-h", "-l", NULL},               /* Help with other flag */
        {"gcov-dump", "-l", "-p", "-r", "-s", NULL},   /* All valid flags */
    };
    
    for (size_t i = 0; i < sizeof(combination_tests)/sizeof(combination_tests[0]); i++) {
        printf("Test %zu: ", i + 1);
        for (int j = 0; combination_tests[i][j] != NULL; j++) {
            printf("%s ", combination_tests[i][j]);
        }
        printf("\n");
        
        int status = exec_gcov_dump(combination_tests[i]);
        printf("Exit status: %d\n\n", status);
    }
    
    /* Test 3: Different flag syntax styles using system() */
    printf("--- Test 3: Different flag syntax (system) ---\n");
    
    const char *syntax_tests[] = {
        "gcov-dump -l -p",                             /* Separate arguments */
        "gcov-dump -lp",                               /* Combined short options */
        "gcov-dump -l dummy.gcda",                     /* With positional argument */
        "gcov-dump -l -- dummy.gcda",                  /* With -- delimiter */
        "gcov-dump -l -p dummy.gcda",                  /* Multiple flags with file */
        "gcov-dump -lprs",                             /* All flags combined */
    };
    
    for (size_t i = 0; i < sizeof(syntax_tests)/sizeof(syntax_tests[0]); i++) {
        system_gcov_dump(syntax_tests[i]);
    }
    
    /* Test 4: Environment and edge cases */
    printf("--- Test 4: Environment and edge cases (system) ---\n");
    
    /* No arguments */
    system_gcov_dump("gcov-dump");
    
    /* Set environment variable if supported */
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    system_gcov_dump("gcov-dump");
    unsetenv("GCOV_DUMP_OPTIONS");
    
    /* Test with real .gcda file if available */
    system_gcov_dump("gcov-dump -l trivial.gcda");
    system_gcov_dump("gcov-dump -p trivial.gcda");
    
    /* Test error output redirection for invalid flag */
    printf("Testing invalid flag with stderr redirection:\n");
    system("gcov-dump -x 2>&1");
    printf("\n");
    
    /* Test multiple invalid flags */
    system_gcov_dump("gcov-dump -x -y -z");
    
    /* Test flag with equals sign (if supported) */
    system_gcov_dump("gcov-dump -l=dummy.gcda");
    
    /* Test very long argument list */
    printf("Testing long argument list:\n");
    system("gcov-dump -l -p -r -s -v -h dummy.gcda 2>&1 | head -5");
    printf("\n");
    
    /* Test 5: Stress tests with optimization flags in mind */
    printf("--- Test 5: Stress tests ---\n");
    
    /* Test with many repeated flags */
    system_gcov_dump("gcov-dump -p -p -p -p -p");
    
    /* Test mixed valid and invalid flags */
    system_gcov_dump("gcov-dump -l -x -p -y");
    
    /* Test empty string as argument */
    system("gcov-dump '' 2>&1");
    
    /* Test with special characters in filename */
    system("touch 'test file.gcda'");
    system_gcov_dump("gcov-dump -l 'test file.gcda'");
    system("rm -f 'test file.gcda'");
    
    /* Cleanup */
    printf("=== Cleaning up test files ===\n");
    system("rm -f dummy.gcda trivial trivial.c trivial.gcda trivial.gcno 2>/dev/null");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
