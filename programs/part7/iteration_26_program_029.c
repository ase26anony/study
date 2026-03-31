#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_ARGS 10

/* Structure to hold test cases */
typedef struct {
    char *description;
    char *args[MAX_ARGS];
    int use_system;  /* 1 = use system(), 0 = use execvp() */
} test_case_t;

/* Create a minimal dummy .gcda file for testing */
int create_dummy_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create dummy .gcda file");
        return 0;
    }
    
    /* Write minimal .gcda header (magic + version) */
    unsigned int magic = 0x67636461; /* 'gcda' in little-endian */
    unsigned int version = 0x4020000; /* GCC 4.2 format */
    
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    
    /* Write a zero terminator */
    unsigned int zero = 0;
    fwrite(&zero, sizeof(zero), 1, fp);
    
    fclose(fp);
    return 1;
}

/* Execute test case using execvp */
void execute_with_execvp(const char *description, char *const args[]) {
    printf("\n=== Testing with execvp: %s ===\n", description);
    printf("Command: ");
    for (int i = 0; args[i] != NULL; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", args);
        /* If execvp returns, it failed */
        perror("execvp failed");
        exit(EXIT_FAILURE);
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

/* Execute test case using system() */
void execute_with_system(const char *description, const char *cmd) {
    printf("\n=== Testing with system(): %s ===\n", description);
    printf("Command: %s\n", cmd);
    
    int status = system(cmd);
    if (status == -1) {
        printf("system() failed\n");
    } else if (WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    } else {
        printf("Process terminated abnormally\n");
    }
}

int main(int argc, char *argv[]) {
    printf("=== gcov-dump Flag Parser Test Suite ===\n");
    
    /* Create a dummy .gcda file for file-based tests */
    const char *dummy_gcda = "dummy.gcda";
    if (!create_dummy_gcda(dummy_gcda)) {
        fprintf(stderr, "Warning: Could not create dummy .gcda file\n");
    }
    
    /* Define test cases */
    test_case_t test_cases[] = {
        /* Individual flag tests (using execvp) */
        {"Help flag", {"gcov-dump", "-h", NULL}, 0},
        {"Version flag", {"gcov-dump", "-v", NULL}, 0},
        {"Contents dump flag", {"gcov-dump", "-l", NULL}, 0},
        {"Positions dump flag", {"gcov-dump", "-p", NULL}, 0},
        {"Raw dump flag", {"gcov-dump", "-r", NULL}, 0},
        {"Stable dump flag", {"gcov-dump", "-s", NULL}, 0},
        {"Invalid flag (trigger default case)", {"gcov-dump", "-x", NULL}, 0},
        
        /* Combination tests (using execvp) */
        {"Combined flags: -l -p", {"gcov-dump", "-l", "-p", NULL}, 0},
        {"Combined flags: -r -s -v", {"gcov-dump", "-r", "-s", "-v", NULL}, 0},
        {"Help with other flags", {"gcov-dump", "-h", "-l", NULL}, 0},
        {"Repeated flag: -p -p", {"gcov-dump", "-p", "-p", NULL}, 0},
        
        /* Combined short options syntax (if supported) */
        {"Combined short options: -lp", {"gcov-dump", "-lp", NULL}, 0},
        {"Combined short options: -rs", {"gcov-dump", "-rs", NULL}, 0},
        
        /* With positional arguments */
        {"With file argument: -l dummy.gcda", {"gcov-dump", "-l", dummy_gcda, NULL}, 0},
        {"Multiple flags with file: -l -p dummy.gcda", {"gcov-dump", "-l", "-p", dummy_gcda, NULL}, 0},
        
        /* With -- delimiter */
        {"With -- delimiter: -l -- dummy.gcda", {"gcov-dump", "-l", "--", dummy_gcda, NULL}, 0},
        
        /* No arguments (using system) */
        {"No arguments", {"gcov-dump", NULL}, 1},
        
        /* Environment variable tests (using system) */
        {"With GCOV_DUMP_OPTIONS env var", {"gcov-dump", "-v", NULL}, 1},
        
        /* Invalid combinations (using system) */
        {"Invalid flag combination: -x -y", {"gcov-dump", "-x", "-y", NULL}, 1},
        
        /* End marker */
        {NULL, {NULL}, 0}
    };
    
    /* Set environment variables for some tests */
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    
    /* Execute all test cases */
    for (int i = 0; test_cases[i].description != NULL; i++) {
        if (test_cases[i].use_system) {
            /* Build command string for system() */
            char cmd[256] = "";
            for (int j = 0; test_cases[i].args[j] != NULL; j++) {
                strcat(cmd, test_cases[i].args[j]);
                strcat(cmd, " ");
            }
            execute_with_system(test_cases[i].description, cmd);
        } else {
            execute_with_execvp(test_cases[i].description, test_cases[i].args);
        }
    }
    
    /* Additional edge cases using system() */
    printf("\n=== Additional Edge Cases ===\n");
    
    /* Test with output redirection */
    execute_with_system("With stderr redirection", "gcov-dump -x 2> stderr.out");
    execute_with_system("Check stderr output", "cat stderr.out");
    
    /* Test with both stdout and stderr redirection */
    execute_with_system("Both stdout and stderr redirected", 
                       "gcov-dump -v > output.txt 2>&1");
    
    /* Test empty string as argument */
    execute_with_system("Empty string argument", "gcov-dump \"\"");
    
    /* Test with very long argument (if supported) */
    char long_arg[256] = "gcov-dump ";
    for (int i = 0; i < 20; i++) {
        strcat(long_arg, "-v ");
    }
    execute_with_system("Many repeated flags", long_arg);
    
    /* Cleanup */
    unlink(dummy_gcda);
    unlink("stderr.out");
    unlink("output.txt");
    
    printf("\n=== Test Suite Complete ===\n");
    return 0;
}
