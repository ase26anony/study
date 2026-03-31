#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_cleanup.c"
#define TEMP_OBJECT_FILE "test_gcc_cleanup.o"

/* Create a simple valid C source file */
static int create_test_source(void) {
    FILE *fp = fopen(TEMP_SOURCE_FILE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(fp, "/* Test file for GCC cleanup coverage */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    printf(\"Hello from test program\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/* Execute GCC with given arguments using fork/exec */
static int run_gcc(const char *description, char *const argv[]) {
    printf("\n=== Running: %s ===\n", description);
    
    /* Print the command for traceability */
    fprintf(stderr, "Command: ");
    for (int i = 0; argv[i] != NULL; i++) {
        fprintf(stderr, "%s ", argv[i]);
    }
    fprintf(stderr, "\n");
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        execvp(argv[0], argv);
        /* If execvp returns, it failed */
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid failed");
            return -1;
        }
        
        if (WIFEXITED(status)) {
            printf("Exit status: %d\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            printf("Terminated by signal: %d\n", WTERMSIG(status));
            return -1;
        }
    }
    
    return 0;
}

int main(void) {
    int ret = 0;
    
    /* Create test source file */
    if (create_test_source() < 0) {
        return EXIT_FAILURE;
    }
    
    printf("Created test source file: %s\n", TEMP_SOURCE_FILE);
    
    /* Test 1: Basic compilation with dump options */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump/",
            "-dumpbase", "myprog",
            "-dumpbase-ext", ".c",
            "-c", TEMP_SOURCE_FILE,
            "-o", TEMP_OBJECT_FILE,
            NULL
        };
        run_gcc("Test 1: Basic dump options", argv);
    }
    
    /* Test 2: With save-temps flag */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump2/",
            "-dumpbase", "test2",
            "-dumpbase-ext", ".c",
            "-save-temps",
            "-c", TEMP_SOURCE_FILE,
            "-o", "test2.o",
            NULL
        };
        run_gcc("Test 2: With save-temps", argv);
    }
    
    /* Test 3: With save-temps=cwd */
    {
        char *argv[] = {
            "gcc",
            "-dumpbase", "test3",
            "-dumpbase-ext", ".c",
            "-save-temps=cwd",
            "-c", TEMP_SOURCE_FILE,
            "-o", "test3.o",
            NULL
        };
        run_gcc("Test 3: With save-temps=cwd (no dumpdir)", argv);
    }
    
    /* Test 4: All options combined */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump_all/",
            "-dumpbase", "fulltest",
            "-dumpbase-ext", ".c",
            "-save-temps",
            "-c", TEMP_SOURCE_FILE,
            "-o", "full_output.o",
            NULL
        };
        run_gcc("Test 4: All options combined", argv);
    }
    
    /* Test 5: Different dumpbase-ext */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump_ext/",
            "-dumpbase", "exttest",
            "-dumpbase-ext", ".special",
            "-save-temps",
            "-c", TEMP_SOURCE_FILE,
            "-o", "ext_output.o",
            NULL
        };
        run_gcc("Test 5: Custom dumpbase-ext", argv);
    }
    
    /* Test 6: Error case - invalid option (should still trigger cleanup) */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump_err/",
            "-dumpbase", "errtest",
            "-invalid-opt",  /* This will cause an error */
            "-c", TEMP_SOURCE_FILE,
            NULL
        };
        run_gcc("Test 6: Invalid option (error case)", argv);
    }
    
    /* Test 7: No output file specified */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump_noout/",
            "-dumpbase", "noout",
            "-dumpbase-ext", ".c",
            "-save-temps",
            "-c", TEMP_SOURCE_FILE,
            NULL
        };
        run_gcc("Test 7: No -o option", argv);
    }
    
    /* Test 8: Using system() for variety */
    printf("\n=== Running with system() ===\n");
    char system_cmd[512];
    snprintf(system_cmd, sizeof(system_cmd),
             "gcc -dumpdir ./dump_sys/ -dumpbase systest -dumpbase-ext .c "
             "-save-temps -c %s -o systest.o",
             TEMP_SOURCE_FILE);
    fprintf(stderr, "Command: %s\n", system_cmd);
    int sys_ret = system(system_cmd);
    printf("System call returned: %d\n", sys_ret);
    
    /* Clean up temporary files */
    printf("\n=== Cleaning up ===\n");
    if (remove(TEMP_SOURCE_FILE) == 0) {
        printf("Removed %s\n", TEMP_SOURCE_FILE);
    }
    
    /* Try to remove object files that might have been created */
    char *objects[] = {
        TEMP_OBJECT_FILE,
        "test2.o",
        "test3.o",
        "full_output.o",
        "ext_output.o",
        "systest.o",
        NULL
    };
    
    for (int i = 0; objects[i] != NULL; i++) {
        if (remove(objects[i]) == 0) {
            printf("Removed %s\n", objects[i]);
        }
    }
    
    /* Try to remove dump directories */
    system("rm -rf ./dump ./dump2 ./dump_all ./dump_ext ./dump_err ./dump_noout ./dump_sys");
    
    printf("\nTest program completed.\n");
    return ret;
}
