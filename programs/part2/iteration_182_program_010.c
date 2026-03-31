/**
 * Test program to exercise GCC driver cleanup logic (lines 11228-11250 in gcc.cc)
 * This program invokes GCC with various flags to set state variables that
 * should be reset during cleanup.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define SIMPLE_C_FILE "simple_test.c"
#define SIMPLE_C_CONTENT "int main(void) { return 0; }"

/**
 * Create a minimal valid C source file for testing
 */
static int create_test_source(void) {
    FILE *fp = fopen(SIMPLE_C_FILE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return 0;
    }
    
    fprintf(fp, "%s\n", SIMPLE_C_CONTENT);
    fclose(fp);
    return 1;
}

/**
 * Execute a GCC command and capture its return status
 */
static int run_gcc_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    
    int status = system(cmd);
    
    if (WIFEXITED(status)) {
        printf("  Exit status: %d\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    } else {
        printf("  Command terminated abnormally\n");
        return -1;
    }
}

/**
 * Clean up generated files
 */
static void cleanup_generated_files(void) {
    // Remove source file
    unlink(SIMPLE_C_FILE);
    
    // Remove common output files
    unlink("test_output.o");
    unlink("fail_output.o");
    unlink("output.o");
    
    // Remove save-temps files
    unlink("simple_test.i");
    unlink("simple_test.s");
    unlink("coverage_test.i");
    unlink("coverage_test.s");
    unlink("fail_test.i");
    unlink("fail_test.s");
    
    // Remove dumpdir artifacts
    system("rm -rf ./test_artifacts");
    system("rm -rf ./fail_artifacts");
    system("rm -rf ./temp");
    
    // Remove any stray files
    unlink("mydump.i");
    unlink("mydump.s");
}

int main(void) {
    int overall_result = 0;
    
    printf("=== GCC Driver Cleanup Coverage Test ===\n\n");
    
    // Create test source file
    if (!create_test_source()) {
        return 1;
    }
    
    printf("Created test source file: %s\n\n", SIMPLE_C_FILE);
    
    // Create directories for dumpdir tests
    mkdir("./test_artifacts", 0755);
    mkdir("./fail_artifacts", 0755);
    mkdir("./temp", 0755);
    
    // INVOCATION A: Sets state with successful compilation (-c flag)
    // This should set: save_temps_flag, dumpdir, dumpbase, outbase
    printf("--- Invocation A: Successful compilation with state setup ---\n");
    const char *cmd_a = "gcc -save-temps -dumpdir ./test_artifacts "
                       "-dumpbase coverage_test -o test_output.o -c " SIMPLE_C_FILE;
    run_gcc_command(cmd_a);
    printf("\n");
    
    // INVOCATION B: Sets state with compilation failure
    // Uses invalid architecture flag to cause backend failure after driver init
    printf("--- Invocation B: Failed compilation with different state ---\n");
    const char *cmd_b = "gcc -save-temps -dumpdir ./fail_artifacts "
                       "-dumpbase fail_test -o fail_output.o "
                       "-march=invalid-arch " SIMPLE_C_FILE;
    run_gcc_command(cmd_b);
    printf("\n");
    
    // INVOCATION C: Tests dumpdir and dumpbase with -o flag
    printf("--- Invocation C: Testing dumpdir/dumpbase/outbase interaction ---\n");
    const char *cmd_c = "gcc -save-temps -dumpdir ./temp -dumpbase mydump "
                       "-o output.o -c " SIMPLE_C_FILE;
    run_gcc_command(cmd_c);
    printf("\n");
    
    // INVOCATION D: Tests print_version flag
    printf("--- Invocation D: Testing version printing ---\n");
    const char *cmd_d = "gcc -V " SIMPLE_C_FILE " 2>&1 | head -5";
    run_gcc_command(cmd_d);
    printf("\n");
    
    // INVOCATION E: Tests specs flag (affects spec_machine)
    printf("--- Invocation E: Testing specs flag ---\n");
    const char *cmd_e = "gcc -specs=nosuch.spec " SIMPLE_C_FILE " 2>&1 | head -2";
    run_gcc_command(cmd_e);
    printf("\n");
    
    // INVOCATION F: Tests verbose flag and help
    printf("--- Invocation F: Testing verbose and help flags ---\n");
    const char *cmd_f = "gcc -v -c " SIMPLE_C_FILE " 2>&1 | tail -3";
    run_gcc_command(cmd_f);
    printf("\n");
    
    // INVOCATION G: Tests -Werror to generate different exit status
    printf("--- Invocation G: Testing -Werror for different failure mode ---\n");
    // First create a file with a warning
    FILE *fp = fopen("warn.c", "w");
    if (fp) {
        fprintf(fp, "int main(void) { int x; return 0; }\n");  // unused variable warning
        fclose(fp);
        
        const char *cmd_g1 = "gcc -Werror -c warn.c 2>&1 | head -2";
        run_gcc_command(cmd_g1);
        
        unlink("warn.c");
        unlink("warn.o");
    }
    printf("\n");
    
    // INVOCATION H: Tests save-temps with obj flag
    printf("--- Invocation H: Testing save-temps=obj ---\n");
    const char *cmd_h = "gcc -save-temps=obj -dumpdir ./temp -o obj_test.o -c " SIMPLE_C_FILE;
    run_gcc_command(cmd_h);
    printf("\n");
    
    // INVOCATION I: Tests multiple flags together
    printf("--- Invocation I: Testing multiple state-setting flags ---\n");
    const char *cmd_i = "gcc -save-temps -dumpdir . -dumpbase fulltest "
                       "-specs=nosuch.spec -v -o final.o -c " SIMPLE_C_FILE " 2>&1 | tail -5";
    run_gcc_command(cmd_i);
    printf("\n");
    
    // Clean up generated files
    printf("--- Cleaning up generated files ---\n");
    cleanup_generated_files();
    
    printf("\n=== Test completed ===\n");
    printf("All GCC invocations attempted. Check coverage data for gcc.cc lines 11228-11250.\n");
    
    return overall_result;
}
