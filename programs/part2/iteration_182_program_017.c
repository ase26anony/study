/* test_gcc_driver_cleanup.c
 * 
 * This test program exercises the GCC driver's cleanup routine by invoking
 * it with various command-line options that set the state variables
 * mentioned in the uncovered lines (11228-11250 of gcc.cc).
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
    return -1;
}

/* Clean up generated files */
void cleanup_files(void) {
    /* Remove generated files from successful compilation */
    remove("simple.o");
    remove("test_output.o");
    remove("fail_output.o");
    
    /* Remove dump files */
    remove("coverage_test.*");
    remove("fail_test.*");
    
    /* Remove temporary directories if they exist */
    rmdir("./test_artifacts");
    rmdir("./fail_artifacts");
    
    /* Remove intermediate files from -save-temps */
    remove("simple.i");
    remove("simple.s");
    remove("test_output.i");
    remove("test_output.s");
    remove("fail_output.i");
    remove("fail_output.s");
}

int main(void) {
    int overall_status = 0;
    
    /* Create the simple C source file */
    if (!create_simple_c_file("simple.c")) {
        return 1;
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* Invocation A: Sets state with -save-temps, -dumpdir, -dumpbase, -o
     * This should succeed and trigger cleanup with allocated variables */
    printf("--- Invocation A: Successful compilation with state variables set ---\n");
    char cmd_a[1024];
    snprintf(cmd_a, sizeof(cmd_a),
             "gcc -save-temps -dumpdir ./test_artifacts "
             "-dumpbase coverage_test -o test_output.o -c simple.c");
    run_gcc_command(cmd_a);
    
    /* Invocation B: Sets state but causes failure with invalid architecture
     * This ensures cleanup runs even on failure */
    printf("--- Invocation B: Failed compilation with different state variables ---\n");
    char cmd_b[1024];
    snprintf(cmd_b, sizeof(cmd_b),
             "gcc -save-temps -dumpdir ./fail_artifacts "
             "-dumpbase fail_test -o fail_output.o -march=invalid-arch -c simple.c 2>/dev/null");
    run_gcc_command(cmd_b);
    
    /* Invocation C: Uses -specs and -V to influence spec_machine and print_version
     * This may set different global state */
    printf("--- Invocation C: Version/specs test ---\n");
    char cmd_c[1024];
    snprintf(cmd_c, sizeof(cmd_c),
             "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5");
    run_gcc_command(cmd_c);
    
    /* Invocation D: Test with -Werror turning warnings into errors
     * This affects greatest_status */
    printf("--- Invocation D: Warning as error test ---\n");
    
    /* First create a file with a warning */
    FILE *fp = fopen("warn.c", "w");
    if (fp) {
        fputs("int main(void) {\n", fp);
        fputs("    int x;\n", fp);  /* Unused variable warning */
        fputs("    return 0;\n", fp);
        fputs("}\n", fp);
        fclose(fp);
        
        char cmd_d[1024];
        snprintf(cmd_d, sizeof(cmd_d),
                 "gcc -Werror -c warn.c 2>/dev/null");
        run_gcc_command(cmd_d);
        remove("warn.c");
    }
    
    /* Invocation E: Test dumpbase_ext and outbase variations */
    printf("--- Invocation E: Extended dump options ---\n");
    char cmd_e[1024];
    snprintf(cmd_e, sizeof(cmd_e),
             "gcc -dumpbase-ext .ext -dumpbase dump_test "
             "-o output_base.o -c simple.c 2>/dev/null");
    run_gcc_command(cmd_e);
    
    /* Invocation F: Test verbose flag */
    printf("--- Invocation F: Verbose flag test ---\n");
    char cmd_f[1024];
    snprintf(cmd_f, sizeof(cmd_f),
             "gcc -v -c simple.c 2>&1 | grep -i 'gcc version'");
    run_gcc_command(cmd_f);
    
    /* Invocation G: Test help flags */
    printf("--- Invocation G: Help flag test ---\n");
    char cmd_g[1024];
    snprintf(cmd_g, sizeof(cmd_g),
             "gcc --help=common 2>&1 | head -3");
    run_gcc_command(cmd_g);
    
    /* Clean up generated files */
    printf("--- Cleaning up test files ---\n");
    cleanup_files();
    remove("simple.c");
    remove("output_base.o");
    remove("dump_test.*");
    
    printf("=== Test completed ===\n");
    printf("The GCC driver's cleanup routine should have been executed multiple times,\n");
    printf("resetting the variables: is_cpp_driver, at_file_supplied, print_help_list,\n");
    printf("print_version, verbose_only_flag, print_subprocess_help, use_ld,\n");
    printf("report_times_to_file, target_system_root, target_system_root_changed,\n");
    printf("target_sysroot_suffix, target_sysroot_hdrs_suffix, save_temps_flag,\n");
    printf("save_temps_overrides_dumpdir, dumpdir_trailing_dash_added,\n");
    printf("dumpdir, dumpbase, dumpbase_ext, outbase, dumpdir_length,\n");
    printf("outbase_length, spec_machine, and greatest_status.\n");
    
    return overall_status;
}
