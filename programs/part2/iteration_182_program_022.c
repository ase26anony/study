/* test_gcc_driver_cleanup.c
 * 
 * This test program exercises the GCC driver's cleanup routine by invoking
 * it multiple times with different command-line flags that set the state
 * variables mentioned in the uncovered lines (11228-11250 of gcc.cc).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Simple C source file content */
const char *simple_c_content = 
"int main(void) {\n"
"    return 0;\n"
"}\n";

/* Create a simple C source file */
int create_simple_c_file(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create simple.c");
        return 0;
    }
    fprintf(fp, "%s", simple_c_content);
    fclose(fp);
    return 1;
}

/* Run a GCC command and capture its exit status */
int run_gcc_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Clean up generated files */
void cleanup_files(void) {
    /* Remove temporary files */
    system("rm -f simple.c simple.o simple.i simple.s");
    system("rm -f test_output.o test_output.i test_output.s");
    system("rm -f fail_output.o fail_output.i fail_output.s");
    system("rm -f output.o output.i output.s");
    
    /* Remove dump directories */
    system("rm -rf ./test_artifacts ./fail_artifacts ./temp");
    
    /* Remove any other generated files */
    system("rm -f *.o *.i *.s coverage_test.* fail_test.*");
}

int main(void) {
    int overall_status = 0;
    
    /* Create the simple C source file */
    if (!create_simple_c_file("simple.c")) {
        return 1;
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* Invocation A: Sets state and succeeds with -c (compile only) */
    printf("--- Invocation A: Successful compilation with state variables set ---\n");
    const char *cmd_a = 
        "gcc -save-temps -dumpdir ./test_artifacts "
        "-dumpbase coverage_test -o test_output.o -c simple.c";
    run_gcc_command(cmd_a);
    
    /* Invocation B: Sets state and fails with invalid architecture */
    printf("--- Invocation B: Late failure with state variables set ---\n");
    const char *cmd_b = 
        "gcc -save-temps -dumpdir ./fail_artifacts "
        "-dumpbase fail_test -o fail_output.o -march=invalid-arch simple.c 2>/dev/null";
    run_gcc_command(cmd_b);
    
    /* Invocation C: Uses -specs and -V to influence spec_machine and print_version */
    printf("--- Invocation C: Testing with -specs and -V flags ---\n");
    const char *cmd_c = 
        "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5";
    run_gcc_command(cmd_c);
    
    /* Invocation D: Tests -Werror turning warnings into errors */
    printf("--- Invocation D: Testing -Werror for error status ---\n");
    const char *cmd_d = 
        "gcc -save-temps -dumpdir ./temp -dumpbase warn_test "
        "-o output.o -Werror -Wunused-parameter -c simple.c 2>/dev/null";
    run_gcc_command(cmd_d);
    
    /* Invocation E: Tests dumpbase_ext and outbase with different extensions */
    printf("--- Invocation E: Testing dumpbase_ext and outbase ---\n");
    const char *cmd_e = 
        "gcc -save-temps -dumpdir . -dumpbase ext_test -dumpbase-ext .foo "
        "-o different_output.o -c simple.c";
    run_gcc_command(cmd_e);
    
    /* Invocation F: Tests with verbose flag to set verbose_only_flag */
    printf("--- Invocation F: Testing verbose flag ---\n");
    const char *cmd_f = 
        "gcc -v -c simple.c 2>&1 | head -10";
    run_gcc_command(cmd_f);
    
    /* Invocation G: Tests with --help to set print_help_list */
    printf("--- Invocation G: Testing help flag ---\n");
    const char *cmd_g = 
        "gcc --help 2>&1 | head -5";
    run_gcc_command(cmd_g);
    
    /* Invocation H: Tests with --version to set print_version */
    printf("--- Invocation H: Testing version flag ---\n");
    const char *cmd_h = 
        "gcc --version 2>&1 | head -2";
    run_gcc_command(cmd_h);
    
    /* Invocation I: Tests with -print-prog-name to potentially affect state */
    printf("--- Invocation I: Testing -print-prog-name ---\n");
    const char *cmd_i = 
        "gcc -print-prog-name=cc1 2>&1";
    run_gcc_command(cmd_i);
    
    /* Invocation J: Tests save-temps=obj to set save_temps_flag differently */
    printf("--- Invocation J: Testing save-temps=obj ---\n");
    const char *cmd_j = 
        "gcc -save-temps=obj -dumpdir ./temp -o obj_output.o -c simple.c";
    run_gcc_command(cmd_j);
    
    /* Clean up generated files */
    printf("--- Cleaning up generated files ---\n");
    cleanup_files();
    
    printf("=== Test completed ===\n");
    printf("The GCC driver's cleanup routine should have been executed multiple times,\n");
    printf("resetting variables like:\n");
    printf("  - save_temps_flag (set by -save-temps)\n");
    printf("  - dumpdir, dumpbase, outbase (allocated by corresponding flags)\n");
    printf("  - spec_machine (potentially affected by -specs)\n");
    printf("  - print_version, print_help_list (set by --version/--help)\n");
    printf("  - greatest_status (tracking subprocess exit codes)\n");
    
    return overall_status;
}
