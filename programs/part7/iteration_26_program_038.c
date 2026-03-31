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
    unsigned int magic = 0x67636461; /* 'gcda' in little-endian */
    unsigned int version = 0x3430372A; /* GCC 9.0+ version marker */
    unsigned int stamp = 0x12345678;
    
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    fwrite(&stamp, sizeof(stamp), 1, fp);
    
    /* Write empty tag and length for EOF */
    unsigned int tag = 0;
    unsigned int length = 0;
    fwrite(&tag, sizeof(tag), 1, fp);
    fwrite(&length, sizeof(length), 1, fp);
    
    fclose(fp);
    return 0;
}

/**
 * Compiles and runs a trivial C program to generate a real .gcda file.
 */
static int generate_real_gcda(const char *base_name) {
    char cmd[512];
    int ret;
    
    /* Create trivial C source */
    const char *source = 
        "int main() { return 0; }\n";
    
    FILE *src = fopen("trivial.c", "w");
    if (!src) {
        perror("Failed to create trivial.c");
        return -1;
    }
    fputs(source, src);
    fclose(src);
    
    /* Compile with coverage flags */
    snprintf(cmd, sizeof(cmd),
             "gcc -fprofile-arcs -ftest-coverage -o trivial trivial.c 2>/dev/null");
    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Warning: gcc compilation failed, using dummy .gcda\n");
        return -1;
    }
    
    /* Run the program to generate .gcda */
    ret = system("./trivial 2>/dev/null");
    if (ret != 0) {
        fprintf(stderr, "Warning: trivial program execution failed\n");
        return -1;
    }
    
    /* Copy the generated .gcda to our test name */
    snprintf(cmd, sizeof(cmd), "cp trivial.gcda %s 2>/dev/null", base_name);
    system(cmd);
    
    /* Cleanup intermediate files */
    system("rm -f trivial trivial.c trivial.gcno 2>/dev/null");
    
    return 0;
}

/**
 * Execute gcov-dump using execvp for precise argument control.
 */
static int exec_gcov_dump(const char *args[]) {
    pid_t pid = fork();
    
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", (char *const *)args);
        
        /* If execvp returns, it failed */
        fprintf(stderr, "execvp failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } 
    else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    } 
    else {
        /* Fork failed */
        perror("fork failed");
        return -1;
    }
}

/**
 * Execute gcov-dump using system() for shell interpretation testing.
 */
static int system_gcov_dump(const char *cmd) {
    int ret = system(cmd);
    
    if (ret == -1) {
        perror("system() failed");
        return -1;
    }
    
    return WEXITSTATUS(ret);
}

int main(void) {
    printf("=== Testing gcov-dump flag parsing logic ===\n\n");
    
    /* Create test .gcda files */
    const char *dummy_gcda = "test_dummy.gcda";
    const char *real_gcda = "test_real.gcda";
    
    /* Try to generate a real .gcda first, fall back to dummy */
    if (generate_real_gcda(real_gcda) != 0) {
        printf("Generating dummy .gcda file instead...\n");
        if (create_dummy_gcda(dummy_gcda) != 0) {
            fprintf(stderr, "Failed to create any .gcda file\n");
            return EXIT_FAILURE;
        }
        real_gcda = dummy_gcda; /* Use dummy for all tests */
    }
    
    /* Test case 1: Individual flags (direct switch cases) */
    printf("1. Testing individual flag cases:\n");
    printf("---------------------------------\n");
    
    const char *individual_flags[][3] = {
        {"gcov-dump", "-h", NULL},           /* Help */
        {"gcov-dump", "-v", NULL},           /* Version */
        {"gcov-dump", "-l", NULL},           /* Contents dump */
        {"gcov-dump", "-p", NULL},           /* Positions dump */
        {"gcov-dump", "-r", NULL},           /* Raw dump */
        {"gcov-dump", "-s", NULL},           /* Stable dump */
        {"gcov-dump", "-x", NULL},           /* Invalid flag (default case) */
        {NULL}
    };
    
    for (int i = 0; individual_flags[i][0] != NULL; i++) {
        printf("Testing: ");
        for (int j = 0; individual_flags[i][j] != NULL; j++) {
            printf("%s ", individual_flags[i][j]);
        }
        printf("\n");
        
        int ret = exec_gcov_dump(individual_flags[i]);
        printf("Exit code: %d\n\n", ret);
    }
    
    /* Test case 2: Flag combinations */
    printf("\n2. Testing flag combinations:\n");
    printf("------------------------------\n");
    
    const char *flag_combinations[][5] = {
        {"gcov-dump", "-l", "-p", NULL},                 /* Two flags */
        {"gcov-dump", "-r", "-s", "-v", NULL},           /* Three flags */
        {"gcov-dump", "-h", "-l", NULL},                 /* Help with other flag */
        {"gcov-dump", "-p", "-p", NULL},                 /* Repeated flag */
        {"gcov-dump", "-l", "-p", "-r", "-s", NULL},     /* All valid flags */
        {NULL}
    };
    
    for (int i = 0; flag_combinations[i][0] != NULL; i++) {
        printf("Testing: ");
        for (int j = 0; flag_combinations[i][j] != NULL; j++) {
            printf("%s ", flag_combinations[i][j]);
        }
        printf("\n");
        
        int ret = exec_gcov_dump(flag_combinations[i]);
        printf("Exit code: %d\n\n", ret);
    }
    
    /* Test case 3: Different syntactic styles using system() */
    printf("\n3. Testing different syntactic styles (using system()):\n");
    printf("--------------------------------------------------------\n");
    
    const char *system_tests[] = {
        /* Separate arguments with file */
        "gcov-dump -l %s",
        
        /* Combined short options */
        "gcov-dump -lp %s",
        
        /* With -- delimiter */
        "gcov-dump -l -- %s",
        
        /* Multiple combined flags */
        "gcov-dump -lprs %s",
        
        /* Mixed separate and combined */
        "gcov-dump -l -pr %s",
        
        /* Flag after file argument */
        "gcov-dump %s -l",
        
        NULL
    };
    
    for (int i = 0; system_tests[i] != NULL; i++) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), system_tests[i], real_gcda);
        
        printf("Testing: %s\n", cmd);
        
        int ret = system_gcov_dump(cmd);
        printf("Exit code: %d\n\n", ret);
    }
    
    /* Test case 4: Edge cases and environment */
    printf("\n4. Testing edge cases:\n");
    printf("-----------------------\n");
    
    /* No arguments */
    printf("Testing: gcov-dump (no arguments)\n");
    {
        const char *args[] = {"gcov-dump", NULL};
        int ret = exec_gcov_dump(args);
        printf("Exit code: %d\n\n", ret);
    }
    
    /* Only file argument */
    printf("Testing: gcov-dump %s\n", real_gcda);
    {
        const char *args[] = {"gcov-dump", real_gcda, NULL};
        int ret = exec_gcov_dump(args);
        printf("Exit code: %d\n\n", ret);
    }
    
    /* Invalid file with valid flags */
    printf("Testing: gcov-dump -l -p nonexistent.gcda\n");
    {
        const char *args[] = {"gcov-dump", "-l", "-p", "nonexistent.gcda", NULL};
        int ret = exec_gcov_dump(args);
        printf("Exit code: %d\n\n", ret);
    }
    
    /* Test with environment variable */
    printf("Testing with GCOV_DUMP_OPTIONS environment variable\n");
    {
        setenv("GCOV_DUMP_OPTIONS", "-v", 1);
        const char *args[] = {"gcov-dump", "-l", real_gcda, NULL};
        int ret = exec_gcov_dump(args);
        printf("Exit code: %d\n\n", ret);
        unsetenv("GCOV_DUMP_OPTIONS");
    }
    
    /* Test error output redirection for invalid flag */
    printf("Testing invalid flag with stderr redirection\n");
    {
        const char *cmd = "gcov-dump -x 2>&1";
        printf("Command: %s\n", cmd);
        int ret = system_gcov_dump(cmd);
        printf("Exit code: %d\n\n", ret);
    }
    
    /* Test multiple invalid flags */
    printf("Testing multiple invalid flags\n");
    {
        const char *args[] = {"gcov-dump", "-x", "-y", "-z", NULL};
        int ret = exec_gcov_dump(args);
        printf("Exit code: %d\n\n", ret);
    }
    
    /* Test case 5: Long arguments (if supported) */
    printf("\n5. Testing long argument equivalents:\n");
    printf("--------------------------------------\n");
    
    const char *long_args_tests[][3] = {
        {"gcov-dump", "--help", NULL},
        {"gcov-dump", "--version", NULL},
        {NULL}
    };
    
    for (int i = 0; long_args_tests[i][0] != NULL; i++) {
        printf("Testing: ");
        for (int j = 0; long_args_tests[i][j] != NULL; j++) {
            printf("%s ", long_args_tests[i][j]);
        }
        printf("\n");
        
        int ret = exec_gcov_dump(long_args_tests[i]);
        printf("Exit code: %d\n\n", ret);
    }
    
    /* Cleanup */
    printf("Cleaning up test files...\n");
    char cleanup_cmd[256];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -f %s %s trivial* 2>/dev/null", 
             dummy_gcda, real_gcda);
    system(cleanup_cmd);
    
    printf("\n=== All tests completed ===\n");
    return EXIT_SUCCESS;
}
