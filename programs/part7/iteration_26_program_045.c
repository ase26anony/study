#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

/* Create a minimal C program to generate a .gcda file */
const char* test_program = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Test program for coverage\\n\");\n"
"    return 0;\n"
"}\n";

/* Function to create a dummy .gcda file */
int create_dummy_gcda(void) {
    FILE *fp = fopen("test_coverage.c", "w");
    if (!fp) {
        perror("Failed to create test_coverage.c");
        return -1;
    }
    fprintf(fp, "%s", test_program);
    fclose(fp);
    
    /* Compile with coverage flags */
    int ret = system("gcc -fprofile-arcs -ftest-coverage -O0 test_coverage.c -o test_coverage");
    if (ret != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return -1;
    }
    
    /* Run to generate .gcda file */
    ret = system("./test_coverage > /dev/null 2>&1");
    if (ret != 0) {
        fprintf(stderr, "Failed to run test program\n");
        return -1;
    }
    
    return 0;
}

/* Execute gcov-dump using execvp */
void test_with_execvp(const char *args[]) {
    pid_t pid = fork();
    
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", (char *const *)args);
        /* If execvp returns, it failed */
        fprintf(stderr, "Failed to execute gcov-dump: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("gcov-dump exited with status: %d\n", WEXITSTATUS(status));
        } else {
            printf("gcov-dump terminated abnormally\n");
        }
    } else {
        perror("fork failed");
    }
}

/* Execute gcov-dump using system() */
void test_with_system(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int ret = system(cmd);
    printf("System call returned: %d\n", ret);
}

int main(void) {
    printf("=== Testing gcov-dump flag parsing ===\n\n");
    
    /* Create dummy .gcda file for tests that need it */
    if (create_dummy_gcda() != 0) {
        fprintf(stderr, "Warning: Could not create dummy .gcda file\n");
    }
    
    /* Test individual flags (using execvp for precise control) */
    printf("\n--- Testing individual flags with execvp ---\n");
    
    const char *help_args[] = {"gcov-dump", "-h", NULL};
    printf("\n1. Testing help flag (-h):\n");
    test_with_execvp(help_args);
    
    const char *version_args[] = {"gcov-dump", "-v", NULL};
    printf("\n2. Testing version flag (-v):\n");
    test_with_execvp(version_args);
    
    const char *contents_args[] = {"gcov-dump", "-l", NULL};
    printf("\n3. Testing contents dump flag (-l):\n");
    test_with_execvp(contents_args);
    
    const char *positions_args[] = {"gcov-dump", "-p", NULL};
    printf("\n4. Testing positions dump flag (-p):\n");
    test_with_execvp(positions_args);
    
    const char *raw_args[] = {"gcov-dump", "-r", NULL};
    printf("\n5. Testing raw dump flag (-r):\n");
    test_with_execvp(raw_args);
    
    const char *stable_args[] = {"gcov-dump", "-s", NULL};
    printf("\n6. Testing stable dump flag (-s):\n");
    test_with_execvp(stable_args);
    
    const char *invalid_args[] = {"gcov-dump", "-x", NULL};
    printf("\n7. Testing invalid flag (-x) to trigger default case:\n");
    test_with_execvp(invalid_args);
    
    /* Test flag combinations (using execvp) */
    printf("\n--- Testing flag combinations with execvp ---\n");
    
    const char *combo1_args[] = {"gcov-dump", "-l", "-p", NULL};
    printf("\n8. Testing combination -l -p:\n");
    test_with_execvp(combo1_args);
    
    const char *combo2_args[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    printf("\n9. Testing combination -r -s -v:\n");
    test_with_execvp(combo2_args);
    
    const char *combo3_args[] = {"gcov-dump", "-h", "-l", NULL};
    printf("\n10. Testing combination -h -l (h may cause early exit):\n");
    test_with_execvp(combo3_args);
    
    const char *combo4_args[] = {"gcov-dump", "-p", "-p", NULL};
    printf("\n11. Testing repeated flag -p -p:\n");
    test_with_execvp(combo4_args);
    
    /* Test combined short options (if supported by getopt) */
    const char *combined_args[] = {"gcov-dump", "-lp", NULL};
    printf("\n12. Testing combined short options -lp:\n");
    test_with_execvp(combined_args);
    
    /* Test with positional arguments (using system for convenience) */
    printf("\n--- Testing with positional arguments using system() ---\n");
    
    printf("\n13. Testing -l with dummy.gcda:\n");
    test_with_system("gcov-dump -l test_coverage.gcda 2>&1");
    
    printf("\n14. Testing -p with dummy.gcda:\n");
    test_with_system("gcov-dump -p test_coverage.gcda 2>&1");
    
    printf("\n15. Testing -- delimiter:\n");
    test_with_system("gcov-dump -l -- test_coverage.gcda 2>&1");
    
    /* Test different syntactic styles (using system) */
    printf("\n--- Testing different syntactic styles using system() ---\n");
    
    printf("\n16. Testing separate arguments -l -p with file:\n");
    test_with_system("gcov-dump -l -p test_coverage.gcda 2>&1");
    
    printf("\n17. Testing combined -lp with file:\n");
    test_with_system("gcov-dump -lp test_coverage.gcda 2>&1");
    
    /* Test environment and error contexts */
    printf("\n--- Testing environment and error contexts ---\n");
    
    printf("\n18. Testing with no arguments:\n");
    test_with_system("gcov-dump 2>&1");
    
    printf("\n19. Testing with environment variable GCOV_DUMP_OPTIONS:\n");
    test_with_system("GCOV_DUMP_OPTIONS='-l' gcov-dump 2>&1");
    
    printf("\n20. Testing invalid flag with stderr redirection:\n");
    test_with_system("gcov-dump -x 2>&1 | grep -q 'unknown flag' && echo 'Default case triggered'");
    
    /* Test with non-existent file */
    printf("\n21. Testing with non-existent file:\n");
    test_with_system("gcov-dump -l non_existent.gcda 2>&1");
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    system("rm -f test_coverage.c test_coverage test_coverage.gcda test_coverage.gcno 2>/dev/null");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
