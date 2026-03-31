/* test_gcc_driver_cleanup.c
 * This test triggers the GCC driver's cleanup routine by invoking it
 * with various flags that set the state variables being reset.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

/* Create a simple C source file for compilation */
void create_simple_c_file(void) {
    FILE *f = fopen("simple.c", "w");
    if (!f) {
        perror("Failed to create simple.c");
        exit(1);
    }
    fprintf(f, "int main(void) { return 0; }\n");
    fclose(f);
}

/* Create a C source file with a syntax error for failure testing */
void create_error_c_file(void) {
    FILE *f = fopen("error.c", "w");
    if (!f) {
        perror("Failed to create error.c");
        exit(1);
    }
    fprintf(f, "int main(void) { return missing_var; }\n");
    fclose(f);
}

/* Run a GCC command and capture its exit status */
int run_gcc_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    printf("Command failed to execute properly\n\n");
    return -1;
}

/* Clean up generated files */
void cleanup_files(void) {
    /* Remove source files */
    remove("simple.c");
    remove("error.c");
    
    /* Remove object files */
    remove("test_output.o");
    remove("fail_output.o");
    remove("output.o");
    
    /* Remove dump/temp files */
    remove("coverage_test.*");
    remove("fail_test.*");
    remove("dump_test.*");
    
    /* Remove directories */
    rmdir("test_artifacts");
    rmdir("fail_artifacts");
    rmdir("dumpdir_test");
    
    /* Remove any other potential temp files */
    system("rm -f *.i *.s *.o a.out 2>/dev/null");
}

int main(void) {
    int overall_status = 0;
    
    /* Create test source files */
    create_simple_c_file();
    create_error_c_file();
    
    /* Create directories for dumpdir tests */
    mkdir("test_artifacts", 0755);
    mkdir("fail_artifacts", 0755);
    mkdir("dumpdir_test", 0755);
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION A: Sets state variables with successful compilation
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Compilation succeeds with -c flag
     */
    printf("1. Successful compilation with state variables set:\n");
    run_gcc_command("gcc -save-temps -dumpdir ./test_artifacts "
                    "-dumpbase coverage_test -o test_output.o -c simple.c");
    
    /* INVOCATION B: Sets state variables with compilation failure
     * Uses invalid architecture flag to cause backend failure
     * after driver initialization
     */
    printf("2. Failed compilation with state variables set:\n");
    run_gcc_command("gcc -save-temps -dumpdir ./fail_artifacts "
                    "-dumpbase fail_test -o fail_output.o "
                    "-march=invalid-arch simple.c 2>/dev/null");
    
    /* INVOCATION C: Different flags to influence spec_machine
     * Uses -specs and -V flags
     */
    printf("3. Version/specs test:\n");
    run_gcc_command("gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5");
    
    /* INVOCATION D: Test with dumpbase_ext and outbase_length
     * Uses multiple dump-related flags
     */
    printf("4. Comprehensive dump flags test:\n");
    run_gcc_command("gcc -save-temps=obj -dumpdir dumpdir_test/ "
                    "-dumpbase dump_test -dumpbase-ext .ext "
                    "-o output.o -c simple.c");
    
    /* INVOCATION E: Test with verbose flag and error
     * Sets verbose_only_flag and causes compilation error
     */
    printf("5. Verbose mode with compilation error:\n");
    run_gcc_command("gcc -save-temps -v -o /dev/null error.c 2>&1 | "
                    "grep -E '(error|Error|ERROR)' | head -3");
    
    /* INVOCATION F: Test with Werror to change exit status
     * This affects greatest_status variable
     */
    printf("6. Werror test (warning becomes error):\n");
    run_gcc_command("gcc -Werror -Wall -o /dev/null -c simple.c 2>&1");
    
    /* INVOCATION G: Test print_help_list and print_version
     * These should set the corresponding flags
     */
    printf("7. Help and version flags:\n");
    run_gcc_command("gcc --help=common 2>&1 | head -3");
    run_gcc_command("gcc --version 2>&1 | head -2");
    
    /* INVOCATION H: Test target system root variables
     * Uses --sysroot flag
     */
    printf("8. Sysroot test:\n");
    run_gcc_command("gcc --sysroot=/nonexistent -c simple.c 2>/dev/null");
    
    /* INVOCATION I: Test with linker flag (use_ld variable)
     * Sets use_ld to a specific value
     */
    printf("9. Linker selection test:\n");
    run_gcc_command("gcc -fuse-ld=bfd -o /dev/null simple.c 2>&1 | "
                    "grep -i 'ld' | head -2");
    
    /* INVOCATION J: Test report_times_to_file
     * Uses -ftime-report flag
     */
    printf("10. Time report test:\n");
    run_gcc_command("gcc -ftime-report -c simple.c 2>&1 | "
                    "grep -E '(Time|time)' | head -3");
    
    /* Clean up generated files */
    printf("Cleaning up test files...\n");
    cleanup_files();
    
    printf("\n=== Test completed ===\n");
    printf("The GCC driver's cleanup routine should have been executed\n");
    printf("multiple times with various state variables set and reset.\n");
    
    return overall_status;
}
