#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

/* Create a minimal C program to generate a .gcda file */
const char* test_program = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Test program for coverage\\n\");\n"
"    return 0;\n"
"}\n";

/* Helper function to execute a command and check status */
int execute_command(const char* description, char* const argv[]) {
    printf("\n=== Testing: %s ===\n", description);
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", argv);
        /* If execvp fails */
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
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
        return -1;
    }
}

/* Helper function using system() */
int execute_system(const char* description, const char* command) {
    printf("\n=== Testing (system): %s ===\n", description);
    int status = system(command);
    printf("Exit status: %d\n", status);
    return status;
}

int main() {
    int overall_status = 0;
    
    /* Step 1: Create a dummy .gcda file for testing */
    printf("Creating test files for gcov-dump...\n");
    
    /* Write test program */
    FILE* fp = fopen("test_gcov.c", "w");
    if (!fp) {
        perror("Failed to create test_gcov.c");
        return EXIT_FAILURE;
    }
    fputs(test_program, fp);
    fclose(fp);
    
    /* Compile with coverage flags */
    int compile_status = system("gcc -fprofile-arcs -ftest-coverage -O0 test_gcov.c -o test_gcov");
    if (compile_status != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        /* Continue anyway - some tests don't need .gcda file */
    } else {
        /* Run the program to generate .gcda file */
        system("./test_gcov");
    }
    
    /* Test 1: Individual flags (direct execvp) */
    printf("\n========== Testing Individual Flags ==========\n");
    
    /* Test help flag (case 'h') */
    char* help_args[] = {"gcov-dump", "-h", NULL};
    execute_command("Help flag (-h)", help_args);
    
    /* Test version flag (case 'v') */
    char* version_args[] = {"gcov-dump", "-v", NULL};
    execute_command("Version flag (-v)", version_args);
    
    /* Test contents dump flag (case 'l') */
    char* contents_args[] = {"gcov-dump", "-l", "test_gcov.gcda", NULL};
    execute_command("Contents dump flag (-l)", contents_args);
    
    /* Test positions dump flag (case 'p') */
    char* positions_args[] = {"gcov-dump", "-p", "test_gcov.gcda", NULL};
    execute_command("Positions dump flag (-p)", positions_args);
    
    /* Test raw dump flag (case 'r') */
    char* raw_args[] = {"gcov-dump", "-r", "test_gcov.gcda", NULL};
    execute_command("Raw dump flag (-r)", raw_args);
    
    /* Test stable dump flag (case 's') */
    char* stable_args[] = {"gcov-dump", "-s", "test_gcov.gcda", NULL};
    execute_command("Stable dump flag (-s)", stable_args);
    
    /* Test invalid flag (default case) */
    char* invalid_args[] = {"gcov-dump", "-x", NULL};
    execute_command("Invalid flag (-x) - should trigger default case", invalid_args);
    
    /* Test 2: Combined flags (using system()) */
    printf("\n========== Testing Combined Flags ==========\n");
    
    /* Combined short options (if supported) */
    execute_system("Combined flags (-lp)", "gcov-dump -lp test_gcov.gcda");
    execute_system("Multiple flags (-r -s -v)", "gcov-dump -r -s -v test_gcov.gcda");
    execute_system("Help with other flags (-h -l)", "gcov-dump -h -l test_gcov.gcda");
    
    /* Repeated flag */
    execute_system("Repeated flag (-p -p)", "gcov-dump -p -p test_gcov.gcda");
    
    /* Test 3: Different syntactic styles */
    printf("\n========== Testing Different Syntax Styles ==========\n");
    
    /* Separate arguments */
    char* separate_args[] = {"gcov-dump", "-l", "-p", "test_gcov.gcda", NULL};
    execute_command("Separate arguments (-l -p)", separate_args);
    
    /* Combined short options */
    char* combined_args[] = {"gcov-dump", "-lp", "test_gcov.gcda", NULL};
    execute_command("Combined short options (-lp)", combined_args);
    
    /* With -- delimiter */
    char* delimiter_args[] = {"gcov-dump", "-l", "--", "test_gcov.gcda", NULL};
    execute_command("With -- delimiter", delimiter_args);
    
    /* Test 4: Environment and error contexts */
    printf("\n========== Testing Environment/Error Contexts ==========\n");
    
    /* No arguments */
    char* no_args[] = {"gcov-dump", NULL};
    execute_command("No arguments", no_args);
    
    /* Set environment variable (if supported) */
    execute_system("With GCOV_DUMP_OPTIONS env var", 
                   "GCOV_DUMP_OPTIONS='-l' gcov-dump test_gcov.gcda");
    
    /* Test with output redirection to capture stderr */
    execute_system("Invalid flag with stderr redirection", 
                   "gcov-dump -x 2>&1");
    
    /* Test with non-existent file */
    char* bad_file_args[] = {"gcov-dump", "-l", "nonexistent.gcda", NULL};
    execute_command("Non-existent file", bad_file_args);
    
    /* Test 5: Additional edge cases */
    printf("\n========== Testing Edge Cases ==========\n");
    
    /* Empty string as argument */
    char* empty_string_args[] = {"gcov-dump", "", NULL};
    execute_command("Empty string argument", empty_string_args);
    
    /* Multiple invalid flags */
    char* multi_invalid_args[] = {"gcov-dump", "-xyz", NULL};
    execute_command("Multiple invalid flags (-xyz)", multi_invalid_args);
    
    /* Flag at end */
    char* flag_end_args[] = {"gcov-dump", "test_gcov.gcda", "-l", NULL};
    execute_command("Flag at end", flag_end_args);
    
    /* Cleanup */
    printf("\nCleaning up test files...\n");
    system("rm -f test_gcov test_gcov.c test_gcov.gcda test_gcov.gcno");
    
    printf("\n========== All tests completed ==========\n");
    return overall_status;
}
