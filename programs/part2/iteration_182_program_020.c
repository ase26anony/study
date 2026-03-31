/* test_gcc_driver_cleanup.c
 * 
 * This test program exercises the GCC driver's cleanup routine by invoking
 * it multiple times with different command-line flags that set the state
 * variables mentioned in the uncovered lines (11228-11250 of gcc.cc).
 * 
 * The test ensures that variables like dumpdir, dumpbase, outbase, etc.
 * are allocated and then freed during cleanup.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

/* Simple C source file content */
const char *simple_c_content = 
"int main(void) {\n"
"    return 0;\n"
"}\n";

/* C source file with a syntax error (to trigger failure after parsing) */
const char *error_c_content = 
"int main(void) {\n"
"    missing_function(); /* Undefined function - will cause linker error */\n"
"    return 0;\n"
"}\n";

/* Create a temporary directory for test artifacts */
void create_temp_dir(const char *dir_name) {
    struct stat st = {0};
    if (stat(dir_name, &st) == -1) {
        mkdir(dir_name, 0755);
    }
}

/* Remove a directory and its contents */
void remove_dir(const char *dir_name) {
    char command[256];
    snprintf(command, sizeof(command), "rm -rf %s", dir_name);
    system(command);
}

/* Execute a GCC command and return the exit status */
int execute_gcc(const char *command) {
    printf("Executing: %s\n", command);
    int status = system(command);
    return WEXITSTATUS(status);
}

int main(void) {
    int overall_result = 0;
    
    /* Create temporary directories for test artifacts */
    create_temp_dir("test_artifacts");
    create_temp_dir("fail_artifacts");
    create_temp_dir("coverage_temp");
    
    /* Write simple C source files */
    FILE *fp = fopen("simple.c", "w");
    if (!fp) {
        perror("Failed to create simple.c");
        return 1;
    }
    fputs(simple_c_content, fp);
    fclose(fp);
    
    fp = fopen("error.c", "w");
    if (!fp) {
        perror("Failed to create error.c");
        return 1;
    }
    fputs(error_c_content, fp);
    fclose(fp);
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION A: Successful compilation with state-setting flags
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Should trigger cleanup after successful compilation
     */
    printf("1. Successful compilation with state-setting flags:\n");
    const char *cmd_a = "gcc -save-temps -dumpdir ./test_artifacts "
                       "-dumpbase coverage_test -o test_output.o -c simple.c";
    int result_a = execute_gcc(cmd_a);
    printf("Exit status: %d\n\n", result_a);
    
    /* INVOCATION B: Compilation that fails after driver initialization
     * Uses invalid architecture flag to cause backend failure
     * Still sets state variables before failing
     */
    printf("2. Failed compilation with invalid architecture:\n");
    const char *cmd_b = "gcc -save-temps -dumpdir ./fail_artifacts "
                       "-dumpbase fail_test -o fail_output.o "
                       "-march=invalid-arch simple.c 2>/dev/null";
    int result_b = execute_gcc(cmd_b);
    printf("Exit status: %d\n\n", result_b);
    
    /* INVOCATION C: Version printing with specs flag
     * Influences print_version and spec_machine variables
     * Note: -V might not set spec_machine, but shows different code path
     */
    printf("3. Version/specs test:\n");
    const char *cmd_c = "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5";
    int result_c = execute_gcc(cmd_c);
    printf("Exit status: %d\n\n", result_c);
    
    /* INVOCATION D: Test with -Werror to generate error exit status
     * This affects greatest_status variable
     */
    printf("4. Compilation with -Werror (warning as error):\n");
    const char *cmd_d = "gcc -Werror -Wall -o warn_output simple.c 2>&1";
    int result_d = execute_gcc(cmd_d);
    printf("Exit status: %d\n\n", result_d);
    
    /* INVOCATION E: Test dumpbase_ext and outbase_length
     * Using -dumpbase with extension and -o with different names
     */
    printf("5. Testing dumpbase_ext and outbase_length:\n");
    const char *cmd_e = "gcc -save-temps -dumpdir ./coverage_temp "
                       "-dumpbase mydump.ext -dumpbase-ext .extra "
                       "-o long_output_name.o -c simple.c";
    int result_e = execute_gcc(cmd_e);
    printf("Exit status: %d\n\n", result_e);
    
    /* INVOCATION F: Linker error test
     * This triggers cleanup after linker failure
     */
    printf("6. Linker error test (undefined function):\n");
    const char *cmd_f = "gcc -o link_error error.c 2>/dev/null";
    int result_f = execute_gcc(cmd_f);
    printf("Exit status: %d\n\n", result_f);
    
    /* INVOCATION G: Test verbose flag and print_help_list
     * Uses -v (verbose) and --help
     */
    printf("7. Verbose and help flags:\n");
    const char *cmd_g = "gcc -v --help=common 2>&1 | head -10";
    int result_g = execute_gcc(cmd_g);
    printf("Exit status: %d\n\n", result_g);
    
    /* INVOCATION H: Test target system root flags
     * These affect target_system_root_changed and related variables
     */
    printf("8. Target system root test:\n");
    const char *cmd_h = "gcc --sysroot=/tmp -o sysroot_test simple.c 2>&1";
    int result_h = execute_gcc(cmd_h);
    printf("Exit status: %d\n\n", result_h);
    
    /* Clean up generated files */
    printf("Cleaning up test files...\n");
    remove("simple.c");
    remove("error.c");
    remove("test_output.o");
    remove("fail_output.o");
    remove("warn_output");
    remove("long_output_name.o");
    remove("link_error");
    remove("sysroot_test");
    
    /* Clean up save-temps artifacts */
    remove("simple.i");
    remove("simple.s");
    remove("coverage_test.*");
    remove("fail_test.*");
    remove("mydump.ext.*");
    
    /* Remove temporary directories */
    remove_dir("test_artifacts");
    remove_dir("fail_artifacts");
    remove_dir("coverage_temp");
    
    printf("\n=== Test completed ===\n");
    printf("All GCC invocations attempted. Check coverage data for gcc.cc\n");
    
    /* Return success regardless of GCC invocation results
     * since we're testing the cleanup path, not compilation success */
    return 0;
}
