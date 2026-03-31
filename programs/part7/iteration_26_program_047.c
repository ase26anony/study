#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Create a minimal C program to generate a .gcda file */
const char* test_program = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Test program for gcov\\n\");\n"
"    return 0;\n"
"}\n";

/* Helper function to execute a command and wait for completion */
int execute_command(const char *description, char *const args[]) {
    printf("\n=== Testing: %s ===\n", description);
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", args);
        /* If execvp returns, there was an error */
        fprintf(stderr, "Failed to execute gcov-dump: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Exit status: %d\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else {
            printf("Process terminated abnormally\n");
            return -1;
        }
    } else {
        /* Fork failed */
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
        return -1;
    }
}

/* Helper function to execute via system() */
int execute_system(const char *description, const char *command) {
    printf("\n=== Testing (system): %s ===\n", description);
    int result = system(command);
    printf("System return: %d\n", result);
    return result;
}

int main() {
    int i;
    
    /* First, create a dummy .gcda file for testing */
    printf("Creating test files for gcov-dump...\n");
    
    /* Write test program */
    FILE *fp = fopen("test_gcov.c", "w");
    if (!fp) {
        perror("Failed to create test_gcov.c");
        return 1;
    }
    fputs(test_program, fp);
    fclose(fp);
    
    /* Compile with coverage flags */
    printf("Compiling test program with coverage...\n");
    int compile_result = system("gcc -fprofile-arcs -ftest-coverage -o test_gcov test_gcov.c");
    if (compile_result != 0) {
        fprintf(stderr, "Compilation failed\n");
        return 1;
    }
    
    /* Run the program to generate .gcda file */
    printf("Running test program to generate .gcda file...\n");
    system("./test_gcov > /dev/null 2>&1");
    
    /* Test 1: Individual flags (using execvp for precise control) */
    printf("\n" "="*50 "\n");
    printf("Testing individual flags with execvp:\n");
    
    /* Array of test cases: description followed by arguments */
    struct {
        const char *description;
        char *args[5];
    } test_cases[] = {
        /* Individual flag tests */
        {"Help flag", {"gcov-dump", "-h", NULL}},
        {"Version flag", {"gcov-dump", "-v", NULL}},
        {"Contents dump flag", {"gcov-dump", "-l", NULL}},
        {"Positions dump flag", {"gcov-dump", "-p", NULL}},
        {"Raw dump flag", {"gcov-dump", "-r", NULL}},
        {"Stable dump flag", {"gcov-dump", "-s", NULL}},
        {"Invalid flag (should trigger default case)", {"gcov-dump", "-x", NULL}},
        
        /* Combination tests */
        {"Combined flags: -l -p", {"gcov-dump", "-l", "-p", NULL}},
        {"Combined flags: -r -s -v", {"gcov-dump", "-r", "-s", "-v", NULL}},
        {"Same flag repeated: -p -p", {"gcov-dump", "-p", "-p", NULL}},
        {"Help with other flags: -h -l", {"gcov-dump", "-h", "-l", NULL}},
        
        /* With file arguments */
        {"Flag with .gcda file: -l test_gcov.gcda", {"gcov-dump", "-l", "test_gcov.gcda", NULL}},
        {"Multiple flags with file: -l -p test_gcov.gcda", {"gcov-dump", "-l", "-p", "test_gcov.gcda", NULL}},
        
        /* With -- delimiter */
        {"With -- delimiter: -l -- test_gcov.gcda", {"gcov-dump", "-l", "--", "test_gcov.gcda", NULL}},
        
        /* No arguments */
        {"No arguments", {"gcov-dump", NULL}},
        
        /* End marker */
        {NULL, {NULL}}
    };
    
    /* Execute all test cases using execvp */
    for (i = 0; test_cases[i].description != NULL; i++) {
        execute_command(test_cases[i].description, test_cases[i].args);
    }
    
    /* Test 2: Combined short options and system() calls */
    printf("\n" "="*50 "\n");
    printf("Testing combined flags and system() calls:\n");
    
    /* Test combined short options (if supported by getopt) */
    execute_system("Combined short options: -lp", "gcov-dump -lp 2>&1");
    execute_system("Combined short options: -lps", "gcov-dump -lps 2>&1");
    execute_system("Invalid combined: -lx", "gcov-dump -lx 2>&1");
    
    /* Test with environment variables */
    printf("\n" "="*50 "\n");
    printf("Testing with environment variables:\n");
    
    /* Set environment variable before execution */
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    execute_system("With GCOV_DUMP_OPTIONS=-v", "gcov-dump -l 2>&1");
    unsetenv("GCOV_DUMP_OPTIONS");
    
    /* Test output redirection */
    printf("\n" "="*50 "\n");
    printf("Testing output redirection:\n");
    
    execute_system("Redirect stderr for invalid flag", "gcov-dump -x 2>&1 | head -5");
    execute_system("Redirect both stdout and stderr", "gcov-dump -v > version_output.txt 2>&1 && cat version_output.txt");
    
    /* Test 3: Edge cases with system() */
    printf("\n" "="*50 "\n");
    printf("Testing edge cases:\n");
    
    /* Empty string as argument */
    execute_system("Empty string argument", "gcov-dump \"\" 2>&1");
    
    /* Very long argument (should be rejected by shell/exec) */
    execute_system("Long flag string", "gcov-dump -llllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll 2>&1");
    
    /* Multiple invalid flags */
    execute_system("Multiple invalid flags", "gcov-dump -x -y -z 2>&1");
    
    /* Mix valid and invalid */
    execute_system("Mix valid and invalid flags", "gcov-dump -l -x -p 2>&1");
    
    /* Test with non-existent file */
    execute_system("Flag with non-existent file", "gcov-dump -l nonexistent.gcda 2>&1");
    
    /* Cleanup */
    printf("\n" "="*50 "\n");
    printf("Cleaning up test files...\n");
    system("rm -f test_gcov.c test_gcov test_gcov.gcda test_gcov.gcno version_output.txt");
    
    printf("\nAll tests completed!\n");
    return 0;
}
