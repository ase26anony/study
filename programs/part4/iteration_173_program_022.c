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

// Test cases targeting the uncovered switch default case
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
    
    // Edge cases with non-alphabetic characters
    { {"gcov-tool", "-@", NULL}, 1, "Non-alphabetic invalid option -@" },
    { {"gcov-tool", "-1", NULL}, 1, "Numeric invalid option -1" },
    { {"gcov-tool", "-2", NULL}, 1, "Numeric invalid option -2" },
    { {"gcov-tool", "-!", NULL}, 1, "Special character invalid option -!" },
    { {"gcov-tool", "-#", NULL}, 1, "Special character invalid option -#" },
    { {"gcov-tool", "-$", NULL}, 1, "Special character invalid option -$" },
    { {"gcov-tool", "-%", NULL}, 1, "Special character invalid option -%" },
    { {"gcov-tool", "-^", NULL}, 1, "Special character invalid option -^" },
    { {"gcov-tool", "-&", NULL}, 1, "Special character invalid option -&" },
    { {"gcov-tool", "-*", NULL}, 1, "Special character invalid option -*" },
    { {"gcov-tool", "-(", NULL}, 1, "Special character invalid option -(" },
    { {"gcov-tool", "-)", NULL}, 1, "Special character invalid option -)" },
    { {"gcov-tool", "-_", NULL}, 1, "Special character invalid option -_" },
    { {"gcov-tool", "-+", NULL}, 1, "Special character invalid option -+" },
    { {"gcov-tool", "-=", NULL}, 1, "Special character invalid option -=" },
    { {"gcov-tool", "-[", NULL}, 1, "Special character invalid option -[" },
    { {"gcov-tool", "-]", NULL}, 1, "Special character invalid option -]" },
    { {"gcov-tool", "-{", NULL}, 1, "Special character invalid option -{" },
    { {"gcov-tool", "-}", NULL}, 1, "Special character invalid option -}" },
    { {"gcov-tool", "-|", NULL}, 1, "Special character invalid option -|" },
    { {"gcov-tool", "-\\", NULL}, 1, "Special character invalid option -\\" },
    { {"gcov-tool", "-:", NULL}, 1, "Special character invalid option -:" },
    { {"gcov-tool", "-;", NULL}, 1, "Special character invalid option -;" },
    { {"gcov-tool", "-'", NULL}, 1, "Special character invalid option -'" },
    { {"gcov-tool", "-<", NULL}, 1, "Special character invalid option -<" },
    { {"gcov-tool", "->", NULL}, 1, "Special character invalid option ->" },
    { {"gcov-tool", "-?", NULL}, 1, "Special character invalid option -?" },
    { {"gcov-tool", "-/", NULL}, 1, "Special character invalid option -/" },
    { {"gcov-tool", "-~", NULL}, 1, "Special character invalid option -~" },
    { {"gcov-tool", "-`", NULL}, 1, "Special character invalid option -`" },
    
    // Empty dash (edge case)
    { {"gcov-tool", "-", NULL}, 1, "Empty dash option -" },
    
    // Multiple invalid options combined
    { {"gcov-tool", "-abc", NULL}, 1, "Combined invalid options -abc" },
    { {"gcov-tool", "-xyz", NULL}, 1, "Combined invalid options -xyz" },
    { {"gcov-tool", "-defg", NULL}, 1, "Combined invalid options -defg" },
    
    // Invalid long options
    { {"gcov-tool", "--invalid-option", NULL}, 1, "Invalid long option" },
    { {"gcov-tool", "--unknown", NULL}, 1, "Unknown long option" },
    { {"gcov-tool", "--help-me", NULL}, 1, "Invalid long option --help-me" },
    { {"gcov-tool", "--version-info", NULL}, 1, "Invalid long option --version-info" },
    
    // Mixed valid and invalid options (stress testing)
    { {"gcov-tool", "-v", "-x", NULL}, 1, "Valid -v followed by invalid -x" },
    { {"gcov-tool", "-f", "-y", "-F", NULL}, 1, "Valid -f, -F with invalid -y" },
    { {"gcov-tool", "-o", "-z", "-h", NULL}, 1, "Valid -o, -h with invalid -z" },
    { {"gcov-tool", "-t", "0.5", "-a", NULL}, 1, "Valid -t with argument followed by invalid -a" },
    { {"gcov-tool", "-v", "-f", "-F", "-o", "-h", "-t", "1.0", "-q", NULL}, 1, "All valid options with invalid -q at end" },
    
    // Complex combinations with merge command
    { {"gcov-tool", "merge", "-o", "output.gcda", "-z", NULL}, 1, "Merge command with invalid -z" },
    { {"gcov-tool", "merge", "-v", "-x", "input1.gcda", "input2.gcda", NULL}, 1, "Merge with valid -v and invalid -x" },
    { {"gcov-tool", "merge", "-t", "0.75", "-@", "input.gcda", NULL}, 1, "Merge with valid -t and invalid -@" },
    
    // Test -t with various numeric arguments (to ensure case 't' is exercised)
    { {"gcov-tool", "-t", "0.0", NULL}, 1, "Valid -t with 0.0 (no command)" },
    { {"gcov-tool", "-t", "0.5", NULL}, 1, "Valid -t with 0.5 (no command)" },
    { {"gcov-tool", "-t", "1.0", NULL}, 1, "Valid -t with 1.0 (no command)" },
    { {"gcov-tool", "-t", "1.0e2", NULL}, 1, "Valid -t with scientific notation 1.0e2" },
    { {"gcov-tool", "-t", "3.14159", NULL}, 1, "Valid -t with pi" },
    { {"gcov-tool", "-t", "100", NULL}, 1, "Valid -t with integer 100" },
    { {"gcov-tool", "-t", "0.001", NULL}, 1, "Valid -t with small decimal 0.001" },
    
    // Invalid -t arguments (should also trigger error)
    { {"gcov-tool", "-t", "invalid", NULL}, 1, "-t with non-numeric argument" },
    { {"gcov-tool", "-t", NULL}, 1, "-t without argument" },
    
    // Valid invocations (to ensure basic functionality works)
    { {"gcov-tool", "--help", NULL}, 0, "Valid --help invocation" },
    { {"gcov-tool", "-v", "--help", NULL}, 0, "Valid -v --help combination" },
    { {"gcov-tool", "merge", "--help", NULL}, 0, "Valid merge --help" },
    
    {NULL, 0, NULL} // Sentinel
};

void run_test_case(test_case_t *test) {
    pid_t pid;
    int status;
    
    printf("Testing: %s\n", test->description);
    printf("Command: ");
    for (int i = 0; test->args[i] != NULL; i++) {
        printf("%s ", test->args[i]);
    }
    printf("\n");
    
    pid = fork();
    if (pid == 0) {
        // Child process
        execvp("gcov-tool", test->args);
        // If execvp fails
        perror("execvp failed");
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
        perror("fork failed");
    }
}

int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("Testing gcov-tool invalid option handling\n");
    printf("Target: Trigger default case in switch statement\n");
    printf("Uncovered lines: 534-554 in gcov-tool.cc\n");
    printf("========================================\n\n");
    
    // First, compile gcov-tool with coverage if needed
    printf("Checking if gcov-tool exists...\n");
    if (access("gcov-tool", X_OK) != 0) {
        printf("gcov-tool not found in current directory.\n");
        printf("Attempting to compile from gcov-tool.cc...\n");
        
        // Try different compilation options
        const char *compile_cmds[] = {
            "g++ -O0 -g -lgcov --coverage gcov-tool.cc -o gcov-tool",
            "g++ -O2 -fsanitize=address -fno-omit-frame-pointer -lgcov gcov-tool.cc -o gcov-tool",
            "g++ -O3 -fprofile-arcs -ftest-coverage -lgcov gcov-tool.cc -o gcov-tool",
            NULL
        };
        
        int compiled = 0;
        for (int i = 0; compile_cmds[i] != NULL; i++) {
            printf("Trying: %s\n", compile_cmds[i]);
            if (system(compile_cmds[i]) == 0) {
                compiled = 1;
                printf("Compilation successful!\n");
                break;
            }
        }
        
        if (!compiled) {
            printf("Failed to compile gcov-tool. Using system gcov-tool if available.\n");
        }
    } else {
        printf("Using existing gcov-tool binary.\n");
    }
    
    printf("\nStarting invalid option tests...\n");
    printf("========================================\n\n");
    
    // Run all test cases
    int total_tests = 0;
    int passed_tests = 0;
    
    for (int i = 0; test_cases[i].args[0] != NULL; i++) {
        total_tests++;
        run_test_case(&test_cases[i]);
        
        // Check if gcov-tool exists in PATH if not in current directory
        if (access("gcov-tool", X_OK) != 0) {
            // Try using system gcov-tool
            test_cases[i].args[0] = "gcov-tool";
        }
    }
    
    // Additional systematic test: all lowercase letters except v,f,o,h,t
    printf("\n========================================\n");
    printf("Systematic test of all invalid single-letter options\n");
    printf("========================================\n\n");
    
    for (char c = 'a'; c <= 'z'; c++) {
        // Skip valid options: v, f, F, o, h, t
        if (c == 'v' || c == 'f' || c == 'o' || c == 'h' || c == 't') {
            continue;
        }
        
        char option[3] = "- ";
        option[1] = c;
        
        char description[50];
        snprintf(description, sizeof(description), "Systematic test: invalid option -%c", c);
        
        char *args[] = {"gcov-tool", option, NULL};
        
        printf("Testing: %s\n", description);
        printf("Command: gcov-tool %s\n", option);
        
        pid_t pid = fork();
        if (pid == 0) {
            if (access("gcov-tool", X_OK) == 0) {
                execvp("./gcov-tool", args);
            } else {
                execvp("gcov-tool", args);
            }
            perror("execvp failed");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                printf("Exit code: %d - %s\n\n", 
                       exit_code,
                       (exit_code != 0) ? "PASS (triggered error)" : "FAIL (unexpected success)");
                if (exit_code != 0) passed_tests++;
            }
            total_tests++;
        }
    }
    
    // Test uppercase letters (except F)
    printf("\n========================================\n");
    printf("Testing uppercase invalid options\n");
    printf("========================================\n\n");
    
    for (char c = 'A'; c <= 'Z'; c++) {
        // Skip valid option: F
        if (c == 'F') {
            continue;
        }
        
        char option[3] = "- ";
        option[1] = c;
        
        char description[50];
        snprintf(description, sizeof(description), "Uppercase test: invalid option -%c", c);
        
        char *args[] = {"gcov-tool", option, NULL};
        
        printf("Testing: %s\n", description);
        printf("Command: gcov-tool %s\n", option);
        
        pid_t pid = fork();
        if (pid == 0) {
            if (access("gcov-tool", X_OK) == 0) {
                execvp("./gcov-tool", args);
            } else {
                execvp("gcov-tool", args);
            }
            perror("execvp failed");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                printf("Exit code: %d - %s\n\n", 
                       exit_code,
                       (exit_code != 0) ? "PASS (triggered error)" : "FAIL (unexpected success)");
                if (exit_code != 0) passed_tests++;
            }
            total_tests++;
        }
    }
    
    printf("\n========================================\n");
    printf("Test Summary\n");
    printf("========================================\n");
    printf("Total tests run: %d\n", total_tests);
    printf("Tests that triggered errors (expected): %d\n", passed_tests);
    printf("\nCoverage impact:\n");
    printf("- Invalid options should trigger the 'default:' case\n");
    printf("- This should call overlap_usage() function\n");
    printf("- Exit code should be non-zero (typically 1)\n");
    
    // Generate a simple shell script alternative
    printf("\n========================================\n");
    printf("Shell script version for reference:\n");
    printf("========================================\n");
    printf("#!/bin/bash\n");
    printf("echo 'Testing invalid gcov-tool options'\n");
    printf("echo 'Target: Trigger default case in switch statement'\n");
    printf("echo ''\n");
    printf("# Test single invalid options\n");
    printf("for opt in a b c d e g i j k l m n p q r s u w x y z; do\n");
    printf("    echo \"Testing -$opt\"\n");
    printf("    gcov-tool -$opt 2>&1 | grep -i 'usage\\|error\\|invalid'\n");
    printf("    echo \"Exit code: $?\"\n");
    printf("    echo ''\n");
    printf("done\n");
    printf("# Test special characters\n");
    printf("for opt in '@' '1' '!' '#' '$' '%%' '^' '&' '*' '(' ')' '_' '+' '='; do\n");
    printf("    echo \"Testing -$opt\"\n");
    printf("    gcov-tool -$opt 2>&1\n");
    printf("    echo \"Exit code: $?\"\n");
    printf("    echo ''\n");
    printf("done\n");
    printf("# Test valid -t with argument\n");
    printf("echo 'Testing valid -t option (for context)'\n");
    printf("gcov-tool -t 0.5 2>&1\n");
    printf("echo ''\n");
    printf("# Test mixed valid and invalid\n");
    printf("echo 'Testing -v -x -f (valid, invalid, valid)'\n");
    printf("gcov-tool -v -x -f 2>&1\n");
    
    return 0;
}
