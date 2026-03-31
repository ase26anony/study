#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Create a minimal valid .gcda file for testing */
void create_dummy_gcda(void) {
    /* Create a minimal .gcda file structure */
    FILE *fp = fopen("dummy.gcda", "wb");
    if (!fp) {
        perror("Failed to create dummy.gcda");
        return;
    }
    
    /* Write minimal GCOV data magic and version */
    unsigned int magic = 0x67636461; /* "gcda" in little-endian */
    unsigned int version = 0x4020000; /* GCOV version 4.2 */
    unsigned int stamp = 0x12345678;
    
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    fwrite(&stamp, sizeof(stamp), 1, fp);
    
    /* Write a zero length indicating end of data */
    unsigned int zero = 0;
    fwrite(&zero, sizeof(zero), 1, fp);
    
    fclose(fp);
}

/* Execute gcov-dump using execvp for precise argument control */
void exec_gcov_dump(const char *args[], const char *description) {
    printf("\n=== Testing: %s ===\n", description);
    fflush(stdout);
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", (char *const *)args);
        /* If execvp returns, it failed */
        perror("execvp failed");
        exit(EXIT_FAILURE);
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
    fflush(stdout);
    
    int ret = system(cmd);
    printf("System return: %d\n", ret);
}

int main(void) {
    /* Create dummy .gcda file for file-based tests */
    create_dummy_gcda();
    
    /* Set environment variable if gcov-dump uses it */
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    
    /* Test cases for execvp (precise argument control) */
    const char *test_cases[][5] = {
        /* Single flag tests - targeting each switch case */
        {"gcov-dump", "-h", NULL},                     /* Help flag - case 'h' */
        {"gcov-dump", "-v", NULL},                     /* Version flag - case 'v' */
        {"gcov-dump", "-l", NULL},                     /* Contents dump - case 'l' */
        {"gcov-dump", "-p", NULL},                     /* Positions dump - case 'p' */
        {"gcov-dump", "-r", NULL},                     /* Raw dump - case 'r' */
        {"gcov-dump", "-s", NULL},                     /* Stable dump - case 's' */
        {"gcov-dump", "-x", NULL},                     /* Invalid flag - default case */
        
        /* Combination of valid flags */
        {"gcov-dump", "-l", "-p", NULL},               /* Two flags */
        {"gcov-dump", "-r", "-s", "-v", NULL},         /* Three flags */
        {"gcov-dump", "-h", "-l", NULL},               /* Help with another flag */
        
        /* Repeated flags */
        {"gcov-dump", "-p", "-p", NULL},               /* Same flag twice */
        {"gcov-dump", "-l", "-l", "-l", NULL},         /* Same flag three times */
        
        /* With positional arguments (gcov files) */
        {"gcov-dump", "-l", "dummy.gcda", NULL},       /* Flag with file */
        {"gcov-dump", "-p", "-r", "dummy.gcda", NULL}, /* Multiple flags with file */
        
        /* With -- delimiter */
        {"gcov-dump", "-l", "--", "dummy.gcda", NULL}, /* Flag, delimiter, file */
        
        /* No arguments */
        {"gcov-dump", NULL},                           /* No flags */
        
        /* End marker */
        {NULL}
    };
    
    const char *descriptions[] = {
        "Help flag (-h)",
        "Version flag (-v)",
        "Contents dump flag (-l)",
        "Positions dump flag (-p)",
        "Raw dump flag (-r)",
        "Stable dump flag (-s)",
        "Invalid flag (-x) - should trigger default case",
        "Combination: -l -p",
        "Combination: -r -s -v",
        "Help with another flag: -h -l",
        "Repeated flag: -p -p",
        "Repeated flag: -l -l -l",
        "With file argument: -l dummy.gcda",
        "Multiple flags with file: -p -r dummy.gcda",
        "With delimiter: -l -- dummy.gcda",
        "No arguments"
    };
    
    /* Execute all test cases using execvp */
    printf("========== Testing with execvp ==========\n");
    for (int i = 0; test_cases[i][0] != NULL; i++) {
        exec_gcov_dump(test_cases[i], descriptions[i]);
    }
    
    /* Test cases for system() - testing shell interpretation */
    printf("\n\n========== Testing with system() ==========\n");
    
    /* Combined short options (if supported by getopt) */
    system_gcov_dump("gcov-dump -lp", "Combined short options: -lp");
    system_gcov_dump("gcov-dump -lps", "Combined short options: -lps");
    
    /* Different flag ordering */
    system_gcov_dump("gcov-dump dummy.gcda -l", "File before flag: dummy.gcda -l");
    system_gcov_dump("gcov-dump -l dummy.gcda -p", "Mixed flags and files: -l dummy.gcda -p");
    
    /* With environment variable */
    setenv("GCOV_DUMP_OPTIONS", "-l -p", 1);
    system_gcov_dump("gcov-dump -v", "With GCOV_DUMP_OPTIONS=-l -p and -v flag");
    
    /* Error stream redirection */
    system_gcov_dump("gcov-dump -x 2>&1", "Invalid flag with stderr redirect: -x");
    
    /* Multiple invalid flags */
    system_gcov_dump("gcov-dump -x -y -z 2>&1", "Multiple invalid flags: -x -y -z");
    
    /* Clean up */
    unlink("dummy.gcda");
    
    printf("\n========== All tests completed ==========\n");
    return 0;
}
