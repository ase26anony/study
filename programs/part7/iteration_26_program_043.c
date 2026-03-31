#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Create a minimal C program to generate a .gcda file */
const char* test_program = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Test program for coverage\\n\");\n"
"    return 0;\n"
"}\n";

/* Function to create a dummy .gcda file for testing */
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
    status = system("gcc -fprofile-arcs -ftest-coverage test_gcov.c -o test_gcov");
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
    
    /* Check if .gcda file was created */
    struct stat st;
    if (stat("test_gcov.gcda", &st) != 0) {
        fprintf(stderr, "No .gcda file generated\n");
        return -1;
    }
    
    return 0;
}

/* Execute gcov-dump with given arguments using execvp */
void test_with_execvp(const char *description, char *const args[]) {
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
        execvp("gcov-dump", args);
        /* If execvp returns, it failed */
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        /* Parent process */
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("Exit status: %d\n", WEXITSTATUS(status));
        }
    } else {
        perror("fork failed");
    }
}

/* Execute gcov-dump with given command string using system() */
void test_with_system(const char *description, const char *command) {
    int status;
    
    printf("\n=== Testing: %s ===\n", description);
    printf("Command: %s\n", command);
    
    status = system(command);
    printf("Exit status: %d\n", status);
}

int main(void) {
    /* Create dummy .gcda file for file-based tests */
    if (create_dummy_gcda() != 0) {
        fprintf(stderr, "Warning: Could not create dummy .gcda file\n");
    }
    
    printf("Starting gcov-dump flag parsing tests...\n");
    
    /* Test individual flags (using execvp) */
    char *test1[] = {"gcov-dump", "-h", NULL};
    test_with_execvp("Help flag", test1);
    
    char *test2[] = {"gcov-dump", "-v", NULL};
    test_with_execvp("Version flag", test2);
    
    char *test3[] = {"gcov-dump", "-l", NULL};
    test_with_execvp("Contents dump flag", test3);
    
    char *test4[] = {"gcov-dump", "-p", NULL};
    test_with_execvp("Positions dump flag", test4);
    
    char *test5[] = {"gcov-dump", "-r", NULL};
    test_with_execvp("Raw dump flag", test5);
    
    char *test6[] = {"gcov-dump", "-s", NULL};
    test_with_execvp("Stable dump flag", test6);
    
    char *test7[] = {"gcov-dump", "-x", NULL};
    test_with_execvp("Invalid flag (should trigger default case)", test7);
    
    /* Test flag combinations (using execvp) */
    char *test8[] = {"gcov-dump", "-l", "-p", NULL};
    test_with_execvp("Combination: -l -p", test8);
    
    char *test9[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    test_with_execvp("Combination: -r -s -v", test9);
    
    char *test10[] = {"gcov-dump", "-h", "-l", NULL};
    test_with_execvp("Combination: -h -l (h may cause early exit)", test10);
    
    char *test11[] = {"gcov-dump", "-p", "-p", NULL};
    test_with_execvp("Repeated flag: -p -p", test11);
    
    /* Test combined short options (if supported) */
    char *test12[] = {"gcov-dump", "-lp", NULL};
    test_with_execvp("Combined short options: -lp", test12);
    
    char *test13[] = {"gcov-dump", "-rs", NULL};
    test_with_execvp("Combined short options: -rs", test13);
    
    /* Test with positional arguments (using system for easier file handling) */
    test_with_system("With positional argument", "gcov-dump -l test_gcov.gcda");
    
    test_with_system("With -- delimiter", "gcov-dump -l -- test_gcov.gcda");
    
    /* Test no arguments */
    char *test14[] = {"gcov-dump", NULL};
    test_with_execvp("No arguments", test14);
    
    /* Test environment variable influence (using system) */
    test_with_system("With GCOV_DUMP_OPTIONS env var", 
                     "GCOV_DUMP_OPTIONS='-v' gcov-dump -l");
    
    /* Test error output redirection for invalid flag */
    test_with_system("Invalid flag with stderr redirection", 
                     "gcov-dump -x 2>&1");
    
    /* Test multiple invalid flags */
    char *test15[] = {"gcov-dump", "-x", "-y", "-z", NULL};
    test_with_execvp("Multiple invalid flags", test15);
    
    /* Test mixed valid and invalid flags */
    char *test16[] = {"gcov-dump", "-l", "-x", "-p", NULL};
    test_with_execvp("Mixed valid and invalid flags", test16);
    
    /* Test with empty string as argument */
    char *test17[] = {"gcov-dump", "", NULL};
    test_with_execvp("Empty string argument", test17);
    
    /* Test with only -- */
    char *test18[] = {"gcov-dump", "--", NULL};
    test_with_execvp("Only -- delimiter", test18);
    
    /* Test with file that doesn't exist */
    test_with_system("With non-existent file", "gcov-dump -l nonexistent.gcda");
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    system("rm -f test_gcov test_gcov.c test_gcov.gcda test_gcov.gcno");
    
    printf("\nAll tests completed.\n");
    return 0;
}
