/* test_gcc_driver_cleanup.c
 * This test triggers the GCC driver's cleanup routine by invoking it
 * with various command-line options that set the state variables
 * mentioned in the uncovered lines.
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

/* C source file with syntax error (to trigger failure path) */
const char *error_c_content = 
"int main(void) {\n"
"    return  // Missing expression\n"
"}\n";

/* Create a temporary directory for test artifacts */
void create_temp_dir(const char *dirname) {
    struct stat st = {0};
    if (stat(dirname, &st) == -1) {
        mkdir(dirname, 0755);
    }
}

/* Remove directory and its contents */
void remove_dir(const char *dirname) {
    char command[256];
    snprintf(command, sizeof(command), "rm -rf %s", dirname);
    system(command);
}

/* Execute GCC command and return exit status */
int run_gcc(const char *command) {
    printf("Executing: %s\n", command);
    int status = system(command);
    return WEXITSTATUS(status);
}

/* Write a file to disk */
void write_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (f) {
        fputs(content, f);
        fclose(f);
    }
}

int main(void) {
    int overall_status = 0;
    
    /* Create directories for test artifacts */
    create_temp_dir("test_artifacts");
    create_temp_dir("fail_artifacts");
    create_temp_dir("temp_output");
    
    /* Write test source files */
    write_file("simple.c", simple_c_content);
    write_file("error.c", error_c_content);
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION 1: Successful compilation with state-setting flags
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Should trigger cleanup after successful compilation
     */
    printf("1. Testing successful compilation with state variables set:\n");
    int status1 = run_gcc("gcc -save-temps -dumpdir ./test_artifacts "
                          "-dumpbase coverage_test -o test_output.o "
                          "-c simple.c 2>&1");
    printf("Exit status: %d\n\n", status1);
    
    /* INVOCATION 2: Compilation that fails late (after driver initialization)
     * Uses invalid architecture flag to ensure driver parses args but backend fails
     */
    printf("2. Testing compilation failure after driver initialization:\n");
    int status2 = run_gcc("gcc -save-temps -dumpdir ./fail_artifacts "
                          "-dumpbase fail_test -o fail_output.o "
                          "-march=invalid-arch simple.c 2>&1");
    printf("Exit status: %d\n\n", status2);
    
    /* INVOCATION 3: Trigger version printing and spec machine handling
     * This influences: print_version, spec_machine
     */
    printf("3. Testing version/specs handling:\n");
    int status3 = run_gcc("gcc -specs=nosuch.spec -V simple.c 2>&1");
    printf("Exit status: %d\n\n", status3);
    
    /* INVOCATION 4: Test with -Werror to create error exit status
     * This should affect greatest_status
     */
    printf("4. Testing with -Werror (warning as error):\n");
    /* First create a file with a warning */
    write_file("warn.c", "int main(void) { int x; return 0; } /* x unused */\n");
    int status4 = run_gcc("gcc -Werror -save-temps -dumpdir ./test_artifacts "
                          "-o warn_output warn.c 2>&1");
    printf("Exit status: %d\n\n", status4);
    
    /* INVOCATION 5: Test dumpbase_ext and outbase_length
     * Using -dumpbase with extension
     */
    printf("5. Testing dumpbase_ext and outbase_length:\n");
    int status5 = run_gcc("gcc -save-temps -dumpdir ./temp_output "
                          "-dumpbase mydump.ext -dumpbase-ext .extra "
                          "-o long_output_name.o -c simple.c 2>&1");
    printf("Exit status: %d\n\n", status5);
    
    /* INVOCATION 6: Test target system root variables
     * Using --sysroot flag
     */
    printf("6. Testing target system root variables:\n");
    int status6 = run_gcc("gcc --sysroot=/tmp/nonexistent "
                          "-c simple.c 2>&1");
    printf("Exit status: %d\n\n", status6);
    
    /* INVOCATION 7: Test print_help_list and print_subprocess_help
     * Using --help and --help= options
     */
    printf("7. Testing help options:\n");
    int status7 = run_gcc("gcc --help=common 2>&1 | head -5");
    printf("Exit status: %d\n\n", status7);
    
    /* INVOCATION 8: Test verbose_only_flag
     * Using -v flag
     */
    printf("8. Testing verbose flag:\n");
    int status8 = run_gcc("gcc -v -c simple.c 2>&1 | head -10");
    printf("Exit status: %d\n\n", status8);
    
    /* INVOCATION 9: Test save_temps_overrides_dumpdir
     * Using both -save-temps and -dumpdir
     */
    printf("9. Testing save-temps with dumpdir:\n");
    int status9 = run_gcc("gcc -save-temps=cwd -dumpdir ./test_artifacts "
                          "-c simple.c 2>&1");
    printf("Exit status: %d\n\n", status9);
    
    /* INVOCATION 10: Test compilation with syntax error
     * This ensures cleanup runs even with compilation errors
     */
    printf("10. Testing compilation with syntax error:\n");
    int status10 = run_gcc("gcc -save-temps -dumpdir ./test_artifacts "
                           "-o error_output error.c 2>&1");
    printf("Exit status: %d\n\n", status10);
    
    /* Clean up generated files */
    printf("Cleaning up test artifacts...\n");
    remove_dir("test_artifacts");
    remove_dir("fail_artifacts");
    remove_dir("temp_output");
    
    /* Remove generated files */
    system("rm -f simple.c error.c warn.c "
           "test_output.o fail_output.o warn_output error_output "
           "long_output_name.o "
           "*.i *.s *.o a.out 2>/dev/null");
    
    printf("\n=== Test completed ===\n");
    printf("All GCC invocations attempted, driver cleanup should have been triggered.\n");
    
    return overall_status;
}
