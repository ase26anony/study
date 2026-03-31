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
#define SIMPLE_C_CONTENT "int main(void) { return 0; }\n"

/* Create a simple C source file */
static int create_simple_c_file(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return 0;
    }
    fputs(SIMPLE_C_CONTENT, fp);
    fclose(fp);
    return 1;
}

/* Run a GCC command and capture its exit status */
static int run_gcc_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (status == -1) {
        perror("system");
        return -1;
    }
    return WEXITSTATUS(status);
}

/* Clean up generated files */
static void cleanup_files(void) {
    /* Remove generated object files */
    system("rm -f test_output.o fail_output.o output.o 2>/dev/null");
    
    /* Remove dump files and directories */
    system("rm -rf ./test_artifacts ./fail_artifacts ./temp 2>/dev/null");
    
    /* Remove save-temps files */
    system("rm -f simple.i simple.s simple.o 2>/dev/null");
    system("rm -f coverage_test.* fail_test.* 2>/dev/null");
    
    /* Remove any other potential artifacts */
    system("rm -f *.i *.s *.o *.out 2>/dev/null");
}

int main(void) {
    int overall_result = 0;
    
    /* Create the simple C source file */
    if (!create_simple_c_file("simple.c")) {
        fprintf(stderr, "Failed to create simple.c\n");
        return 1;
    }
    
    printf("=== GCC Driver Cleanup Test ===\n\n");
    
    /* INVOCATION A: Sets state with -save-temps, -dumpdir, -dumpbase, -o
     * This should succeed (compiles to object file only with -c) */
    printf("1. Testing successful compilation with state variables set:\n");
    const char *cmd_a = "gcc -save-temps -dumpdir ./test_artifacts "
                       "-dumpbase coverage_test -o test_output.o -c simple.c";
    int status_a = run_gcc_command(cmd_a);
    printf("Exit status: %d\n\n", status_a);
    
    /* INVOCATION B: Sets state but fails with invalid architecture
     * This triggers cleanup after backend failure */
    printf("2. Testing failed compilation with state variables set:\n");
    const char *cmd_b = "gcc -save-temps -dumpdir ./fail_artifacts "
                       "-dumpbase fail_test -o fail_output.o "
                       "-march=invalid-arch -mtune=invalid-tune simple.c 2>/dev/null";
    int status_b = run_gcc_command(cmd_b);
    printf("Exit status: %d\n\n", status_b);
    
    /* INVOCATION C: Tests -specs= and -V flags
     * -V triggers print_version, -specs influences spec_machine */
    printf("3. Testing with -specs and -V flags:\n");
    const char *cmd_c = "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5";
    int status_c = run_gcc_command(cmd_c);
    printf("Exit status: %d\n\n", status_c);
    
    /* INVOCATION D: Tests -Werror turning warnings into errors
     * This sets greatest_status to non-zero */
    printf("4. Testing -Werror for error status:\n");
    const char *cmd_d = "gcc -Werror -Wall -o output.o -c simple.c 2>&1";
    int status_d = run_gcc_command(cmd_d);
    printf("Exit status: %d\n\n", status_d);
    
    /* INVOCATION E: Tests verbose flag and report times
     * This influences verbose_only_flag and report_times_to_file */
    printf("5. Testing verbose and time reporting:\n");
    const char *cmd_e = "gcc -v -ftime-report -o output.o -c simple.c 2>&1 | "
                       "grep -i 'time report' | head -2";
    int status_e = run_gcc_command(cmd_e);
    printf("Exit status: %d\n\n", status_e);
    
    /* INVOCATION F: Tests multiple dumpbase options
     * This exercises dumpbase_ext and outbase logic */
    printf("6. Testing dumpbase extensions:\n");
    const char *cmd_f = "gcc -dumpbase mytest.c -dumpbase-ext .obj "
                       "-o final.o -c simple.c 2>&1";
    int status_f = run_gcc_command(cmd_f);
    printf("Exit status: %d\n\n", status_f);
    
    /* INVOCATION G: Tests help flags
     * This influences print_help_list and print_subprocess_help */
    printf("7. Testing help flags:\n");
    const char *cmd_g = "gcc --help=common 2>&1 | head -3";
    int status_g = run_gcc_command(cmd_g);
    printf("Exit status: %d\n\n", status_g);
    
    /* INVOCATION H: Tests target system root flags
     * This influences target_system_root and related variables */
    printf("8. Testing sysroot flags:\n");
    const char *cmd_h = "gcc --sysroot=/tmp/dummy_sysroot "
                       "-print-sysroot 2>&1 | head -2";
    int status_h = run_gcc_command(cmd_h);
    printf("Exit status: %d\n\n", status_h);
    
    /* INVOCATION I: Tests save-temps with different modes
     * This exercises save_temps_flag values */
    printf("9. Testing save-temps modes:\n");
    const char *cmd_i = "gcc -save-temps=obj -dumpdir ./temp "
                       "-o temp.o -c simple.c 2>&1";
    int status_i = run_gcc_command(cmd_i);
    printf("Exit status: %d\n\n", status_i);
    
    /* INVOCATION J: Tests at-file (@file) syntax
     * This influences at_file_supplied */
    printf("10. Testing at-file syntax:\n");
    /* Create a response file */
    FILE *resp = fopen("args.txt", "w");
    if (resp) {
        fputs("-c simple.c -o response.o\n", resp);
        fclose(resp);
        const char *cmd_j = "gcc @args.txt 2>&1";
        int status_j = run_gcc_command(cmd_j);
        printf("Exit status: %d\n\n", status_j);
        unlink("args.txt");
    }
    
    /* Clean up generated files */
    printf("Cleaning up test artifacts...\n");
    cleanup_files();
    unlink("simple.c");
    
    printf("=== Test Complete ===\n");
    printf("All GCC invocations attempted. Check coverage data for gcc.cc cleanup block.\n");
    
    return overall_result;
}
