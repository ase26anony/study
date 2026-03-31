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
    if (fp) {
        /* Write GCOV data magic number and version */
        unsigned int magic = 0x67636461; /* 'gcda' */
        unsigned int version = 0x3430392a; /* GCC 9* format */
        unsigned int stamp = 0x12345678;
        
        fwrite(&magic, sizeof(magic), 1, fp);
        fwrite(&version, sizeof(version), 1, fp);
        fwrite(&stamp, sizeof(stamp), 1, fp);
        
        /* Write a simple tag (end of file) */
        unsigned int tag = 0;
        unsigned int length = 0;
        fwrite(&tag, sizeof(tag), 1, fp);
        fwrite(&length, sizeof(length), 1, fp);
        
        fclose(fp);
        printf("Created dummy.gcda file for testing\n");
    }
}

/* Execute gcov-dump using execvp */
void test_with_execvp(const char *args[]) {
    pid_t pid = fork();
    
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", (char *const *)args);
        /* If execvp returns, there was an error */
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        printf("Executed: ");
        for (int i = 0; args[i] != NULL; i++) {
            printf("%s ", args[i]);
        }
        printf("(exit status: %d)\n", WEXITSTATUS(status));
    } else {
        perror("fork failed");
    }
}

/* Execute gcov-dump using system() */
void test_with_system(const char *cmd) {
    int status = system(cmd);
    printf("System call: %s (exit status: %d)\n", cmd, WEXITSTATUS(status));
}

int main(void) {
    printf("=== Testing gcov-dump flag parsing logic ===\n\n");
    
    /* Create dummy .gcda file for file-based tests */
    create_dummy_gcda();
    
    /* Set environment variable if supported */
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    
    /* Test individual flags (using execvp for precise control) */
    printf("\n--- Testing individual flags ---\n");
    
    const char *help_args[] = {"gcov-dump", "-h", NULL};
    test_with_execvp(help_args);
    
    const char *version_args[] = {"gcov-dump", "-v", NULL};
    test_with_execvp(version_args);
    
    const char *contents_args[] = {"gcov-dump", "-l", NULL};
    test_with_execvp(contents_args);
    
    const char *positions_args[] = {"gcov-dump", "-p", NULL};
    test_with_execvp(positions_args);
    
    const char *raw_args[] = {"gcov-dump", "-r", NULL};
    test_with_execvp(raw_args);
    
    const char *stable_args[] = {"gcov-dump", "-s", NULL};
    test_with_execvp(stable_args);
    
    /* Test invalid flag (default case) */
    printf("\n--- Testing invalid flag (should trigger default case) ---\n");
    const char *invalid_args[] = {"gcov-dump", "-x", NULL};
    test_with_execvp(invalid_args);
    
    /* Test flag combinations (using system for variety) */
    printf("\n--- Testing flag combinations ---\n");
    test_with_system("gcov-dump -l -p");
    test_with_system("gcov-dump -r -s -v");
    test_with_system("gcov-dump -h -l");  /* -h may cause early exit */
    
    /* Test repeated flags */
    printf("\n--- Testing repeated flags ---\n");
    test_with_system("gcov-dump -p -p");
    test_with_system("gcov-dump -l -l -l");
    
    /* Test different flag syntax styles */
    printf("\n--- Testing different flag syntax ---\n");
    
    /* Combined short options (if supported by getopt) */
    const char *combined_args[] = {"gcov-dump", "-lp", NULL};
    test_with_execvp(combined_args);
    
    /* With positional arguments */
    const char *with_file_args[] = {"gcov-dump", "-l", "dummy.gcda", NULL};
    test_with_execvp(with_file_args);
    
    /* With -- delimiter */
    const char *with_delimiter_args[] = {"gcov-dump", "-l", "--", "dummy.gcda", NULL};
    test_with_execvp(with_delimiter_args);
    
    /* Test no arguments */
    printf("\n--- Testing no arguments ---\n");
    const char *no_args[] = {"gcov-dump", NULL};
    test_with_execvp(no_args);
    
    /* Test with environment variable cleared */
    printf("\n--- Testing with cleared environment ---\n");
    unsetenv("GCOV_DUMP_OPTIONS");
    test_with_system("gcov-dump -v");
    
    /* Test error stream redirection for invalid flag */
    printf("\n--- Testing error stream capture ---\n");
    test_with_system("gcov-dump -x 2>&1 | head -1");
    
    /* Test multiple files with flags */
    printf("\n--- Testing with multiple files ---\n");
    test_with_system("gcov-dump -l -p dummy.gcda 2>/dev/null");
    
    /* Test edge cases */
    printf("\n--- Testing edge cases ---\n");
    
    /* Empty string as argument */
    const char *edge_args1[] = {"gcov-dump", "", NULL};
    test_with_execvp(edge_args1);
    
    /* Very long flag string */
    char long_flag[100] = "gcov-dump -";
    for (int i = 0; i < 20; i++) {
        strcat(long_flag, "l");
    }
    strcat(long_flag, " 2>/dev/null");
    test_with_system(long_flag);
    
    /* Clean up */
    unlink("dummy.gcda");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
