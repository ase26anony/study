/* test_gcc_driver_cleanup.c
 * 
 * This test program exercises the GCC driver's cleanup routine by invoking
 * it multiple times with different command-line flags that set the state
 * variables in the uncovered block.
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
int run_gcc_command(const char *command) {
    printf("Executing: %s\n", command);
    int status = system(command);
    if (status != -1) {
        if (WIFEXITED(status)) {
            printf("Exit status: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Terminated by signal: %d\n", WTERMSIG(status));
        }
    }
    return status;
}

/* Clean up generated files */
void cleanup_files(void) {
    /* Remove generated source file */
    remove("simple.c");
    
    /* Remove object files */
    remove("test_output.o");
    remove("fail_output.o");
    
    /* Remove dump files */
    remove("coverage_test.*");
    remove("fail_test.*");
    
    /* Remove temporary files from -save-temps */
    remove("simple.i");
    remove("simple.s");
    remove("simple.o");
    
    /* Remove output files */
    remove("output");
    
    /* Remove dump directories */
    system("rm -rf ./test_artifacts");
    system("rm -rf ./fail_artifacts");
}

int main(void) {
    int overall_status = 0;
    
    /* Create the simple C source file */
    if (!create_simple_c_file("simple.c")) {
        return 1;
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* Invocation A: Sets state with -save-temps, -dumpdir, -dumpbase, -o
     * This should succeed and trigger cleanup with allocated state */
    printf("--- Invocation A: Successful compilation with state variables set ---\n");
    char cmd_a[512];
    snprintf(cmd_a, sizeof(cmd_a),
             "gcc -save-temps -dumpdir ./test_artifacts -dumpbase coverage_test "
             "-o test_output.o -c simple.c");
    run_gcc_command(cmd_a);
    printf("\n");
    
    /* Invocation B: Sets state but fails with invalid architecture
     * This ensures cleanup runs even on failure */
    printf("--- Invocation B: Failed compilation with different state ---\n");
    char cmd_b[512];
    snprintf(cmd_b, sizeof(cmd_b),
             "gcc -save-temps -dumpdir ./fail_artifacts -dumpbase fail_test "
             "-o fail_output.o -march=invalid-arch -c simple.c 2>/dev/null");
    run_gcc_command(cmd_b);
    printf("\n");
    
    /* Invocation C: Tests -specs and -V flags
     * These influence spec_machine and print_version */
    printf("--- Invocation C: Testing -specs and -V flags ---\n");
    char cmd_c[512];
    snprintf(cmd_c, sizeof(cmd_c),
             "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5");
    run_gcc_command(cmd_c);
    printf("\n");
    
    /* Invocation D: Tests -Werror to generate different exit status
     * This affects greatest_status */
    printf("--- Invocation D: Testing -Werror for different exit status ---\n");
    
    /* First create a file with a warning */
    FILE *fp = fopen("warn.c", "w");
    if (fp) {
        fputs("int main(void) {\n", fp);
        fputs("    int x;\n", fp);  /* Unused variable warning */
        fputs("    return 0;\n", fp);
        fputs("}\n", fp);
        fclose(fp);
        
        char cmd_d[512];
        snprintf(cmd_d, sizeof(cmd_d),
                 "gcc -Werror -c warn.c 2>/dev/null");
        run_gcc_command(cmd_d);
        remove("warn.c");
    }
    printf("\n");
    
    /* Invocation E: Tests dumpbase_ext and outbase with different extensions */
    printf("--- Invocation E: Testing dumpbase_ext and outbase ---\n");
    char cmd_e[512];
    snprintf(cmd_e, sizeof(cmd_e),
             "gcc -dumpbase simple.c -dumpbase-ext .c "
             "-o output -c simple.c 2>/dev/null");
    run_gcc_command(cmd_e);
    printf("\n");
    
    /* Invocation F: Tests save_temps_flag variations */
    printf("--- Invocation F: Testing save_temps=cwd ---\n");
    char cmd_f[512];
    snprintf(cmd_f, sizeof(cmd_f),
             "gcc -save-temps=cwd -c simple.c 2>/dev/null");
    run_gcc_command(cmd_f);
    printf("\n");
    
    /* Invocation G: Tests target system root flags */
    printf("--- Invocation G: Testing target system root ---\n");
    char cmd_g[512];
    snprintf(cmd_g, sizeof(cmd_g),
             "gcc --sysroot=/ -c simple.c 2>/dev/null");
    run_gcc_command(cmd_g);
    printf("\n");
    
    /* Clean up generated files */
    printf("--- Cleaning up test files ---\n");
    cleanup_files();
    
    printf("=== Test completed ===\n");
    printf("The GCC driver's cleanup routine should have been executed multiple times,\n");
    printf("resetting the variables in the uncovered block:\n");
    printf("- is_cpp_driver, at_file_supplied, print_help_list\n");
    printf("- print_version, verbose_only_flag, print_subprocess_help\n");
    printf("- use_ld, report_times_to_file\n");
    printf("- target_system_root, target_system_root_changed\n");
    printf("- target_sysroot_suffix, target_sysroot_hdrs_suffix\n");
    printf("- save_temps_flag, save_temps_overrides_dumpdir\n");
    printf("- dumpdir_trailing_dash_added\n");
    printf("- dumpdir, dumpbase, dumpbase_ext, outbase (freed and set to NULL)\n");
    printf("- dumpdir_length, outbase_length\n");
    printf("- spec_machine, greatest_status\n");
    
    return overall_status;
}
