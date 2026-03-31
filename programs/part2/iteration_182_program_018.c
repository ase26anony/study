/* test_gcc_driver_cleanup.c
 * This test triggers the GCC driver's cleanup routine with specific state variables set.
 * It invokes GCC multiple times with different flags to ensure the uncovered lines are executed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define SIMPLE_C_CONTENT "int main(void) { return 0; }\n"
#define INVALID_ARCH_C_CONTENT "int main(void) { return 0; }\n"
#define WARNING_C_CONTENT "int main(void) { int x; return 0; }\n"

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

/* Execute a GCC command and capture its return code */
static int run_gcc_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int ret = system(cmd);
    if (ret == -1) {
        perror("system");
        return -1;
    }
    return WEXITSTATUS(ret);
}

/* Clean up generated files */
static void cleanup_files(void) {
    /* Remove source files */
    remove("simple.c");
    remove("invalid_arch.c");
    remove("warning.c");
    
    /* Remove object files */
    remove("test_output.o");
    remove("fail_output.o");
    remove("warning_output.o");
    
    /* Remove dump files */
    remove("coverage_test.*");
    remove("fail_test.*");
    remove("warning_test.*");
    
    /* Remove temp directories */
    system("rm -rf ./test_artifacts ./fail_artifacts ./warning_artifacts");
    
    /* Remove any other generated files */
    remove("output");
    remove("a.out");
}

int main(void) {
    int overall_result = 0;
    
    /* Create necessary directories */
    mkdir("./test_artifacts", 0755);
    mkdir("./fail_artifacts", 0755);
    mkdir("./warning_artifacts", 0755);
    
    /* Create source files */
    if (!create_source_file("simple.c", SIMPLE_C_CONTENT)) {
        fprintf(stderr, "Failed to create simple.c\n");
        return 1;
    }
    
    if (!create_source_file("invalid_arch.c", INVALID_ARCH_C_CONTENT)) {
        fprintf(stderr, "Failed to create invalid_arch.c\n");
        return 1;
    }
    
    if (!create_source_file("warning.c", WARNING_C_CONTENT)) {
        fprintf(stderr, "Failed to create warning.c\n");
        return 1;
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION A: Sets state with successful compilation
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Successful compilation with -c flag
     */
    printf("1. Testing successful compilation with state variables set:\n");
    const char *cmd_a = "gcc -save-temps -dumpdir ./test_artifacts "
                       "-dumpbase coverage_test -o test_output.o -c simple.c";
    int result_a = run_gcc_command(cmd_a);
    printf("Result: %d\n\n", result_a);
    
    /* INVOCATION B: Sets state with compilation failure
     * Uses invalid architecture to cause backend failure after driver initialization
     */
    printf("2. Testing failed compilation with state variables set:\n");
    const char *cmd_b = "gcc -save-temps -dumpdir ./fail_artifacts "
                       "-dumpbase fail_test -o fail_output.o "
                       "-march=invalid-arch invalid_arch.c 2>/dev/null";
    int result_b = run_gcc_command(cmd_b);
    printf("Result: %d\n\n", result_b);
    
    /* INVOCATION C: Tests with -specs and -V flags
     * Influences spec_machine and print_version
     */
    printf("3. Testing with specs and version flags:\n");
    const char *cmd_c = "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5";
    int result_c = run_gcc_command(cmd_c);
    printf("Result: %d\n\n", result_c);
    
    /* INVOCATION D: Tests with -Werror to generate error from warning
     * Tests greatest_status with different error code
     */
    printf("4. Testing with -Werror to turn warnings into errors:\n");
    const char *cmd_d = "gcc -save-temps -dumpdir ./warning_artifacts "
                       "-dumpbase warning_test -o warning_output.o "
                       "-Werror -Wall warning.c 2>/dev/null";
    int result_d = run_gcc_command(cmd_d);
    printf("Result: %d\n\n", result_d);
    
    /* INVOCATION E: Tests dumpbase_ext and outbase with different extensions
     * Uses -dumpbase-ext to set dumpbase_ext
     */
    printf("5. Testing with dumpbase_ext and complex output names:\n");
    const char *cmd_e = "gcc -save-temps=obj -dumpdir . "
                       "-dumpbase complex -dumpbase-ext .extra "
                       "-o complex_output.o -c simple.c";
    int result_e = run_gcc_command(cmd_e);
    printf("Result: %d\n\n", result_e);
    
    /* INVOCATION F: Tests verbose flag and print_subprocess_help
     * Uses -v and --help=common
     */
    printf("6. Testing verbose and help flags:\n");
    const char *cmd_f = "gcc -v --help=common simple.c 2>&1 | head -10";
    int result_f = run_gcc_command(cmd_f);
    printf("Result: %d\n\n", result_f);
    
    /* INVOCATION G: Tests with -print-version and -print-prog-name
     * Sets print_version flag
     */
    printf("7. Testing print version and prog name:\n");
    const char *cmd_g = "gcc -print-version -print-prog-name=cc1 2>&1";
    int result_g = run_gcc_command(cmd_g);
    printf("Result: %d\n\n", result_g);
    
    /* INVOCATION H: Tests target system root flags
     * May influence target_system_root and related variables
     */
    printf("8. Testing with sysroot flags:\n");
    const char *cmd_h = "gcc --sysroot=/tmp/nonexistent -c simple.c 2>/dev/null";
    int result_h = run_gcc_command(cmd_h);
    printf("Result: %d\n\n", result_h);
    
    /* Clean up generated files */
    printf("Cleaning up test files...\n");
    cleanup_files();
    
    printf("=== Test completed ===\n");
    printf("All GCC invocations attempted. Check coverage data for gcc.cc\n");
    
    /* Always return 0 to indicate test was run successfully
     * The individual GCC invocations may fail, but that's expected
     * and necessary to trigger different cleanup paths
     */
    return 0;
}
