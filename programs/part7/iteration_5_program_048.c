#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define INSTRUMENTED_BINARY "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file format */
static const unsigned char minimal_gcda[] = {
    /* Magic: 'gcda' */
    0x67, 0x63, 0x64, 0x61,
    /* Version: 0x76312e2a (gcov 12*) */
    0x2a, 0x2e, 0x31, 0x76,
    /* Stamp: 0x12345678 */
    0x78, 0x56, 0x34, 0x12,
    /* Length: 0 (no records) */
    0x00, 0x00, 0x00, 0x00
};

/* Execute command and capture output */
static char *run_command(const char *cmd, int capture_stdout, int *exit_status) {
    char buffer[4096];
    FILE *fp;
    char *result = NULL;
    size_t total_size = 0;
    
    fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return NULL;
    }
    
    if (capture_stdout) {
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            size_t len = strlen(buffer);
            result = realloc(result, total_size + len + 1);
            if (!result) {
                pclose(fp);
                return NULL;
            }
            memcpy(result + total_size, buffer, len);
            total_size += len;
            result[total_size] = '\0';
        }
    } else {
        /* Just consume output without capturing */
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* Do nothing */
        }
    }
    
    *exit_status = pclose(fp);
    return result;
}

/* Execute command and capture stderr */
static char *run_command_stderr(const char *cmd, int *exit_status) {
    int pipefd[2];
    pid_t pid;
    char buffer[4096];
    char *result = NULL;
    size_t total_size = 0;
    
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return NULL;
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork failed");
        close(pipefd[0]);
        close(pipefd[1]);
        return NULL;
    }
    
    if (pid == 0) {
        /* Child process */
        close(pipefd[0]);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        /* Execute command */
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        perror("execl failed");
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        close(pipefd[1]);
        
        /* Read stderr */
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            result = realloc(result, total_size + bytes_read + 1);
            if (!result) {
                close(pipefd[0]);
                waitpid(pid, exit_status, 0);
                return NULL;
            }
            memcpy(result + total_size, buffer, bytes_read);
            total_size += bytes_read;
            result[total_size] = '\0';
        }
        
        close(pipefd[0]);
        waitpid(pid, exit_status, 0);
    }
    
    return result;
}

/* Create minimal valid .gcda file */
static int create_minimal_gcda(void) {
    FILE *fp = fopen(TEMP_GCDA_FILE, "wb");
    if (!fp) {
        perror("Failed to create temporary .gcda file");
        return 0;
    }
    
    fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    return 1;
}

/* Build instrumented gcov-dump */
static int build_gcov_dump(void) {
    char cmd[1024];
    int exit_status;
    
    printf("Building instrumented gcov-dump...\n");
    
    /* Try to find gcov-dump.cc in common locations */
    const char *possible_paths[] = {
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "../../../gcc/gcov-dump.cc",
        "/usr/src/gcc/gcc/gcov-dump.cc",
        NULL
    };
    
    const char *source_path = NULL;
    struct stat st;
    
    for (int i = 0; possible_paths[i]; i++) {
        if (stat(possible_paths[i], &st) == 0 && S_ISREG(st.st_mode)) {
            source_path = possible_paths[i];
            break;
        }
    }
    
    if (!source_path) {
        fprintf(stderr, "Could not find gcov-dump.cc\n");
        return 0;
    }
    
    printf("Found gcov-dump.cc at: %s\n", source_path);
    
    /* Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Compiling: %s\n", cmd);
    exit_status = system(cmd);
    
    if (exit_status != 0) {
        fprintf(stderr, "Failed to compile gcov-dump\n");
        return 0;
    }
    
    /* Verify the binary exists */
    if (access(INSTRUMENTED_BINARY, X_OK) != 0) {
        perror("Instrumented binary not created");
        return 0;
    }
    
    printf("Instrumented gcov-dump built successfully\n");
    return 1;
}

/* Test -h flag (help) */
static void test_help_flag(void) {
    char cmd[256];
    int exit_status;
    
    printf("\n=== Testing -h flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    
    char *output = run_command(cmd, 1, &exit_status);
    if (output) {
        printf("Help output (first 200 chars):\n%.200s\n", output);
        free(output);
    }
    
    if (WIFEXITED(exit_status) && WEXITSTATUS(exit_status) == 0) {
        printf("✓ -h flag test passed (exit code 0)\n");
    } else {
        printf("✗ -h flag test failed (exit code %d)\n", WEXITSTATUS(exit_status));
    }
}

/* Test -v flag (version) */
static void test_version_flag(void) {
    char cmd[256];
    int exit_status;
    
    printf("\n=== Testing -v flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    
    char *output = run_command(cmd, 1, &exit_status);
    if (output) {
        printf("Version output: %s", output);
        free(output);
    }
    
    if (WIFEXITED(exit_status) && WEXITSTATUS(exit_status) == 0) {
        printf("✓ -v flag test passed\n");
    } else {
        printf("✗ -v flag test failed\n");
    }
}

/* Test invalid flag */
static void test_invalid_flag(void) {
    char cmd[256];
    int exit_status;
    
    printf("\n=== Testing invalid flag -X ===\n");
    snprintf(cmd, sizeof(cmd), "%s -X", INSTRUMENTED_BINARY);
    
    char *stderr_output = run_command_stderr(cmd, &exit_status);
    if (stderr_output) {
        printf("Stderr output: %s", stderr_output);
        
        /* Check for expected error message */
        if (strstr(stderr_output, "unknown flag `X'") != NULL) {
            printf("✓ Found expected error message: 'unknown flag `X''\n");
        } else {
            printf("✗ Expected error message not found\n");
        }
        free(stderr_output);
    } else {
        printf("✗ No stderr output captured\n");
    }
}

/* Test flag with minimal .gcda file */
static void test_flag_with_file(const char *flag, const char *description) {
    char cmd[256];
    int exit_status;
    
    printf("\n=== Testing %s flag (%s) ===\n", flag, description);
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flag, TEMP_GCDA_FILE);
    
    char *output = run_command(cmd, 1, &exit_status);
    if (output) {
        printf("Output (first 100 chars):\n%.100s\n", output);
        free(output);
    }
    
    if (WIFEXITED(exit_status) && WEXITSTATUS(exit_status) == 0) {
        printf("✓ %s flag test passed\n", flag);
    } else {
        printf("✗ %s flag test failed (exit code %d)\n", flag, WEXITSTATUS(exit_status));
    }
}

/* Test flag combinations */
static void test_flag_combination(const char *flags, const char *description) {
    char cmd[256];
    int exit_status;
    
    printf("\n=== Testing flag combination: %s ===\n", description);
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flags, TEMP_GCDA_FILE);
    
    char *output = run_command(cmd, 1, &exit_status);
    if (output) {
        printf("Output (first 100 chars):\n%.100s\n", output);
        free(output);
    }
    
    if (WIFEXITED(exit_status) && WEXITSTATUS(exit_status) == 0) {
        printf("✓ Flag combination %s test passed\n", description);
    } else {
        printf("✗ Flag combination %s test failed\n", description);
    }
}

/* Test flag ordering variations */
static void test_flag_ordering(void) {
    char cmd[256];
    int exit_status;
    
    printf("\n=== Testing flag ordering (-l -p vs -p -l) ===\n");
    
    /* Test -l -p */
    printf("Testing: -l -p\n");
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    char *output1 = run_command(cmd, 1, &exit_status);
    int success1 = (WIFEXITED(exit_status) && WEXITSTATUS(exit_status) == 0);
    
    /* Test -p -l */
    printf("Testing: -p -l\n");
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    char *output2 = run_command(cmd, 1, &exit_status);
    int success2 = (WIFEXITED(exit_status) && WEXITSTATUS(exit_status) == 0);
    
    if (output1) free(output1);
    if (output2) free(output2);
    
    if (success1 && success2) {
        printf("✓ Both flag orderings work correctly\n");
    } else {
        printf("✗ Flag ordering test failed\n");
    }
}

int main(void) {
    printf("=== Starting gcov-dump coverage tests ===\n");
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_gcov_dump()) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return EXIT_FAILURE;
    }
    
    /* Step 2: Create minimal coverage file */
    if (!create_minimal_gcda()) {
        fprintf(stderr, "Failed to create minimal .gcda file\n");
        unlink(INSTRUMENTED_BINARY);
        return EXIT_FAILURE;
    }
    
    /* Step 3: Execute test sequence */
    
    /* Test flags without file arguments */
    test_help_flag();      /* -h */
    test_version_flag();   /* -v */
    test_invalid_flag();   /* -X */
    
    /* Test flags with minimal .gcda file */
    test_flag_with_file("-l", "dump contents");
    test_flag_with_file("-p", "dump positions");
    test_flag_with_file("-r", "dump raw");
    test_flag_with_file("-s", "dump stable");
    
    /* Test flag combinations */
    test_flag_combination("-l -p", "-l -p");
    test_flag_combination("-r -s", "-r -s");
    test_flag_combination("-l -p -r -s", "all flags");
    
    /* Test flag ordering */
    test_flag_ordering();
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(TEMP_GCDA_FILE);
    unlink(INSTRUMENTED_BINARY);
    
    /* Also clean up coverage data files created by instrumented binary */
    char coverage_files[][64] = {
        INSTRUMENTED_BINARY ".gcda",
        INSTRUMENTED_BINARY ".gcno",
        "gcov-dump-instrumented.gcda",
        "gcov-dump-instrumented.gcno"
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (unlink(coverage_files[i]) == 0) {
            printf("Removed: %s\n", coverage_files[i]);
        }
    }
    
    printf("\n=== All tests completed ===\n");
    return EXIT_SUCCESS;
}
