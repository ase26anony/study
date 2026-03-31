#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_cleanup.c"
#define TEMP_OUTPUT_FILE "test_gcc_cleanup.o"

/* Create a simple valid C source file */
static int create_test_source(void) {
    FILE *f = fopen(TEMP_SOURCE_FILE, "w");
    if (!f) {
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(f, "/* Test file for GCC cleanup coverage */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main(void) {\n");
    fprintf(f, "    printf(\"Hello from test program\\n\");\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
    return 0;
}

/* Execute GCC with given arguments using fork/exec */
static int run_gcc(const char *description, const char *arg1, const char *arg2, 
                   const char *arg3, const char *arg4, const char *arg5, 
                   const char *arg6, const char *arg7) {
    pid_t pid;
    int status;
    
    fprintf(stderr, "\n=== %s ===\n", description);
    
    pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process: execute GCC */
        const char *argv[20];
        int i = 0;
        
        argv[i++] = "gcc";
        if (arg1) argv[i++] = arg1;
        if (arg2) argv[i++] = arg2;
        if (arg3) argv[i++] = arg3;
        if (arg4) argv[i++] = arg4;
        if (arg5) argv[i++] = arg5;
        if (arg6) argv[i++] = arg6;
        if (arg7) argv[i++] = arg7;
        argv[i++] = TEMP_SOURCE_FILE;
        argv[i++] = NULL;
        
        /* Print the command for traceability */
        fprintf(stderr, "Executing:");
        for (int j = 0; j < i; j++) {
            if (argv[j]) fprintf(stderr, " %s", argv[j]);
        }
        fprintf(stderr, "\n");
        
        execvp("gcc", (char *const *)argv);
        
        /* If we get here, exec failed */
        perror("execvp failed");
        _exit(EXIT_FAILURE);
    } else {
        /* Parent process: wait for GCC to complete */
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid failed");
            return -1;
        }
        
        if (WIFEXITED(status)) {
            fprintf(stderr, "GCC exited with status: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "GCC terminated by signal: %d\n", WTERMSIG(status));
        }
        
        return 0;
    }
}

/* Alternative using system() for simpler cases */
static int run_gcc_system(const char *description, const char *command) {
    fprintf(stderr, "\n=== %s ===\n", description);
    fprintf(stderr, "Executing: %s\n", command);
    
    int status = system(command);
    if (status == -1) {
        perror("system failed");
        return -1;
    }
    
    fprintf(stderr, "GCC returned: %d\n", WEXITSTATUS(status));
    return 0;
}

int main(void) {
    char command[1024];
    
    /* Create test source file */
    if (create_test_source() < 0) {
        return EXIT_FAILURE;
    }
    
    fprintf(stderr, "Created test source file: %s\n", TEMP_SOURCE_FILE);
    
    /* Test 1: Basic compilation to trigger initialization */
    run_gcc("Basic compilation", "-c", NULL, NULL, NULL, NULL, NULL, NULL);
    
    /* Test 2: With dumpdir and save-temps (triggers dumpdir allocation) */
    run_gcc("With dumpdir and save-temps", 
            "-dumpdir", "./dumpdir_test/",
            "-save-temps",
            "-c",
            NULL, NULL, NULL);
    
    /* Test 3: With dumpbase and dumpbase-ext (triggers dumpbase/dumpbase_ext allocation) */
    run_gcc("With dumpbase and dumpbase-ext",
            "-dumpbase", "myprogram",
            "-dumpbase-ext", ".c",
            "-c",
            NULL, NULL);
    
    /* Test 4: With outbase via -o option (triggers outbase allocation) */
    snprintf(command, sizeof(command),
             "gcc -dumpbase outbase_test -dumpbase-ext .c -save-temps=cwd "
             "-o %s -c %s",
             TEMP_OUTPUT_FILE, TEMP_SOURCE_FILE);
    run_gcc_system("With outbase (-o) and all dump options", command);
    
    /* Test 5: All options combined */
    snprintf(command, sizeof(command),
             "gcc -dumpdir ./full_dump/ -dumpbase full_program "
             "-dumpbase-ext .c -save-temps -o %s -c %s",
             TEMP_OUTPUT_FILE, TEMP_SOURCE_FILE);
    run_gcc_system("All options combined", command);
    
    /* Test 6: Invalid option to test cleanup after error */
    run_gcc("Invalid option (tests cleanup on error)",
            "-invalid-option-that-does-not-exist",
            "-dumpdir", "./error_test/",
            "-dumpbase", "error",
            "-c",
            NULL);
    
    /* Test 7: Multiple dumpdir variations */
    run_gcc("Dumpdir with trailing slash",
            "-dumpdir", "trailingslash/",
            "-dumpbase", "trailing",
            "-save-temps=cwd",
            "-c",
            NULL);
    
    run_gcc("Dumpdir without trailing slash",
            "-dumpdir", "notrailing",
            "-dumpbase", "notrail",
            "-save-temps",
            "-c",
            NULL);
    
    /* Test 8: Different save-temps modes */
    run_gcc("save-temps=obj",
            "-dumpdir", "./obj_dump/",
            "-save-temps=obj",
            "-dumpbase", "objtest",
            "-c",
            NULL);
    
    /* Cleanup temporary files */
    fprintf(stderr, "\n=== Cleaning up ===\n");
    if (unlink(TEMP_SOURCE_FILE) == 0) {
        fprintf(stderr, "Removed %s\n", TEMP_SOURCE_FILE);
    }
    
    if (unlink(TEMP_OUTPUT_FILE) == 0) {
        fprintf(stderr, "Removed %s\n", TEMP_OUTPUT_FILE);
    }
    
    /* Also clean up any dump directories created */
    system("rm -rf ./dumpdir_test/ ./full_dump/ ./obj_dump/ trailingslash/ notrailing");
    
    fprintf(stderr, "\nTest program completed successfully.\n");
    return 0;
}
