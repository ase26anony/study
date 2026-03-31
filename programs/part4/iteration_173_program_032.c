#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_ARGS 20
#define MAX_TEST_CASES 100

typedef struct {
    char *args[MAX_ARGS];
    int expected_exit_code;
    char *description;
} test_case_t;

void run_test_case(test_case_t *test_case) {
    printf("\n=== Running test: %s ===\n", test_case->description);
    printf("Command: gcov-tool");
    for (int i = 0; test_case->args[i] != NULL; i++) {
        printf(" %s", test_case->args[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execvp("gcov-tool", test_case->args);
        // If execvp fails
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Exit code: %d (expected: %d)\n", exit_code, test_case->expected_exit_code);
            
            if (exit_code == test_case->expected_exit_code) {
                printf("✓ Test passed\n");
            } else {
                printf("✗ Test failed - unexpected exit code\n");
            }
        } else {
            printf("✗ Process terminated abnormally\n");
        }
    } else {
        perror("fork failed");
    }
}

int main(int argc, char *argv[]) {
    printf("=== GCOV-TOOL Option Parser Test Suite ===\n");
    printf("Target: Trigger default case in option parser switch (lines 534-554)\n");
    
    // First, test basic functionality to ensure the tool works
    printf("\n--- Testing basic functionality ---\n");
    test_case_t basic_test = {
        .args = {"gcov-tool", "--help", NULL},
        .expected_exit_code = 0,
        .description = "Basic help test"
    };
    run_test_case(&basic_test);
    
    // Test valid -t option to ensure case 't' is covered
    printf("\n--- Testing valid -t option (to cover case 't') ---\n");
    test_case_t valid_t_test = {
        .args = {"gcov-tool", "merge", "-t", "0.5", NULL},
        .expected_exit_code = 1,  // Will fail due to missing input files, but option parsing should work
        .description = "Valid -t option with numeric argument"
    };
    run_test_case(&valid_t_test);
    
    // Test another valid -t option with different format
    test_case_t valid_t_test2 = {
        .args = {"gcov-tool", "merge", "-t", "1.0e2", NULL},
        .expected_exit_code = 1,
        .description = "Valid -t option with scientific notation"
    };
    run_test_case(&valid_t_test2);
    
    // ============================================
    // INVALID OPTION TESTS - Targeting default case
    // ============================================
    
    printf("\n--- Testing invalid single-character options ---\n");
    
    // Test all invalid single-character options not in {v, f, F, o, h, t}
    char invalid_chars[] = "abcdegijklmnpqrsuwxyzABCDEGHIJKLMNOPQRSTUVWXYZ";
    
    for (int i = 0; i < strlen(invalid_chars); i++) {
        char option[3] = "- ";
        option[1] = invalid_chars[i];
        
        test_case_t invalid_test = {
            .args = {"gcov-tool", option, NULL},
            .expected_exit_code = 1,  // Should exit with error due to invalid option
            .description = malloc(50)
        };
        sprintf(invalid_test.description, "Invalid single option: %s", option);
        
        run_test_case(&invalid_test);
        free(invalid_test.description);
    }
    
    // Test invalid non-alphabetic characters
    printf("\n--- Testing invalid non-alphabetic options ---\n");
    char non_alpha_chars[] = "@#$%^&*()_+{}|:\"<>?~`1234567890-=[]\\;',./";
    
    for (int i = 0; i < strlen(non_alpha_chars); i++) {
        char option[3] = "- ";
        option[1] = non_alpha_chars[i];
        
        test_case_t invalid_test = {
            .args = {"gcov-tool", option, NULL},
            .expected_exit_code = 1,
            .description = malloc(50)
        };
        sprintf(invalid_test.description, "Invalid non-alpha option: %s", option);
        
        run_test_case(&invalid_test);
        free(invalid_test.description);
    }
    
    // Test edge cases
    printf("\n--- Testing edge cases ---\n");
    
    // Test single dash
    test_case_t single_dash = {
        .args = {"gcov-tool", "-", NULL},
        .expected_exit_code = 1,
        .description = "Single dash '-'"
    };
    run_test_case(&single_dash);
    
    // Test double dash with invalid option
    test_case_t double_dash_invalid = {
        .args = {"gcov-tool", "--invalid-option", NULL},
        .expected_exit_code = 1,
        .description = "Invalid long option"
    };
    run_test_case(&double_dash_invalid);
    
    // Test empty string after dash
    test_case_t empty_after_dash = {
        .args = {"gcov-tool", "-", "", NULL},
        .expected_exit_code = 1,
        .description = "Empty string after dash"
    };
    run_test_case(&empty_after_dash);
    
    // ============================================
    // COMBINATION TESTS - Stress the parser
    // ============================================
    
    printf("\n--- Testing option combinations ---\n");
    
    // Test valid followed by invalid
    test_case_t combo1 = {
        .args = {"gcov-tool", "merge", "-v", "-x", "-f", NULL},
        .expected_exit_code = 1,
        .description = "Valid -v, invalid -x, valid -f"
    };
    run_test_case(&combo1);
    
    // Test invalid in the middle of valid options
    test_case_t combo2 = {
        .args = {"gcov-tool", "-o", "-z", "-h", NULL},
        .expected_exit_code = 1,
        .description = "Valid -o, invalid -z, valid -h"
    };
    run_test_case(&combo2);
    
    // Test multiple invalid options together
    test_case_t combo3 = {
        .args = {"gcov-tool", "-a", "-b", "-c", "-d", "-e", "-g", NULL},
        .expected_exit_code = 1,
        .description = "Multiple invalid options: -a -b -c -d -e -g"
    };
    run_test_case(&combo3);
    
    // Test with valid -t and invalid options
    test_case_t combo4 = {
        .args = {"gcov-tool", "merge", "-t", "0.75", "-q", "-r", NULL},
        .expected_exit_code = 1,
        .description = "Valid -t with numeric arg, invalid -q -r"
    };
    run_test_case(&combo4);
    
    // Test complex combination
    test_case_t combo5 = {
        .args = {"gcov-tool", "-v", "-F", "-@", "-o", "-t", "1.5", "-j", NULL},
        .expected_exit_code = 1,
        .description = "Complex mix: valid -v -F -o -t, invalid -@ -j"
    };
    run_test_case(&combo5);
    
    // Test invalid option after valid merge command with output
    test_case_t combo6 = {
        .args = {"gcov-tool", "merge", "-o", "output.gcda", "-z", NULL},
        .expected_exit_code = 1,
        .description = "Valid merge with -o output, invalid -z"
    };
    run_test_case(&combo6);
    
    // Test with positional arguments
    test_case_t combo7 = {
        .args = {"gcov-tool", "input1.gcda", "input2.gcda", "-k", NULL},
        .expected_exit_code = 1,
        .description = "Positional arguments with invalid -k"
    };
    run_test_case(&combo7);
    
    // Test dash-dash separator with invalid option after
    test_case_t combo8 = {
        .args = {"gcov-tool", "--", "-v", "-m", NULL},
        .expected_exit_code = 1,
        .description = "Dash-dash separator with invalid -m after"
    };
    run_test_case(&combo8);
    
    printf("\n=== Test Suite Complete ===\n");
    printf("All tests designed to trigger the default case in the option parser.\n");
    printf("Check coverage with: gcov -b gcov-tool.cc\n");
    
    return 0;
}
