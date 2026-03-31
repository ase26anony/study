/* test_gcc_driver_cleanup.c
 * This test triggers the GCC driver's cleanup routine by invoking it
 * with various flags that set the state variables being reset in the
 * uncovered block (lines 11228-11250 of gcc.cc).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define SIMPLE_C_CONTENT "int main(void) { return 0; }\n"
#define ERROR_C_CONTENT "int main(void) { return undefined_var; }\n"

/* Create a simple C source file */
static int create_source_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fputs(content, f);
    fclose(f);
    return 1;
}

/* Remove a file if it exists */
static void remove_file(const char *filename) {
    if (access(filename, F_OK) == 0) {
        unlink(filename);
    }
}

/* Remove a directory and its contents */
static void remove_directory(const char *dirname) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dirname);
    system(cmd);
}

/* Execute a GCC command and return the exit status */
static int execute_gcc(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    printf("Exit status: %d\n\n", status);
    return status;
}

int main(void) {
    int overall_status = 0;
    
    /* Create necessary directories */
    mkdir("test_artifacts", 0755);
    mkdir("fail_artifacts", 0755);
    
    /* Create source files */
    if (!create_source_file("simple.c", SIMPLE_C_CONTENT)) {
        fprintf(stderr, "Failed to create simple.c\n");
        return 1;
    }
    
    if (!create_source_file("error.c", ERROR_C_CONTENT)) {
        fprintf(stderr, "Failed to create error.c\n");
        return 1;
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION A: Sets state variables with successful compilation
     * This sets:
     * - save_temps_flag via -save-temps
     * - dumpdir via -dumpdir
     * - dumpbase via -dumpbase
     * - outbase via -o
     * Compilation succeeds with -c flag
     */
    printf("Invocation A: Successful compilation with state variables set\n");
    execute_gcc("gcc -save-temps -dumpdir ./test_artifacts -dumpbase coverage_test "
                "-o test_output.o -c simple.c");
    
    /* INVOCATION B: Sets state variables with compilation failure
     * Uses invalid architecture to cause backend failure after driver initialization
     * This ensures cleanup runs with failure status
     */
    printf("Invocation B: Failed compilation with state variables set\n");
    execute_gcc("gcc -save-temps -dumpdir ./fail_artifacts -dumpbase fail_test "
                "-o fail_output.o -march=invalid-arch simple.c 2>/dev/null");
    
    /* INVOCATION C: Tests with -specs and -V flags
     * These may influence spec_machine and print_version
     */
    printf("Invocation C: Testing with -specs and -V flags\n");
    execute_gcc("gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5");
    
    /* INVOCATION D: Tests with -Werror to generate error exit status
     * This affects greatest_status variable
     */
    printf("Invocation D: Testing with -Werror for error status\n");
    execute_gcc("gcc -Werror -Wall -o warn_output error.c 2>/dev/null");
    
    /* INVOCATION E: Tests dumpbase_ext and outbase with different extensions
     * Uses -dumpbase-ext to set dumpbase_ext
     */
    printf("Invocation E: Testing dumpbase_ext and complex output names\n");
    execute_gcc("gcc -save-temps -dumpdir ./test_artifacts -dumpbase complex "
                "-dumpbase-ext .ext -o complex_output.obj -c simple.c");
    
    /* INVOCATION F: Tests with verbose flag (affects verbose_only_flag)
     * Uses -v to trigger verbose output
     */
    printf("Invocation F: Testing verbose flag\n");
    execute_gcc("gcc -v -c simple.c 2>&1 | head -10");
    
    /* INVOCATION G: Tests help flags (affects print_help_list, print_subprocess_help)
     */
    printf("Invocation G: Testing help flags\n");
    execute_gcc("gcc --help=common 2>&1 | head -5");
    
    /* INVOCATION H: Tests version printing (affects print_version)
     */
    printf("Invocation H: Testing version flag\n");
    execute_gcc("gcc --version 2>&1 | head -2");
    
    /* INVOCATION I: Tests with custom linker (affects use_ld)
     */
    printf("Invocation I: Testing custom linker specification\n");
    execute_gcc("gcc -fuse-ld=bfd -o linker_test simple.c 2>/dev/null");
    
    /* INVOCATION J: Tests timing report (affects report_times_to_file)
     */
    printf("Invocation J: Testing time report\n");
    execute_gcc("gcc -ftime-report -c simple.c 2>&1 | head -5");
    
    /* INVOCATION K: Tests sysroot flags (affects target_system_root, target_sysroot_suffix)
     */
    printf("Invocation K: Testing sysroot flags\n");
    execute_gcc("gcc --sysroot=/tmp -c simple.c 2>/dev/null");
    
    /* INVOCATION L: Multiple flags combination for comprehensive coverage
     */
    printf("Invocation L: Comprehensive flag combination\n");
    execute_gcc("gcc -save-temps=obj -dumpdir ./combo -dumpbase combo_test "
                "-dumpbase-ext .myext -o final_output -v -ftime-report "
                "--sysroot=/tmp -c simple.c 2>&1 | head -15");
    
    /* Clean up generated files */
    printf("\n=== Cleaning up test artifacts ===\n");
    
    /* Remove object and output files */
    remove_file("test_output.o");
    remove_file("fail_output.o");
    remove_file("warn_output");
    remove_file("linker_test");
    remove_file("final_output");
    remove_file("complex_output.obj");
    
    /* Remove temporary files from -save-temps */
    remove_file("simple.i");
    remove_file("simple.s");
    remove_file("error.i");
    remove_file("error.s");
    remove_file("complex.i");
    remove_file("complex.s");
    
    /* Remove source files */
    remove_file("simple.c");
    remove_file("error.c");
    
    /* Remove directories */
    remove_directory("test_artifacts");
    remove_directory("fail_artifacts");
    remove_directory("combo");
    
    printf("\nTest completed. The GCC driver's cleanup routine should have been\n");
    printf("executed multiple times with various state variable configurations.\n");
    
    return overall_status;
}
