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
#include <sys/types.h>

/* Simple C source file content */
#define SIMPLE_C_CONTENT "int main(void) { return 0; }\n"

/* Create a simple C source file */
static int create_simple_c_file(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create simple.c");
        return -1;
    }
    fputs(SIMPLE_C_CONTENT, fp);
    fclose(fp);
    return 0;
}

/* Run a GCC command and capture its exit status */
static int run_gcc_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (status == -1) {
        perror("system() failed");
        return -1;
    }
    return WEXITSTATUS(status);
}

/* Clean up generated files */
static void cleanup_files(void) {
    /* Remove generated files from successful compilation */
    remove("simple.o");
    remove("test_output.o");
    remove("fail_output.o");
    remove("simple.i");
    remove("simple.s");
    remove("coverage_test.i");
    remove("coverage_test.s");
    remove("fail_test.i");
    remove("fail_test.s");
    
    /* Remove temporary directories */
    system("rm -rf ./test_artifacts ./fail_artifacts ./temp_dir");
    
    /* Remove the simple source file */
    remove("simple.c");
}

int main(void) {
    int overall_status = 0;
    
    /* Create the simple C source file */
    if (create_simple_c_file("simple.c") != 0) {
        return 1;
    }
    
    /* Create temporary directories for dumpdir */
    mkdir("./test_artifacts", 0755);
    mkdir("./fail_artifacts", 0755);
    mkdir("./temp_dir", 0755);
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION 1: Sets state variables and succeeds
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Uses -c to stop before linking (successful compilation) */
    printf("1. Testing successful compilation with state variables set:\n");
    const char *cmd1 = "gcc -save-temps -dumpdir ./test_artifacts "
                      "-dumpbase coverage_test -o test_output.o -c simple.c";
    int status1 = run_gcc_command(cmd1);
    printf("Exit status: %d\n\n", status1);
    
    /* INVOCATION 2: Sets different state and fails late
     * This should trigger cleanup after backend failure
     * Uses invalid architecture flag */
    printf("2. Testing failed compilation with different state:\n");
    const char *cmd2 = "gcc -save-temps -dumpdir ./fail_artifacts "
                      "-dumpbase fail_test -o fail_output.o "
                      "-march=invalid-arch simple.c 2>/dev/null";
    int status2 = run_gcc_command(cmd2);
    printf("Exit status: %d\n\n", status2);
    
    /* INVOCATION 3: Tests spec machine and version printing
     * This influences spec_machine and print_version */
    printf("3. Testing with specs and version flag:\n");
    const char *cmd3 = "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5";
    int status3 = run_gcc_command(cmd3);
    printf("Exit status: %d\n\n", status3);
    
    /* INVOCATION 4: Tests dumpbase_ext and outbase with different extensions */
    printf("4. Testing with dumpbase_ext and custom output:\n");
    const char *cmd4 = "gcc -save-temps -dumpdir ./temp_dir "
                      "-dumpbase mytest -dumpbase-ext .ext "
                      "-o custom_output.o -c simple.c";
    int status4 = run_gcc_command(cmd4);
    printf("Exit status: %d\n\n", status4);
    
    /* INVOCATION 5: Tests verbose flag and help list
     * This sets verbose_only_flag and potentially print_help_list */
    printf("5. Testing verbose and help flags:\n");
    const char *cmd5 = "gcc -v -save-temps -dumpbase verbose_test "
                      "-o verbose.o -c simple.c 2>&1 | head -10";
    int status5 = run_gcc_command(cmd5);
    printf("Exit status: %d\n\n", status5);
    
    /* INVOCATION 6: Tests -Werror turning warnings into errors
     * This affects greatest_status with non-zero exit */
    printf("6. Testing -Werror for non-zero exit status:\n");
    const char *cmd6 = "echo 'int main(void) { int x; return 0; }' | "
                      "gcc -save-temps -dumpbase werror_test "
                      "-o werror.o -Werror -x c - 2>/dev/null";
    int status6 = run_gcc_command(cmd6);
    printf("Exit status: %d\n\n", status6);
    
    /* INVOCATION 7: Tests target system root variables
     * This may set target_system_root_changed */
    printf("7. Testing with sysroot flags:\n");
    const char *cmd7 = "gcc -save-temps --sysroot=/tmp/nonexistent "
                      "-dumpbase sysroot_test -o sysroot.o -c simple.c 2>/dev/null";
    int status7 = run_gcc_command(cmd7);
    printf("Exit status: %d\n\n", status7);
    
    /* INVOCATION 8: Tests print_subprocess_help with --help= */
    printf("8. Testing subprocess help:\n");
    const char *cmd8 = "gcc --help=common 2>&1 | head -5";
    int status8 = run_gcc_command(cmd8);
    printf("Exit status: %d\n\n", status8);
    
    /* INVOCATION 9: Tests save_temps_flag variations */
    printf("9. Testing different save-temps options:\n");
    const char *cmd9 = "gcc -save-temps=obj -dumpdir ./temp_dir "
                      "-dumpbase savetemps_test -o savetemps.o -c simple.c";
    int status9 = run_gcc_command(cmd9);
    printf("Exit status: %d\n\n", status9);
    
    /* INVOCATION 10: Tests use_ld with different linker */
    printf("10. Testing use_ld variable:\n");
    const char *cmd10 = "gcc -save-temps -fuse-ld=bfd -dumpbase ld_test "
                       "-o ld_test.o -c simple.c 2>/dev/null";
    int status10 = run_gcc_command(cmd10);
    printf("Exit status: %d\n\n", status10);
    
    printf("=== All invocations completed ===\n");
    printf("Note: Some invocations are expected to fail.\n");
    printf("The goal is to trigger the driver's cleanup routine, which runs regardless of success/failure.\n\n");
    
    /* Clean up generated files */
    cleanup_files();
    
    return overall_status;
}
