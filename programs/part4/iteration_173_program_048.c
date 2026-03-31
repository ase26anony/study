#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_ARGS 20
#define MAX_TEST_CASES 50

typedef struct {
    char *args[MAX_ARGS];
    int expected_exit_code;
    char *description;
} test_case_t;

// Test cases designed to trigger the uncovered default case
test_case_t test_cases[] = {
    // Single invalid options (not in v,f,F,o,h,t)
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
    
    // Non-alphabetic invalid options
    { {"gcov-tool", "-@", NULL}, 1, "Non-alphabetic invalid option -@" },
    { {"gcov-tool", "-1", NULL}, 1, "Numeric invalid option -1" },
    { {"gcov-tool", "-2", NULL}, 1, "Numeric invalid option -2" },
    { {"gcov-tool", "-!", NULL}, 1, "Special character invalid option -!" },
    { {"gcov-tool", "-#", NULL}, 1, "Special character invalid option -#" },
    { {"gcov-tool", "-$", NULL}, 1, "Special character invalid option -$" },
    { {"gcov-tool", "-%", NULL}, 1, "Special character invalid option -%" },
    { {"gcov-tool", "-&", NULL}, 1, "Special character invalid option -&" },
    { {"gcov-tool", "-*", NULL}, 1, "Special character invalid option -*" },
    { {"gcov-tool", "-(", NULL}, 1, "Special character invalid option -(" },
    { {"gcov-tool", "-)", NULL}, 1, "Special character invalid option -)" },
    { {"gcov-tool", "-_", NULL}, 1, "Special character invalid option -_" },
    { {"gcov-tool", "-=", NULL}, 1, "Special character invalid option -=" },
    { {"gcov-tool", "-+", NULL}, 1, "Special character invalid option -+" },
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
    
    // Edge cases for option parsing
    { {"gcov-tool", "-", NULL}, 1, "Single dash only" },
    { {"gcov-tool", "--", NULL}, 1, "Double dash only" },
    { {"gcov-tool", "---", NULL}, 1, "Triple dash" },
    { {"gcov-tool", "--invalid-option", NULL}, 1, "Long invalid option" },
    { {"gcov-tool", "--unknown", NULL}, 1, "Long unknown option" },
    
    // Combinations of valid and invalid options
    { {"gcov-tool", "-v", "-x", NULL}, 1, "Valid -v followed by invalid -x" },
    { {"gcov-tool", "-f", "-z", NULL}, 1, "Valid -f followed by invalid -z" },
    { {"gcov-tool", "-F", "-@", NULL}, 1, "Valid -F followed by invalid -@" },
    { {"gcov-tool", "-o", "-1", NULL}, 1, "Valid -o followed by invalid -1" },
    { {"gcov-tool", "-h", "-!", NULL}, 1, "Valid -h followed by invalid -!" },
    { {"gcov-tool", "-t", "0.5", "-a", NULL}, 1, "Valid -t with arg followed by invalid -a" },
    { {"gcov-tool", "-v", "-f", "-F", "-o", "-h", "-t", "1.0", "-z", NULL}, 1, "All valid options followed by invalid -z" },
    
    // Multiple invalid options chained together
    { {"gcov-tool", "-abc", NULL}, 1, "Multiple invalid options chained -abc" },
    { {"gcov-tool", "-xyz", NULL}, 1, "Multiple invalid options chained -xyz" },
    { {"gcov-tool", "-@#$", NULL}, 1, "Multiple special chars chained -@#$" },
    { {"gcov-tool", "-aBcD", NULL}, 1, "Mixed case invalid options -aBcD" },
    
    // Invalid options with merge command
    { {"gcov-tool", "merge", "-o", "output.gcda", "-z", NULL}, 1, "Invalid option -z with merge command" },
    { {"gcov-tool", "merge", "-v", "-x", "file1.gcda", "file2.gcda", NULL}, 1, "Invalid -x in merge with files" },
    { {"gcov-tool", "merge", "-t", "0.75", "-@", "file1.gcda", NULL}, 1, "Valid -t then invalid -@ in merge" },
    
    // Valid test cases to ensure basic functionality
    { {"gcov-tool", "--help", NULL}, 0, "Valid: --help" },
    { {"gcov-tool", "-v", NULL}, 0, "Valid: -v" },
    { {"gcov-tool", "-f", NULL}, 0, "Valid: -f" },
    { {"gcov-tool", "-F", NULL}, 0, "Valid: -F" },
    { {"gcov-tool", "-o", NULL}, 0, "Valid: -o" },
    { {"gcov-tool", "-h", NULL}, 0, "Valid: -h" },
    { {"gcov-tool", "-t", "0.5", NULL}, 0, "Valid: -t 0.5" },
    { {"gcov-tool", "-t", "1.0e2", NULL}, 0, "Valid: -t 1.0e2" },
    { {"gcov-tool", "-t", "100", NULL}, 0, "Valid: -t 100" },
    { {"gcov-tool", "-t", "0.001", NULL}, 0, "Valid: -t 0.001" },
    { {"gcov-tool", "-t", "1e-3", NULL}, 0, "Valid: -t 1e-3" },
    
    // End marker
    { {NULL}, 0, NULL }
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
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("========================================\n");
    printf("Testing gcov-tool invalid option handling\n");
    printf("Target: Trigger default case in switch statement\n");
    printf("========================================\n\n");
    
    // First, compile gcov-tool with coverage instrumentation
    printf("Compiling gcov-tool with coverage instrumentation...\n");
    system("g++ -O0 -g -fprofile-arcs -ftest-coverage -lgcov gcov-tool.cc -o gcov-tool 2>/dev/null");
    
    // Alternative compilation options (uncomment as needed)
    // system("g++ -O0 -g -lgcov --coverage gcov-tool.cc -o gcov-tool 2>/dev/null");
    // system("g++ -O2 -fsanitize=address -fno-omit-frame-pointer -lgcov gcov-tool.cc -o gcov-tool 2>/dev/null");
    // system("g++ -O3 -fprofile-arcs -ftest-coverage -lgcov gcov-tool.cc -o gcov-tool 2>/dev/null");
    
    printf("Running test cases...\n\n");
    
    // Run all test cases
    for (int i = 0; test_cases[i].description != NULL; i++) {
        total_tests++;
        run_test_case(&test_cases[i]);
        
        // Simple check - if exit code matches expected, count as passed
        // In a real test, we'd capture stderr to verify usage was printed
        pid_t pid;
        int status;
        int pipefd[2];
        
        // Create a pipe to capture stderr
        if (pipe(pipefd) == -1) {
            perror("pipe");
            continue;
        }
        
        pid = fork();
        if (pid == 0) {
            // Child process
            close(pipefd[0]); // Close read end
            dup2(pipefd[1], STDERR_FILENO); // Redirect stderr to pipe
            close(pipefd[1]);
            
            execvp("gcov-tool", test_cases[i].args);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            // Parent process
            close(pipefd[1]); // Close write end
            
            char buffer[4096];
            ssize_t bytes_read;
            int usage_printed = 0;
            
            // Read stderr output
            while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer)-1)) > 0) {
                buffer[bytes_read] = '\0';
                // Check if usage message was printed
                if (strstr(buffer, "usage") != NULL || 
                    strstr(buffer, "Usage") != NULL ||
                    strstr(buffer, "option") != NULL) {
                    usage_printed = 1;
                }
            }
            
            close(pipefd[0]);
            waitpid(pid, &status, 0);
            
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                if (exit_code == test_cases[i].expected_exit_code) {
                    // For invalid options, we expect usage to be printed
                    if (test_cases[i].expected_exit_code == 1 && usage_printed) {
                        printf("✓ Usage message printed as expected\n");
                        passed_tests++;
                    } else if (test_cases[i].expected_exit_code == 0) {
                        // Valid options shouldn't print usage
                        passed_tests++;
                    }
                }
            }
        }
    }
    
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    printf("========================================\n\n");
    
    // Generate coverage report
    printf("Generating coverage report...\n");
    system("gcov gcov-tool.cc 2>/dev/null");
    
    // Check if our target lines are covered
    printf("\nChecking coverage of target lines (534-554)...\n");
    system("grep -n -A 20 -B 5 \"case 'v':\" gcov-tool.cc.gcov 2>/dev/null | head -40");
    
    return 0;
}
