/*
 * Test program to cover GCC driver cleanup lines in gcc.cc
 * This program invokes the GCC driver with various flags to ensure
 * the cleanup routine (lines 11228-11250) is executed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define SIMPLE_C_CONTENT "int main(void) { return 0; }\n"
#define ERROR_C_CONTENT "int main(void) { return undefined_var; }\n"

/* Create a simple C source file */
static int create_source_file(const char *filename, const char *content) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return 0;
    }
    fputs(content, fp);
    fclose(fp);
    return 1;
}

/* Remove a file if it exists */
static void remove_file(const char *filename) {
    if (access(filename, F_OK) == 0) {
        unlink(filename);
    }
}

/* Remove directory and its contents */
static void remove_directory(const char *dirpath) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dirpath);
    system(cmd);
}

/* Execute a GCC command and return its exit status */
static int run_gcc(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status == -1) {
        perror("system");
        return -1;
    }
    return WEXITSTATUS(status);
}

int main(void) {
    int overall_status = 0;
    
    /* Create directories for test artifacts */
    mkdir("./test_artifacts", 0755);
    mkdir("./fail_artifacts", 0755);
    mkdir("./dumpdir_test", 0755);
    
    /* Create test source files */
    if (!create_source_file("simple.c", SIMPLE_C_CONTENT)) {
        fprintf(stderr, "Failed to create simple.c\n");
        return 1;
    }
    
    if (!create_source_file("error.c", ERROR_C_CONTENT)) {
        fprintf(stderr, "Failed to create error.c\n");
        return 1;
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* 
     * Invocation 1: Successful compilation with -save-temps and dump options
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     */
    printf("1. Testing successful compilation with state variables set:\n");
    const char *cmd1 = "gcc -save-temps -dumpdir ./test_artifacts "
                       "-dumpbase coverage_test -o test_output.o -c simple.c";
    int status1 = run_gcc(cmd1);
    printf("Exit status: %d\n\n", status1);
    
    /*
     * Invocation 2: Compilation that fails after driver initialization
     * Uses invalid architecture flag to trigger backend failure
     */
    printf("2. Testing compilation failure after driver initialization:\n");
    const char *cmd2 = "gcc -save-temps -dumpdir ./fail_artifacts "
                       "-dumpbase fail_test -o fail_output.o "
                       "-march=invalid-arch simple.c 2>/dev/null";
    int status2 = run_gcc(cmd2);
    printf("Exit status: %d\n\n", status2);
    
    /*
     * Invocation 3: Using -specs and -V flags
     * This may influence spec_machine and print_version
     */
    printf("3. Testing with -specs and -V flags:\n");
    const char *cmd3 = "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5";
    int status3 = run_gcc(cmd3);
    printf("Exit status: %d\n\n", status3);
    
    /*
     * Invocation 4: Using -Werror to turn warnings into errors
     * This tests different failure path for greatest_status
     */
    printf("4. Testing with -Werror (warning as error):\n");
    const char *cmd4 = "gcc -Werror -save-temps -dumpdir ./dumpdir_test "
                       "-dumpbase werror_test -o werror.o error.c 2>/dev/null";
    int status4 = run_gcc(cmd4);
    printf("Exit status: %d\n\n", status4);
    
    /*
     * Invocation 5: Testing dumpbase_ext and outbase_length
     * Using -dumpbase-ext to set dumpbase_ext
     */
    printf("5. Testing with -dumpbase-ext:\n");
    const char *cmd5 = "gcc -save-temps -dumpdir ./test_artifacts "
                       "-dumpbase ext_test -dumpbase-ext .extra "
                       "-o long_output_name.o -c simple.c";
    int status5 = run_gcc(cmd5);
    printf("Exit status: %d\n\n", status5);
    
    /*
     * Invocation 6: Testing verbose flag and print_help_list
     * Using -v (verbose) and --help
     */
    printf("6. Testing verbose and help flags:\n");
    const char *cmd6 = "gcc -v --help=common 2>&1 | head -10";
    int status6 = run_gcc(cmd6);
    printf("Exit status: %d\n\n", status6);
    
    /*
     * Invocation 7: Testing target system root flags
     * Using --sysroot to influence target_system_root
     */
    printf("7. Testing with --sysroot flag:\n");
    const char *cmd7 = "gcc --sysroot=/tmp/nonexistent -c simple.c 2>/dev/null";
    int status7 = run_gcc(cmd7);
    printf("Exit status: %d\n\n", status7);
    
    /*
     * Invocation 8: Testing multiple flags together
     * Comprehensive test combining many state-setting flags
     */
    printf("8. Comprehensive test with multiple state variables:\n");
    const char *cmd8 = "gcc -save-temps=obj -dumpdir ./test_artifacts "
                       "-dumpbase comprehensive -dumpbase-ext .test "
                       "-o comp_output.o --verbose -c simple.c 2>&1 | tail -5";
    int status8 = run_gcc(cmd8);
    printf("Exit status: %d\n\n", status8);
    
    /* Clean up generated files */
    printf("Cleaning up test files...\n");
    
    /* Remove source files */
    remove_file("simple.c");
    remove_file("error.c");
    
    /* Remove object and temporary files */
    remove_file("test_output.o");
    remove_file("fail_output.o");
    remove_file("werror.o");
    remove_file("long_output_name.o");
    remove_file("comp_output.o");
    
    /* Remove save-temps files */
    remove_file("simple.i");
    remove_file("simple.s");
    remove_file("error.i");
    remove_file("error.s");
    remove_file("coverage_test.*");
    remove_file("fail_test.*");
    remove_file("ext_test.*");
    remove_file("werror_test.*");
    remove_file("comprehensive.*");
    
    /* Remove test directories */
    remove_directory("./test_artifacts");
    remove_directory("./fail_artifacts");
    remove_directory("./dumpdir_test");
    
    printf("\n=== Test completed ===\n");
    printf("All GCC invocations attempted. Driver cleanup should have been triggered.\n");
    
    return overall_status;
}
