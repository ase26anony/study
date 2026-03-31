#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

/* Create a minimal C program to generate a .gcda file */
void create_dummy_gcda(void) {
    /* Create a simple C source file */
    FILE *fp = fopen("dummy.c", "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        exit(1);
    }
    fprintf(fp, "int main() { return 0; }\n");
    fclose(fp);
    
    /* Compile with coverage flags */
    printf("Compiling dummy program for coverage...\n");
    int compile_status = system("gcc -fprofile-arcs -ftest-coverage -O0 dummy.c -o dummy");
    if (compile_status != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        exit(1);
    }
    
    /* Run it to generate .gcda file */
    printf("Running dummy program to generate .gcda...\n");
    int run_status = system("./dummy");
    if (run_status != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        exit(1);
    }
}

/* Execute gcov-dump using execvp with precise argument control */
void exec_gcov_dump(const char *args[], const char *description) {
    printf("\n=== Testing: %s ===\n", description);
    printf("Command: ");
    for (int i = 0; args[i] != NULL; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", (char * const *)args);
        /* If execvp returns, it failed */
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        /* Parent process */
        int status;
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

/* Execute using system() for comparison */
void system_gcov_dump(const char *cmd, const char *description) {
    printf("\n=== Testing (system): %s ===\n", description);
    printf("Command: %s\n", cmd);
    
    int status = system(cmd);
    if (status == -1) {
        perror("system failed");
    } else {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    }
}

int main(void) {
    printf("=== Starting gcov-dump flag parsing tests ===\n");
    
    /* Create a dummy .gcda file for file-based tests */
    create_dummy_gcda();
    
    /* Test cases using execvp for precise argument passing */
    
    /* 1. Individual flag tests (direct switch cases) */
    const char *help_args[] = {"gcov-dump", "-h", NULL};
    exec_gcov_dump(help_args, "Help flag (-h)");
    
    const char *version_args[] = {"gcov-dump", "-v", NULL};
    exec_gcov_dump(version_args, "Version flag (-v)");
    
    const char *contents_args[] = {"gcov-dump", "-l", NULL};
    exec_gcov_dump(contents_args, "Contents dump flag (-l)");
    
    const char *positions_args[] = {"gcov-dump", "-p", NULL};
    exec_gcov_dump(positions_args, "Positions dump flag (-p)");
    
    const char *raw_args[] = {"gcov-dump", "-r", NULL};
    exec_gcov_dump(raw_args, "Raw dump flag (-r)");
    
    const char *stable_args[] = {"gcov-dump", "-s", NULL};
    exec_gcov_dump(stable_args, "Stable dump flag (-s)");
    
    const char *invalid_args[] = {"gcov-dump", "-x", NULL};
    exec_gcov_dump(invalid_args, "Invalid flag (-x) to trigger default case");
    
    /* 2. Combination of valid flags */
    const char *combo1_args[] = {"gcov-dump", "-l", "-p", NULL};
    exec_gcov_dump(combo1_args, "Combination -l -p");
    
    const char *combo2_args[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    exec_gcov_dump(combo2_args, "Combination -r -s -v");
    
    const char *combo3_args[] = {"gcov-dump", "-h", "-l", NULL};
    exec_gcov_dump(combo3_args, "Combination -h -l (help with other flag)");
    
    /* 3. Repeated flags */
    const char *repeat_args[] = {"gcov-dump", "-p", "-p", NULL};
    exec_gcov_dump(repeat_args, "Repeated flag -p -p");
    
    /* 4. Tests with file arguments */
    const char *file_args[] = {"gcov-dump", "-l", "dummy.gcda", NULL};
    exec_gcov_dump(file_args, "Flag with file argument -l dummy.gcda");
    
    const char *file_combo_args[] = {"gcov-dump", "-l", "-p", "dummy.gcda", NULL};
    exec_gcov_dump(file_combo_args, "Multiple flags with file -l -p dummy.gcda");
    
    /* 5. No arguments */
    const char *no_args[] = {"gcov-dump", NULL};
    exec_gcov_dump(no_args, "No arguments");
    
    /* 6. Using -- delimiter */
    const char *delim_args[] = {"gcov-dump", "-l", "--", "dummy.gcda", NULL};
    exec_gcov_dump(delim_args, "With -- delimiter -l -- dummy.gcda");
    
    /* 7. Tests using system() for different syntactic styles */
    system_gcov_dump("gcov-dump -lp", "Combined short options -lp");
    system_gcov_dump("gcov-dump -l -p dummy.gcda", "Separate args with file");
    system_gcov_dump("gcov-dump -lps", "Multiple combined flags -lps");
    
    /* 8. Test with environment variable */
    printf("\n=== Testing with environment variable ===\n");
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    const char *env_args[] = {"gcov-dump", "-l", NULL};
    exec_gcov_dump(env_args, "With GCOV_DUMP_OPTIONS=-v and -l flag");
    unsetenv("GCOV_DUMP_OPTIONS");
    
    /* 9. Test error output redirection */
    printf("\n=== Testing error output capture ===\n");
    system("gcov-dump -x 2> error_output.txt");
    system("echo 'Error output:' && cat error_output.txt");
    
    /* 10. Test with non-existent file */
    const char *badfile_args[] = {"gcov-dump", "-l", "nonexistent.gcda", NULL};
    exec_gcov_dump(badfile_args, "Flag with non-existent file");
    
    /* 11. Test multiple invalid flags */
    const char *multi_invalid_args[] = {"gcov-dump", "-x", "-y", "-z", NULL};
    exec_gcov_dump(multi_invalid_args, "Multiple invalid flags");
    
    /* 12. Test flag with equals sign (if supported) */
    system_gcov_dump("gcov-dump --help", "Long option --help (if supported)");
    
    /* 13. Test mixed valid and invalid */
    const char *mixed_args[] = {"gcov-dump", "-l", "-x", "-p", NULL};
    exec_gcov_dump(mixed_args, "Mixed valid and invalid flags");
    
    printf("\n=== All tests completed ===\n");
    
    /* Cleanup */
    system("rm -f dummy.c dummy dummy.gcda dummy.gcno error_output.txt");
    
    return 0;
}
