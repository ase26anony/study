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
 * Uses gcc's coverage instrumentation to generate a real .gcda file.
 */
static int create_dummy_gcda_file(void) {
    const char *source_code = 
        "#include <stdio.h>\n"
        "int main() {\n"
        "    printf(\"Hello from dummy program\\n\");\n"
        "    return 0;\n"
        "}\n";
    
    const char *source_file = "dummy_test.c";
    const char *executable = "dummy_test";
    const char *gcda_file = "dummy_test.gcda";
    
    FILE *fp = fopen(source_file, "w");
    if (!fp) {
        perror("Failed to create dummy source file");
        return -1;
    }
    fputs(source_code, fp);
    fclose(fp);
    
    // Compile with coverage flags
    char compile_cmd[256];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -o %s %s 2>/dev/null",
             executable, source_file);
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Warning: Failed to compile dummy program\n");
        // Try without coverage flags as fallback
        snprintf(compile_cmd, sizeof(compile_cmd),
                 "gcc -o %s %s 2>/dev/null", executable, source_file);
        if (system(compile_cmd) != 0) {
            fprintf(stderr, "Failed to compile even without coverage flags\n");
            return -1;
        }
        // Create a placeholder .gcda file
        fp = fopen(gcda_file, "wb");
        if (fp) {
            // Write minimal .gcda header (magic + version)
            unsigned char placeholder[] = {0x67, 0x63, 0x6f, 0x76, 0x00, 0x00, 0x00, 0x01};
            fwrite(placeholder, 1, sizeof(placeholder), fp);
            fclose(fp);
        }
    } else {
        // Run the executable to generate .gcda
        char run_cmd[256];
        snprintf(run_cmd, sizeof(run_cmd), "./%s 2>/dev/null", executable);
        system(run_cmd);
    }
    
    // Cleanup intermediate files (keep .gcda)
    remove(source_file);
    remove(executable);
    
    return 0;
}

/**
 * Execute gcov-dump using execvp for precise argument control.
 * Returns the exit status of gcov-dump.
 */
static int exec_gcov_dump(const char *args[]) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        // Child process
        execvp("gcov-dump", (char *const *)args);
        // If execvp returns, it failed
        fprintf(stderr, "execvp failed for gcov-dump: %s\n", strerror(errno));
        exit(127);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    }
}

/**
 * Execute gcov-dump using system() call.
 */
static void system_gcov_dump(const char *cmd) {
    printf("Executing via system(): %s\n", cmd);
    int ret = system(cmd);
    printf("Exit status: %d\n\n", ret);
}

/**
 * Main test driver.
 */
int main(int argc, char *argv[]) {
    printf("=== Testing gcov-dump Flag Parser ===\n\n");
    
    // Create a dummy .gcda file for file-based tests
    printf("Creating dummy .gcda file...\n");
    if (create_dummy_gcda_file() == 0) {
        printf("Successfully created dummy.gcda\n\n");
    } else {
        printf("Using placeholder file name for tests\n\n");
    }
    
    // Set environment variable that might affect gcov-dump
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    
    // Test cases targeting specific switch cases
    printf("--- Testing Individual Flags (execvp) ---\n");
    
    // Individual flag tests - each targets one case in the switch
    const char *test_cases[][4] = {
        // Help flag - case 'h'
        {"gcov-dump", "-h", NULL},
        
        // Version flag - case 'v'
        {"gcov-dump", "-v", NULL},
        
        // Contents dump flag - case 'l'
        {"gcov-dump", "-l", NULL},
        
        // Positions dump flag - case 'p'
        {"gcov-dump", "-p", NULL},
        
        // Raw dump flag - case 'r'
        {"gcov-dump", "-r", NULL},
        
        // Stable dump flag - case 's'
        {"gcov-dump", "-s", NULL},
        
        // Invalid flag - default case
        {"gcov-dump", "-x", NULL},
        
        // No arguments
        {"gcov-dump", NULL},
    };
    
    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        printf("Test %zu: ", i + 1);
        for (int j = 0; test_cases[i][j] != NULL; j++) {
            printf("%s ", test_cases[i][j]);
        }
        printf("\n");
        
        int status = exec_gcov_dump(test_cases[i]);
        printf("Exit status: %d\n\n", status);
    }
    
    printf("--- Testing Flag Combinations (execvp) ---\n");
    
    // Combination tests
    const char *combo_tests[][5] = {
        // Multiple valid flags
        {"gcov-dump", "-l", "-p", NULL},
        {"gcov-dump", "-r", "-s", "-v", NULL},
        {"gcov-dump", "-l", "-p", "-r", "-s", NULL},
        
        // Same flag repeated
        {"gcov-dump", "-p", "-p", NULL},
        
        // Help with other flags (may exit early)
        {"gcov-dump", "-h", "-l", NULL},
        
        // Combined short options (if supported by getopt)
        {"gcov-dump", "-lp", NULL},
        {"gcov-dump", "-rs", NULL},
        {"gcov-dump", "-lprs", NULL},
    };
    
    for (size_t i = 0; i < sizeof(combo_tests) / sizeof(combo_tests[0]); i++) {
        printf("Combo test %zu: ", i + 1);
        for (int j = 0; combo_tests[i][j] != NULL; j++) {
            printf("%s ", combo_tests[i][j]);
        }
        printf("\n");
        
        int status = exec_gcov_dump(combo_tests[i]);
        printf("Exit status: %d\n\n", status);
    }
    
    printf("--- Testing with File Arguments (execvp) ---\n");
    
    // Tests with file arguments
    const char *file_tests[][5] = {
        // With positional file argument
        {"gcov-dump", "-l", "dummy_test.gcda", NULL},
        
        // Multiple flags with file
        {"gcov-dump", "-l", "-p", "dummy_test.gcda", NULL},
        
        // With -- delimiter
        {"gcov-dump", "-l", "--", "dummy_test.gcda", NULL},
        
        // Combined flags with file
        {"gcov-dump", "-lp", "dummy_test.gcda", NULL},
        
        // Invalid flag with file (should still trigger default case)
        {"gcov-dump", "-x", "dummy_test.gcda", NULL},
    };
    
    for (size_t i = 0; i < sizeof(file_tests) / sizeof(file_tests[0]); i++) {
        printf("File test %zu: ", i + 1);
        for (int j = 0; file_tests[i][j] != NULL; j++) {
            printf("%s ", file_tests[i][j]);
        }
        printf("\n");
        
        int status = exec_gcov_dump(file_tests[i]);
        printf("Exit status: %d\n\n", status);
    }
    
    printf("--- Testing via system() calls ---\n");
    
    // Additional tests using system() for different code path
    const char *system_tests[] = {
        // Basic flags
        "gcov-dump -h",
        "gcov-dump -v",
        "gcov-dump -l",
        
        // Combinations with shell interpretation
        "gcov-dump -l -p 2>&1",
        "gcov-dump -r -s -v 2>&1",
        
        // Invalid flag (capture stderr)
        "gcov-dump -x 2>&1",
        
        // With file redirection
        "gcov-dump -l dummy_test.gcda 2>&1",
        
        // Empty call
        "gcov-dump 2>&1",
    };
    
    for (size_t i = 0; i < sizeof(system_tests) / sizeof(system_tests[0]); i++) {
        system_gcov_dump(system_tests[i]);
    }
    
    // Test with different environment
    printf("--- Testing with Modified Environment ---\n");
    unsetenv("GCOV_DUMP_OPTIONS");
    setenv("GCOV_DUMP_OPTIONS", "-l -p", 1);
    
    const char *env_test[] = {"gcov-dump", "-v", NULL};
    printf("Testing with GCOV_DUMP_OPTIONS='-l -p' and argument '-v'\n");
    int status = exec_gcov_dump(env_test);
    printf("Exit status: %d\n\n", status);
    
    // Cleanup
    remove("dummy_test.gcda");
    remove("dummy_test.gcno");
    
    printf("=== All tests completed ===\n");
    return 0;
}
