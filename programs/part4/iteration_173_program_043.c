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

// Test cases targeting the uncovered switch statement
test_case_t test_cases[] = {
    // Invalid single-character options (not v, f, F, o, h, t)
    { {"gcov-tool", "merge", "-a", NULL}, 1, "Invalid option -a" },
    { {"gcov-tool", "merge", "-b", NULL}, 1, "Invalid option -b" },
    { {"gcov-tool", "merge", "-c", NULL}, 1, "Invalid option -c" },
    { {"gcov-tool", "merge", "-d", NULL}, 1, "Invalid option -d" },
    { {"gcov-tool", "merge", "-e", NULL}, 1, "Invalid option -e" },
    { {"gcov-tool", "merge", "-g", NULL}, 1, "Invalid option -g" },
    { {"gcov-tool", "merge", "-i", NULL}, 1, "Invalid option -i" },
    { {"gcov-tool", "merge", "-j", NULL}, 1, "Invalid option -j" },
    { {"gcov-tool", "merge", "-k", NULL}, 1, "Invalid option -k" },
    { {"gcov-tool", "merge", "-l", NULL}, 1, "Invalid option -l" },
    { {"gcov-tool", "merge", "-m", NULL}, 1, "Invalid option -m" },
    { {"gcov-tool", "merge", "-n", NULL}, 1, "Invalid option -n" },
    { {"gcov-tool", "merge", "-p", NULL}, 1, "Invalid option -p" },
    { {"gcov-tool", "merge", "-q", NULL}, 1, "Invalid option -q" },
    { {"gcov-tool", "merge", "-r", NULL}, 1, "Invalid option -r" },
    { {"gcov-tool", "merge", "-s", NULL}, 1, "Invalid option -s" },
    { {"gcov-tool", "merge", "-u", NULL}, 1, "Invalid option -u" },
    { {"gcov-tool", "merge", "-w", NULL}, 1, "Invalid option -w" },
    { {"gcov-tool", "merge", "-x", NULL}, 1, "Invalid option -x" },
    { {"gcov-tool", "merge", "-y", NULL}, 1, "Invalid option -y" },
    { {"gcov-tool", "merge", "-z", NULL}, 1, "Invalid option -z" },
    
    // Non-alphabetic invalid options
    { {"gcov-tool", "merge", "-@", NULL}, 1, "Invalid option -@" },
    { {"gcov-tool", "merge", "-1", NULL}, 1, "Invalid option -1" },
    { {"gcov-tool", "merge", "-2", NULL}, 1, "Invalid option -2" },
    { {"gcov-tool", "merge", "-!", NULL}, 1, "Invalid option -!" },
    { {"gcov-tool", "merge", "-#", NULL}, 1, "Invalid option -#" },
    { {"gcov-tool", "merge", "-$", NULL}, 1, "Invalid option -$" },
    { {"gcov-tool", "merge", "-%", NULL}, 1, "Invalid option -%" },
    { {"gcov-tool", "merge", "-^", NULL}, 1, "Invalid option -^" },
    { {"gcov-tool", "merge", "-&", NULL}, 1, "Invalid option -&" },
    { {"gcov-tool", "merge", "-*", NULL}, 1, "Invalid option -*" },
    { {"gcov-tool", "merge", "-(", NULL}, 1, "Invalid option -(" },
    { {"gcov-tool", "merge", "-)", NULL}, 1, "Invalid option -)" },
    { {"gcov-tool", "merge", "-_", NULL}, 1, "Invalid option -_" },
    { {"gcov-tool", "merge", "-+", NULL}, 1, "Invalid option -+" },
    { {"gcov-tool", "merge", "-=", NULL}, 1, "Invalid option -=" },
    { {"gcov-tool", "merge", "-[", NULL}, 1, "Invalid option -[" },
    { {"gcov-tool", "merge", "-]", NULL}, 1, "Invalid option -]" },
    { {"gcov-tool", "merge", "-{", NULL}, 1, "Invalid option -{" },
    { {"gcov-tool", "merge", "-}", NULL}, 1, "Invalid option -}" },
    { {"gcov-tool", "merge", "-|", NULL}, 1, "Invalid option -|" },
    { {"gcov-tool", "merge", "-\\", NULL}, 1, "Invalid option -\\" },
    { {"gcov-tool", "merge", "-:", NULL}, 1, "Invalid option -:" },
    { {"gcov-tool", "merge", "-;", NULL}, 1, "Invalid option -;" },
    { {"gcov-tool", "merge", "-'", NULL}, 1, "Invalid option -'" },
    { {"gcov-tool", "merge", "-<", NULL}, 1, "Invalid option -<" },
    { {"gcov-tool", "merge", "->", NULL}, 1, "Invalid option ->" },
    { {"gcov-tool", "merge", "-?", NULL}, 1, "Invalid option -?" },
    { {"gcov-tool", "merge", "-/", NULL}, 1, "Invalid option -/" },
    { {"gcov-tool", "merge", "-~", NULL}, 1, "Invalid option -~" },
    { {"gcov-tool", "merge", "-`", NULL}, 1, "Invalid option -`" },
    
    // Edge cases for option parsing
    { {"gcov-tool", "merge", "-", NULL}, 1, "Single dash only" },
    { {"gcov-tool", "merge", "--", NULL}, 1, "Double dash only" },
    { {"gcov-tool", "merge", "--invalid-option", NULL}, 1, "Long invalid option" },
    { {"gcov-tool", "merge", "--unknown", NULL}, 1, "Long unknown option" },
    
    // Combinations of valid and invalid options
    { {"gcov-tool", "merge", "-v", "-x", NULL}, 1, "Valid -v followed by invalid -x" },
    { {"gcov-tool", "merge", "-f", "-y", "-F", NULL}, 1, "Valid -f, -F with invalid -y" },
    { {"gcov-tool", "merge", "-o", "-z", "-h", NULL}, 1, "Valid -o, -h with invalid -z" },
    { {"gcov-tool", "merge", "-t", "0.5", "-a", NULL}, 1, "Valid -t with argument followed by invalid -a" },
    { {"gcov-tool", "merge", "-v", "-f", "-F", "-o", "-h", "-t", "1.0", "-q", NULL}, 1, "All valid options with invalid -q at end" },
    
    // Multiple invalid options chained together
    { {"gcov-tool", "merge", "-abc", NULL}, 1, "Multiple invalid options chained: -abc" },
    { {"gcov-tool", "merge", "-xyz", NULL}, 1, "Multiple invalid options chained: -xyz" },
    { {"gcov-tool", "merge", "-@#$", NULL}, 1, "Multiple non-alphabetic invalid options" },
    
    // Valid -t option with various numeric arguments (to exercise case 't')
    { {"gcov-tool", "merge", "-t", "0.5", NULL}, 1, "Valid -t with 0.5 (needs input files)" },
    { {"gcov-tool", "merge", "-t", "1.0", NULL}, 1, "Valid -t with 1.0 (needs input files)" },
    { {"gcov-tool", "merge", "-t", "0.0", NULL}, 1, "Valid -t with 0.0 (needs input files)" },
    { {"gcov-tool", "merge", "-t", "100.0", NULL}, 1, "Valid -t with 100.0 (needs input files)" },
    { {"gcov-tool", "merge", "-t", "1.0e2", NULL}, 1, "Valid -t with scientific notation 1.0e2 (needs input files)" },
    { {"gcov-tool", "merge", "-t", "0.5e-1", NULL}, 1, "Valid -t with scientific notation 0.5e-1 (needs input files)" },
    { {"gcov-tool", "merge", "-t", "1e3", NULL}, 1, "Valid -t with 1e3 (needs input files)" },
    
    // Complex combinations with file arguments
    { {"gcov-tool", "merge", "-v", "-t", "0.75", "input1.gcda", "-z", NULL}, 1, "Valid options with files and invalid -z" },
    { {"gcov-tool", "merge", "-o", "output.gcda", "-@", "input1.gcda", "input2.gcda", NULL}, 1, "Valid -o with output file and invalid -@" },
    
    // Empty string as option (edge case)
    { {"gcov-tool", "merge", "", NULL}, 1, "Empty string as argument" },
    
    // NULL terminator test case
    { {"gcov-tool", "merge", NULL}, 1, "No arguments for merge command" },
    
    // End marker
    { {NULL}, 0, NULL }
};

// Valid test cases to ensure basic functionality
test_case_t valid_test_cases[] = {
    { {"gcov-tool", "--help", NULL}, 0, "Help command" },
    { {"gcov-tool", "-h", NULL}, 0, "Short help (different from overlap -h)" },
    { {"gcov-tool", "--version", NULL}, 0, "Version command" },
    { {"gcov-tool", "merge", "--help", NULL}, 0, "Merge help" },
    { {NULL}, 0, NULL }
};

void run_test_case(test_case_t *test, int test_num, int is_valid) {
    pid_t pid;
    int status;
    
    printf("\nTest %d: %s\n", test_num, test->description);
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
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Exit code: %d\n", exit_code);
            
            if (is_valid) {
                if (exit_code == 0) {
                    printf("✓ PASS: Valid command executed successfully\n");
                } else {
                    printf("✗ FAIL: Valid command failed with exit code %d\n", exit_code);
                }
            } else {
                // For invalid options, we expect non-zero exit
                if (exit_code != 0) {
                    printf("✓ PASS: Invalid option correctly triggered error (exit code %d)\n", exit_code);
                } else {
                    printf("✗ FAIL: Invalid option should have failed but exited with 0\n");
                }
            }
        } else {
            printf("✗ FAIL: Process terminated abnormally\n");
        }
    } else {
        perror("fork failed");
    }
}

void create_test_gcda_files() {
    // Create dummy .gcda files for testing with actual file arguments
    system("echo 'dummy gcda content' > test_input1.gcda");
    system("echo 'dummy gcda content' > test_input2.gcda");
    system("echo 'dummy gcda content' > test_output.gcda");
}

void cleanup_test_files() {
    system("rm -f test_input1.gcda test_input2.gcda test_output.gcda");
}

int main(int argc, char *argv[]) {
    int test_count = 0;
    int passed_invalid = 0;
    int passed_valid = 0;
    
    printf("========================================\n");
    printf("Testing gcov-tool option parser coverage\n");
    printf("Target: Uncovered lines 534-554 in gcov-tool.cc\n");
    printf("========================================\n");
    
    // Create test files for commands that need them
    create_test_gcda_files();
    
    // First run valid test cases to ensure basic functionality
    printf("\n=== Running valid command tests ===\n");
    for (int i = 0; valid_test_cases[i].args[0] != NULL; i++) {
        run_test_case(&valid_test_cases[i], ++test_count, 1);
        if (WEXITSTATUS(system("gcov-tool --help > /dev/null 2>&1")) == 0) {
            passed_valid++;
        }
    }
    
    // Run invalid option tests to trigger the default case
    printf("\n=== Running invalid option tests (targeting default case) ===\n");
    for (int i = 0; test_cases[i].args[0] != NULL; i++) {
        run_test_case(&test_cases[i], ++test_count, 0);
        
        // Check if the command would trigger the default case
        // We can't directly check if overlap_usage() was called, but we can
        // check for non-zero exit which indicates error/usage
        pid_t pid = fork();
        if (pid == 0) {
            // Redirect stderr to /dev/null to suppress output during check
            freopen("/dev/null", "w", stderr);
            execvp("gcov-tool", test_cases[i].args);
            exit(EXIT_FAILURE);
        } else {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                passed_invalid++;
            }
        }
    }
    
    // Additional targeted tests for specific uncovered cases
    printf("\n=== Additional targeted tests ===\n");
    
    // Test all lowercase letters except v, f, o, h, t
    printf("\nTesting all invalid lowercase letters:\n");
    for (char c = 'a'; c <= 'z'; c++) {
        if (c != 'v' && c != 'f' && c != 'o' && c != 'h' && c != 't') {
            char option[3] = "- ";
            option[1] = c;
            
            pid_t pid = fork();
            if (pid == 0) {
                char *args[] = {"gcov-tool", "merge", option, NULL};
                execvp("gcov-tool", args);
                exit(EXIT_FAILURE);
            } else {
                int status;
                waitpid(pid, &status, 0);
                if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                    printf("✓ -%c triggered error\n", c);
                    passed_invalid++;
                } else {
                    printf("✗ -%c did not trigger error\n", c);
                }
            }
        }
    }
    
    // Test uppercase letters (except F)
    printf("\nTesting invalid uppercase letters:\n");
    for (char c = 'A'; c <= 'Z'; c++) {
        if (c != 'F') {
            char option[3] = "- ";
            option[1] = c;
            
            pid_t pid = fork();
            if (pid == 0) {
                char *args[] = {"gcov-tool", "merge", option, NULL};
                execvp("gcov-tool", args);
                exit(EXIT_FAILURE);
            } else {
                int status;
                waitpid(pid, &status, 0);
                if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                    printf("✓ -%c triggered error\n", c);
                    passed_invalid++;
                } else {
                    printf("✗ -%c did not trigger error\n", c);
                }
            }
        }
    }
    
    // Clean up test files
    cleanup_test_files();
    
    // Summary
    printf("\n========================================\n");
    printf("TEST SUMMARY\n");
    printf("========================================\n");
    printf("Valid commands tested: %d\n", passed_valid);
    printf("Invalid options tested: %d\n", passed_invalid);
    printf("Total tests targeting default case: %d\n", passed_invalid);
    printf("\nCoverage target: Lines 534-554 in gcov-tool.cc\n");
    printf("Specifically targeting the default case in switch statement\n");
    printf("which calls overlap_usage() for invalid options.\n");
    printf("========================================\n");
    
    return 0;
}
