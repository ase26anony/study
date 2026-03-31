#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Create a minimal dummy .gcda file for testing */
void create_dummy_gcda(void) {
    /* Create a minimal valid .gcda file header */
    FILE *fp = fopen("dummy.gcda", "wb");
    if (fp) {
        /* GCOV data magic number and version */
        unsigned int magic = 0x67636461; /* 'gcda' */
        unsigned int version = 0x4020000; /* GCC 8.2.0 */
        unsigned int stamp = 0x12345678;
        
        fwrite(&magic, sizeof(magic), 1, fp);
        fwrite(&version, sizeof(version), 1, fp);
        fwrite(&stamp, sizeof(stamp), 1, fp);
        
        /* Write a zero length indicating end of data */
        unsigned int zero = 0;
        fwrite(&zero, sizeof(zero), 1, fp);
        
        fclose(fp);
        printf("Created dummy.gcda file for testing\n");
    }
}

/* Test using execvp for precise argument control */
void test_with_execvp(const char *test_name, char *const args[]) {
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

/* Test using system() calls */
void test_with_system(const char *test_name, const char *command) {
    printf("\n=== Testing with system(): %s ===\n", test_name);
    printf("Command: %s\n", command);
    
    int status = system(command);
    if (status == -1) {
        perror("system() failed");
    } else {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    }
}

int main(void) {
    printf("=== Starting gcov-dump flag parsing tests ===\n");
    
    /* Create dummy .gcda file for file-based tests */
    create_dummy_gcda();
    
    /* Set environment variable that might affect gcov-dump */
    setenv("GCOV_DUMP_OPTIONS", "--help", 1);
    
    /* ========== Test individual flags with execvp ========== */
    
    /* Case 'h': Help flag */
    char *help_args[] = {"gcov-dump", "-h", NULL};
    test_with_execvp("Help flag (-h)", help_args);
    
    /* Case 'v': Version flag */
    char *version_args[] = {"gcov-dump", "-v", NULL};
    test_with_execvp("Version flag (-v)", version_args);
    
    /* Case 'l': Contents dump flag */
    char *contents_args[] = {"gcov-dump", "-l", NULL};
    test_with_execvp("Contents dump flag (-l)", contents_args);
    
    /* Case 'p': Positions dump flag */
    char *positions_args[] = {"gcov-dump", "-p", NULL};
    test_with_execvp("Positions dump flag (-p)", positions_args);
    
    /* Case 'r': Raw dump flag */
    char *raw_args[] = {"gcov-dump", "-r", NULL};
    test_with_execvp("Raw dump flag (-r)", raw_args);
    
    /* Case 's': Stable dump flag */
    char *stable_args[] = {"gcov-dump", "-s", NULL};
    test_with_execvp("Stable dump flag (-s)", stable_args);
    
    /* Default case: Invalid flag */
    char *invalid_args[] = {"gcov-dump", "-x", NULL};
    test_with_execvp("Invalid flag (-x) - should trigger default case", invalid_args);
    
    /* ========== Test flag combinations with execvp ========== */
    
    /* Combination 1: -l -p */
    char *combo1_args[] = {"gcov-dump", "-l", "-p", NULL};
    test_with_execvp("Combination -l -p", combo1_args);
    
    /* Combination 2: -r -s -v */
    char *combo2_args[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    test_with_execvp("Combination -r -s -v", combo2_args);
    
    /* Combination 3: -h -l (help might exit early) */
    char *combo3_args[] = {"gcov-dump", "-h", "-l", NULL};
    test_with_execvp("Combination -h -l", combo3_args);
    
    /* Repeated flag: -p -p */
    char *repeat_args[] = {"gcov-dump", "-p", "-p", NULL};
    test_with_execvp("Repeated flag -p -p", repeat_args);
    
    /* ========== Test different flag syntax with execvp ========== */
    
    /* Combined short options: -lp */
    char *combined_args[] = {"gcov-dump", "-lp", NULL};
    test_with_execvp("Combined short options -lp", combined_args);
    
    /* With positional argument (dummy.gcda) */
    char *with_file_args[] = {"gcov-dump", "-l", "dummy.gcda", NULL};
    test_with_execvp("With file argument -l dummy.gcda", with_file_args);
    
    /* With -- delimiter */
    char *with_delimiter_args[] = {"gcov-dump", "-l", "--", "dummy.gcda", NULL};
    test_with_execvp("With delimiter -l -- dummy.gcda", with_delimiter_args);
    
    /* ========== Test with system() calls ========== */
    
    /* No arguments */
    test_with_system("No arguments", "gcov-dump");
    
    /* Combined flags with system() */
    test_with_system("Combined flags -rsv", "gcov-dump -r -s -v");
    
    /* Invalid flag with system() */
    test_with_system("Invalid flag -z", "gcov-dump -z 2>&1");
    
    /* Test with environment variable */
    test_with_system("With GCOV_DUMP_OPTIONS env var", 
                    "GCOV_DUMP_OPTIONS='-v' gcov-dump -l 2>&1");
    
    /* Test output redirection */
    test_with_system("With stderr redirection for invalid flag", 
                    "gcov-dump -x 2>&1 | head -5");
    
    /* ========== Additional edge cases ========== */
    
    /* Empty string as argument */
    char *empty_arg_args[] = {"gcov-dump", "", NULL};
    test_with_execvp("Empty string argument", empty_arg_args);
    
    /* Multiple files */
    char *multi_file_args[] = {"gcov-dump", "-l", "dummy.gcda", "dummy.gcda", NULL};
    test_with_execvp("Multiple file arguments", multi_file_args);
    
    /* Flag at end */
    char *flag_end_args[] = {"gcov-dump", "dummy.gcda", "-l", NULL};
    test_with_execvp("Flag at end dummy.gcda -l", flag_end_args);
    
    /* Clean up */
    unlink("dummy.gcda");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
