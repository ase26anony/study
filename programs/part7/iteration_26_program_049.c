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
        /* Write GCC GCOV data magic number and version */
        unsigned int magic = 0x67636461; /* 'gcda' */
        unsigned int version = 0x4020000; /* GCC 8.2.0 */
        unsigned int stamp = 0x12345678;
        
        fwrite(&magic, sizeof(magic), 1, fp);
        fwrite(&version, sizeof(version), 1, fp);
        fwrite(&stamp, sizeof(stamp), 1, fp);
        
        /* Write a zero terminator for empty data */
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

/* Test using system() calls */
void test_with_system(const char *test_name, const char *command) {
    printf("\n=== Testing with system(): %s ===\n", test_name);
    printf("Command: %s\n", command);
    
    int status = system(command);
    if (status == -1) {
        perror("system() failed");
    } else if (WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    }
}

int main(void) {
    printf("Starting gcov-dump flag parsing tests...\n");
    
    /* Create dummy .gcda file for file-based tests */
    create_dummy_gcda();
    
    /* Set environment variable that might affect gcov-dump */
    setenv("GCOV_DUMP_OPTIONS", "--help", 1);
    
    /* ========== Individual flag tests using execvp ========== */
    
    /* 1. Help flag (case 'h') */
    char *help_args[] = {"gcov-dump", "-h", NULL};
    test_with_execvp("Help flag (-h)", help_args);
    
    /* 2. Version flag (case 'v') */
    char *version_args[] = {"gcov-dump", "-v", NULL};
    test_with_execvp("Version flag (-v)", version_args);
    
    /* 3. Contents dump flag (case 'l') */
    char *contents_args[] = {"gcov-dump", "-l", NULL};
    test_with_execvp("Contents dump flag (-l)", contents_args);
    
    /* 4. Positions dump flag (case 'p') */
    char *positions_args[] = {"gcov-dump", "-p", NULL};
    test_with_execvp("Positions dump flag (-p)", positions_args);
    
    /* 5. Raw dump flag (case 'r') */
    char *raw_args[] = {"gcov-dump", "-r", NULL};
    test_with_execvp("Raw dump flag (-r)", raw_args);
    
    /* 6. Stable dump flag (case 's') */
    char *stable_args[] = {"gcov-dump", "-s", NULL};
    test_with_execvp("Stable dump flag (-s)", stable_args);
    
    /* 7. Invalid flag (default case) */
    char *invalid_args[] = {"gcov-dump", "-x", NULL};
    test_with_execvp("Invalid flag (-x) - should trigger default case", invalid_args);
    
    /* ========== Combination flag tests ========== */
    
    /* 8. Multiple valid flags combined */
    char *combo1_args[] = {"gcov-dump", "-l", "-p", NULL};
    test_with_execvp("Multiple flags (-l -p)", combo1_args);
    
    /* 9. Three flags combined */
    char *combo2_args[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    test_with_execvp("Three flags (-r -s -v)", combo2_args);
    
    /* 10. Help with other flags (may exit early) */
    char *combo3_args[] = {"gcov-dump", "-h", "-l", NULL};
    test_with_execvp("Help with other flag (-h -l)", combo3_args);
    
    /* 11. Repeated same flag */
    char *repeat_args[] = {"gcov-dump", "-p", "-p", NULL};
    test_with_execvp("Repeated flag (-p -p)", repeat_args);
    
    /* ========== Different syntactic styles ========== */
    
    /* 12. Combined short options (if supported) */
    char *combined_args[] = {"gcov-dump", "-lp", NULL};
    test_with_execvp("Combined short options (-lp)", combined_args);
    
    /* 13. With positional argument (gcov file) */
    char *with_file_args[] = {"gcov-dump", "-l", "dummy.gcda", NULL};
    test_with_execvp("With file argument (-l dummy.gcda)", with_file_args);
    
    /* 14. With -- delimiter */
    char *with_delimiter_args[] = {"gcov-dump", "-l", "--", "dummy.gcda", NULL};
    test_with_execvp("With -- delimiter (-l -- dummy.gcda)", with_delimiter_args);
    
    /* ========== Environment and error contexts ========== */
    
    /* 15. No arguments */
    char *no_args[] = {"gcov-dump", NULL};
    test_with_execvp("No arguments", no_args);
    
    /* 16. Clear environment variable and test */
    unsetenv("GCOV_DUMP_OPTIONS");
    char *no_env_args[] = {"gcov-dump", "-v", NULL};
    test_with_execvp("Version flag with cleared env", no_env_args);
    
    /* ========== Tests using system() ========== */
    
    /* 17. Test with system() - basic flags */
    test_with_system("System test: help", "gcov-dump -h");
    
    /* 18. Test with system() - invalid flag (should go to stderr) */
    test_with_system("System test: invalid flag", "gcov-dump -x 2>&1");
    
    /* 19. Test with system() - combination with output redirection */
    test_with_system("System test: combination with file", "gcov-dump -l -p dummy.gcda 2>&1");
    
    /* 20. Test with system() - empty command (just program name) */
    test_with_system("System test: empty", "gcov-dump");
    
    /* 21. Test with system() - complex combination */
    test_with_system("System test: complex", "gcov-dump -rsv 2>&1");
    
    /* Clean up */
    unlink("dummy.gcda");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
