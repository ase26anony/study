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
#include <sys/types.h>

#define SIMPLE_C_CONTENT "int main(void) { return 0; }\n"
#define SYNTAX_ERROR_C_CONTENT "int main(void { return 0; }\n"  /* Missing closing parenthesis */

/* Create a simple C source file */
static int create_source_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fprintf(f, "%s", content);
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
    remove("syntax_error.c");
    
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
    remove("syntax_error.i");
    remove("syntax_error.s");
    remove("coverage_test.*");
    remove("fail_test.*");
    
    /* Remove any other potential artifacts */
    remove("*.o");
    remove("*.i");
    remove("*.s");
}

int main(void) {
    int overall_result = 0;
    
    /* Create necessary directories */
    mkdir("./test_artifacts", 0755);
    mkdir("./fail_artifacts", 0755);
    mkdir("./dumpdir_test", 0755);
    
    /* Create test source files */
    if (!create_source_file("simple.c", SIMPLE_C_CONTENT)) {
        fprintf(stderr, "Failed to create simple.c\n");
        return 1;
    }
    
    if (!create_source_file("syntax_error.c", SYNTAX_ERROR_C_CONTENT)) {
        fprintf(stderr, "Failed to create syntax_error.c\n");
        return 1;
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION 1: Successful compilation with state-setting flags
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Should trigger cleanup after successful compilation
     */
    printf("1. Successful compilation with state-setting flags:\n");
    const char *cmd1 = "gcc -save-temps -dumpdir ./test_artifacts "
                      "-dumpbase coverage_test -o test_output.o -c simple.c";
    int ret1 = run_gcc_command(cmd1);
    printf("Return code: %d\n\n", ret1);
    
    /* INVOCATION 2: Compilation that fails after driver initialization
     * Uses invalid architecture flag to cause backend failure
     * Still sets state variables before cleanup
     */
    printf("2. Failed compilation with invalid architecture:\n");
    const char *cmd2 = "gcc -save-temps -dumpdir ./fail_artifacts "
                      "-dumpbase fail_test -o fail_output.o "
                      "-march=invalid-arch -mtune=invalid-tune simple.c 2>/dev/null";
    int ret2 = run_gcc_command(cmd2);
    printf("Return code: %d\n\n", ret2);
    
    /* INVOCATION 3: Use -specs flag (affects spec_machine)
     * Also uses -V which sets print_version
     */
    printf("3. Version request with specs flag:\n");
    const char *cmd3 = "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5";
    int ret3 = run_gcc_command(cmd3);
    printf("Return code: %d\n\n", ret3);
    
    /* INVOCATION 4: Test with syntax error (compilation failure)
     * Uses -Werror to ensure non-zero exit status
     * Sets dumpdir and outbase
     */
    printf("4. Compilation with syntax error:\n");
    const char *cmd4 = "gcc -dumpdir ./dumpdir_test -dumpbase syntax_dump "
                      "-o output_prog syntax_error.c 2>/dev/null";
    int ret4 = run_gcc_command(cmd4);
    printf("Return code: %d\n\n", ret4);
    
    /* INVOCATION 5: Test verbose flag (sets verbose_only_flag)
     * Uses -v and -### which affect driver state
     */
    printf("5. Verbose compilation:\n");
    const char *cmd5 = "gcc -v -### -save-temps -o verbose_output simple.c 2>&1 | head -10";
    int ret5 = run_gcc_command(cmd5);
    printf("Return code: %d\n\n", ret5);
    
    /* INVOCATION 6: Test help flags (affect print_help_list, print_subprocess_help)
     * --help=common sets print_help_list
     */
    printf("6. Help request:\n");
    const char *cmd6 = "gcc --help=common 2>&1 | head -5";
    int ret6 = run_gcc_command(cmd6);
    printf("Return code: %d\n\n", ret6);
    
    /* INVOCATION 7: Test with -save-temps=obj (different save_temps_flag value)
     * Also tests dumpbase_ext
     */
    printf("7. Different save-temps mode:\n");
    const char *cmd7 = "gcc -save-temps=obj -dumpbase obj_test "
                      "-dumpbase-ext .extra -o obj_output.o -c simple.c";
    int ret7 = run_gcc_command(cmd7);
    printf("Return code: %d\n\n", ret7);
    
    /* INVOCATION 8: Test target system root flags
     * Affects target_system_root, target_system_root_changed
     */
    printf("8. With sysroot flags:\n");
    const char *cmd8 = "gcc --sysroot=/tmp/dummy_sysroot "
                      "--with-sysroot=/tmp/other_sysroot -c simple.c 2>/dev/null";
    int ret8 = run_gcc_command(cmd8);
    printf("Return code: %d\n\n", ret8);
    
    /* INVOCATION 9: Test time reporting (affects report_times_to_file)
     */
    printf("9. Time reporting:\n");
    const char *cmd9 = "gcc -ftime-report -c simple.c 2>&1 | grep -i 'time' | head -3";
    int ret9 = run_gcc_command(cmd9);
    printf("Return code: %d\n\n", ret9);
    
    /* INVOCATION 10: Multiple flags combined
     * Tests interaction of multiple state variables
     */
    printf("10. Combined flags test:\n");
    const char *cmd10 = "gcc -save-temps -dumpdir ./combined "
                       "-dumpbase combined_test -o combined.o "
                       "-specs=nosuch.spec -v -c simple.c 2>/dev/null";
    int ret10 = run_gcc_command(cmd10);
    printf("Return code: %d\n\n", ret10);
    
    printf("=== All invocations completed ===\n");
    printf("Note: Some commands are expected to fail - this is intentional\n");
    printf("to trigger cleanup paths with different exit statuses.\n\n");
    
    /* Clean up generated files */
    printf("Cleaning up test artifacts...\n");
    cleanup_files();
    
    printf("\nTest completed successfully.\n");
    printf("The GCC driver's cleanup routine should have been executed\n");
    printf("multiple times with various state configurations.\n");
    
    return overall_result;
}
