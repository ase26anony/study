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
int execute_command(const char *description, char *const args[]) {
    printf("\n=== Testing: %s ===\n", description);
    printf("Command: ");
    for (int i = 0; args[i] != NULL; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
    
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
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
        return -1;
    }
}

/* Helper function using system() call */
int execute_system(const char *description, const char *cmd) {
    printf("\n=== Testing (system): %s ===\n", description);
    printf("Command: %s\n", cmd);
    int status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(status));
    return WEXITSTATUS(status);
}

int main() {
    int ret;
    
    printf("=== Creating dummy .gcda file for testing ===\n");
    
    /* Step 1: Create test source file */
    FILE *fp = fopen("test_coverage.c", "w");
    if (!fp) {
        perror("Failed to create test_coverage.c");
        return 1;
    }
    fprintf(fp, "%s", test_program);
    fclose(fp);
    
    /* Step 2: Compile with coverage flags */
    ret = system("gcc -fprofile-arcs -ftest-coverage -O0 test_coverage.c -o test_coverage");
    if (ret != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return 1;
    }
    
    /* Step 3: Run the program to generate .gcda file */
    ret = system("./test_coverage");
    if (ret != 0) {
        fprintf(stderr, "Failed to run test program\n");
        return 1;
    }
    
    /* Now test gcov-dump with various flag combinations */
    
    /* Individual flag tests using execvp */
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
    
    /* Invalid flag to trigger default case */
    char *invalid_args[] = {"gcov-dump", "-x", NULL};
    execute_command("Invalid flag (-x) to trigger default case", invalid_args);
    
    /* Combination of valid flags */
    char *combo1_args[] = {"gcov-dump", "-l", "-p", NULL};
    execute_command("Combination: -l -p", combo1_args);
    
    char *combo2_args[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    execute_command("Combination: -r -s -v", combo2_args);
    
    /* Same flag repeated */
    char *repeat_args[] = {"gcov-dump", "-p", "-p", NULL};
    execute_command("Repeated flag: -p -p", repeat_args);
    
    /* Combined short options (if supported) */
    char *combined_args[] = {"gcov-dump", "-lp", NULL};
    execute_command("Combined short options: -lp", combined_args);
    
    /* Tests using system() calls */
    execute_system("No arguments", "gcov-dump");
    
    execute_system("With positional argument", "gcov-dump -l test_coverage.gcda");
    
    execute_system("With -- delimiter", "gcov-dump -l -- test_coverage.gcda");
    
    execute_system("Help with other flags (may exit early)", "gcov-dump -h -l");
    
    /* Test with environment variable */
    execute_system("With GCOV_DUMP_OPTIONS env var", 
                   "GCOV_DUMP_OPTIONS='-v' gcov-dump -l test_coverage.gcda");
    
    /* Test error output redirection */
    printf("\n=== Testing stderr output for invalid flag ===\n");
    system("gcov-dump -x 2>&1 | grep 'unknown flag'");
    
    /* Test with multiple positional arguments */
    char *multi_file_args[] = {"gcov-dump", "-l", "test_coverage.gcda", "test_coverage.gcno", NULL};
    execute_command("Multiple file arguments", multi_file_args);
    
    /* Test flag ordering variations */
    char *order1_args[] = {"gcov-dump", "test_coverage.gcda", "-l", NULL};
    execute_command("Flag after file argument", order1_args);
    
    char *order2_args[] = {"gcov-dump", "-l", "--", "test_coverage.gcda", NULL};
    execute_command("With -- separator", order2_args);
    
    /* Cleanup */
    printf("\n=== Cleaning up test files ===\n");
    system("rm -f test_coverage.c test_coverage test_coverage.gcda test_coverage.gcno test_coverage.c.gcov");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
