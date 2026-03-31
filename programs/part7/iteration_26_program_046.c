/**
 * gcov-dump_parser_test.c
 * 
 * Comprehensive test program for gcov-dump command-line flag parsing.
 * Specifically targets the switch statement in gcov-dump.cc lines 111-130.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/**
 * Creates a minimal valid .gcda file for testing.
 * Returns 0 on success, -1 on failure.
 */
int create_dummy_gcda_file(void) {
    FILE *fp = fopen("dummy.gcda", "wb");
    if (!fp) {
        perror("Failed to create dummy.gcda");
        return -1;
    }
    
    /* Write minimal Gcda file header (magic + version) */
    unsigned int magic = 0x67636461; /* 'gcda' in little-endian */
    unsigned int version = 0x20190503; /* Typical GCC version */
    
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    
    /* Write zero tag to terminate */
    unsigned int zero_tag = 0;
    fwrite(&zero_tag, sizeof(zero_tag), 1, fp);
    
    fclose(fp);
    return 0;
}

/**
 * Compiles a trivial C program with coverage flags to generate a real .gcda file.
 */
int compile_coverage_test(void) {
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
    int result = system("gcc -fprofile-arcs -ftest-coverage -O0 trivial.c -o trivial");
    if (result != 0) {
        fprintf(stderr, "Warning: Failed to compile with coverage flags\n");
        return -1;
    }
    
    /* Run to generate .gcda */
    system("./trivial > /dev/null 2>&1");
    
    return 0;
}

/**
 * Executes gcov-dump with given arguments using execvp.
 * Returns exit status of gcov-dump.
 */
int exec_gcov_dump(const char *args[]) {
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
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    } else {
        /* Fork failed */
        perror("fork failed");
        return -1;
    }
}

/**
 * Executes gcov-dump using system() call.
 */
void system_gcov_dump(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int result = system(cmd);
    printf("Exit status: %d\n\n", result);
}

int main(void) {
    printf("=== Testing gcov-dump Flag Parser ===\n\n");
    
    /* Create test files */
    printf("Creating test files...\n");
    create_dummy_gcda_file();
    compile_coverage_test();
    printf("Test files created.\n\n");
    
    /* Test cases targeting specific switch cases */
    
    /* 1. Individual flag tests (direct switch cases) */
    printf("--- Testing Individual Flags ---\n");
    
    /* Case 'h': Help flag */
    const char *help_args[] = {"gcov-dump", "-h", NULL};
    exec_gcov_dump(help_args);
    
    /* Case 'v': Version flag */
    const char *version_args[] = {"gcov-dump", "-v", NULL};
    exec_gcov_dump(version_args);
    
    /* Case 'l': Contents dump flag */
    const char *contents_args[] = {"gcov-dump", "-l", NULL};
    exec_gcov_dump(contents_args);
    
    /* Case 'p': Positions dump flag */
    const char *positions_args[] = {"gcov-dump", "-p", NULL};
    exec_gcov_dump(positions_args);
    
    /* Case 'r': Raw dump flag */
    const char *raw_args[] = {"gcov-dump", "-r", NULL};
    exec_gcov_dump(raw_args);
    
    /* Case 's': Stable dump flag */
    const char *stable_args[] = {"gcov-dump", "-s", NULL};
    exec_gcov_dump(stable_args);
    
    /* Default case: Invalid flag */
    const char *invalid_args[] = {"gcov-dump", "-x", NULL};
    exec_gcov_dump(invalid_args);
    
    /* 2. Combination of valid flags */
    printf("\n--- Testing Flag Combinations ---\n");
    
    /* Combined short options (if supported) */
    const char *combined_args1[] = {"gcov-dump", "-lp", NULL};
    exec_gcov_dump(combined_args1);
    
    const char *combined_args2[] = {"gcov-dump", "-rsv", NULL};
    exec_gcov_dump(combined_args2);
    
    /* Separate arguments */
    const char *separate_args1[] = {"gcov-dump", "-l", "-p", NULL};
    exec_gcov_dump(separate_args1);
    
    const char *separate_args2[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    exec_gcov_dump(separate_args2);
    
    /* Same flag repeated */
    const char *repeat_args[] = {"gcov-dump", "-p", "-p", NULL};
    exec_gcov_dump(repeat_args);
    
    /* 3. Flags with positional arguments */
    printf("\n--- Testing Flags with File Arguments ---\n");
    
    const char *with_file1[] = {"gcov-dump", "-l", "dummy.gcda", NULL};
    exec_gcov_dump(with_file1);
    
    const char *with_file2[] = {"gcov-dump", "-p", "-r", "trivial.gcda", NULL};
    exec_gcov_dump(with_file2);
    
    /* With -- delimiter */
    const char *with_delimiter[] = {"gcov-dump", "-l", "--", "dummy.gcda", NULL};
    exec_gcov_dump(with_delimiter);
    
    /* 4. Environment and error contexts using system() */
    printf("\n--- Testing with system() calls ---\n");
    
    /* No arguments */
    system_gcov_dump("gcov-dump");
    
    /* Help with system() */
    system_gcov_dump("gcov-dump -h");
    
    /* Invalid flag with system() */
    system_gcov_dump("gcov-dump -x 2>&1");
    
    /* Combined flags with file */
    system_gcov_dump("gcov-dump -l -p dummy.gcda");
    
    /* 5. Environment variable tests */
    printf("\n--- Testing with Environment Variables ---\n");
    
    /* Test with GCOV_DUMP_OPTIONS if supported */
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    const char *env_args[] = {"gcov-dump", NULL};
    exec_gcov_dump(env_args);
    unsetenv("GCOV_DUMP_OPTIONS");
    
    /* 6. Edge cases */
    printf("\n--- Testing Edge Cases ---\n");
    
    /* Empty argument */
    const char *empty_args[] = {"gcov-dump", "", NULL};
    exec_gcov_dump(empty_args);
    
    /* Multiple invalid flags */
    const char *multi_invalid[] = {"gcov-dump", "-xyz", NULL};
    exec_gcov_dump(multi_invalid);
    
    /* Flag at end */
    const char *flag_end[] = {"gcov-dump", "dummy.gcda", "-l", NULL};
    exec_gcov_dump(flag_end);
    
    /* Mixed valid and invalid */
    const char *mixed_args[] = {"gcov-dump", "-l", "-x", "-p", NULL};
    exec_gcov_dump(mixed_args);
    
    /* 7. Output redirection tests */
    printf("\n--- Testing Output Redirection ---\n");
    
    /* Redirect stderr to capture unknown flag message */
    system("gcov-dump -x 2> unknown_flag.txt");
    printf("Check unknown_flag.txt for 'unknown flag' message\n");
    
    /* Redirect both stdout and stderr */
    system("gcov-dump -v -l > output.txt 2>&1");
    
    /* Cleanup */
    printf("\n=== Cleaning up test files ===\n");
    remove("dummy.gcda");
    remove("trivial.c");
    remove("trivial");
    remove("trivial.gcda");
    remove("trivial.gcno");
    remove("unknown_flag.txt");
    remove("output.txt");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
