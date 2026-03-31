#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_ARGS 20
#define MAX_TEST_CASES 100

typedef struct {
    char *args[MAX_ARGS];
    int expected_exit_code;
    char *description;
} test_case_t;

// Helper function to execute a test case
int execute_test_case(test_case_t *test, const char *gcov_tool_path) {
    pid_t pid;
    int status;
    
    printf("Test: %s\n", test->description);
    printf("Command: %s", gcov_tool_path);
    for (int i = 0; test->args[i] != NULL; i++) {
        printf(" %s", test->args[i]);
    }
    printf("\n");
    
    pid = fork();
    if (pid == 0) {
        // Child process
        execvp(gcov_tool_path, test->args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Exit code: %d (expected: %d)\n", exit_code, test->expected_exit_code);
            printf("Result: %s\n\n", 
                   (exit_code == test->expected_exit_code) ? "PASS" : "FAIL");
            return (exit_code == test->expected_exit_code) ? 0 : 1;
        }
    } else {
        perror("fork failed");
    }
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path-to-gcov-tool>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    const char *gcov_tool_path = argv[1];
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    
    // Initialize test cases array
    test_case_t tests[MAX_TEST_CASES];
    memset(tests, 0, sizeof(tests));
    
    // Test 1: Basic valid invocation to ensure tool works
    tests[total_tests++] = (test_case_t){
        .args = { "gcov-tool", "--help", NULL },
        .expected_exit_code = 0,
        .description = "Basic valid invocation (help)"
    };
    
    // Test 2: Valid invocation with -v option
    tests[total_tests++] = (test_case_t){
        .args = { "gcov-tool", "merge", "-v", NULL },
        .expected_exit_code = 0,  // May fail without proper arguments, but should parse -v
        .description = "Valid -v option test"
    };
    
    // Test 3: Valid invocation with -t option (to cover case 't')
    tests[total_tests++] = (test_case_t){
        .args = { "gcov-tool", "merge", "-t", "0.5", NULL },
        .expected_exit_code = 0,  // May fail without proper arguments
        .description = "Valid -t option with numeric argument"
    };
    
    // Test 4: Valid invocation with -t option (scientific notation)
    tests[total_tests++] = (test_case_t){
        .args = { "gcov-tool", "merge", "-t", "1.0e2", NULL },
        .expected_exit_code = 0,
        .description = "Valid -t option with scientific notation"
    };
    
    // Test 5: Valid invocation with -t option (negative value)
    tests[total_tests++] = (test_case_t){
        .args = { "gcov-tool", "merge", "-t", "-0.5", NULL },
        .expected_exit_code = 0,
        .description = "Valid -t option with negative value"
    };
    
    // Test 6: Valid invocation with multiple valid options
    tests[total_tests++] = (test_case_t){
        .args = { "gcov-tool", "merge", "-v", "-f", "-F", "-o", "-h", "-t", "0.75", NULL },
        .expected_exit_code = 0,
        .description = "All valid options together"
    };
    
    // ========== INVALID OPTION TESTS ==========
    // These should trigger the default case and call overlap_usage()
    
    // Test invalid single-character options (not in: v, f, F, o, h, t)
    char invalid_chars[] = "abcdegijklmnpqrsuwxyzABCDEGIJKLMNPQRSUVWXYZ";
    
    for (int i = 0; invalid_chars[i] != '\0'; i++) {
        char option[3] = "-x";
        option[1] = invalid_chars[i];
        
        char description[50];
        snprintf(description, sizeof(description), "Invalid single option -%c", invalid_chars[i]);
        
        tests[total_tests++] = (test_case_t){
            .args = { "gcov-tool", "merge", option, NULL },
            .expected_exit_code = 1,  // Expected non-zero for invalid option
            .description = strdup(description)
        };
    }
    
    // Test invalid options with non-alphabetic characters
    char non_alpha_chars[] = "@#$%^&*()_+{}|:\"<>?~`1234567890-=[]\\;',./";
    
    for (int i = 0; non_alpha_chars[i] != '\0'; i++) {
        char option[3] = "-x";
        option[1] = non_alpha_chars[i];
        
        char description[50];
        snprintf(description, sizeof(description), "Invalid non-alpha option -%c", non_alpha_chars[i]);
        
        tests[total_tests++] = (test_case_t){
            .args = { "gcov-tool", "merge", option, NULL },
            .expected_exit_code = 1,
            .description = strdup(description)
        };
    }
    
    // Test edge cases
    tests[total_tests++] = (test_case_t){
        .args = { "gcov-tool", "merge", "-", NULL },
        .expected_exit_code = 1,
        .description = "Single dash only"
    };
    
    tests[total_tests++] = (test_case_t){
        .args = { "gcov-tool", "merge", "--", NULL },
        .expected_exit_code = 1,
        .description = "Double dash only"
    };
    
    tests[total_tests++] = (test_case_t){
        .args = { "gcov-tool", "merge", "--invalid-long-option", NULL },
        .expected_exit_code = 1,
        .description = "Invalid long option"
    };
    
    // Test combinations of valid and invalid options
    tests[total_tests++] = (test_case_t){
        .args = { "gcov-tool", "merge", "-v", "-x", "-f", NULL },
        .expected_exit_code = 1,
        .description = "Valid -v, invalid -x, valid -f"
    };
    
    tests[total_tests++] = (test_case_t){
        .args = { "gcov-tool", "merge", "-t", "0.5", "-z", NULL },
        .expected_exit_code = 1,
        .description = "Valid -t with argument, then invalid -z"
    };
    
    tests[total_tests++] = (test_case_t){
        .args = { "gcov-tool", "merge", "-o", "output.gcda", "-q", NULL },
        .expected_exit_code = 1,
        .description = "Valid -o, invalid -q"
    };
    
    // Test multiple invalid options together
    tests[total_tests++] = (test_case_t){
        .args = { "gcov-tool", "merge", "-a", "-b", "-c", "-d", "-e", "-g", NULL },
        .expected_exit_code = 1,
        .description = "Multiple invalid options chained"
    };
    
    tests[total_tests++] = (test_case_t){
        .args = { "gcov-tool", "merge", "-abcdeg", NULL },
        .expected_exit_code = 1,
        .description = "Invalid options combined in single argument"
    };
    
    // Test invalid option after valid complex argument
    tests[total_tests++] = (test_case_t){
        .args = { "gcov-tool", "merge", "-t", "1.0e-5", "-v", "-@", NULL },
        .expected_exit_code = 1,
        .description = "Valid -t with scientific notation, valid -v, invalid -@"
    };
    
    // Test with subcommand and invalid option
    tests[total_tests++] = (test_case_t){
        .args = { "gcov-tool", "overlay", "-v", "-j", NULL },
        .expected_exit_code = 1,
        .description = "Overlay subcommand with invalid -j"
    };
    
    // Test with overlap subcommand (since uncovered code is in overlap_parse_options)
    tests[total_tests++] = (test_case_t){
        .args = { "gcov-tool", "overlap", "-v", "-k", NULL },
        .expected_exit_code = 1,
        .description = "Overlap subcommand with invalid -k"
    };
    
    // Test with merge subcommand and invalid option
    tests[total_tests++] = (test_case_t){
        .args = { "gcov-tool", "merge", "-v", "-f", "-F", "-o", "-h", "-t", "50", "-X", NULL },
        .expected_exit_code = 1,
        .description = "All valid options plus invalid -X at end"
    };
    
    // Execute all tests
    printf("========================================\n");
    printf("Starting gcov-tool option parser tests\n");
    printf("Targeting uncovered lines 534-554 in gcov-tool.cc\n");
    printf("========================================\n\n");
    
    for (int i = 0; i < total_tests; i++) {
        // Update the gcov-tool path in the args
        tests[i].args[0] = (char *)gcov_tool_path;
        
        int result = execute_test_case(&tests[i], gcov_tool_path);
        if (result == 0) {
            passed_tests++;
        } else if (result == 1) {
            failed_tests++;
        }
        
        // Free dynamically allocated description strings
        if (i >= 7 && i < 7 + (int)strlen(invalid_chars)) {
            free(tests[i].description);
        } else if (i >= 7 + (int)strlen(invalid_chars) && 
                   i < 7 + (int)strlen(invalid_chars) + (int)strlen(non_alpha_chars)) {
            free(tests[i].description);
        }
    }
    
    // Summary
    printf("========================================\n");
    printf("Test Summary:\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", failed_tests);
    printf("========================================\n");
    
    if (failed_tests > 0) {
        printf("\nNote: Some tests may fail for reasons other than option parsing.\n");
        printf("The key goal is to trigger the default case in the switch statement.\n");
        printf("Check gcov coverage data to verify uncovered lines were executed.\n");
    }
    
    return (failed_tests == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
