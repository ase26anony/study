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
        perror("Failed to create test program");
        return -1;
    }
    fprintf(fp, "%s", test_program);
    fclose(fp);
    
    /* Compile with coverage flags */
    printf("Compiling test program with coverage...\n");
    pid = fork();
    if (pid == 0) {
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
    printf("Running test program to generate .gcda...\n");
    pid = fork();
    if (pid == 0) {
        execl("./test_gcov", "./test_gcov", NULL);
        perror("exec test_gcov failed");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Test program execution failed\n");
            return -1;
        }
    }
    
    printf("Dummy .gcda file created successfully\n");
    return 0;
}

/* Execute gcov-dump with given arguments using execvp */
void test_gcov_dump_execvp(const char *description, char *const args[]) {
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
        fprintf(stderr, "execvp failed: %s\n", strerror(errno));
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

/* Execute gcov-dump using system() */
void test_gcov_dump_system(const char *description, const char *command) {
    printf("\n=== Testing (system): %s ===\n", description);
    printf("Command: %s\n", command);
    
    int status = system(command);
    if (status == -1) {
        perror("system() failed");
    } else if (WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    } else {
        printf("Command terminated abnormally\n");
    }
}

int main(void) {
    /* Create dummy .gcda file for file-based tests */
    if (create_dummy_gcda() != 0) {
        fprintf(stderr, "Warning: Could not create dummy .gcda file\n");
        fprintf(stderr, "File-based tests will use placeholder\n");
    }
    
    printf("\n========== Testing gcov-dump Flag Parsing ==========\n");
    
    /* Test 1: Individual flags (targeting uncovered switch cases) */
    char *test1_h[] = {"gcov-dump", "-h", NULL};
    char *test1_v[] = {"gcov-dump", "-v", NULL};
    char *test1_l[] = {"gcov-dump", "-l", NULL};
    char *test1_p[] = {"gcov-dump", "-p", NULL};
    char *test1_r[] = {"gcov-dump", "-r", NULL};
    char *test1_s[] = {"gcov-dump", "-s", NULL};
    char *test1_x[] = {"gcov-dump", "-x", NULL};  /* Invalid flag */
    
    test_gcov_dump_execvp("Help flag (-h)", test1_h);
    test_gcov_dump_execvp("Version flag (-v)", test1_v);
    test_gcov_dump_execvp("Contents dump flag (-l)", test1_l);
    test_gcov_dump_execvp("Positions dump flag (-p)", test1_p);
    test_gcov_dump_execvp("Raw dump flag (-r)", test1_r);
    test_gcov_dump_execvp("Stable dump flag (-s)", test1_s);
    test_gcov_dump_execvp("Invalid flag (-x) - should trigger default case", test1_x);
    
    /* Test 2: Combination of valid flags */
    char *test2_lp[] = {"gcov-dump", "-l", "-p", NULL};
    char *test2_rsv[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    char *test2_hl[] = {"gcov-dump", "-h", "-l", NULL};  /* -h may cause early exit */
    char *test2_pp[] = {"gcov-dump", "-p", "-p", NULL};  /* Repeated flag */
    
    test_gcov_dump_execvp("Combination: -l -p", test2_lp);
    test_gcov_dump_execvp("Combination: -r -s -v", test2_rsv);
    test_gcov_dump_execvp("Combination: -h -l (h may exit early)", test2_hl);
    test_gcov_dump_execvp("Repeated flag: -p -p", test2_pp);
    
    /* Test 3: Different flag syntax styles using system() */
    test_gcov_dump_system("Combined short options: -lp", "gcov-dump -lp");
    test_gcov_dump_system("Combined short options: -lps", "gcov-dump -lps");
    
    /* Test 4: With positional arguments (gcov files) */
    char *test4_l_file[] = {"gcov-dump", "-l", "test_gcov.gcda", NULL};
    char *test4_lp_file[] = {"gcov-dump", "-l", "-p", "test_gcov.gcda", NULL};
    char *test4_file_l[] = {"gcov-dump", "test_gcov.gcda", "-l", NULL};
    
    test_gcov_dump_execvp("Flag with file: -l dummy.gcda", test4_l_file);
    test_gcov_dump_execvp("Multiple flags with file: -l -p dummy.gcda", test4_lp_file);
    test_gcov_dump_execvp("File then flag: dummy.gcda -l", test4_file_l);
    
    /* Test 5: With -- delimiter */
    char *test5_delim[] = {"gcov-dump", "-l", "--", "test_gcov.gcda", NULL};
    test_gcov_dump_execvp("With -- delimiter: -l -- dummy.gcda", test5_delim);
    
    /* Test 6: Environment variable tests */
    printf("\n=== Testing with environment variables ===\n");
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    test_gcov_dump_system("With GCOV_DUMP_OPTIONS=-v", "gcov-dump");
    unsetenv("GCOV_DUMP_OPTIONS");
    
    /* Test 7: No arguments and error contexts */
    char *test7_noargs[] = {"gcov-dump", NULL};
    char *test7_empty[] = {"gcov-dump", "", NULL};
    
    test_gcov_dump_execvp("No arguments", test7_noargs);
    test_gcov_dump_execvp("Empty argument", test7_empty);
    
    /* Test 8: Output redirection to capture stderr */
    printf("\n=== Testing output redirection ===\n");
    test_gcov_dump_system("Redirect stderr for invalid flag", 
                         "gcov-dump -x 2>&1 | head -5");
    test_gcov_dump_system("Redirect both stdout and stderr", 
                         "gcov-dump -v -l 2>&1 | head -10");
    
    /* Test 9: Edge cases with system() */
    test_gcov_dump_system("Multiple files: -l file1 file2", 
                         "gcov-dump -l test_gcov.gcda test_gcov.gcno 2>&1");
    test_gcov_dump_system("Flag at end: file -l", 
                         "gcov-dcov test_gcov.gcda -l 2>&1");
    
    /* Cleanup */
    printf("\n========== Cleaning up ==========\n");
    system("rm -f test_gcov test_gcov.c test_gcov.gcda test_gcov.gcno test_gcov.o");
    
    printf("\nAll tests completed. Check coverage of gcov-dump lines 111-130.\n");
    return 0;
}
