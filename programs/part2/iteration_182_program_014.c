/* test_gcc_driver_cleanup.c
 * This test triggers the GCC driver's cleanup routine by invoking it
 * with various command-line options that set the state variables
 * that are reset in the uncovered block (lines 11228-11250).
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
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return 0;
    }
    fputs(content, fp);
    fclose(fp);
    return 1;
}

/* Execute a GCC command and return its exit status */
static int run_gcc_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status == -1) {
        perror("system");
        return -1;
    }
    return WEXITSTATUS(status);
}

/* Clean up generated files */
static void cleanup_files(void) {
    /* Remove source files */
    remove("simple.c");
    remove("error.c");
    
    /* Remove object files and temporary files */
    remove("test_output.o");
    remove("fail_output.o");
    remove("output_prog");
    
    /* Remove dump files and directories */
    system("rm -rf ./test_artifacts ./fail_artifacts ./temp_dir");
    system("rm -f *.i *.s *.o coverage_test.* fail_test.* mydump.*");
    system("rm -f test_output.* fail_output.*");
}

int main(void) {
    int overall_result = 0;
    
    /* Create necessary directories */
    mkdir("./test_artifacts", 0755);
    mkdir("./fail_artifacts", 0755);
    mkdir("./temp_dir", 0755);
    
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
    
    /* INVOCATION A: Sets state with successful compilation
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Successful compilation triggers cleanup with greatest_status=0
     */
    printf("1. Successful compilation with state variables set:\n");
    int status_a = run_gcc_command(
        "gcc -save-temps -dumpdir ./test_artifacts "
        "-dumpbase coverage_test -o test_output.o -c simple.c"
    );
    printf("Exit status: %d\n\n", status_a);
    
    /* INVOCATION B: Sets state with compilation failure
     * Uses invalid architecture flag to cause backend failure
     * after driver initialization
     */
    printf("2. Failed compilation with state variables set:\n");
    int status_b = run_gcc_command(
        "gcc -save-temps -dumpdir ./fail_artifacts "
        "-dumpbase fail_test -o fail_output.o -march=invalid-arch simple.c 2>/dev/null"
    );
    printf("Exit status: %d\n\n", status_b);
    
    /* INVOCATION C: Uses -specs= to influence spec_machine
     * Also uses -V to set print_version
     */
    printf("3. Version/specs test (will likely fail):\n");
    int status_c = run_gcc_command(
        "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5"
    );
    printf("Exit status: %d\n\n", status_c);
    
    /* INVOCATION D: Tests dumpbase_ext and outbase with linking
     * Uses -dumpbase with extension and -o with executable name
     */
    printf("4. Test with dumpbase_ext and executable output:\n");
    int status_d = run_gcc_command(
        "gcc -dumpbase mydump.c -dumpbase-ext .ext "
        "-o output_prog simple.c 2>&1"
    );
    printf("Exit status: %d\n\n", status_d);
    
    /* INVOCATION E: Tests save_temps overrides and dumpdir
     * Uses -save-temps=obj and explicit dumpdir
     */
    printf("5. Test save-temps overrides:\n");
    int status_e = run_gcc_command(
        "gcc -save-temps=obj -dumpdir ./temp_dir "
        "-dumpbase temp_test -c simple.c 2>&1"
    );
    printf("Exit status: %d\n\n", status_e);
    
    /* INVOCATION F: Tests verbose flag and help
     * Sets verbose_only_flag and print_help_list
     */
    printf("6. Test verbose and help flags:\n");
    int status_f = run_gcc_command(
        "gcc -v --help=common simple.c 2>&1 | head -10"
    );
    printf("Exit status: %d\n\n", status_f);
    
    /* INVOCATION G: Tests with syntax error (late failure)
     * Driver parses args successfully, compiler proper fails
     */
    printf("7. Test with compilation error (undefined variable):\n");
    int status_g = run_gcc_command(
        "gcc -save-temps -dumpdir ./test_artifacts "
        "-o error_output.o -c error.c 2>&1"
    );
    printf("Exit status: %d\n\n", status_g);
    
    /* INVOCATION H: Tests -Werror turning warnings into errors
     * Tests greatest_status with warning-as-error
     */
    printf("8. Test with -Werror (warning as error):\n");
    int status_h = run_gcc_command(
        "gcc -Werror -Wall -o warn_output.o -c simple.c 2>&1"
    );
    printf("Exit status: %d\n\n", status_h);
    
    /* INVOCATION I: Tests target system root variables
     * May influence target_system_root_changed
     */
    printf("9. Test with sysroot flags:\n");
    int status_i = run_gcc_command(
        "gcc --sysroot=/tmp -o sysroot_output.o -c simple.c 2>&1"
    );
    printf("Exit status: %d\n\n", status_i);
    
    /* INVOCATION J: Final test with multiple state variables
     * Comprehensive test hitting many uncovered variables
     */
    printf("10. Comprehensive state test:\n");
    int status_j = run_gcc_command(
        "gcc -save-temps -dumpdir . -dumpbase final "
        "-dumpbase-ext .test -o final.o -v -c simple.c 2>&1 | tail -5"
    );
    printf("Exit status: %d\n\n", status_j);
    
    printf("=== All GCC invocations completed ===\n");
    printf("Note: Some invocations are expected to fail.\n");
    printf("The goal is to trigger driver cleanup, not successful compilation.\n\n");
    
    /* Clean up generated files */
    printf("Cleaning up test files...\n");
    cleanup_files();
    
    return overall_result;
}
