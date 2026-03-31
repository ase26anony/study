#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Create a minimal valid .gcda file for testing */
void create_dummy_gcda(void) {
    /* Create a minimal .gcda file structure */
    FILE *fp = fopen("dummy.gcda", "wb");
    if (!fp) {
        perror("Failed to create dummy.gcda");
        return;
    }
    
    /* Write minimal GCOV data magic and version */
    unsigned int magic = 0x67636461; /* "gcda" in little-endian */
    unsigned int version = 0x4020000; /* GCOV version */
    unsigned int stamp = 0x12345678;
    
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    fwrite(&stamp, sizeof(stamp), 1, fp);
    
    /* Write a zero terminator for empty records */
    unsigned int zero = 0;
    fwrite(&zero, sizeof(zero), 1, fp);
    
    fclose(fp);
}

/* Execute gcov-dump using execvp for precise argument control */
void test_execvp(const char *args[]) {
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

/* Execute using system() for shell interpretation testing */
void test_system(const char *cmd) {
    printf("Testing with system(): %s\n", cmd);
    int status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(status));
}

int main(void) {
    printf("=== Testing gcov-dump flag parsing ===\n\n");
    
    /* Create dummy .gcda file for file-based tests */
    create_dummy_gcda();
    
    /* Test 1: Individual flags (direct switch cases) */
    printf("--- Testing individual flags ---\n");
    
    const char *help_args[] = {"gcov-dump", "-h", NULL};
    printf("\n1. Help flag (-h):\n");
    test_execvp(help_args);
    
    const char *version_args[] = {"gcov-dump", "-v", NULL};
    printf("\n2. Version flag (-v):\n");
    test_execvp(version_args);
    
    const char *contents_args[] = {"gcov-dump", "-l", NULL};
    printf("\n3. Contents dump flag (-l):\n");
    test_execvp(contents_args);
    
    const char *positions_args[] = {"gcov-dump", "-p", NULL};
    printf("\n4. Positions dump flag (-p):\n");
    test_execvp(positions_args);
    
    const char *raw_args[] = {"gcov-dump", "-r", NULL};
    printf("\n5. Raw dump flag (-r):\n");
    test_execvp(raw_args);
    
    const char *stable_args[] = {"gcov-dump", "-s", NULL};
    printf("\n6. Stable dump flag (-s):\n");
    test_execvp(stable_args);
    
    /* Test 2: Invalid flag (default case) */
    printf("\n--- Testing invalid flag (default case) ---\n");
    const char *invalid_args[] = {"gcov-dump", "-x", NULL};
    test_execvp(invalid_args);
    
    /* Test 3: No arguments */
    printf("\n--- Testing no arguments ---\n");
    const char *no_args[] = {"gcov-dump", NULL};
    test_execvp(no_args);
    
    /* Test 4: Combined valid flags */
    printf("\n--- Testing combined valid flags ---\n");
    
    const char *combo1_args[] = {"gcov-dump", "-l", "-p", NULL};
    printf("\n1. -l -p combination:\n");
    test_execvp(combo1_args);
    
    const char *combo2_args[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    printf("\n2. -r -s -v combination:\n");
    test_execvp(combo2_args);
    
    const char *combo3_args[] = {"gcov-dump", "-h", "-l", NULL};
    printf("\n3. -h -l combination (h may exit early):\n");
    test_execvp(combo3_args);
    
    /* Test 5: Repeated flags */
    printf("\n--- Testing repeated flags ---\n");
    const char *repeat_args[] = {"gcov-dump", "-p", "-p", NULL};
    test_execvp(repeat_args);
    
    /* Test 6: Different flag syntax styles using system() */
    printf("\n--- Testing different flag syntax styles ---\n");
    
    /* Combined short options (if supported by getopt) */
    test_system("gcov-dump -lp");
    
    /* Separate arguments */
    test_system("gcov-dump -l -p");
    
    /* With positional arguments */
    test_system("gcov-dump -l dummy.gcda");
    
    /* With -- delimiter */
    test_system("gcov-dump -l -- dummy.gcda");
    
    /* Test 7: Environment variable testing */
    printf("\n--- Testing with environment variables ---\n");
    
    /* Set environment variable before execution */
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    test_system("gcov-dump");
    
    /* Clear environment variable */
    unsetenv("GCOV_DUMP_OPTIONS");
    
    /* Test 8: Error stream redirection */
    printf("\n--- Testing error stream capture ---\n");
    
    /* Test invalid flag with stderr redirection */
    test_system("gcov-dump -x 2>&1");
    
    /* Test 9: Multiple combinations with file arguments */
    printf("\n--- Testing with file arguments ---\n");
    
    const char *file_args1[] = {"gcov-dump", "-l", "dummy.gcda", NULL};
    test_execvp(file_args1);
    
    const char *file_args2[] = {"gcov-dump", "-p", "-r", "dummy.gcda", NULL};
    test_execvp(file_args2);
    
    /* Test 10: Edge cases */
    printf("\n--- Testing edge cases ---\n");
    
    /* Empty string as argument */
    const char *edge_args1[] = {"gcov-dump", "", NULL};
    test_execvp(edge_args1);
    
    /* Very long flag string */
    test_system("gcov-dump -lprs 2>&1");
    
    /* Clean up */
    remove("dummy.gcda");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
