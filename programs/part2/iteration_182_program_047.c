/* test_gcc_driver_cleanup.c
 * This test triggers the GCC driver's cleanup routine to cover lines 11228-11250 in gcc.cc
 * It invokes GCC multiple times with different flags to set and reset the target variables.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Create a simple C source file for testing */
void create_simple_c_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create test C file");
        exit(1);
    }
    
    fprintf(f, "/* Simple test file for GCC driver cleanup coverage */\n");
    fprintf(f, "int main(void) {\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
}

/* Create a C source file with a syntax error for failure testing */
void create_error_c_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create error test C file");
        exit(1);
    }
    
    fprintf(f, "/* C file with syntax error for failure testing */\n");
    fprintf(f, "int main(void) {\n");
    fprintf(f, "    missing_semicolon /* This will cause a compilation error */\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
}

/* Clean up generated files */
void cleanup_files(void) {
    /* Remove temporary files that might have been created */
    system("rm -f simple.c error.c test_output.o fail_output.o output.o 2>/dev/null");
    system("rm -f *.i *.s *.o 2>/dev/null");
    system("rm -rf ./test_artifacts ./fail_artifacts 2>/dev/null");
    system("rm -f coverage_test.* fail_test.* 2>/dev/null");
}

/* Run a GCC command and return the exit status */
int run_gcc_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    } else {
        printf("Command terminated abnormally\n\n");
        return -1;
    }
}

int main(void) {
    int overall_status = 0;
    
    /* Clean up any existing files first */
    cleanup_files();
    
    /* Create test source files */
    create_simple_c_file("simple.c");
    create_error_c_file("error.c");
    
    /* Create directories for dumpdir */
    mkdir("./test_artifacts", 0755);
    mkdir("./fail_artifacts", 0755);
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* 
     * Invocation 1: Sets state variables and succeeds
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Uses -c to stop before linking (successful compilation)
     */
    printf("Test 1: Successful compilation with state variables set\n");
    const char *cmd1 = "gcc -save-temps -dumpdir ./test_artifacts "
                      "-dumpbase coverage_test -o test_output.o -c simple.c";
    run_gcc_command(cmd1);
    
    /* 
     * Invocation 2: Sets different state variables and fails
     * Uses invalid architecture flag to cause backend failure
     * This ensures cleanup runs with failure status
     */
    printf("Test 2: Failed compilation with different state variables\n");
    const char *cmd2 = "gcc -save-temps=obj -dumpdir ./fail_artifacts "
                      "-dumpbase fail_test -o fail_output.o "
                      "-march=invalid-arch-that-doesnt-exist simple.c 2>/dev/null";
    run_gcc_command(cmd2);
    
    /* 
     * Invocation 3: Tests with -specs and -V flags
     * These influence spec_machine and print_version
     * Note: -specs with non-existent file will cause error
     */
    printf("Test 3: Testing with -specs and -V flags\n");
    const char *cmd3 = "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5";
    run_gcc_command(cmd3);
    
    /* 
     * Invocation 4: Tests -Werror turning warnings into errors
     * Uses a file with an intentional warning/error
     */
    printf("Test 4: Testing -Werror for different failure mode\n");
    const char *cmd4 = "gcc -Werror -Wall -o output.o -c error.c 2>/dev/null";
    run_gcc_command(cmd4);
    
    /* 
     * Invocation 5: Tests multiple dumpbase options
     * Uses -dumpbase-ext and different combinations
     */
    printf("Test 5: Testing dumpbase extensions\n");
    const char *cmd5 = "gcc -dumpdir ./test_artifarts -dumpbase mytest "
                      "-dumpbase-ext .ext -o output.o -c simple.c";
    run_gcc_command(cmd5);
    
    /* 
     * Invocation 6: Tests target system root flags
     * These influence target_system_root and related variables
     */
    printf("Test 6: Testing target system root flags\n");
    const char *cmd6 = "gcc --sysroot=/tmp/nonexistent -o output.o -c simple.c 2>/dev/null";
    run_gcc_command(cmd6);
    
    /* 
     * Invocation 7: Tests verbose and help flags
     * These set print_help_list, print_version, verbose_only_flag
     */
    printf("Test 7: Testing verbose and help flags\n");
    const char *cmd7 = "gcc --verbose -### simple.c 2>&1 | head -10";
    run_gcc_command(cmd7);
    
    /* 
     * Invocation 8: Tests time reporting
     * Sets report_times_to_file
     */
    printf("Test 8: Testing time reporting flag\n");
    const char *cmd8 = "gcc -ftime-report -o output.o -c simple.c 2>/dev/null";
    run_gcc_command(cmd8);
    
    /* 
     * Invocation 9: Tests linker selection
     * Sets use_ld variable
     */
    printf("Test 9: Testing linker selection\n");
    const char *cmd9 = "gcc -fuse-ld=bfd -o output simple.c 2>&1 | head -5";
    run_gcc_command(cmd9);
    
    /* 
     * Invocation 10: Tests save-temps with different options
     * Tests save_temps_flag variations
     */
    printf("Test 10: Testing save-temps variations\n");
    const char *cmd10 = "gcc -save-temps=cwd -o output.o -c simple.c";
    run_gcc_command(cmd10);
    
    /* Clean up generated files */
    cleanup_files();
    
    printf("=== All tests completed ===\n");
    printf("Note: Some tests are expected to fail - this is intentional to trigger\n");
    printf("different cleanup paths in the GCC driver.\n");
    
    return overall_status;
}
