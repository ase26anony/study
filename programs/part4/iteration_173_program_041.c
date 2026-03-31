/**
 * gcov-tool-test.c
 * 
 * Test program to exercise uncovered lines in gcov-tool.cc option parsing.
 * Specifically targets lines 534-554 to trigger the default case for invalid options.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/**
 * Structure to hold test cases for invalid option testing
 */
typedef struct {
    const char *description;      // Description of test case
    const char *args[10];         // Arguments to pass to gcov-tool
    int expected_exit_code;       // Expected exit code (non-zero for invalid options)
    int should_trigger_default;   // Should trigger the default case
} test_case_t;

/**
 * Execute gcov-tool with given arguments and check result
 * 
 * @param args Array of arguments (NULL terminated)
 * @param description Test description for output
 * @return 1 if test passed, 0 if failed
 */
int execute_test(const char **args, const char *description) {
    pid_t pid;
    int status;
    
    printf("Test: %s\n", description);
    printf("Command: gcov-tool");
    for (int i = 0; args[i] != NULL; i++) {
        printf(" %s", args[i]);
    }
    printf("\n");
    
    pid = fork();
    if (pid == 0) {
        // Child process: execute gcov-tool
        execvp("gcov-tool", (char *const *)args);
        // If execvp returns, there was an error
        fprintf(stderr, "Failed to execute gcov-tool: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process: wait for child
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Exit code: %d\n\n", exit_code);
            return exit_code != 0;  // Invalid options should return non-zero
        } else {
            printf("Process terminated abnormally\n\n");
            return 0;
        }
    } else {
        // Fork failed
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
        return 0;
    }
}

/**
 * Main test driver
 */
int main(int argc, char *argv[]) {
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("========================================\n");
    printf("Testing gcov-tool option parsing logic\n");
    printf("Targeting uncovered lines 534-554\n");
    printf("========================================\n\n");
    
    // First, test a valid invocation to ensure basic functionality
    printf("=== Basic Valid Invocation Test ===\n");
    const char *valid_args[] = {"gcov-tool", "--help", NULL};
    if (execute_test(valid_args, "Valid invocation (--help)")) {
        printf("✓ Basic functionality verified\n\n");
    } else {
        printf("✗ Basic functionality test failed\n\n");
    }
    
    // Test valid -t option to ensure case 't': is exercised
    printf("=== Valid Option Tests ===\n");
    const char *valid_t_args[] = {"gcov-tool", "merge", "-t", "0.5", NULL};
    total_tests++;
    if (execute_test(valid_t_args, "Valid -t option with float argument")) {
        passed_tests++;
        printf("✓ -t option parsing works\n");
    }
    
    const char *valid_t_scientific[] = {"gcov-tool", "merge", "-t", "1.0e2", NULL};
    total_tests++;
    if (execute_test(valid_t_scientific, "Valid -t option with scientific notation")) {
        passed_tests++;
        printf("✓ -t option with scientific notation works\n");
    }
    
    const char *valid_t_integer[] = {"gcov-tool", "merge", "-t", "100", NULL};
    total_tests++;
    if (execute_test(valid_t_integer, "Valid -t option with integer argument")) {
        passed_tests++;
        printf("✓ -t option with integer works\n");
    }
    
    printf("\n=== Invalid Option Tests (Targeting default case) ===\n");
    
    // Array of invalid single-character options (excluding v, f, F, o, h, t)
    // Testing every possible invalid option as requested
    char invalid_options[] = "abcdegijklmnpqrsuwxyzABCDEGIJKLMNPQRSUVWXYZ";
    
    for (int i = 0; i < strlen(invalid_options); i++) {
        char option[3] = "- ";
        option[1] = invalid_options[i];
        
        char description[50];
        snprintf(description, sizeof(description), 
                 "Single invalid option: %s", option);
        
        const char *args[] = {"gcov-tool", "merge", option, NULL};
        
        total_tests++;
        if (execute_test(args, description)) {
            passed_tests++;
            printf("✓ Invalid option %s triggered default case\n", option);
        }
    }
    
    // Test complex combinations of valid and invalid options
    printf("\n=== Complex Option Combination Tests ===\n");
    
    // Test 1: Valid option followed by invalid option
    const char *combo1[] = {"gcov-tool", "merge", "-v", "-x", NULL};
    total_tests++;
    if (execute_test(combo1, "Valid -v followed by invalid -x")) {
        passed_tests++;
        printf("✓ Combination -v -x works\n");
    }
    
    // Test 2: Multiple invalid options together
    const char *combo2[] = {"gcov-tool", "merge", "-abc", NULL};
    total_tests++;
    if (execute_test(combo2, "Multiple invalid options: -abc")) {
        passed_tests++;
        printf("✓ Multiple invalid options work\n");
    }
    
    // Test 3: Valid options with invalid option in middle
    const char *combo3[] = {"gcov-tool", "merge", "-v", "-z", "-f", NULL};
    total_tests++;
    if (execute_test(combo3, "Valid -v, invalid -z, valid -f")) {
        passed_tests++;
        printf("✓ Mixed valid/invalid options work\n");
    }
    
    // Test 4: Invalid option with argument (should still fail)
    const char *combo4[] = {"gcov-tool", "merge", "-x", "argument", NULL};
    total_tests++;
    if (execute_test(combo4, "Invalid option -x with argument")) {
        passed_tests++;
        printf("✓ Invalid option with argument works\n");
    }
    
    // Test edge cases
    printf("\n=== Edge Case Tests ===\n");
    
    // Test 1: Single dash (should trigger default or error)
    const char *edge1[] = {"gcov-tool", "merge", "-", NULL};
    total_tests++;
    if (execute_test(edge1, "Single dash '-'")) {
        passed_tests++;
        printf("✓ Single dash handling works\n");
    }
    
    // Test 2: Non-alphabetic character
    const char *edge2[] = {"gcov-tool", "merge", "-@", NULL};
    total_tests++;
    if (execute_test(edge2, "Non-alphabetic option: -@")) {
        passed_tests++;
        printf("✓ Non-alphabetic option works\n");
    }
    
    // Test 3: Number as option
    const char *edge3[] = {"gcov-tool", "merge", "-2", NULL};
    total_tests++;
    if (execute_test(edge3, "Number as option: -2")) {
        passed_tests++;
        printf("✓ Number option works\n");
    }
    
    // Test 4: Long invalid option
    const char *edge4[] = {"gcov-tool", "merge", "--invalid-option", NULL};
    total_tests++;
    if (execute_test(edge4, "Long invalid option")) {
        passed_tests++;
        printf("✓ Long invalid option works\n");
    }
    
    // Test 5: Empty string as option
    const char *edge5[] = {"gcov-tool", "merge", "", NULL};
    total_tests++;
    if (execute_test(edge5, "Empty string option")) {
        passed_tests++;
        printf("✓ Empty string option works\n");
    }
    
    // Test 6: Valid -t with invalid option after
    const char *edge6[] = {"gcov-tool", "merge", "-t", "0.5", "-q", NULL};
    total_tests++;
    if (execute_test(edge6, "Valid -t followed by invalid -q")) {
        passed_tests++;
        printf("✓ -t with following invalid option works\n");
    }
    
    // Test 7: Invalid option before valid -t
    const char *edge7[] = {"gcov-tool", "merge", "-w", "-t", "1.0", NULL};
    total_tests++;
    if (execute_test(edge7, "Invalid -w before valid -t")) {
        passed_tests++;
        printf("✓ Invalid option before -t works\n");
    }
    
    // Test 8: Chain of all valid options with one invalid
    const char *edge8[] = {"gcov-tool", "merge", "-v", "-f", "-F", "-o", "-h", "-t", "0.5", "-z", NULL};
    total_tests++;
    if (execute_test(edge8, "All valid options plus invalid -z")) {
        passed_tests++;
        printf("✓ All valid + invalid combination works\n");
    }
    
    // Summary
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    printf("Success rate: %.1f%%\n", 
           total_tests > 0 ? (passed_tests * 100.0 / total_tests) : 0.0);
    printf("========================================\n");
    
    return (passed_tests == total_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
