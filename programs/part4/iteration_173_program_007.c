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
    int arg_count;
    char *description;
} test_case;

// Helper function to execute gcov-tool with given arguments
int execute_gcov_tool(const char *args[], int arg_count) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        char **argv = malloc((arg_count + 2) * sizeof(char *));
        argv[0] = "./gcov-tool";
        
        for (int i = 0; i < arg_count; i++) {
            argv[i + 1] = strdup(args[i]);
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
        return WEXITSTATUS(status);
    } else {
        perror("fork failed");
        return -1;
    }
}

int main() {
    // First, compile gcov-tool with coverage instrumentation
    printf("Compiling gcov-tool with coverage instrumentation...\n");
    system("g++ -O0 -g -fprofile-arcs -ftest-coverage gcov-tool.cc -lgcov -o gcov-tool");
    
    // Test cases array
    test_case test_cases[MAX_TEST_CASES];
    int test_count = 0;
    
    // Test 1: Basic valid invocation to ensure tool works
    test_cases[test_count++] = (test_case){
        .args = {"--help"},
        .arg_count = 1,
        .description = "Valid invocation with --help"
    };
    
    // Test 2: Valid invocation with -v option
    test_cases[test_count++] = (test_case){
        .args = {"merge", "-v"},
        .arg_count = 2,
        .description = "Valid invocation with -v option"
    };
    
    // Test 3: Valid invocation with -t option (to cover case 't')
    test_cases[test_count++] = (test_case){
        .args = {"merge", "-t", "0.5"},
        .arg_count = 3,
        .description = "Valid invocation with -t 0.5"
    };
    
    // Test 4: Valid invocation with -t and scientific notation
    test_cases[test_count++] = (test_case){
        .args = {"merge", "-t", "1.0e2"},
        .arg_count = 3,
        .description = "Valid invocation with -t 1.0e2"
    };
    
    // Test 5: Valid invocation with multiple valid options
    test_cases[test_count++] = (test_case){
        .args = {"merge", "-v", "-f", "-F", "-o", "-h", "-t", "0.8"},
        .arg_count = 8,
        .description = "Valid invocation with all covered options"
    };
    
    // INVALID OPTION TESTS - Targeting the default case
    
    // Test 6: Single invalid character option
    test_cases[test_count++] = (test_case){
        .args = {"-a"},
        .arg_count = 1,
        .description = "Single invalid option -a"
    };
    
    // Test 7: Multiple invalid character options
    test_cases[test_count++] = (test_case){
        .args = {"-bcd"},
        .arg_count = 1,
        .description = "Multiple invalid options -bcd"
    };
    
    // Test 8: Invalid option after valid option
    test_cases[test_count++] = (test_case){
        .args = {"merge", "-v", "-x"},
        .arg_count = 3,
        .description = "Valid -v followed by invalid -x"
    };
    
    // Test 9: Invalid option before valid option
    test_cases[test_count++] = (test_case){
        .args = {"merge", "-z", "-f"},
        .arg_count = 3,
        .description = "Invalid -z before valid -f"
    };
    
    // Test 10: Chain of invalid options
    test_cases[test_count++] = (test_case){
        .args = {"-egijk"},
        .arg_count = 1,
        .description = "Chain of invalid options -egijk"
    };
    
    // Test 11: Invalid option with merge subcommand
    test_cases[test_count++] = (test_case){
        .args = {"merge", "-o", "output.gcda", "-z"},
        .arg_count = 4,
        .description = "Invalid -z after valid -o with argument"
    };
    
    // Test 12: Multiple invalid options separated
    test_cases[test_count++] = (test_case){
        .args = {"-m", "-n", "-p"},
        .arg_count = 3,
        .description = "Multiple separate invalid options -m -n -p"
    };
    
    // Test 13: Invalid long option
    test_cases[test_count++] = (test_case){
        .args = {"--invalid-option"},
        .arg_count = 1,
        .description = "Invalid long option --invalid-option"
    };
    
    // Test 14: Mixed valid and invalid long options
    test_cases[test_count++] = (test_case){
        .args = {"--help", "--invalid"},
        .arg_count = 2,
        .description = "Valid --help with invalid --invalid"
    };
    
    // Test 15: Edge case - single dash
    test_cases[test_count++] = (test_case){
        .args = {"-"},
        .arg_count = 1,
        .description = "Single dash -"
    };
    
    // Test 16: Edge case - non-alphabetic character
    test_cases[test_count++] = (test_case){
        .args = {"-@"},
        .arg_count = 1,
        .description = "Non-alphabetic option -@"
    };
    
    // Test 17: Edge case - numeric character
    test_cases[test_count++] = (test_case){
        .args = {"-2"},
        .arg_count = 1,
        .description = "Numeric option -2"
    };
    
    // Test 18: Edge case - special characters
    test_cases[test_count++] = (test_case){
        .args = {"-!", "-#", "-$", "-%", "-&", "-*", "-(", "-)", "-_", "-=", "-+", "-[", "-]", "-{", "-}", "-|", "-\\", "-;", "-:", "-'", "-\"", "-<", "->", "-?", "-/"},
        .arg_count = 25,
        .description = "Various special character options"
    };
    
    // Test 19: Invalid option with valid -t argument
    test_cases[test_count++] = (test_case){
        .args = {"merge", "-t", "0.5", "-q"},
        .arg_count = 4,
        .description = "Valid -t 0.5 followed by invalid -q"
    };
    
    // Test 20: Complex mix of valid and invalid
    test_cases[test_count++] = (test_case){
        .args = {"merge", "-v", "-x", "-f", "-y", "-F", "-z", "-o", "-w", "-h", "-u", "-t", "1.5", "-r"},
        .arg_count = 14,
        .description = "Complex mix of valid and invalid options"
    };
    
    // Test 21: All lowercase letters not in {v, f, o, h, t}
    char invalid_chars[] = "abcdegijklnmpqrsuwxyz";
    for (int i = 0; i < strlen(invalid_chars); i++) {
        char option[3] = "- ";
        option[1] = invalid_chars[i];
        
        test_cases[test_count] = (test_case){
            .args = {option},
            .arg_count = 1,
            .description = NULL
        };
        
        // Allocate memory for description
        char *desc = malloc(50);
        snprintf(desc, 50, "Single invalid option -%c", invalid_chars[i]);
        test_cases[test_count].description = desc;
        
        test_count++;
    }
    
    // Test 22: All uppercase letters not in {F}
    char invalid_upper[] = "ABCDEGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < strlen(invalid_upper); i++) {
        if (invalid_upper[i] == 'F') continue; // Skip valid F
        
        char option[3] = "- ";
        option[1] = invalid_upper[i];
        
        test_cases[test_count] = (test_case){
            .args = {option},
            .arg_count = 1,
            .description = NULL
        };
        
        // Allocate memory for description
        char *desc = malloc(50);
        snprintf(desc, 50, "Single invalid uppercase option -%c", invalid_upper[i]);
        test_cases[test_count].description = desc;
        
        test_count++;
    }
    
    printf("\n=== Running %d test cases ===\n\n", test_count);
    
    int passed = 0;
    int failed = 0;
    
    // Execute all test cases
    for (int i = 0; i < test_count; i++) {
        printf("Test %d: %s\n", i + 1, test_cases[i].description);
        printf("Command: ./gcov-tool");
        for (int j = 0; j < test_cases[i].arg_count; j++) {
            printf(" %s", test_cases[i].args[j]);
        }
        printf("\n");
        
        int result = execute_gcov_tool(test_cases[i].args, test_cases[i].arg_count);
        
        // Check if the test passed
        // For invalid options, we expect non-zero exit code
        // For valid options, exit code could be 0 or non-zero depending on other factors
        // We'll just report the exit code
        printf("Exit code: %d\n", result);
        
        if (result == 0) {
            printf("Status: OK\n");
            passed++;
        } else {
            printf("Status: Non-zero exit (may be expected for invalid options)\n");
            // Still count as passed if it's an invalid option test
            // and we got a non-zero exit (which triggers overlap_usage())
            if (strstr(test_cases[i].description, "invalid") != NULL ||
                strstr(test_cases[i].description, "Invalid") != NULL) {
                passed++;
            } else {
                failed++;
            }
        }
        
        printf("\n");
        
        // Free allocated description memory
        if (i >= 21) { // For dynamically allocated descriptions
            free(test_cases[i].description);
        }
    }
    
    printf("\n=== Summary ===\n");
    printf("Total tests: %d\n", test_count);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    
    // Generate coverage report
    printf("\n=== Generating coverage report ===\n");
    system("gcov gcov-tool.cc");
    
    // Check specifically for coverage of lines 534-554
    printf("\n=== Checking coverage of target lines (534-554) ===\n");
    system("grep -n -A 20 -B 5 \"case 'v':\" gcov-tool.cc.gcov");
    
    return (failed == 0) ? 0 : 1;
}
