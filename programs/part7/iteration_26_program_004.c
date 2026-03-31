#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Create a minimal C program to generate a .gcda file */
const char *test_program = 
    "#include <stdio.h>\n"
    "int main() {\n"
    "    printf(\"Test program for coverage\\n\");\n"
    "    return 0;\n"
    "}\n";

/* Helper function to execute a command and wait for completion */
int execute_command(const char *description, char *const argv[]) {
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

/* Helper function using system() call */
int execute_system(const char *description, const char *command) {
    printf("\n=== Testing (system): %s ===\n", description);
    int status = system(command);
    printf("Exit status: %d\n", status);
    return status;
}

int main() {
    int ret;
    
    /* First, create a dummy .gcda file for testing */
    printf("Creating test files for gcov-dump...\n");
    
    /* Write test C program */
    FILE *fp = fopen("test_gcov.c", "w");
    if (!fp) {
        perror("Failed to create test_gcov.c");
        return 1;
    }
    fprintf(fp, "%s", test_program);
    fclose(fp);
    
    /* Compile with coverage flags */
    ret = system("gcc -fprofile-arcs -ftest-coverage -o test_gcov test_gcov.c 2>/dev/null");
    if (ret != 0) {
        fprintf(stderr, "Warning: Failed to compile test program with coverage flags\n");
        fprintf(stderr, "Creating empty dummy.gcda file instead\n");
        /* Create empty dummy file */
        fp = fopen("dummy.gcda", "w");
        if (fp) fclose(fp);
    } else {
        /* Run the program to generate .gcda file */
        system("./test_gcov 2>/dev/null");
        /* Copy the generated file */
        system("cp test_gcov.gcda dummy.gcda 2>/dev/null");
    }
    
    /* Test 1: Individual flags (using execvp for precise control) */
    printf("\n========== Testing Individual Flags ==========\n");
    
    /* Help flag - case 'h' */
    char *help_args[] = {"gcov-dump", "-h", NULL};
    execute_command("Help flag (-h)", help_args);
    
    /* Version flag - case 'v' */
    char *version_args[] = {"gcov-dump", "-v", NULL};
    execute_command("Version flag (-v)", version_args);
    
    /* Contents dump flag - case 'l' */
    char *contents_args[] = {"gcov-dump", "-l", NULL};
    execute_command("Contents dump flag (-l)", contents_args);
    
    /* Positions dump flag - case 'p' */
    char *positions_args[] = {"gcov-dump", "-p", NULL};
    execute_command("Positions dump flag (-p)", positions_args);
    
    /* Raw dump flag - case 'r' */
    char *raw_args[] = {"gcov-dump", "-r", NULL};
    execute_command("Raw dump flag (-r)", raw_args);
    
    /* Stable dump flag - case 's' */
    char *stable_args[] = {"gcov-dump", "-s", NULL};
    execute_command("Stable dump flag (-s)", stable_args);
    
    /* Invalid flag - default case */
    char *invalid_args[] = {"gcov-dump", "-x", NULL};
    execute_command("Invalid flag (-x) - should trigger 'unknown flag'", invalid_args);
    
    /* Test 2: No arguments */
    printf("\n========== Testing No Arguments ==========\n");
    char *no_args[] = {"gcov-dump", NULL};
    execute_command("No arguments", no_args);
    
    /* Test 3: Flag combinations (using execvp) */
    printf("\n========== Testing Flag Combinations ==========\n");
    
    /* Combination 1: -l -p */
    char *combo1_args[] = {"gcov-dump", "-l", "-p", NULL};
    execute_command("Combination: -l -p", combo1_args);
    
    /* Combination 2: -r -s -v */
    char *combo2_args[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    execute_command("Combination: -r -s -v", combo2_args);
    
    /* Combination 3: -h -l (help might cause early exit) */
    char *combo3_args[] = {"gcov-dump", "-h", "-l", NULL};
    execute_command("Combination: -h -l (help with other flags)", combo3_args);
    
    /* Test 4: Repeated flags */
    printf("\n========== Testing Repeated Flags ==========\n");
    char *repeat_args[] = {"gcov-dump", "-p", "-p", NULL};
    execute_command("Repeated flag: -p -p", repeat_args);
    
    /* Test 5: Different syntactic styles (using system() for shell interpretation) */
    printf("\n========== Testing Different Syntactic Styles ==========\n");
    
    /* Combined short options */
    execute_system("Combined short options: -lp", "gcov-dump -lp 2>&1");
    
    /* With positional argument */
    execute_system("With positional argument: -l dummy.gcda", "gcov-dump -l dummy.gcda 2>&1");
    
    /* With -- delimiter */
    execute_system("With -- delimiter: -l -- dummy.gcda", "gcov-dump -l -- dummy.gcda 2>&1");
    
    /* Combined flags with file */
    execute_system("Combined flags with file: -lp dummy.gcda", "gcov-dump -lp dummy.gcda 2>&1");
    
    /* Test 6: Environment variable testing */
    printf("\n========== Testing with Environment Variables ==========\n");
    
    /* Test with GCOV_DUMP_OPTIONS if supported */
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    char *env_args[] = {"gcov-dump", NULL};
    execute_command("With GCOV_DUMP_OPTIONS=-v", env_args);
    unsetenv("GCOV_DUMP_OPTIONS");
    
    /* Test 7: Edge cases with system() */
    printf("\n========== Testing Edge Cases ==========\n");
    
    /* Empty string as argument */
    execute_system("Empty string argument", "gcov-dump '' 2>&1");
    
    /* Multiple invalid flags */
    execute_system("Multiple invalid flags", "gcov-dump -x -y -z 2>&1");
    
    /* Mix valid and invalid */
    execute_system("Mix valid and invalid flags", "gcov-dump -l -x -p 2>&1");
    
    /* Long argument (if supported) */
    execute_system("Long argument test", "gcov-dump --help 2>&1");
    
    /* Test 8: Output redirection tests */
    printf("\n========== Testing Output Redirection ==========\n");
    
    /* Redirect stderr to capture unknown flag message */
    execute_system("Redirect stderr for invalid flag", "gcov-dump -x 2>&1 | head -5");
    
    /* Redirect both stdout and stderr */
    execute_system("Redirect both stdout and stderr", "gcov-dump -v > /dev/null 2>&1; echo 'Exit: $?'");
    
    /* Cleanup */
    printf("\n========== Cleaning up ==========\n");
    system("rm -f test_gcov test_gcov.c test_gcov.gcda test_gcov.gcno dummy.gcda 2>/dev/null");
    
    printf("\nAll tests completed!\n");
    return 0;
}
