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

// Test cases targeting the uncovered switch statement
test_case_t test_cases[] = {
    // Single invalid options (not v, f, F, o, h, t)
    { (char*[]){"gcov-tool", "-a", NULL}, 1, "Single invalid option -a" },
    { (char*[]){"gcov-tool", "-b", NULL}, 1, "Single invalid option -b" },
    { (char*[]){"gcov-tool", "-c", NULL}, 1, "Single invalid option -c" },
    { (char*[]){"gcov-tool", "-d", NULL}, 1, "Single invalid option -d" },
    { (char*[]){"gcov-tool", "-e", NULL}, 1, "Single invalid option -e" },
    { (char*[]){"gcov-tool", "-g", NULL}, 1, "Single invalid option -g" },
    { (char*[]){"gcov-tool", "-i", NULL}, 1, "Single invalid option -i" },
    { (char*[]){"gcov-tool", "-j", NULL}, 1, "Single invalid option -j" },
    { (char*[]){"gcov-tool", "-k", NULL}, 1, "Single invalid option -k" },
    { (char*[]){"gcov-tool", "-l", NULL}, 1, "Single invalid option -l" },
    { (char*[]){"gcov-tool", "-m", NULL}, 1, "Single invalid option -m" },
    { (char*[]){"gcov-tool", "-n", NULL}, 1, "Single invalid option -n" },
    { (char*[]){"gcov-tool", "-p", NULL}, 1, "Single invalid option -p" },
    { (char*[]){"gcov-tool", "-q", NULL}, 1, "Single invalid option -q" },
    { (char*[]){"gcov-tool", "-r", NULL}, 1, "Single invalid option -r" },
    { (char*[]){"gcov-tool", "-s", NULL}, 1, "Single invalid option -s" },
    { (char*[]){"gcov-tool", "-u", NULL}, 1, "Single invalid option -u" },
    { (char*[]){"gcov-tool", "-w", NULL}, 1, "Single invalid option -w" },
    { (char*[]){"gcov-tool", "-x", NULL}, 1, "Single invalid option -x" },
    { (char*[]){"gcov-tool", "-y", NULL}, 1, "Single invalid option -y" },
    { (char*[]){"gcov-tool", "-z", NULL}, 1, "Single invalid option -z" },
    
    // Non-alphabetic invalid options
    { (char*[]){"gcov-tool", "-@", NULL}, 1, "Non-alphabetic invalid option -@" },
    { (char*[]){"gcov-tool", "-1", NULL}, 1, "Numeric invalid option -1" },
    { (char*[]){"gcov-tool", "-2", NULL}, 1, "Numeric invalid option -2" },
    { (char*[]){"gcov-tool", "-!", NULL}, 1, "Special character invalid option -!" },
    { (char*[]){"gcov-tool", "-#", NULL}, 1, "Special character invalid option -#" },
    { (char*[]){"gcov-tool", "-$", NULL}, 1, "Special character invalid option -$" },
    { (char*[]){"gcov-tool", "-%", NULL}, 1, "Special character invalid option -%" },
    { (char*[]){"gcov-tool", "-^", NULL}, 1, "Special character invalid option -^" },
    { (char*[]){"gcov-tool", "-&", NULL}, 1, "Special character invalid option -&" },
    { (char*[]){"gcov-tool", "-*", NULL}, 1, "Special character invalid option -*" },
    { (char*[]){"gcov-tool", "-(", NULL}, 1, "Special character invalid option -(" },
    { (char*[]){"gcov-tool", "-)", NULL}, 1, "Special character invalid option -)" },
    { (char*[]){"gcov-tool", "-_", NULL}, 1, "Special character invalid option -_" },
    { (char*[]){"gcov-tool", "-+", NULL}, 1, "Special character invalid option -+" },
    { (char*[]){"gcov-tool", "-=", NULL}, 1, "Special character invalid option -=" },
    { (char*[]){"gcov-tool", "-[", NULL}, 1, "Special character invalid option -[" },
    { (char*[]){"gcov-tool", "-]", NULL}, 1, "Special character invalid option -]" },
    { (char*[]){"gcov-tool", "-{", NULL}, 1, "Special character invalid option -{" },
    { (char*[]){"gcov-tool", "-}", NULL}, 1, "Special character invalid option -}" },
    { (char*[]){"gcov-tool", "-|", NULL}, 1, "Special character invalid option -|" },
    { (char*[]){"gcov-tool", "-\\", NULL}, 1, "Special character invalid option -\\" },
    { (char*[]){"gcov-tool", "-:", NULL}, 1, "Special character invalid option -:" },
    { (char*[]){"gcov-tool", "-;", NULL}, 1, "Special character invalid option -;" },
    { (char*[]){"gcov-tool", "-'", NULL}, 1, "Special character invalid option -'" },
    { (char*[]){"gcov-tool", "-<", NULL}, 1, "Special character invalid option -<" },
    { (char*[]){"gcov-tool", "->", NULL}, 1, "Special character invalid option ->" },
    { (char*[]){"gcov-tool", "-?", NULL}, 1, "Special character invalid option -?" },
    { (char*[]){"gcov-tool", "-/", NULL}, 1, "Special character invalid option -/" },
    { (char*[]){"gcov-tool", "-~", NULL}, 1, "Special character invalid option -~" },
    { (char*[]){"gcov-tool", "-`", NULL}, 1, "Special character invalid option -`" },
    
    // Edge case: single dash only
    { (char*[]){"gcov-tool", "-", NULL}, 1, "Single dash only" },
    
    // Long invalid options
    { (char*[]){"gcov-tool", "--invalid-option", NULL}, 1, "Long invalid option" },
    { (char*[]){"gcov-tool", "--unknown", NULL}, 1, "Long unknown option" },
    { (char*[]){"gcov-tool", "--bad-option", NULL}, 1, "Long bad option" },
    
    // Combinations of valid and invalid options
    { (char*[]){"gcov-tool", "-v", "-x", NULL}, 1, "Valid -v followed by invalid -x" },
    { (char*[]){"gcov-tool", "-f", "-y", "-F", NULL}, 1, "Valid -f, invalid -y, valid -F" },
    { (char*[]){"gcov-tool", "-o", "-z", "-h", NULL}, 1, "Valid -o, invalid -z, valid -h" },
    { (char*[]){"gcov-tool", "-v", "-f", "-F", "-o", "-h", "-t", "0.5", "-x", NULL}, 1, 
      "All valid options plus invalid -x at end" },
    { (char*[]){"gcov-tool", "-x", "-v", "-f", NULL}, 1, "Invalid -x before valid options" },
    
    // Multiple invalid options combined
    { (char*[]){"gcov-tool", "-a", "-b", "-c", NULL}, 1, "Multiple invalid options a,b,c" },
    { (char*[]){"gcov-tool", "-d", "-e", "-f", "-g", NULL}, 1, "Mixed invalid and valid (-f)" },
    { (char*[]){"gcov-tool", "-x", "-y", "-z", NULL}, 1, "Multiple invalid options x,y,z" },
    
    // Valid -t option with various numeric arguments (to exercise case 't')
    { (char*[]){"gcov-tool", "-t", "0.5", NULL}, 1, "Valid -t with 0.5 (but missing required subcommand)" },
    { (char*[]){"gcov-tool", "-t", "1.0", NULL}, 1, "Valid -t with 1.0 (but missing required subcommand)" },
    { (char*[]){"gcov-tool", "-t", "0.0", NULL}, 1, "Valid -t with 0.0 (but missing required subcommand)" },
    { (char*[]){"gcov-tool", "-t", "100.0", NULL}, 1, "Valid -t with 100.0 (but missing required subcommand)" },
    { (char*[]){"gcov-tool", "-t", "1.5e2", NULL}, 1, "Valid -t with scientific notation 1.5e2" },
    { (char*[]){"gcov-tool", "-t", "0.001", NULL}, 1, "Valid -t with small decimal 0.001" },
    { (char*[]){"gcov-tool", "-t", "999.999", NULL}, 1, "Valid -t with large decimal 999.999" },
    
    // Valid -t followed by invalid option
    { (char*[]){"gcov-tool", "-t", "0.5", "-x", NULL}, 1, "Valid -t 0.5 followed by invalid -x" },
    { (char*[]){"gcov-tool", "-v", "-t", "1.0", "-z", NULL}, 1, "Valid -v -t 1.0 followed by invalid -z" },
    
    // Invalid option in merge subcommand context
    { (char*[]){"gcov-tool", "merge", "-o", "output.gcda", "-z", NULL}, 1, "Invalid -z in merge context" },
    { (char*[]){"gcov-tool", "merge", "-v", "-x", "input.gcda", NULL}, 1, "Invalid -x in merge with -v" },
    { (char*[]){"gcov-tool", "merge", "-t", "0.5", "-y", "input.gcda", NULL}, 1, "Invalid -y after -t in merge" },
    
    // Invalid option in overlap subcommand context
    { (char*[]){"gcov-tool", "overlap", "-v", "-x", NULL}, 1, "Invalid -x in overlap context" },
    { (char*[]){"gcov-tool", "overlap", "-f", "-F", "-o", "-h", "-t", "0.5", "-a", NULL}, 1, 
      "All valid overlap options plus invalid -a" },
    
    // Complex combinations with subcommands
    { (char*[]){"gcov-tool", "merge", "-v", "-f", "-F", "-o", "-h", "-t", "0.5", "-@", "input.gcda", NULL}, 1,
      "All valid merge options plus invalid -@" },
    
    // Multiple invalid options in different positions
    { (char*[]){"gcov-tool", "-a", "merge", "-b", "-v", "-c", NULL}, 1, "Invalid options before and after subcommand" },
    
    // Empty string as option (edge case)
    { (char*[]){"gcov-tool", "", NULL}, 1, "Empty string argument" },
    
    // Double dash with invalid option
    { (char*[]){"gcov-tool", "--", "-x", NULL}, 1, "Double dash followed by invalid -x" },
    
    // NULL terminator
    { NULL, 0, NULL }
};

// Valid test cases to ensure basic functionality
test_case_t valid_test_cases[] = {
    { (char*[]){"gcov-tool", "--help", NULL}, 0, "Valid --help option" },
    { (char*[]){"gcov-tool", "-h", NULL}, 0, "Valid -h option (help)" },
    { (char*[]){"gcov-tool", "merge", "--help", NULL}, 0, "Valid merge --help" },
    { (char*[]){"gcov-tool", "overlap", "--help", NULL}, 0, "Valid overlap --help" },
    { NULL, 0, NULL }
};

int run_test_case(test_case_t *test, const char *gcov_tool_path) {
    pid_t pid;
    int status;
    
    printf("Running: %s\n", test->description);
    printf("Command: %s", gcov_tool_path);
    for (int i = 1; test->args[i] != NULL; i++) {
        printf(" %s", test->args[i]);
    }
    printf("\n");
    
    pid = fork();
    if (pid == 0) {
        // Child process
        execvp(gcov_tool_path, test->args);
        // If execvp returns, there was an error
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Exit code: %d (expected: %d)\n", exit_code, test->expected_exit_code);
            
            if ((test->expected_exit_code == 0 && exit_code == 0) ||
                (test->expected_exit_code != 0 && exit_code != 0)) {
                printf("✓ Test passed\n");
                return 1;
            } else {
                printf("✗ Test failed - unexpected exit code\n");
                return 0;
            }
        } else {
            printf("✗ Test failed - child did not exit normally\n");
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

int main(int argc, char *argv[]) {
    const char *gcov_tool_path = "./gcov-tool";
    int passed = 0;
    int total = 0;
    
    printf("========================================\n");
    printf("Testing gcov-tool option parser coverage\n");
    printf("Target: lines 534-554 (switch default case)\n");
    printf("========================================\n\n");
    
    // First, run valid test cases to ensure basic functionality
    printf("=== Running valid test cases ===\n");
    for (int i = 0; valid_test_cases[i].args != NULL; i++) {
        // Replace the first argument with actual gcov-tool path
        valid_test_cases[i].args[0] = (char*)gcov_tool_path;
        if (run_test_case(&valid_test_cases[i], gcov_tool_path)) {
            passed++;
        }
        total++;
        printf("\n");
    }
    
    // Run invalid option test cases targeting the uncovered switch default case
    printf("\n=== Running invalid option test cases ===\n");
    printf("Testing all single-character options NOT in {v, f, F, o, h, t}\n");
    
    for (int i = 0; test_cases[i].args != NULL; i++) {
        // Replace the first argument with actual gcov-tool path
        test_cases[i].args[0] = (char*)gcov_tool_path;
        if (run_test_case(&test_cases[i], gcov_tool_path)) {
            passed++;
        }
        total++;
        printf("\n");
    }
    
    // Additional systematic test: all lowercase letters except v,f,o,h,t
    printf("\n=== Systematic test of all invalid lowercase letters ===\n");
    for (char c = 'a'; c <= 'z'; c++) {
        // Skip valid options: v, f, o, h, t
        if (c == 'v' || c == 'f' || c == 'o' || c == 'h' || c == 't') {
            continue;
        }
        
        char option[3] = "- ";
        option[1] = c;
        
        char description[50];
        snprintf(description, sizeof(description), "Systematic test: invalid option -%c", c);
        
        test_case_t test = {
            .args = { (char*)gcov_tool_path, option, NULL },
            .expected_exit_code = 1,
            .description = description
        };
        
        printf("Testing: %s\n", description);
        if (run_test_case(&test, gcov_tool_path)) {
            passed++;
        }
        total++;
        printf("\n");
    }
    
    // Test uppercase letters (except F)
    printf("\n=== Systematic test of all invalid uppercase letters ===\n");
    for (char c = 'A'; c <= 'Z'; c++) {
        // Skip valid option: F
        if (c == 'F') {
            continue;
        }
        
        char option[3] = "- ";
        option[1] = c;
        
        char description[50];
        snprintf(description, sizeof(description), "Systematic test: invalid option -%c", c);
        
        test_case_t test = {
            .args = { (char*)gcov_tool_path, option, NULL },
            .expected_exit_code = 1,
            .description = description
        };
        
        printf("Testing: %s\n", description);
        if (run_test_case(&test, gcov_tool_path)) {
            passed++;
        }
        total++;
        printf("\n");
    }
    
    printf("========================================\n");
    printf("Summary: %d/%d tests passed\n", passed, total);
    printf("Coverage target: switch default case (overlap_usage())\n");
    printf("========================================\n");
    
    return (passed == total) ? 0 : 1;
}
