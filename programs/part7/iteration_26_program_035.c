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
"    printf(\"Test program for coverage\\n\");\n"
"    return 0;\n"
"}\n";

/* Execute a command and wait for completion */
int execute_command(const char *description, char *const argv[]) {
    printf("\n=== Testing: %s ===\n", description);
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execvp(argv[0], argv);
        /* If execvp returns, there was an error */
        fprintf(stderr, "Failed to execute %s: %s\n", argv[0], strerror(errno));
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

/* Execute using system() call */
int execute_system(const char *description, const char *command) {
    printf("\n=== Testing (system): %s ===\n", description);
    int status = system(command);
    printf("Exit status: %d\n", status);
    return status;
}

int main() {
    int result;
    
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
    result = system("gcc -fprofile-arcs -ftest-coverage -O0 test_gcov.c -o test_gcov");
    if (result != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        /* Continue anyway - some tests don't need the .gcda file */
    } else {
        /* Run the program to generate .gcda file */
        system("./test_gcov");
    }
    
    /* Test cases using execvp for precise argument control */
    
    /* 1. Individual flag tests (targeting specific switch cases) */
    char *help_args[] = {"gcov-dump", "-h", NULL};
    execute_command("Help flag (-h)", help_args);
    
    char *version_args[] = {"gcov-dump", "-v", NULL};
    execute_command("Version flag (-v)", version_args);
    
    char *contents_args[] = {"gcov-dump", "-l", NULL};
    execute_command("Contents dump flag (-l)", contents_args);
    
    char *positions_args[] = {"gcov-dump", "-p", NULL};
    execute_command("Positions dump flag (-p)", positions_args);
    
    char *raw_args[] = {"gcov-dump", "-r", NULL};
    execute_command("Raw dump flag (-r)", raw_args);
    
    char *stable_args[] = {"gcov-dump", "-s", NULL};
    execute_command("Stable dump flag (-s)", stable_args);
    
    /* 2. Invalid flag (to trigger default case) */
    char *invalid_args[] = {"gcov-dump", "-x", NULL};
    execute_command("Invalid flag (-x) - should print 'unknown flag'", invalid_args);
    
    /* 3. Combination of valid flags */
    char *combo1_args[] = {"gcov-dump", "-l", "-p", NULL};
    execute_command("Combination -l -p", combo1_args);
    
    char *combo2_args[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    execute_command("Combination -r -s -v", combo2_args);
    
    char *combo3_args[] = {"gcov-dump", "-h", "-l", NULL};
    execute_command("Combination -h -l (h may cause early exit)", combo3_args);
    
    /* 4. Repeated same flag */
    char *repeat_args[] = {"gcov-dump", "-p", "-p", NULL};
    execute_command("Repeated flag -p -p", repeat_args);
    
    /* 5. Tests with positional arguments (gcov files) */
    char *with_file_args[] = {"gcov-dump", "-l", "test_gcov.gcda", NULL};
    execute_command("With file argument -l test_gcov.gcda", with_file_args);
    
    char *with_file_combo[] = {"gcov-dump", "-l", "-p", "test_gcov.gcda", NULL};
    execute_command("With file and flags -l -p test_gcov.gcda", with_file_combo);
    
    /* 6. Tests using system() call for different syntactic styles */
    execute_system("Combined short options -lp", "gcov-dump -lp");
    execute_system("Combined short options -lps", "gcov-dump -lps");
    
    /* 7. Tests with -- delimiter */
    char *delimiter_args[] = {"gcov-dump", "-l", "--", "test_gcov.gcda", NULL};
    execute_command("With -- delimiter", delimiter_args);
    
    /* 8. No arguments */
    char *no_args[] = {"gcov-dump", NULL};
    execute_command("No arguments", no_args);
    
    /* 9. Environment variable tests */
    printf("\n=== Testing with environment variables ===\n");
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    char *env_args[] = {"gcov-dump", "-l", NULL};
    execute_command("With GCOV_DUMP_OPTIONS=-v", env_args);
    unsetenv("GCOV_DUMP_OPTIONS");
    
    /* 10. Test with output redirection */
    execute_system("With stderr redirection for invalid flag", 
                   "gcov-dump -x 2>&1");
    execute_system("With stdout and stderr redirection", 
                   "gcov-dump -v > output.txt 2>&1");
    
    /* 11. Edge cases */
    char *empty_flag_args[] = {"gcov-dump", "-", NULL};
    execute_command("Single dash (-)", empty_flag_args);
    
    char *double_dash_args[] = {"gcov-dump", "--", NULL};
    execute_command("Double dash only", double_dash_args);
    
    /* 12. Multiple files with flags */
    char *multi_file_args[] = {"gcov-dump", "-l", "-p", "test_gcov.gcda", "test_gcov.gcno", NULL};
    execute_command("Multiple files with flags", multi_file_args);
    
    /* Cleanup */
    printf("\n=== Cleaning up test files ===\n");
    remove("test_gcov.c");
    remove("test_gcov");
    remove("test_gcov.gcda");
    remove("test_gcov.gcno");
    remove("output.txt");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
