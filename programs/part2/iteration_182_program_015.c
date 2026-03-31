/* test_gcc_driver_cleanup.c
 * This test triggers the GCC driver's cleanup routine to cover lines 11228-11250 in gcc.cc
 * It invokes GCC multiple times with different flags to set the target state variables.
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

/* Create a simple C source file */
int create_simple_c_file(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create simple.c");
        return 0;
    }
    fputs(simple_c_content, fp);
    fclose(fp);
    return 1;
}

/* Run a GCC command and capture its exit status */
int run_gcc_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    printf("Exit status: %d\n\n", status);
    return status;
}

/* Clean up generated files */
void cleanup_files(void) {
    /* Remove generated files from successful compilation */
    remove("simple.o");
    remove("simple.i");
    remove("simple.s");
    remove("test_output.o");
    remove("test_output.i");
    remove("test_output.s");
    remove("fail_output.o");
    remove("fail_output.i");
    remove("fail_output.s");
    
    /* Remove any dump files */
    remove("coverage_test.*");
    remove("fail_test.*");
    
    /* Remove temporary directories if they exist */
    rmdir("./test_artifacts");
    rmdir("./fail_artifacts");
    
    /* Remove the simple.c file */
    remove("simple.c");
}

int main(void) {
    int overall_result = 0;
    
    /* Create the simple C source file */
    if (!create_simple_c_file("simple.c")) {
        return 1;
    }
    
    /* Create temporary directories for dumpdir */
    mkdir("./test_artifacts", 0755);
    mkdir("./fail_artifacts", 0755);
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION A: Sets state variables with successful compilation
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Uses -c to stop before linking (successful compilation)
     */
    printf("1. Successful compilation with state variables set:\n");
    const char *cmd_a = "gcc -save-temps -dumpdir ./test_artifacts "
                        "-dumpbase coverage_test -o test_output.o -c simple.c";
    run_gcc_command(cmd_a);
    
    /* INVOCATION B: Sets state variables with compilation failure
     * Uses invalid architecture flag to cause backend failure
     * after driver initialization
     */
    printf("2. Failed compilation with state variables set:\n");
    const char *cmd_b = "gcc -save-temps -dumpdir ./fail_artifacts "
                        "-dumpbase fail_test -o fail_output.o "
                        "-march=invalid-arch -c simple.c 2>/dev/null";
    run_gcc_command(cmd_b);
    
    /* INVOCATION C: Tests different state variables
     * Sets spec_machine and print_version
     */
    printf("3. Testing spec machine and version flags:\n");
    const char *cmd_c = "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5";
    run_gcc_command(cmd_c);
    
    /* INVOCATION D: Tests -Werror turning warnings into errors
     * This affects greatest_status
     */
    printf("4. Testing -Werror for error status:\n");
    const char *cmd_d = "gcc -Werror -Wall -c simple.c -o simple.o 2>&1";
    run_gcc_command(cmd_d);
    
    /* INVOCATION E: Tests dumpbase_ext and outbase_length
     * Using different output extensions
     */
    printf("5. Testing with different output names:\n");
    const char *cmd_e = "gcc -dumpbase myprog.c -dumpbase-ext .exe "
                        "-o myoutput.exe -c simple.c 2>/dev/null";
    run_gcc_command(cmd_e);
    remove("myoutput.exe");
    
    /* INVOCATION F: Tests verbose flag and help
     * Sets verbose_only_flag and print_help_list
     */
    printf("6. Testing verbose and help flags:\n");
    const char *cmd_f = "gcc -v -c simple.c -o simple.o 2>&1 | head -10";
    run_gcc_command(cmd_f);
    
    /* INVOCATION G: Tests target system root variables
     * May set target_system_root_changed
     */
    printf("7. Testing with sysroot flags:\n");
    const char *cmd_g = "gcc --sysroot=/tmp/nonexistent -c simple.c "
                        "-o simple.o 2>/dev/null";
    run_gcc_command(cmd_g);
    
    /* INVOCATION H: Tests print_subprocess_help
     * Using --help=common flag
     */
    printf("8. Testing subprocess help:\n");
    const char *cmd_h = "gcc --help=common 2>&1 | head -5";
    run_gcc_command(cmd_h);
    
    /* INVOCATION I: Tests report_times_to_file
     * Using -ftime-report
     */
    printf("9. Testing time reporting:\n");
    const char *cmd_i = "gcc -ftime-report -c simple.c -o simple.o 2>&1 | head -20";
    run_gcc_command(cmd_i);
    
    /* INVOCATION J: Tests use_ld with explicit linker
     * Note: -fuse-ld affects use_ld variable
     */
    printf("10. Testing linker specification:\n");
    const char *cmd_j = "gcc -fuse-ld=bfd -c simple.c -o simple.o 2>&1";
    run_gcc_command(cmd_j);
    
    printf("\n=== Cleanup complete ===\n");
    
    /* Clean up generated files */
    cleanup_files();
    
    return overall_result;
}
