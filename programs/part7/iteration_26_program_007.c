#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

/* Create a minimal dummy .gcda file for testing */
static void create_dummy_gcda(void) {
    /* Write a minimal valid .gcda header */
    FILE *f = fopen("dummy.gcda", "wb");
    if (f) {
        /* Gcda magic number and version */
        unsigned int magic = 0x67636461; /* 'gcda' */
        unsigned int version = 0x3430312A; /* '401*' */
        fwrite(&magic, sizeof(magic), 1, f);
        fwrite(&version, sizeof(version), 1, f);
        fclose(f);
        printf("Created dummy.gcda file\n");
    }
}

/* Test using execvp for precise argument control */
static void test_with_execvp(const char *test_name, char *const args[]) {
    printf("\n=== Testing with execvp: %s ===\n", test_name);
    printf("Command: ");
    for (int i = 0; args[i] != NULL; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", args);
        /* If execvp returns, there was an error */
        fprintf(stderr, "execvp failed: %s\n", strerror(errno));
        exit(1);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("Exit status: %d\n", WEXITSTATUS(status));
        }
    } else {
        fprintf(stderr, "fork failed: %s\n", strerror(errno));
    }
}

/* Test using system() for shell interpretation */
static void test_with_system(const char *test_name, const char *command) {
    printf("\n=== Testing with system(): %s ===\n", test_name);
    printf("Command: %s\n", command);
    
    int status = system(command);
    if (status == -1) {
        fprintf(stderr, "system() failed\n");
    } else if (WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    }
}

int main(void) {
    printf("=== Starting gcov-dump flag parsing tests ===\n");
    
    /* Create dummy .gcda file for file-based tests */
    create_dummy_gcda();
    
    /* Test individual flags (execvp tests) */
    char *help_args[] = {"gcov-dump", "-h", NULL};
    test_with_execvp("Help flag", help_args);
    
    char *version_args[] = {"gcov-dump", "-v", NULL};
    test_with_execvp("Version flag", version_args);
    
    char *contents_args[] = {"gcov-dump", "-l", NULL};
    test_with_execvp("Contents dump flag", contents_args);
    
    char *positions_args[] = {"gcov-dump", "-p", NULL};
    test_with_execvp("Positions dump flag", positions_args);
    
    char *raw_args[] = {"gcov-dump", "-r", NULL};
    test_with_execvp("Raw dump flag", raw_args);
    
    char *stable_args[] = {"gcov-dump", "-s", NULL};
    test_with_execvp("Stable dump flag", stable_args);
    
    char *invalid_args[] = {"gcov-dump", "-x", NULL};
    test_with_execvp("Invalid flag (should trigger default case)", invalid_args);
    
    /* Test flag combinations (execvp) */
    char *combo1_args[] = {"gcov-dump", "-l", "-p", NULL};
    test_with_execvp("Combination -l -p", combo1_args);
    
    char *combo2_args[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    test_with_execvp("Combination -r -s -v", combo2_args);
    
    char *combo3_args[] = {"gcov-dump", "-h", "-l", NULL};
    test_with_execvp("Combination -h -l (h may cause early exit)", combo3_args);
    
    char *repeat_args[] = {"gcov-dump", "-p", "-p", NULL};
    test_with_execvp("Repeated flag -p -p", repeat_args);
    
    /* Test with positional arguments (gcov files) */
    char *with_file_args[] = {"gcov-dump", "-l", "dummy.gcda", NULL};
    test_with_execvp("Flag with file argument -l dummy.gcda", with_file_args);
    
    char *with_file_combo_args[] = {"gcov-dump", "-r", "-s", "dummy.gcda", NULL};
    test_with_execvp("Multiple flags with file -r -s dummy.gcda", with_file_combo_args);
    
    /* Test with -- delimiter */
    char *delimiter_args[] = {"gcov-dump", "-l", "--", "dummy.gcda", NULL};
    test_with_execvp("Flag with -- delimiter", delimiter_args);
    
    /* Test no arguments */
    char *no_args[] = {"gcov-dump", NULL};
    test_with_execvp("No arguments", no_args);
    
    /* System() tests for different syntactic styles */
    test_with_system("Combined short options -lp", "gcov-dump -lp");
    test_with_system("Combined short options -lps", "gcov-dump -lps");
    test_with_system("Invalid combined options -lx", "gcov-dump -lx");
    
    /* Test with environment variable */
    test_with_system("With GCOV_DUMP_OPTIONS env var", 
                     "GCOV_DUMP_OPTIONS='-v' gcov-dump -l dummy.gcda");
    
    /* Test output redirection */
    test_with_system("Redirect stderr for invalid flag", 
                     "gcov-dump -x 2>stderr_output.txt; echo 'Stderr saved to file'");
    
    /* Test with empty string as argument (edge case) */
    char *empty_arg_args[] = {"gcov-dump", "", NULL};
    test_with_execvp("Empty string argument", empty_arg_args);
    
    /* Test with multiple files */
    char *multi_file_args[] = {"gcov-dump", "-l", "dummy.gcda", "nonexistent.gcda", NULL};
    test_with_execvp("Multiple file arguments", multi_file_args);
    
    printf("\n=== All tests completed ===\n");
    
    /* Cleanup */
    remove("dummy.gcda");
    remove("stderr_output.txt");
    
    return 0;
}
