#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Create a minimal valid .gcda file for testing */
void create_dummy_gcda(void) {
    /* Create a simple C program that will generate coverage data */
    FILE *fp = fopen("test_coverage.c", "w");
    if (!fp) {
        perror("Failed to create test_coverage.c");
        exit(1);
    }
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Hello, coverage!\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    fclose(fp);
    
    /* Compile with coverage flags */
    printf("Compiling test program with coverage...\n");
    if (system("gcc -fprofile-arcs -ftest-coverage test_coverage.c -o test_coverage") != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        exit(1);
    }
    
    /* Run it to generate .gcda file */
    printf("Running test program to generate coverage data...\n");
    if (system("./test_coverage > /dev/null 2>&1") != 0) {
        fprintf(stderr, "Failed to run test program\n");
        exit(1);
    }
}

/* Execute gcov-dump using execvp for precise argument control */
void exec_gcov_dump(const char *args[], const char *description) {
    printf("\n=== Testing: %s ===\n", description);
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", (char *const *)args);
        /* If execvp returns, it failed */
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("Exit status: %d\n", WEXITSTATUS(status));
        }
    } else {
        perror("fork failed");
    }
}

/* Execute using system() for shell interpretation testing */
void system_gcov_dump(const char *cmd, const char *description) {
    printf("\n=== Testing (system): %s ===\n", description);
    int status = system(cmd);
    printf("System call returned: %d\n", status);
}

int main(void) {
    /* First, create a dummy .gcda file for file-based tests */
    create_dummy_gcda();
    
    /* Set environment variable that might affect gcov-dump */
    setenv("GCOV_DUMP_OPTIONS", "--help", 1);
    
    /* Array of test cases using execvp */
    struct {
        const char *args[5];
        const char *description;
    } test_cases[] = {
        /* Individual flag tests (targeting specific switch cases) */
        {{"gcov-dump", "-h", NULL}, "Help flag (-h) - case 'h'"},
        {{"gcov-dump", "-v", NULL}, "Version flag (-v) - case 'v'"},
        {{"gcov-dump", "-l", NULL}, "Contents dump flag (-l) - case 'l'"},
        {{"gcov-dump", "-p", NULL}, "Positions dump flag (-p) - case 'p'"},
        {{"gcov-dump", "-r", NULL}, "Raw dump flag (-r) - case 'r'"},
        {{"gcov-dump", "-s", NULL}, "Stable dump flag (-s) - case 's'"},
        {{"gcov-dump", "-x", NULL}, "Invalid flag (-x) - default case"},
        
        /* Combination of valid flags */
        {{"gcov-dump", "-l", "-p", NULL}, "Combination: -l -p"},
        {{"gcov-dump", "-r", "-s", "-v", NULL}, "Combination: -r -s -v"},
        {{"gcov-dump", "-h", "-l", NULL}, "Combination: -h -l (help with other flag)"},
        
        /* Repeated flags */
        {{"gcov-dump", "-p", "-p", NULL}, "Repeated flag: -p -p"},
        {{"gcov-dump", "-l", "-l", "-l", NULL}, "Repeated flag: -l -l -l"},
        
        /* With positional arguments (gcov files) */
        {{"gcov-dump", "-l", "test_coverage.gcda", NULL}, "With file: -l test_coverage.gcda"},
        {{"gcov-dump", "-p", "-r", "test_coverage.gcda", NULL}, "With file: -p -r test_coverage.gcda"},
        
        /* With -- delimiter */
        {{"gcov-dump", "-l", "--", "test_coverage.gcda", NULL}, "With -- delimiter: -l -- test_coverage.gcda"},
        
        /* No arguments */
        {{"gcov-dump", NULL}, "No arguments"},
        
        /* End marker */
        {{NULL}, NULL}
    };
    
    /* Execute all execvp test cases */
    for (int i = 0; test_cases[i].args[0] != NULL; i++) {
        exec_gcov_dump(test_cases[i].args, test_cases[i].description);
    }
    
    /* Additional tests using system() for different syntactic styles */
    system_gcov_dump("gcov-dump -lp", "Combined short options: -lp");
    system_gcov_dump("gcov-dump -l -p test_coverage.gcda", "Mixed flags and file");
    system_gcov_dump("gcov-dump -h 2>&1", "Help with stderr redirect");
    system_gcov_dump("gcov-dump -x 2>&1", "Invalid flag with stderr redirect");
    
    /* Test with different environment variable */
    unsetenv("GCOV_DUMP_OPTIONS");
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    exec_gcov_dump((const char *[]){"gcov-dump", "-l", NULL}, 
                   "With GCOV_DUMP_OPTIONS=-v");
    
    /* Clean up */
    printf("\n=== Cleaning up ===\n");
    system("rm -f test_coverage.c test_coverage test_coverage.gcda test_coverage.gcno");
    
    return 0;
}
