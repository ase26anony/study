#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_input.c"
#define TEMP_OUTPUT_FILE "test_coverage_output.o"

/* Create a simple valid C source file for compilation */
static int create_test_source(void) {
    FILE *fp = fopen(TEMP_SOURCE_FILE, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create source file: %s\n", strerror(errno));
        return -1;
    }
    
    fprintf(fp, "/* Test file for GCC coverage */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    printf(\"Hello from coverage test!\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/* Execute GCC with specific options */
static int run_gcc(const char *description, const char *options) {
    printf("\n=== Running: %s ===\n", description);
    printf("Command: gcc %s\n", options);
    
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "fork failed: %s\n", strerror(errno));
        return -1;
    }
    
    if (pid == 0) {
        /* Child process - execute GCC */
        execlp("gcc", "gcc", options, NULL);
        /* If execlp returns, it failed */
        fprintf(stderr, "execlp failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        /* Parent process - wait for GCC to complete */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("GCC exited with status: %d\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            printf("GCC terminated by signal: %d\n", WTERMSIG(status));
            return -1;
        }
    }
    return 0;
}

int main(void) {
    char cmd[1024];
    int ret;
    
    /* Step 1: Create a valid C source file */
    printf("Creating test source file: %s\n", TEMP_SOURCE_FILE);
    if (create_test_source() < 0) {
        return EXIT_FAILURE;
    }
    
    /* Step 2: Multiple distinct GCC invocations with varying options */
    
    /* Invocation 1: With -dumpdir and -save-temps */
    snprintf(cmd, sizeof(cmd), 
             "-dumpdir ./coverage_dump/ -save-temps=cwd -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    run_gcc("Test with -dumpdir and -save-temps", cmd);
    
    /* Invocation 2: With -dumpbase and -dumpbase-ext but no -dumpdir */
    snprintf(cmd, sizeof(cmd),
             "-dumpbase mycoverage -dumpbase-ext .c -c %s -o %s_2.o",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    run_gcc("Test with -dumpbase and -dumpbase-ext", cmd);
    
    /* Invocation 3: With all options combined */
    snprintf(cmd, sizeof(cmd),
             "-dumpdir ./full_dump/ -dumpbase fulltest -dumpbase-ext .c "
             "-save-temps -o %s_full.o -c %s",
             TEMP_OUTPUT_FILE, TEMP_SOURCE_FILE);
    run_gcc("Test with all dump options and -save-temps", cmd);
    
    /* Invocation 4: With -o option to influence outbase */
    snprintf(cmd, sizeof(cmd),
             "-o custom_output.o -dumpdir ./custom/ -dumpbase custom "
             "-dumpbase-ext .src -c %s",
             TEMP_SOURCE_FILE);
    run_gcc("Test with custom -o and dump options", cmd);
    
    /* Invocation 5: Trigger cleanup after error exit */
    snprintf(cmd, sizeof(cmd),
             "-dumpdir ./error_dump/ -dumpbase error -invalid-opt -c %s",
             TEMP_SOURCE_FILE);
    run_gcc("Test error cleanup with dump options", cmd);
    
    /* Invocation 6: Minimal compilation to ensure basic path */
    snprintf(cmd, sizeof(cmd), "-c %s", TEMP_SOURCE_FILE);
    run_gcc("Minimal compilation test", cmd);
    
    /* Invocation 7: With verbose flag (verbose_only_flag in target block) */
    snprintf(cmd, sizeof(cmd),
             "-dumpdir ./verbose_dump/ -v -save-temps -c %s",
             TEMP_SOURCE_FILE);
    run_gcc("Test with verbose flag", cmd);
    
    /* Invocation 8: With version printing (print_version in target block) */
    snprintf(cmd, sizeof(cmd), "-dumpbase versiontest --version");
    run_gcc("Test version printing with dumpbase", cmd);
    
    /* Step 3: Clean up temporary files */
    printf("\n=== Cleaning up temporary files ===\n");
    
    /* Remove the source file */
    if (unlink(TEMP_SOURCE_FILE) == 0) {
        printf("Removed: %s\n", TEMP_SOURCE_FILE);
    } else {
        fprintf(stderr, "Failed to remove %s: %s\n", 
                TEMP_SOURCE_FILE, strerror(errno));
    }
    
    /* Remove generated output files */
    char *output_files[] = {
        TEMP_OUTPUT_FILE,
        "test_coverage_output_2.o",
        "test_coverage_output_full.o",
        "custom_output.o",
        TEMP_SOURCE_FILE ".i",  /* Preprocessed output from -save-temps */
        TEMP_SOURCE_FILE ".s",  /* Assembly output from -save-temps */
        NULL
    };
    
    for (int i = 0; output_files[i] != NULL; i++) {
        if (unlink(output_files[i]) == 0) {
            printf("Removed: %s\n", output_files[i]);
        }
    }
    
    /* Try to remove dump directories */
    system("rm -rf ./coverage_dump/ ./full_dump/ ./custom/ ./error_dump/ ./verbose_dump/");
    
    printf("\n=== Test completed ===\n");
    return EXIT_SUCCESS;
}
