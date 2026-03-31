/* test_gcc_driver_cleanup.c
 * 
 * This test program exercises the GCC driver's cleanup routine by invoking
 * it multiple times with different command-line flags that set the state
 * variables that need to be reset in the uncovered block.
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
        return 0;
    }
    fputs(SIMPLE_C_CONTENT, fp);
    fclose(fp);
    return 1;
}

/* Run a GCC command and capture its exit status */
static int run_gcc_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    } else {
        printf("Command terminated abnormally\n");
    }
    return status;
}

/* Clean up generated files */
static void cleanup_generated_files(void) {
    /* Remove object files */
    system("rm -f test_output.o fail_output.o output.o 2>/dev/null");
    
    /* Remove intermediate files from -save-temps */
    system("rm -f simple.i simple.s simple.o 2>/dev/null");
    system("rm -f test_output.i test_output.s 2>/dev/null");
    system("rm -f fail_output.i fail_output.s fail_output.o 2>/dev/null");
    
    /* Remove dump directories */
    system("rm -rf ./test_artifacts ./fail_artifacts ./temp 2>/dev/null");
    
    /* Remove any other temporary files */
    system("rm -f *.gcda *.gcno 2>/dev/null");
}

int main(void) {
    int overall_result = 0;
    
    printf("=== GCC Driver Cleanup Coverage Test ===\n\n");
    
    /* Create the simple C source file */
    if (!create_simple_c_file("simple.c")) {
        return 1;
    }
    
    printf("Created simple.c source file\n\n");
    
    /* Clean up any existing files from previous runs */
    cleanup_generated_files();
    
    /* Create directories for dumpdir tests */
    mkdir("./test_artifacts", 0755);
    mkdir("./fail_artifacts", 0755);
    
    /* INVOCATION A: Sets state with -save-temps, -dumpdir, -dumpbase, -o
     * This should succeed (compilation only, no linking) */
    printf("--- Invocation A: Successful compilation with state variables set ---\n");
    const char *cmd_a = "gcc -save-temps -dumpdir ./test_artifacts "
                        "-dumpbase coverage_test -o test_output.o -c simple.c";
    run_gcc_command(cmd_a);
    printf("\n");
    
    /* INVOCATION B: Sets state but fails with invalid architecture
     * This triggers cleanup after backend failure */
    printf("--- Invocation B: Failed compilation with different state variables ---\n");
    const char *cmd_b = "gcc -save-temps -dumpdir ./fail_artifacts "
                        "-dumpbase fail_test -o fail_output.o "
                        "-march=invalid-arch simple.c 2>/dev/null";
    run_gcc_command(cmd_b);
    printf("\n");
    
    /* INVOCATION C: Tests -specs and -V flags
     * -V triggers print_version, -specs influences spec_machine */
    printf("--- Invocation C: Testing -specs and -V flags ---\n");
    const char *cmd_c = "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5";
    run_gcc_command(cmd_c);
    printf("\n");
    
    /* INVOCATION D: Tests -Werror turning warnings into errors
     * This affects greatest_status */
    printf("--- Invocation D: Testing -Werror for error status ---\n");
    const char *cmd_d = "gcc -Werror -Wall -o output.o -c simple.c 2>&1";
    run_gcc_command(cmd_d);
    printf("\n");
    
    /* INVOCATION E: Tests dumpbase_ext with different output extensions */
    printf("--- Invocation E: Testing dumpbase_ext with -save-temps=obj ---\n");
    const char *cmd_e = "gcc -save-temps=obj -dumpbase myprog -dumpbase-ext .ext "
                        "-o output.o -c simple.c 2>&1";
    run_gcc_command(cmd_e);
    printf("\n");
    
    /* INVOCATION F: Tests verbose flag (verbose_only_flag) */
    printf("--- Invocation F: Testing verbose output flag ---\n");
    const char *cmd_f = "gcc -v -o /dev/null -c simple.c 2>&1 | tail -3";
    run_gcc_command(cmd_f);
    printf("\n");
    
    /* INVOCATION G: Tests help flags (print_help_list, print_subprocess_help) */
    printf("--- Invocation G: Testing help flags ---\n");
    const char *cmd_g = "gcc --help=common 2>&1 | head -5";
    run_gcc_command(cmd_g);
    printf("\n");
    
    /* INVOCATION H: Tests target system root variables */
    printf("--- Invocation H: Testing with sysroot ---\n");
    const char *cmd_h = "gcc --sysroot=/tmp/nonexistent -o /dev/null -c simple.c 2>&1";
    run_gcc_command(cmd_h);
    printf("\n");
    
    /* INVOCATION I: Tests multiple output-related flags together */
    printf("--- Invocation I: Comprehensive output flag test ---\n");
    const char *cmd_i = "gcc -save-temps -dumpdir ./temp -dumpbase comprehensive "
                        "-dumpbase-ext .test -o final_output.o -c simple.c 2>&1";
    run_gcc_command(cmd_i);
    printf("\n");
    
    /* INVOCATION J: Tests with at_file_supplied (using @file for arguments) */
    printf("--- Invocation J: Testing @file argument passing ---\n");
    FILE *args_file = fopen("args.txt", "w");
    if (args_file) {
        fputs("-c simple.c -o test_from_file.o\n", args_file);
        fclose(args_file);
        const char *cmd_j = "gcc @args.txt 2>&1";
        run_gcc_command(cmd_j);
        system("rm -f args.txt test_from_file.o 2>/dev/null");
    }
    printf("\n");
    
    /* Clean up generated files */
    printf("--- Cleaning up generated files ---\n");
    cleanup_generated_files();
    system("rm -f simple.c 2>/dev/null");
    
    printf("=== Test completed ===\n");
    printf("The GCC driver's cleanup routine should have been exercised multiple times.\n");
    printf("Check coverage data for gcc.cc lines 11228-11250.\n");
    
    return overall_result;
}
