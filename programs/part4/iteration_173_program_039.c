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

// Test cases designed to trigger the uncovered default case
test_case_t test_cases[] = {
    // Single invalid options (not in: v, f, F, o, h, t)
    { {"gcov-tool", "-a", NULL}, 1, "Single invalid option -a" },
    { {"gcov-tool", "-b", NULL}, 1, "Single invalid option -b" },
    { {"gcov-tool", "-c", NULL}, 1, "Single invalid option -c" },
    { {"gcov-tool", "-d", NULL}, 1, "Single invalid option -d" },
    { {"gcov-tool", "-e", NULL}, 1, "Single invalid option -e" },
    { {"gcov-tool", "-g", NULL}, 1, "Single invalid option -g" },
    { {"gcov-tool", "-i", NULL}, 1, "Single invalid option -i" },
    { {"gcov-tool", "-j", NULL}, 1, "Single invalid option -j" },
    { {"gcov-tool", "-k", NULL}, 1, "Single invalid option -k" },
    { {"gcov-tool", "-l", NULL}, 1, "Single invalid option -l" },
    { {"gcov-tool", "-m", NULL}, 1, "Single invalid option -m" },
    { {"gcov-tool", "-n", NULL}, 1, "Single invalid option -n" },
    { {"gcov-tool", "-p", NULL}, 1, "Single invalid option -p" },
    { {"gcov-tool", "-q", NULL}, 1, "Single invalid option -q" },
    { {"gcov-tool", "-r", NULL}, 1, "Single invalid option -r" },
    { {"gcov-tool", "-s", NULL}, 1, "Single invalid option -s" },
    { {"gcov-tool", "-u", NULL}, 1, "Single invalid option -u" },
    { {"gcov-tool", "-w", NULL}, 1, "Single invalid option -w" },
    { {"gcov-tool", "-x", NULL}, 1, "Single invalid option -x" },
    { {"gcov-tool", "-y", NULL}, 1, "Single invalid option -y" },
    { {"gcov-tool", "-z", NULL}, 1, "Single invalid option -z" },
    
    // Multiple invalid options chained together
    { {"gcov-tool", "-abc", NULL}, 1, "Multiple invalid options chained -abc" },
    { {"gcov-tool", "-xyz", NULL}, 1, "Multiple invalid options chained -xyz" },
    { {"gcov-tool", "-defg", NULL}, 1, "Multiple invalid options chained -defg" },
    
    // Invalid options with non-alphabetic characters
    { {"gcov-tool", "-@", NULL}, 1, "Non-alphabetic invalid option -@" },
    { {"gcov-tool", "-1", NULL}, 1, "Numeric invalid option -1" },
    { {"gcov-tool", "-2", NULL}, 1, "Numeric invalid option -2" },
    { {"gcov-tool", "-!", NULL}, 1, "Special character invalid option -!" },
    { {"gcov-tool", "-$", NULL}, 1, "Special character invalid option -$" },
    { {"gcov-tool", "-%", NULL}, 1, "Special character invalid option -%" },
    
    // Edge cases for option parsing
    { {"gcov-tool", "-", NULL}, 1, "Single dash only" },
    { {"gcov-tool", "--", NULL}, 0, "Double dash (should be valid)" },
    { {"gcov-tool", "--invalid-long-option", NULL}, 1, "Invalid long option" },
    { {"gcov-tool", "--help", NULL}, 0, "Valid long option --help" },
    
    // Complex combinations: valid options followed by invalid ones
    { {"gcov-tool", "merge", "-v", "-x", NULL}, 1, "Valid -v followed by invalid -x" },
    { {"gcov-tool", "merge", "-f", "-g", NULL}, 1, "Valid -f followed by invalid -g" },
    { {"gcov-tool", "merge", "-F", "-h", "-z", NULL}, 1, "Valid -F and -h followed by invalid -z" },
    { {"gcov-tool", "merge", "-o", "-a", "-b", NULL}, 1, "Valid -o followed by invalid -a -b" },
    
    // Valid -t option with argument, followed by invalid option
    { {"gcov-tool", "merge", "-t", "0.5", "-x", NULL}, 1, "Valid -t 0.5 followed by invalid -x" },
    { {"gcov-tool", "merge", "-t", "1.0e2", "-y", NULL}, 1, "Valid -t 1.0e2 followed by invalid -y" },
    { {"gcov-tool", "merge", "-t", "100", "-z", NULL}, 1, "Valid -t 100 followed by invalid -z" },
    
    // Invalid option in the middle of valid options
    { {"gcov-tool", "merge", "-v", "-x", "-f", NULL}, 1, "Invalid -x between valid -v and -f" },
    { {"gcov-tool", "merge", "-o", "-@", "-h", NULL}, 1, "Invalid -@ between valid -o and -h" },
    
    // Multiple invalid options separated
    { {"gcov-tool", "merge", "-a", "-b", "-c", NULL}, 1, "Multiple separate invalid options" },
    { {"gcov-tool", "merge", "-1", "-2", "-3", NULL}, 1, "Multiple numeric invalid options" },
    
    // Mixed case: uppercase invalid options (except F which is valid)
    { {"gcov-tool", "merge", "-A", "-B", "-C", NULL}, 1, "Uppercase invalid options" },
    { {"gcov-tool", "merge", "-G", "-H", "-I", NULL}, 1, "Uppercase invalid options (H is valid lowercase)" },
    
    // Stress test: many options including invalid ones
    { {"gcov-tool", "merge", "-v", "-f", "-F", "-o", "-h", "-t", "0.75", "-x", "-y", "-z", NULL}, 1, 
      "All valid options plus trailing invalid ones" },
    
    // Test with no subcommand (should also trigger usage)
    { {"gcov-tool", "-a", NULL}, 1, "Invalid option without subcommand" },
    { {"gcov-tool", "-v", "-x", NULL}, 1, "Valid -v with invalid -x without subcommand" },
    
    // Valid test cases to ensure basic functionality works
    { {"gcov-tool", "--help", NULL}, 0, "Valid: --help" },
    { {"gcov-tool", "-v", "--help", NULL}, 0, "Valid: -v --help" },
    { {"gcov-tool", "merge", "-v", "-f", "-F", "-o", "-h", "-t", "0.5", NULL}, 0, 
      "Valid: all covered options with -t argument" },
    
    { NULL, 0, NULL } // Sentinel
};

void run_test_case(test_case_t *test) {
    pid_t pid;
    int status;
    
    printf("Running: %s\n", test->description);
    printf("Command: ");
    for (int i = 0; test->args[i] != NULL; i++) {
        printf("%s ", test->args[i]);
    }
    printf("\n");
    
    pid = fork();
    if (pid == 0) {
        // Child process
        execvp("gcov-tool", test->args);
        // If execvp returns, there was an error
        fprintf(stderr, "Failed to execute gcov-tool: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Exit code: %d (expected: %d) - %s\n\n", 
                   exit_code, test->expected_exit_code,
                   (exit_code == test->expected_exit_code) ? "PASS" : "FAIL");
        } else {
            printf("Process terminated abnormally\n\n");
        }
    } else {
        fprintf(stderr, "Failed to fork: %s\n", strerror(errno));
    }
}

void generate_invalid_option_combinations() {
    // Generate all possible invalid single-character options
    printf("\n=== Testing all invalid single-character options ===\n");
    
    for (char c = 'a'; c <= 'z'; c++) {
        // Skip valid options: v, f, F, o, h, t
        if (c == 'v' || c == 'f' || c == 'o' || c == 'h' || c == 't') {
            continue;
        }
        // Skip uppercase F (valid) but test other uppercase
        if (c == 'F') {
            continue;
        }
        
        char option[3] = "- ";
        option[1] = c;
        
        char *args[] = {"gcov-tool", "merge", option, NULL};
        test_case_t test = { {"gcov-tool", "merge", option, NULL}, 1, 
                           "Generated test for invalid option" };
        
        run_test_case(&test);
    }
    
    // Test uppercase letters (except F)
    printf("\n=== Testing invalid uppercase options ===\n");
    for (char c = 'A'; c <= 'Z'; c++) {
        if (c == 'F') continue; // F is valid
        
        char option[3] = "- ";
        option[1] = c;
        
        char *args[] = {"gcov-tool", "merge", option, NULL};
        test_case_t test = { {"gcov-tool", "merge", option, NULL}, 1, 
                           "Generated test for invalid uppercase option" };
        
        run_test_case(&test);
    }
}

int main(int argc, char *argv[]) {
    printf("=== GCOV-TOOL Invalid Option Test Suite ===\n");
    printf("Target: Trigger default case in option parser (lines 534-554)\n");
    printf("Goal: Force call to overlap_usage() via unrecognized options\n\n");
    
    // Run predefined test cases
    printf("=== Running predefined test cases ===\n");
    for (int i = 0; test_cases[i].args[0] != NULL; i++) {
        run_test_case(&test_cases[i]);
    }
    
    // Generate comprehensive invalid option tests
    generate_invalid_option_combinations();
    
    // Special stress tests
    printf("\n=== Special stress tests ===\n");
    
    // Test with extremely long invalid option string
    char long_invalid[100] = "-";
    for (int i = 0; i < 26; i++) {
        char c = 'a' + i;
        // Include some valid options to test parsing
        if (c != 'v' && c != 'f' && c != 'o' && c != 'h' && c != 't') {
            strncat(long_invalid, &c, 1);
        }
    }
    
    test_case_t stress_test = { 
        {"gcov-tool", "merge", long_invalid, NULL}, 
        1, 
        "Stress test: long chain of invalid options" 
    };
    run_test_case(&stress_test);
    
    // Test option with argument that looks like another option
    test_case_t tricky_test = { 
        {"gcov-tool", "merge", "-t", "-v", NULL}, 
        1, 
        "Tricky: -t with -v as argument (should fail)" 
    };
    run_test_case(&tricky_test);
    
    // Test empty argument after -t
    test_case_t empty_arg_test = { 
        {"gcov-tool", "merge", "-t", "", NULL}, 
        1, 
        "Empty argument after -t" 
    };
    run_test_case(&empty_arg_test);
    
    printf("\n=== Test Summary ===\n");
    printf("All invalid options should trigger the default case in the switch statement,\n");
    printf("calling overlap_usage() and returning non-zero exit code.\n");
    printf("Valid options (v, f, F, o, h, t with numeric arg) should succeed.\n");
    
    return 0;
}
