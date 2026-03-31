#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_source.c"
#define TEMP_OUTPUT_FILE "test_coverage_output.o"

/* Create a simple valid C source file */
static int create_test_source(void) {
    FILE *fp = fopen(TEMP_SOURCE_FILE, "w");
    if (!fp) {
        perror("Failed to create test source file");
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

/* Clean up temporary files */
static void cleanup_files(void) {
    unlink(TEMP_SOURCE_FILE);
    unlink(TEMP_OUTPUT_FILE);
    
    /* Also clean up any dump files that might have been created */
    system("rm -f ./dump/*.i ./dump/*.s ./dump/*.o 2>/dev/null");
    system("rm -f *.i *.s *.o 2>/dev/null");
    system("rmdir ./dump 2>/dev/null");
}

/* Execute GCC with given arguments using fork/exec */
static int execute_gcc(const char *description, char *const argv[]) {
    printf("\n=== %s ===\n", description);
    printf("Command: ");
    for (int i = 0; argv[i] != NULL; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        execvp("gcc", argv);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Exit status: %d\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else {
            printf("Process terminated abnormally\n");
            return -1;
        }
    }
}

int main(void) {
    /* Create test source file */
    if (create_test_source() != 0) {
        return EXIT_FAILURE;
    }
    
    /* Ensure we have a dump directory */
    system("mkdir -p ./dump");
    
    int overall_status = 0;
    
    /* Test 1: Basic compilation with dumpdir and save-temps */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump/",
            "-dumpbase", "myprog",
            "-dumpbase-ext", ".c",
            "-save-temps",
            "-c", TEMP_SOURCE_FILE,
            "-o", TEMP_OUTPUT_FILE,
            NULL
        };
        execute_gcc("Test 1: Basic with dumpdir and save-temps", argv);
    }
    
    /* Test 2: Without dumpdir, but with dumpbase and dumpbase-ext */
    {
        char *argv[] = {
            "gcc",
            "-dumpbase", "test2",
            "-dumpbase-ext", ".c",
            "-save-temps=cwd",
            "-c", TEMP_SOURCE_FILE,
            "-o", "test2_output.o",
            NULL
        };
        execute_gcc("Test 2: Without dumpdir, with save-temps=cwd", argv);
        unlink("test2_output.o");
    }
    
    /* Test 3: All options combined */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump/",
            "-dumpbase", "fulltest",
            "-dumpbase-ext", ".c",
            "-save-temps=obj",
            "-c", TEMP_SOURCE_FILE,
            "-o", "full_output.o",
            NULL
        };
        execute_gcc("Test 3: All options with save-temps=obj", argv);
        unlink("full_output.o");
    }
    
    /* Test 4: Different dumpbase-ext values */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump/",
            "-dumpbase", "different_ext",
            "-dumpbase-ext", ".special",
            "-save-temps",
            "-c", TEMP_SOURCE_FILE,
            "-o", "diff_ext.o",
            NULL
        };
        execute_gcc("Test 4: Different dumpbase-ext value", argv);
        unlink("diff_ext.o");
    }
    
    /* Test 5: No dump options, just save-temps */
    {
        char *argv[] = {
            "gcc",
            "-save-temps",
            "-c", TEMP_SOURCE_FILE,
            "-o", "simple.o",
            NULL
        };
        execute_gcc("Test 5: Only save-temps", argv);
        unlink("simple.o");
    }
    
    /* Test 6: Error case - invalid option to test cleanup after error */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump/",
            "-dumpbase", "errortest",
            "-invalid-opt",  /* This should cause an error */
            "-c", TEMP_SOURCE_FILE,
            NULL
        };
        execute_gcc("Test 6: Invalid option to test error cleanup", argv);
    }
    
    /* Test 7: Multiple output specifications */
    {
        char *argv[] = {
            "gcc",
            "-dumpdir", "./dump/",
            "-dumpbase", "multitest",
            "-dumpbase-ext", ".c",
            "-save-temps",
            "-o", "custom_output_name.o",
            "-c", TEMP_SOURCE_FILE,
            NULL
        };
        execute_gcc("Test 7: Custom output name with all dump options", argv);
        unlink("custom_output_name.o");
    }
    
    /* Clean up */
    cleanup_files();
    
    printf("\n=== All tests completed ===\n");
    return overall_status;
}
