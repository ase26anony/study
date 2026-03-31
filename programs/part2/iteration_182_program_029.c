/* test_gcc_driver_cleanup.c
 * This test triggers the GCC driver's cleanup routine by invoking it
 * with various flags that set the state variables being reset in the
 * uncovered lines (11228-11250 of gcc.cc).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define SIMPLE_C_CONTENT "int main(void) { return 0; }\n"
#define INVALID_C_CONTENT "int main(void) { invalid syntax here; return 0; }\n"

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

/* Execute a GCC command and return its exit status */
static int run_gcc_command(const char *command) {
    printf("Executing: %s\n", command);
    int status = system(command);
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
    remove("invalid.c");
    
    /* Remove object files and temporary files */
    remove("test_output.o");
    remove("fail_output.o");
    remove("output_prog");
    
    /* Remove dump files and directories */
    system("rm -rf ./test_artifacts");
    system("rm -rf ./fail_artifacts");
    system("rm -rf ./dumpdir_test");
    
    /* Remove save-temps files */
    remove("simple.i");
    remove("simple.s");
    remove("simple.o");
    remove("invalid.i");
    remove("invalid.s");
    remove("invalid.o");
    remove("coverage_test.*");
    remove("fail_test.*");
    
    /* Remove any other potential artifacts */
    remove("*.i");
    remove("*.s");
    remove("*.o");
}

int main(void) {
    int overall_status = 0;
    
    /* Create necessary directories */
    mkdir("./test_artifacts", 0755);
    mkdir("./fail_artifacts", 0755);
    mkdir("./dumpdir_test", 0755);
    
    /* Create test source files */
    if (!create_source_file("simple.c", SIMPLE_C_CONTENT)) {
        fprintf(stderr, "Failed to create simple.c\n");
        return 1;
    }
    
    if (!create_source_file("invalid.c", INVALID_C_CONTENT)) {
        fprintf(stderr, "Failed to create invalid.c\n");
        return 1;
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION 1: Successful compilation with various state-setting flags
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     */
    printf("1. Successful compilation with state-setting flags:\n");
    int status1 = run_gcc_command(
        "gcc -save-temps -dumpdir ./test_artifacts "
        "-dumpbase coverage_test -o test_output.o -c simple.c"
    );
    printf("Exit status: %d\n\n", status1);
    
    /* INVOCATION 2: Compilation that fails after driver initialization
     * Uses invalid architecture to trigger backend failure
     */
    printf("2. Failed compilation with different state:\n");
    int status2 = run_gcc_command(
        "gcc -save-temps -dumpdir ./fail_artifacts "
        "-dumpbase fail_test -o fail_output.o -march=invalid-arch simple.c 2>/dev/null"
    );
    printf("Exit status: %d\n\n", status2);
    
    /* INVOCATION 3: Use -specs flag to influence spec_machine
     * Also uses -V to set print_version
     */
    printf("3. Testing with -specs and -V flags:\n");
    int status3 = run_gcc_command(
        "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5"
    );
    printf("Exit status: %d\n\n", status3);
    
    /* INVOCATION 4: Test with -dumpdir and -o flags
     * Sets dumpdir and outbase explicitly
     */
    printf("4. Testing dumpdir and output base:\n");
    int status4 = run_gcc_command(
        "gcc -dumpdir ./dumpdir_test -dumpbase mydump "
        "-o output_prog simple.c 2>&1"
    );
    printf("Exit status: %d\n\n", status4);
    
    /* INVOCATION 5: Test with -Werror to generate error exit status
     * This affects greatest_status
     */
    printf("5. Testing with -Werror for error status:\n");
    int status5 = run_gcc_command(
        "echo 'int main(void) { int x; return 0; }' | "
        "gcc -Werror -Wall -x c - -o /dev/null 2>&1"
    );
    printf("Exit status: %d\n\n", status5);
    
    /* INVOCATION 6: Test verbose flag (verbose_only_flag)
     * Also tests print_help_list indirectly
     */
    printf("6. Testing verbose and help flags:\n");
    int status6 = run_gcc_command(
        "gcc -v -c simple.c 2>&1 | head -10"
    );
    printf("Exit status: %d\n\n", status6);
    
    /* INVOCATION 7: Test help flag (print_help_list)
     */
    printf("7. Testing help flag:\n");
    int status7 = run_gcc_command(
        "gcc --help | head -5"
    );
    printf("Exit status: %d\n\n", status7);
    
    /* INVOCATION 8: Test version flag (print_version)
     */
    printf("8. Testing version flag:\n");
    int status8 = run_gcc_command(
        "gcc --version | head -2"
    );
    printf("Exit status: %d\n\n", status8);
    
    /* INVOCATION 9: Test with syntax error (late failure)
     * Driver parses args successfully but compiler fails
     */
    printf("9. Testing with syntax error:\n");
    int status9 = run_gcc_command(
        "gcc -save-temps -dumpdir ./test_artifacts "
        "-o syntax_error.o -c invalid.c 2>/dev/null"
    );
    printf("Exit status: %d\n\n", status9);
    
    /* INVOCATION 10: Test linker error
     * Driver succeeds through compilation but fails at link time
     */
    printf("10. Testing linker error:\n");
    int status10 = run_gcc_command(
        "gcc -save-temps -o link_test simple.c -lnonexistentlibrary 2>/dev/null"
    );
    printf("Exit status: %d\n\n", status10);
    
    /* INVOCATION 11: Test multiple flags combination
     * Exercises many state variables at once
     */
    printf("11. Testing comprehensive flag combination:\n");
    int status11 = run_gcc_command(
        "gcc -save-temps=obj -dumpdir . -dumpbase comprehensive "
        "-dumpbase-ext .ext -o comprehensive.o -c simple.c"
    );
    printf("Exit status: %d\n\n", status11);
    
    /* INVOCATION 12: Test target system root flags
     * Affects target_system_root_changed and related variables
     */
    printf("12. Testing target system root flags:\n");
    int status12 = run_gcc_command(
        "gcc -c simple.c --sysroot=/ --print-sysroot 2>&1 | tail -2"
    );
    printf("Exit status: %d\n\n", status12);
    
    printf("=== All invocations completed ===\n");
    printf("Note: Some invocations are expected to fail.\n");
    printf("The goal is to trigger the driver's cleanup routine.\n\n");
    
    /* Clean up generated files */
    printf("Cleaning up test artifacts...\n");
    cleanup_files();
    
    /* Always return 0 to indicate test execution completed */
    return 0;
}
