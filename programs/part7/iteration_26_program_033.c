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

/* Function to create a dummy .gcda file for testing */
int create_dummy_gcda(void) {
    FILE *fp;
    int status;
    pid_t pid;
    
    /* Write test program */
    fp = fopen("test_gcov.c", "w");
    if (!fp) {
        perror("Failed to create test_gcov.c");
        return -1;
    }
    fprintf(fp, "%s", test_program);
    fclose(fp);
    
    /* Compile with coverage flags */
    pid = fork();
    if (pid == 0) {
        /* Child process: compile */
        execlp("gcc", "gcc", "-fprofile-arcs", "-ftest-coverage", 
               "test_gcov.c", "-o", "test_gcov", NULL);
        perror("exec gcc failed");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Compilation failed\n");
            return -1;
        }
    } else {
        perror("fork failed");
        return -1;
    }
    
    /* Run the program to generate .gcda */
    pid = fork();
    if (pid == 0) {
        execl("./test_gcov", "./test_gcov", NULL);
        perror("exec test_gcov failed");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
    }
    
    return 0;
}

/* Execute gcov-dump with given arguments using execvp */
void test_with_execvp(const char *description, char *const args[]) {
    pid_t pid;
    int status;
    
    printf("\n=== Testing: %s ===\n", description);
    printf("Command: ");
    for (int i = 0; args[i] != NULL; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
    
    pid = fork();
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", args);
        /* If execvp returns, it failed */
        fprintf(stderr, "Failed to execute gcov-dump: %s\n", strerror(errno));
        exit(1);
    } else if (pid > 0) {
        /* Parent process */
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("Exit status: %d\n", WEXITSTATUS(status));
        } else {
            printf("Process terminated abnormally\n");
        }
    } else {
        perror("fork failed");
    }
}

/* Execute gcov-dump with system() call */
void test_with_system(const char *description, const char *command) {
    printf("\n=== Testing (system): %s ===\n", description);
    printf("Command: %s\n", command);
    
    int status = system(command);
    if (status == -1) {
        printf("system() failed\n");
    } else if (WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    } else {
        printf("Process terminated abnormally\n");
    }
}

int main(void) {
    /* Create dummy .gcda file for testing */
    if (create_dummy_gcda() != 0) {
        fprintf(stderr, "Warning: Could not create dummy .gcda file\n");
        fprintf(stderr, "Tests requiring file arguments will fail\n");
    }
    
    printf("Starting gcov-dump flag parsing tests...\n");
    
    /* Test 1: Individual flags (using execvp) */
    char *test1[] = {"gcov-dump", "-h", NULL};
    test_with_execvp("Help flag", test1);
    
    char *test2[] = {"gcov-dump", "-v", NULL};
    test_with_execvp("Version flag", test2);
    
    char *test3[] = {"gcov-dump", "-l", NULL};
    test_with_execvp("Contents dump flag", test3);
    
    char *test4[] = {"gcov-dump", "-p", NULL};
    test_with_execvp("Positions dump flag", test4);
    
    char *test5[] = {"gcov-dump", "-r", NULL};
    test_with_execvp("Raw dump flag", test5);
    
    char *test6[] = {"gcov-dump", "-s", NULL};
    test_with_execvp("Stable dump flag", test6);
    
    char *test7[] = {"gcov-dump", "-x", NULL};
    test_with_execvp("Invalid flag (should trigger default case)", test7);
    
    /* Test 2: No arguments */
    char *test8[] = {"gcov-dump", NULL};
    test_with_execvp("No arguments", test8);
    
    /* Test 3: Combined valid flags in separate arguments */
    char *test9[] = {"gcov-dump", "-l", "-p", NULL};
    test_with_execvp("Combined flags: -l -p", test9);
    
    char *test10[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    test_with_execvp("Combined flags: -r -s -v", test10);
    
    char *test11[] = {"gcov-dump", "-h", "-l", NULL};
    test_with_execvp("Help with other flag: -h -l", test11);
    
    /* Test 4: Repeated flags */
    char *test12[] = {"gcov-dump", "-p", "-p", NULL};
    test_with_execvp("Repeated flag: -p -p", test12);
    
    /* Test 5: Flags with positional arguments (gcov files) */
    char *test13[] = {"gcov-dump", "-l", "test_gcov.gcda", NULL};
    test_with_execvp("Flag with file argument: -l test_gcov.gcda", test13);
    
    char *test14[] = {"gcov-dump", "-l", "-p", "test_gcov.gcda", NULL};
    test_with_execvp("Multiple flags with file: -l -p test_gcov.gcda", test14);
    
    /* Test 6: Using -- delimiter */
    char *test15[] = {"gcov-dump", "-l", "--", "test_gcov.gcda", NULL};
    test_with_execvp("Flag with -- delimiter: -l -- test_gcov.gcda", test15);
    
    /* Test 7: Combined short options (if supported) using system() */
    test_with_system("Combined short options: -lp", "gcov-dump -lp");
    test_with_system("Combined short options with file: -lp test_gcov.gcda", 
                     "gcov-dump -lp test_gcov.gcda");
    
    /* Test 8: Environment variable simulation using system() */
    test_with_system("With GCOV_DUMP_OPTIONS env var", 
                     "GCOV_DUMP_OPTIONS='-v' gcov-dump -l");
    
    /* Test 9: Output redirection to capture stderr */
    test_with_system("Invalid flag with stderr redirection", 
                     "gcov-dump -x 2>&1");
    
    /* Test 10: Multiple invalid flags */
    char *test16[] = {"gcov-dump", "-x", "-y", "-z", NULL};
    test_with_execvp("Multiple invalid flags", test16);
    
    /* Test 11: Mixed valid and invalid flags */
    char *test17[] = {"gcov-dump", "-l", "-x", "-p", NULL};
    test_with_execvp("Mixed valid and invalid flags", test17);
    
    /* Test 12: Long arguments (if supported) */
    test_with_system("Long argument test", "gcov-dump --help 2>&1");
    
    printf("\n=== All tests completed ===\n");
    
    /* Cleanup */
    unlink("test_gcov.c");
    unlink("test_gcov");
    unlink("test_gcov.gcno");
    unlink("test_gcov.gcda");
    
    return 0;
}
