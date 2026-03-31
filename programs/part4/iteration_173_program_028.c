/**
 * gcov-tool_invalid_option_tester.c
 * 
 * Test program to exercise uncovered lines in gcov-tool.cc option parsing.
 * Specifically targets the default case in the switch statement that calls
 * overlap_usage() for invalid options.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/**
 * Test case structure for invalid option testing
 */
typedef struct {
    const char *description;      // Test description
    const char *args[10];         // Arguments to pass to gcov-tool
    int expected_exit_code;       // Expected exit code (non-zero for invalid options)
    int should_trigger_usage;     // Should trigger overlap_usage()?
} test_case_t;

/**
 * Execute gcov-tool with given arguments and check results
 */
int execute_test(const char *test_name, const char *description, 
                 const char *args[], int arg_count) {
    printf("\n=== Test: %s ===\n", test_name);
    printf("Description: %s\n", description);
    printf("Command: gcov-tool");
    
    for (int i = 0; i < arg_count; i++) {
        printf(" %s", args[i]);
    }
    printf("\n");
    
    // Fork and execute
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        char **argv = malloc((arg_count + 2) * sizeof(char *));
        argv[0] = "./gcov-tool";
        
        for (int i = 0; i < arg_count; i++) {
            argv[i + 1] = (char *)args[i];
        }
        argv[arg_count + 1] = NULL;
        
        execvp("./gcov-tool", argv);
        
        // If execvp fails
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Exit code: %d\n", exit_code);
            
            // Check if usage was likely triggered
            if (exit_code != 0) {
                printf("✓ Likely triggered overlap_usage()\n");
                return 1;
            } else {
                printf("✗ Did not trigger error (exit code 0)\n");
                return 0;
            }
        } else {
            printf("✗ Process terminated abnormally\n");
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

/**
 * Main test driver
 */
int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("gcov-tool Invalid Option Parser Tester\n");
    printf("Target: Trigger default case in switch statement\n");
    printf("========================================\n");
    
    int total_tests = 0;
    int triggered_usage = 0;
    
    // First, test a valid invocation to ensure basic functionality
    printf("\n--- Valid Invocation Test (Baseline) ---\n");
    const char *valid_args[] = {"--help"};
    execute_test("BASELINE", "Valid invocation with --help", valid_args, 1);
    
    // Test 1: Single invalid character options (not v, f, F, o, h, t)
    printf("\n--- Test Set 1: Single Invalid Character Options ---\n");
    
    // Test all lowercase letters except v, f, o, h, t
    const char *invalid_chars[] = {"a", "b", "c", "d", "e", "g", "i", "j", "k", 
                                   "l", "m", "n", "p", "q", "r", "s", "u", "w", 
                                   "x", "y", "z"};
    
    for (int i = 0; i < sizeof(invalid_chars)/sizeof(invalid_chars[0]); i++) {
        char test_name[32];
        char description[64];
        const char *args[2];
        
        snprintf(test_name, sizeof(test_name), "INVALID_%s", invalid_chars[i]);
        snprintf(description, sizeof(description), 
                 "Single invalid option -%s", invalid_chars[i]);
        
        args[0] = "-";
        args[1] = invalid_chars[i];
        
        total_tests++;
        if (execute_test(test_name, description, args, 2)) {
            triggered_usage++;
        }
    }
    
    // Test 2: Invalid uppercase letters
    printf("\n--- Test Set 2: Invalid Uppercase Options ---\n");
    const char *invalid_upper[] = {"A", "B", "C", "D", "E", "G", "H", "I", "J", 
                                   "K", "L", "M", "N", "O", "P", "Q", "R", "S", 
                                   "T", "U", "V", "W", "X", "Y", "Z"};
    
    for (int i = 0; i < sizeof(invalid_upper)/sizeof(invalid_upper[0]); i++) {
        // Skip 'F' which is valid
        if (strcmp(invalid_upper[i], "F") == 0) continue;
        
        char test_name[32];
        char description[64];
        const char *args[2];
        
        snprintf(test_name, sizeof(test_name), "INVALID_%s", invalid_upper[i]);
        snprintf(description, sizeof(description), 
                 "Single invalid uppercase option -%s", invalid_upper[i]);
        
        args[0] = "-";
        args[1] = invalid_upper[i];
        
        total_tests++;
        if (execute_test(test_name, description, args, 2)) {
            triggered_usage++;
        }
    }
    
    // Test 3: Non-alphabetic invalid options
    printf("\n--- Test Set 3: Non-Alphabetic Invalid Options ---\n");
    const char *non_alpha[] = {"@", "#", "$", "%", "&", "*", "(", ")", "_", 
                               "+", "=", "[", "]", "{", "}", "|", "\\", ":", 
                               ";", "\"", "'", "<", ">", ",", ".", "?", "/", 
                               "~", "`", "!", "0", "1", "2", "3", "4", "5", 
                               "6", "7", "8", "9"};
    
    for (int i = 0; i < sizeof(non_alpha)/sizeof(non_alpha[0]); i++) {
        char test_name[32];
        char description[64];
        const char *args[2];
        
        snprintf(test_name, sizeof(test_name), "NONALPHA_%s", non_alpha[i]);
        snprintf(description, sizeof(description), 
                 "Non-alphabetic invalid option -%s", non_alpha[i]);
        
        args[0] = "-";
        args[1] = non_alpha[i];
        
        total_tests++;
        if (execute_test(test_name, description, args, 2)) {
            triggered_usage++;
        }
    }
    
    // Test 4: Edge cases and special characters
    printf("\n--- Test Set 4: Edge Cases and Special Characters ---\n");
    
    // Empty option (just dash)
    const char *empty_dash[] = {"-"};
    total_tests++;
    if (execute_test("EMPTY_DASH", "Single dash with no character", 
                     empty_dash, 1)) {
        triggered_usage++;
    }
    
    // Double dash with invalid option
    const char *double_dash[] = {"--invalid-option"};
    total_tests++;
    if (execute_test("LONG_INVALID", "Long form invalid option", 
                     double_dash, 1)) {
        triggered_usage++;
    }
    
    // Test 5: Combination tests with valid and invalid options
    printf("\n--- Test Set 5: Valid and Invalid Combinations ---\n");
    
    // Combination 1: Valid -v followed by invalid -x
    const char *combo1[] = {"-v", "-x"};
    total_tests++;
    if (execute_test("COMBO_V_X", "Valid -v followed by invalid -x", 
                     combo1, 2)) {
        triggered_usage++;
    }
    
    // Combination 2: Invalid -a followed by valid -f
    const char *combo2[] = {"-a", "-f"};
    total_tests++;
    if (execute_test("COMBO_A_F", "Invalid -a followed by valid -f", 
                     combo2, 2)) {
        triggered_usage++;
    }
    
    // Combination 3: Multiple invalid options
    const char *combo3[] = {"-a", "-b", "-c", "-d"};
    total_tests++;
    if (execute_test("MULTI_INVALID", "Multiple invalid options", 
                     combo3, 4)) {
        triggered_usage++;
    }
    
    // Combination 4: Valid -t with argument followed by invalid option
    const char *combo4[] = {"-t", "0.5", "-z"};
    total_tests++;
    if (execute_test("COMBO_T_Z", "Valid -t 0.5 followed by invalid -z", 
                     combo4, 3)) {
        triggered_usage++;
    }
    
    // Combination 5: Complex mix
    const char *combo5[] = {"-v", "-f", "-x", "-o", "-y"};
    total_tests++;
    if (execute_test("COMPLEX_MIX", "Mix of valid (-v, -f, -o) and invalid (-x, -y)", 
                     combo5, 5)) {
        triggered_usage++;
    }
    
    // Test 6: Valid -t with various numeric arguments (to exercise case 't')
    printf("\n--- Test Set 6: Valid -t Arguments (Context) ---\n");
    
    const char *t_tests[][3] = {
        {"-t", "0.5", "Simple decimal"},
        {"-t", "1.0", "One point zero"},
        {"-t", "1.0e2", "Scientific notation"},
        {"-t", "0.001", "Small decimal"},
        {"-t", "100.0", "Large value"}
    };
    
    for (int i = 0; i < 5; i++) {
        char test_name[32];
        snprintf(test_name, sizeof(test_name), "VALID_T_%d", i);
        
        total_tests++;
        // These should NOT trigger usage (valid options)
        if (!execute_test(test_name, t_tests[i][2], t_tests[i], 2)) {
            // Count as successful if they don't trigger usage
            printf("✓ Valid -t option accepted\n");
        }
    }
    
    // Test 7: Invalid options in merge command context
    printf("\n--- Test Set 7: Invalid Options in Command Context ---\n");
    
    const char *merge_invalid[] = {"merge", "-o", "output.gcda", "-z"};
    total_tests++;
    if (execute_test("MERGE_INVALID", "Invalid -z in merge command", 
                     merge_invalid, 4)) {
        triggered_usage++;
    }
    
    // Summary
    printf("\n========================================\n");
    printf("TEST SUMMARY\n");
    printf("========================================\n");
    printf("Total tests executed: %d\n", total_tests);
    printf("Tests that likely triggered overlap_usage(): %d\n", triggered_usage);
    printf("Success rate: %.1f%%\n", 
           (triggered_usage * 100.0) / total_tests);
    
    if (triggered_usage > 0) {
        printf("\n✓ SUCCESS: Successfully triggered the default case\n");
        printf("  in the option parser switch statement.\n");
    } else {
        printf("\n✗ FAILURE: No tests triggered the default case.\n");
        printf("  The option parser may be accepting invalid options.\n");
    }
    
    return 0;
}
