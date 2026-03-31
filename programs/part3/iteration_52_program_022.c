#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_source.c"
#define TEMP_OUTPUT_FILE "test_coverage_output.o"

/* Create a simple valid C source file for compilation */
static int create_test_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create source file %s: %s\n", 
                filename, strerror(errno));
        return -1;
    }
    
    fprintf(fp, "/* Test source for GCC coverage */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    printf(\"Hello from coverage test!\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/* Execute GCC with given arguments using fork/exec */
static int run_gcc(const char **argv) {
    pid_t pid = fork();
    
    if (pid < 0) {
        fprintf(stderr, "fork failed: %s\n", strerror(errno));
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        execvp("gcc", (char * const *)argv);
        fprintf(stderr, "execvp failed: %s\n", strerror(errno));
        _exit(127);
    }
    
    /* Parent process */
    int status;
    if (waitpid(pid, &status, 0) < 0) {
        fprintf(stderr, "waitpid failed: %s\n", strerror(errno));
        return -1;
    }
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    
    return -1;
}

/* Build command line arguments array */
static void build_args(const char ***argv_ptr, int *argc_ptr, 
                      const char *source_file, const char *output_file,
                      const char *dumpdir, const char *dumpbase, 
                      const char *dumpbase_ext, const char *save_temps,
                      int use_o) {
    int argc = 0;
    const char **argv = malloc(20 * sizeof(char *));
    
    argv[argc++] = "gcc";
    
    if (dumpdir) {
        argv[argc++] = "-dumpdir";
        argv[argc++] = dumpdir;
    }
    
    if (dumpbase) {
        argv[argc++] = "-dumpbase";
        argv[argc++] = dumpbase;
    }
    
    if (dumpbase_ext) {
        argv[argc++] = "-dumpbase-ext";
        argv[argc++] = dumpbase_ext;
    }
    
    if (save_temps) {
        argv[argc++] = save_temps;
    }
    
    argv[argc++] = "-c";
    argv[argc++] = source_file;
    
    if (use_o && output_file) {
        argv[argc++] = "-o";
        argv[argc++] = output_file;
    }
    
    argv[argc] = NULL;
    
    *argv_ptr = argv;
    *argc_ptr = argc;
}

/* Print command for traceability */
static void print_command(const char **argv) {
    fprintf(stderr, "Running: ");
    for (int i = 0; argv[i]; i++) {
        fprintf(stderr, "%s ", argv[i]);
    }
    fprintf(stderr, "\n");
}

int main(void) {
    int ret;
    
    /* Step 1: Create temporary source file */
    if (create_test_source(TEMP_SOURCE_FILE) < 0) {
        return 1;
    }
    
    fprintf(stderr, "Created test source file: %s\n", TEMP_SOURCE_FILE);
    
    /* Step 2: Multiple GCC invocations with different option combinations */
    
    /* Invocation 1: With dumpdir and save-temps */
    fprintf(stderr, "\n=== Invocation 1: -dumpdir and -save-temps ===\n");
    const char **argv1;
    int argc1;
    build_args(&argv1, &argc1, TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE,
               "./dump/", NULL, NULL, "-save-temps=cwd", 1);
    print_command(argv1);
    ret = run_gcc(argv1);
    fprintf(stderr, "Exit code: %d\n", ret);
    free(argv1);
    
    /* Invocation 2: With dumpbase and dumpbase-ext, no dumpdir */
    fprintf(stderr, "\n=== Invocation 2: -dumpbase and -dumpbase-ext ===\n");
    const char **argv2;
    int argc2;
    build_args(&argv2, &argc2, TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE,
               NULL, "myprog", ".c", NULL, 1);
    print_command(argv2);
    ret = run_gcc(argv2);
    fprintf(stderr, "Exit code: %d\n", ret);
    free(argv2);
    
    /* Invocation 3: All options combined */
    fprintf(stderr, "\n=== Invocation 3: All options combined ===\n");
    const char **argv3;
    int argc3;
    build_args(&argv3, &argc3, TEMP_SOURCE_FILE, "custom_output.o",
               "./full_dump/", "full_prog", ".ext", "-save-temps", 1);
    print_command(argv3);
    ret = run_gcc(argv3);
    fprintf(stderr, "Exit code: %d\n", ret);
    free(argv3);
    
    /* Invocation 4: Minimal options, just to ensure cleanup path */
    fprintf(stderr, "\n=== Invocation 4: Minimal compilation ===\n");
    const char **argv4;
    int argc4;
    build_args(&argv4, &argc4, TEMP_SOURCE_FILE, NULL,
               NULL, NULL, NULL, NULL, 0);
    print_command(argv4);
    ret = run_gcc(argv4);
    fprintf(stderr, "Exit code: %d\n", ret);
    free(argv4);
    
    /* Invocation 5: Trigger error exit with invalid option, then cleanup */
    fprintf(stderr, "\n=== Invocation 5: Invalid option test ===\n");
    const char *argv5[] = {
        "gcc",
        "-invalid-opt",  /* This should cause an error exit */
        "-dumpdir", "./error_dump/",
        "-dumpbase", "error_prog",
        "-dumpbase-ext", ".err",
        "-save-temps",
        "-c", TEMP_SOURCE_FILE,
        "-o", "error_output.o",
        NULL
    };
    print_command(argv5);
    ret = run_gcc(argv5);
    fprintf(stderr, "Exit code: %d (expected non-zero)\n", ret);
    
    /* Invocation 6: Different dumpdir format with trailing slash handling */
    fprintf(stderr, "\n=== Invocation 6: Dumpdir with special format ===\n");
    const char **argv6;
    int argc6;
    build_args(&argv6, &argc6, TEMP_SOURCE_FILE, "special.o",
               "dumpdir-", "special", ".c", "-save-temps=cwd", 1);
    print_command(argv6);
    ret = run_gcc(argv6);
    fprintf(stderr, "Exit code: %d\n", ret);
    free(argv6);
    
    /* Step 3: Clean up temporary files */
    fprintf(stderr, "\nCleaning up temporary files...\n");
    unlink(TEMP_SOURCE_FILE);
    
    /* Also clean up any output files created */
    unlink(TEMP_OUTPUT_FILE);
    unlink("custom_output.o");
    unlink("error_output.o");
    unlink("special.o");
    
    /* Clean up save-temps files if they exist */
    unlink("test_coverage_source.i");
    unlink("test_coverage_source.s");
    unlink("myprog.i");
    unlink("myprog.s");
    unlink("full_prog.i");
    unlink("full_prog.s");
    unlink("special.i");
    unlink("special.s");
    
    fprintf(stderr, "Test completed successfully.\n");
    return 0;
}
