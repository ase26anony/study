#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Create a minimal valid .gcda file for testing */
void create_dummy_gcda(void) {
    /* Create a minimal .gcda file header */
    FILE *fp = fopen("dummy.gcda", "wb");
    if (fp) {
        /* Write minimal GCOV data magic and version */
        unsigned int magic = 0x67636461; /* 'gcda' */
        unsigned int version = 0x4030000; /* GCOV version */
        unsigned int stamp = 0x12345678;
        
        fwrite(&magic, sizeof(magic), 1, fp);
        fwrite(&version, sizeof(version), 1, fp);
        fwrite(&stamp, sizeof(stamp), 1, fp);
        
        /* Write a zero tag to terminate */
        unsigned int zero = 0;
        fwrite(&zero, sizeof(zero), 1, fp);
        fwrite(&zero, sizeof(zero), 1, fp);
        
        fclose(fp);
        printf("Created dummy.gcda for testing\n");
    }
}

/* Execute gcov-dump using execvp with precise argument control */
void exec_gcov_dump(const char *args[], const char *description) {
    printf("\n=== Testing: %s ===\n", description);
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", (char *const *)args);
        /* If execvp fails */
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("Exit status: %d\n", WEXITSTATUS(status));
        }
    } else {
        perror("fork failed");
    }
}

/* Execute using system() for comparison */
void system_gcov_dump(const char *cmd, const char *description) {
    printf("\n=== Testing (system): %s ===\n", description);
    int status = system(cmd);
    printf("Exit status: %d\n", status);
}

int main(void) {
    printf("=== gcov-dump Flag Parser Test Suite ===\n");
    
    /* Create dummy .gcda file for file-based tests */
    create_dummy_gcda();
    
    /* Test 1: Individual flags (using execvp for precise control) */
    const char *test1[] = {"gcov-dump", "-h", NULL};
    const char *test2[] = {"gcov-dump", "-v", NULL};
    const char *test3[] = {"gcov-dump", "-l", NULL};
    const char *test4[] = {"gcov-dump", "-p", NULL};
    const char *test5[] = {"gcov-dump", "-r", NULL};
    const char *test6[] = {"gcov-dump", "-s", NULL};
    const char *test7[] = {"gcov-dump", "-x", NULL}; /* Invalid flag */
    
    exec_gcov_dump(test1, "Help flag (-h)");
    exec_gcov_dump(test2, "Version flag (-v)");
    exec_gcov_dump(test3, "Contents dump flag (-l)");
    exec_gcov_dump(test4, "Positions dump flag (-p)");
    exec_gcov_dump(test5, "Raw dump flag (-r)");
    exec_gcov_dump(test6, "Stable dump flag (-s)");
    exec_gcov_dump(test7, "Invalid flag (-x) - should trigger default case");
    
    /* Test 2: Multiple valid flags in separate arguments */
    const char *test8[] = {"gcov-dump", "-l", "-p", NULL};
    const char *test9[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    const char *test10[] = {"gcov-dump", "-h", "-l", NULL}; /* -h may exit early */
    const char *test11[] = {"gcov-dump", "-p", "-p", NULL}; /* Repeated flag */
    
    exec_gcov_dump(test8, "Multiple flags: -l -p");
    exec_gcov_dump(test9, "Multiple flags: -r -s -v");
    exec_gcov_dump(test10, "Multiple flags: -h -l (h may cause early exit)");
    exec_gcov_dump(test11, "Repeated flag: -p -p");
    
    /* Test 3: Combined short options (if getopt supports it) */
    const char *test12[] = {"gcov-dump", "-lp", NULL};
    const char *test13[] = {"gcov-dump", "-rs", NULL};
    
    exec_gcov_dump(test12, "Combined flags: -lp");
    exec_gcov_dump(test13, "Combined flags: -rs");
    
    /* Test 4: Flags with positional arguments (.gcda files) */
    const char *test14[] = {"gcov-dump", "-l", "dummy.gcda", NULL};
    const char *test15[] = {"gcov-dump", "-p", "-r", "dummy.gcda", NULL};
    const char *test16[] = {"gcov-dump", "-l", "--", "dummy.gcda", NULL};
    
    exec_gcov_dump(test14, "Flag with file: -l dummy.gcda");
    exec_gcov_dump(test15, "Multiple flags with file: -p -r dummy.gcda");
    exec_gcov_dump(test16, "Flag with -- delimiter: -l -- dummy.gcda");
    
    /* Test 5: No arguments */
    const char *test17[] = {"gcov-dump", NULL};
    exec_gcov_dump(test17, "No arguments");
    
    /* Test 6: Using system() calls for different execution path */
    system_gcov_dump("gcov-dump -h 2>&1", "system(): Help flag with stderr redirect");
    system_gcov_dump("gcov-dump -x 2>&1", "system(): Invalid flag with stderr redirect");
    system_gcov_dump("gcov-dump -l -p dummy.gcda 2>&1", "system(): Multiple flags with file");
    
    /* Test 7: Environment variable testing */
    printf("\n=== Testing with environment variables ===\n");
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    const char *test18[] = {"gcov-dump", "-l", NULL};
    exec_gcov_dump(test18, "With GCOV_DUMP_OPTIONS=-v and -l flag");
    unsetenv("GCOV_DUMP_OPTIONS");
    
    /* Test 8: Edge cases with special characters */
    const char *test19[] = {"gcov-dump", "-", NULL}; /* Just a dash */
    const char *test20[] = {"gcov-dump", "--", NULL}; /* Just double dash */
    const char *test21[] = {"gcov-dump", "-l", "-", NULL}; /* Flag with dash as file */
    
    exec_gcov_dump(test19, "Single dash argument");
    exec_gcov_dump(test20, "Double dash only");
    exec_gcov_dump(test21, "Flag with dash as filename");
    
    /* Test 9: Long arguments (if supported) */
    const char *test22[] = {"gcov-dump", "--help", NULL};
    const char *test23[] = {"gcov-dump", "--version", NULL};
    
    exec_gcov_dump(test22, "Long option --help");
    exec_gcov_dump(test23, "Long option --version");
    
    /* Clean up */
    remove("dummy.gcda");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
