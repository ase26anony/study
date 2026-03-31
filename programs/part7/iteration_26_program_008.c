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
"    printf(\"Test program for gcov-dump\\n\");\n"
"    return 0;\n"
"}\n";

/* Helper function to execute a command and wait for completion */
int execute_command(const char *description, char *const args[]) {
    printf("\n=== Testing: %s ===\n", description);
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execvp(args[0], args);
        /* If execvp returns, there was an error */
        fprintf(stderr, "Failed to execute %s: %s\n", args[0], strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        printf("Exit status: %d\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    } else {
        /* Fork failed */
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
        return -1;
    }
}

/* Helper function using system() call */
int execute_system(const char *description, const char *command) {
    printf("\n=== Testing (system): %s ===\n", description);
    int status = system(command);
    printf("Exit status: %d\n", WEXITSTATUS(status));
    return WEXITSTATUS(status);
}

int main(int argc, char *argv[]) {
    printf("=== Starting gcov-dump flag parsing tests ===\n");
    
    /* First, create a dummy .gcda file for testing */
    printf("\n=== Creating test .gcda file ===\n");
    
    /* Write test program */
    FILE *fp = fopen("test_gcov.c", "w");
    if (!fp) {
        perror("Failed to create test_gcov.c");
        return 1;
    }
    fputs(test_program, fp);
    fclose(fp);
    
    /* Compile with coverage flags */
    char compile_cmd[256];
    snprintf(compile_cmd, sizeof(compile_cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 test_gcov.c -o test_gcov");
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return 1;
    }
    
    /* Run the program to generate .gcda file */
    if (system("./test_gcov") != 0) {
        fprintf(stderr, "Failed to run test program\n");
        return 1;
    }
    
    /* Test 1: Individual flags (using execvp) */
    printf("\n=== Testing individual flags ===\n");
    
    /* Help flag */
    char *help_args[] = {"gcov-dump", "-h", NULL};
    execute_command("Help flag (-h)", help_args);
    
    /* Version flag */
    char *version_args[] = {"gcov-dump", "-v", NULL};
    execute_command("Version flag (-v)", version_args);
    
    /* Contents dump flag */
    char *contents_args[] = {"gcov-dump", "-l", NULL};
    execute_command("Contents dump flag (-l)", contents_args);
    
    /* Positions dump flag */
    char *positions_args[] = {"gcov-dump", "-p", NULL};
    execute_command("Positions dump flag (-p)", positions_args);
    
    /* Raw dump flag */
    char *raw_args[] = {"gcov-dump", "-r", NULL};
    execute_command("Raw dump flag (-r)", raw_args);
    
    /* Stable dump flag */
    char *stable_args[] = {"gcov-dump", "-s", NULL};
    execute_command("Stable dump flag (-s)", stable_args);
    
    /* Invalid flag (to trigger default case) */
    char *invalid_args[] = {"gcov-dump", "-x", NULL};
    execute_command("Invalid flag (-x)", invalid_args);
    
    /* Test 2: No arguments */
    char *no_args[] = {"gcov-dump", NULL};
    execute_command("No arguments", no_args);
    
    /* Test 3: Combination of flags (using execvp) */
    printf("\n=== Testing flag combinations ===\n");
    
    /* Two valid flags */
    char *combo1_args[] = {"gcov-dump", "-l", "-p", NULL};
    execute_command("Combination -l -p", combo1_args);
    
    /* Three valid flags */
    char *combo2_args[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    execute_command("Combination -r -s -v", combo2_args);
    
    /* Help with other flags (may exit early) */
    char *combo3_args[] = {"gcov-dump", "-h", "-l", NULL};
    execute_command("Combination -h -l", combo3_args);
    
    /* Repeated flag */
    char *combo4_args[] = {"gcov-dump", "-p", "-p", NULL};
    execute_command("Repeated flag -p -p", combo4_args);
    
    /* Test 4: Combined short options syntax */
    printf("\n=== Testing combined short options ===\n");
    
    char *combined_args[] = {"gcov-dump", "-lp", NULL};
    execute_command("Combined flags -lp", combined_args);
    
    char *combined2_args[] = {"gcov-dump", "-rs", NULL};
    execute_command("Combined flags -rs", combined2_args);
    
    /* Test 5: Flags with positional arguments (using system) */
    printf("\n=== Testing with positional arguments ===\n");
    
    execute_system("Flag with .gcda file", "gcov-dump -l test_gcov.gcda");
    execute_system("Multiple flags with file", "gcov-dump -l -p test_gcov.gcda");
    
    /* Test 6: Using -- delimiter */
    printf("\n=== Testing with -- delimiter ===\n");
    
    char *delim_args[] = {"gcov-dump", "-l", "--", "test_gcov.gcda", NULL};
    execute_command("Flag with -- delimiter", delim_args);
    
    /* Test 7: Environment variable simulation (using system) */
    printf("\n=== Testing with environment variables ===\n");
    
    /* Set environment variable before execution */
    execute_system("With GCOV_DUMP_OPTIONS env var", 
                   "GCOV_DUMP_OPTIONS='-v' gcov-dump -l test_gcov.gcda");
    
    /* Test 8: Error stream redirection */
    printf("\n=== Testing error stream capture ===\n");
    
    /* Capture stderr for invalid flag */
    execute_system("Invalid flag with stderr redirect", 
                   "gcov-dump -x 2>&1");
    
    /* Test 9: Various flag orderings */
    printf("\n=== Testing flag ordering variations ===\n");
    
    char *order1_args[] = {"gcov-dump", "test_gcov.gcda", "-l", NULL};
    execute_command("File before flag", order1_args);
    
    char *order2_args[] = {"gcov-dump", "-l", "test_gcov.gcda", "-p", NULL};
    execute_command("Mixed flags and file", order2_args);
    
    /* Test 10: Edge cases with system() */
    printf("\n=== Testing edge cases with system() ===\n");
    
    /* Empty string */
    execute_system("Empty command string", "gcov-dump ''");
    
    /* Only spaces */
    execute_system("Command with only spaces", "gcov-dump ' '");
    
    /* Clean up */
    printf("\n=== Cleaning up test files ===\n");
    system("rm -f test_gcov.c test_gcov test_gcov.gcda test_gcov.gcno");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
