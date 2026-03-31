/* test_gcc_driver_cleanup.c
 * This test program exercises the GCC driver's cleanup routine
 * by invoking it with various flags that set the target variables.
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
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    printf("Command failed to execute properly\n\n");
    return -1;
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
    
    /* Remove temporary directories */
    system("rm -rf ./test_artifacts");
    system("rm -rf ./fail_artifacts");
    
    /* Remove source file */
    remove("simple.c");
}

int main(void) {
    int overall_result = 0;
    
    /* Create the simple C source file */
    if (!create_simple_c_file("simple.c")) {
        return 1;
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* Invocation A: Sets state variables with successful compilation
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * The -c flag ensures compilation stops before linking (successful)
     */
    printf("--- Invocation A: Setting state with successful compilation ---\n");
    const char *cmd_a = "gcc -save-temps -dumpdir ./test_artifacts "
                       "-dumpbase coverage_test -o test_output.o -c simple.c";
    run_gcc_command(cmd_a);
    
    /* Invocation B: Sets state but causes backend failure
     * Uses invalid architecture to trigger failure after driver initialization
     * This ensures cleanup runs with error state
     */
    printf("--- Invocation B: Setting state with backend failure ---\n");
    const char *cmd_b = "gcc -save-temps -dumpdir ./fail_artifacts "
                       "-dumpbase fail_test -o fail_output.o "
                       "-march=invalid-arch -c simple.c 2>/dev/null";
    run_gcc_command(cmd_b);
    
    /* Invocation C: Tests spec_machine and print_version
     * The -V flag influences print_version
     * The -specs flag may influence spec_machine
     */
    printf("--- Invocation C: Testing spec_machine and version ---\n");
    const char *cmd_c = "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5";
    run_gcc_command(cmd_c);
    
    /* Invocation D: Tests -Werror turning warnings into errors
     * This affects greatest_status with a different failure mode
     */
    printf("--- Invocation D: Testing -Werror failure path ---\n");
    const char *cmd_d = "gcc -Werror -Wall -c simple.c -o simple.o 2>&1";
    run_gcc_command(cmd_d);
    
    /* Invocation E: Tests dumpbase_ext and outbase_length
     * Using -dumpbase with extension and -o with different name
     */
    printf("--- Invocation E: Testing dumpbase_ext and outbase_length ---\n");
    const char *cmd_e = "gcc -dumpbase myprog.c -dumpbase-ext .ext "
                       "-o myoutput.o -c simple.c 2>/dev/null";
    run_gcc_command(cmd_e);
    remove("myoutput.o");
    
    /* Invocation F: Tests verbose_only_flag and print_subprocess_help
     * Using -### (verbose only) and --help=common
     */
    printf("--- Invocation F: Testing verbose and help flags ---\n");
    const char *cmd_f = "gcc -### -c simple.c 2>&1 | head -3";
    run_gcc_command(cmd_f);
    
    const char *cmd_f2 = "gcc --help=common 2>&1 | head -3";
    run_gcc_command(cmd_f2);
    
    /* Invocation G: Tests target_system_root related variables
     * Using --sysroot flag
     */
    printf("--- Invocation G: Testing sysroot flags ---\n");
    const char *cmd_g = "gcc --sysroot=/tmp/nonexistent -c simple.c 2>/dev/null";
    run_gcc_command(cmd_g);
    
    /* Invocation H: Tests save_temps_overrides_dumpdir
     * Using both -save-temps and -dumpdir
     */
    printf("--- Invocation H: Testing save-temps with dumpdir ---\n");
    const char *cmd_h = "gcc -save-temps=cwd -dumpdir ./mydir -c simple.c 2>/dev/null";
    run_gcc_command(cmd_h);
    
    /* Clean up generated files */
    printf("--- Cleaning up generated files ---\n");
    cleanup_files();
    
    printf("=== Test completed ===\n");
    printf("The GCC driver's cleanup routine should have been executed multiple times,\n");
    printf("covering the reset of variables like dumpdir, dumpbase, outbase,\n");
    printf("save_temps_flag, spec_machine, and greatest_status.\n");
    
    return overall_result;
}
