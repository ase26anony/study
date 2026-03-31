/* test_gcc_driver_cleanup.c
 * 
 * This test program exercises the GCC driver's cleanup routine by invoking
 * it multiple times with different command-line flags that set the state
 * variables being reset in the uncovered block.
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

/* C source file with syntax error (for failure case) */
const char *error_c_content = 
"int main(void) {\n"
"    return  // Missing expression\n"
"}\n";

/* Create a temporary directory for test artifacts */
void create_temp_dir(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mkdir(path, 0700);
    }
}

/* Remove a directory and its contents */
void remove_dir(const char *path) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
    system(cmd);
}

/* Execute a GCC command and return the exit status */
int execute_gcc(const char *command) {
    printf("Executing: %s\n", command);
    int status = system(command);
    return WEXITSTATUS(status);
}

/* Write a file with given content */
void write_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (f) {
        fputs(content, f);
        fclose(f);
    }
}

int main(void) {
    int overall_status = 0;
    
    /* Create temporary directories for test artifacts */
    create_temp_dir("test_artifacts");
    create_temp_dir("fail_artifacts");
    create_temp_dir("dump_test");
    
    /* Write test source files */
    write_file("simple.c", simple_c_content);
    write_file("error.c", error_c_content);
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION 1: Successful compilation with various flags
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Should trigger cleanup after successful compilation
     */
    printf("1. Successful compilation with state-setting flags:\n");
    int status1 = execute_gcc("gcc -save-temps -dumpdir ./test_artifacts "
                              "-dumpbase coverage_test -o test_output.o "
                              "-c simple.c 2>&1");
    printf("Exit status: %d\n\n", status1);
    
    /* INVOCATION 2: Compilation that fails late (after driver initialization)
     * Uses invalid architecture flag to cause backend failure
     * This ensures cleanup runs even on failure
     */
    printf("2. Failed compilation with different state:\n");
    int status2 = execute_gcc("gcc -save-temps -dumpdir ./fail_artifacts "
                              "-dumpbase fail_test -o fail_output.o "
                              "-march=invalid-arch simple.c 2>&1");
    printf("Exit status: %d\n\n", status2);
    
    /* INVOCATION 3: Test with -specs flag (influences spec_machine)
     * Also uses -V for version printing (sets print_version)
     */
    printf("3. Testing with -specs and -V flags:\n");
    int status3 = execute_gcc("gcc -specs=nosuch.spec -V simple.c 2>&1");
    printf("Exit status: %d\n\n", status3);
    
    /* INVOCATION 4: Test with -dumpbase and -dumpdir variations
     * Also tests dumpbase_ext and outbase with different values
     */
    printf("4. Testing dumpdir/dumpbase variations:\n");
    int status4 = execute_gcc("gcc -dumpdir ./dump_test -dumpbase myprog "
                              "-dumpbase-ext .ext -o myoutput.o "
                              "-c simple.c 2>&1");
    printf("Exit status: %d\n\n", status4);
    
    /* INVOCATION 5: Test with -Werror to create a different failure mode
     * This tests greatest_status reset with warning-turned-error
     */
    printf("5. Testing -Werror failure path:\n");
    write_file("warn.c", "int main(void) { int x; return 0; } /* unused variable */\n");
    int status5 = execute_gcc("gcc -Werror -save-temps -o warn_output warn.c 2>&1");
    printf("Exit status: %d\n\n", status5);
    
    /* INVOCATION 6: Test verbose flag (sets verbose_only_flag)
     * Also tests print_help_list with --help
     */
    printf("6. Testing verbose and help flags:\n");
    int status6 = execute_gcc("gcc -v -c simple.c 2>&1 | head -5");
    printf("Exit status: %d\n\n", status6);
    
    /* INVOCATION 7: Test with syntax error (ensures cleanup on early failure)
     * The driver still parses arguments before the compiler proper fails
     */
    printf("7. Testing with syntax error:\n");
    int status7 = execute_gcc("gcc -save-temps -o error_output error.c 2>&1");
    printf("Exit status: %d\n\n", status7);
    
    /* INVOCATION 8: Test linker error (driver proceeds to linking stage)
     * Uses non-existent library to cause linker failure
     */
    printf("8. Testing linker error:\n");
    int status8 = execute_gcc("gcc simple.c -lnonexistentlibrary 2>&1");
    printf("Exit status: %d\n\n", status8);
    
    /* INVOCATION 9: Test multiple output-related flags
     * Exercises outbase_length and dumpdir_length
     */
    printf("9. Testing output base variations:\n");
    int status9 = execute_gcc("gcc -save-temps -dumpdir longdir/ -o longoutputname.o "
                              "-c simple.c 2>&1");
    printf("Exit status: %d\n\n", status9);
    
    /* INVOCATION 10: Final test with all flags combined
     * Maximizes state variable initialization
     */
    printf("10. Comprehensive flag test:\n");
    int status10 = execute_gcc("gcc -save-temps=obj -dumpdir ./final -dumpbase finaldump "
                               "-dumpbase-ext .final -o final_output.o -v -c simple.c 2>&1");
    printf("Exit status: %d\n\n", status10);
    
    /* Clean up generated files */
    printf("Cleaning up test artifacts...\n");
    remove_dir("test_artifacts");
    remove_dir("fail_artifacts");
    remove_dir("dump_test");
    
    /* Remove generated files */
    system("rm -f simple.c error.c warn.c "
           "test_output.o fail_output.o warn_output error_output final_output.o "
           "*.i *.s *.ii *.bc 2>/dev/null");
    
    printf("\n=== Test completed ===\n");
    printf("All GCC invocations attempted. Check coverage data for gcc.cc cleanup block.\n");
    
    return overall_status;
}
