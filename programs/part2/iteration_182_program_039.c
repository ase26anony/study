/* test_gcc_driver_cleanup.c
 * This test triggers the GCC driver's cleanup routine by invoking it
 * with various flags that set the state variables being reset in the
 * uncovered lines (11228-11250 of gcc.cc).
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
    fprintf(fp, "%s", simple_c_content);
    fclose(fp);
    return 1;
}

/* Run a GCC command and capture its exit status */
int run_gcc_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (status != -1) {
        if (WIFEXITED(status)) {
            printf("Exit status: %d\n", WEXITSTATUS(status));
        } else {
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
    
    /* Remove save-temps files */
    remove("simple.i");
    remove("simple.s");
    remove("coverage_test.i");
    remove("coverage_test.s");
    remove("fail_test.i");
    remove("fail_test.s");
    
    /* Remove dumpdir directories if empty */
    rmdir("./test_artifacts");
    rmdir("./fail_artifacts");
    
    /* Remove any other temporary files */
    remove("output.o");
}

int main(void) {
    int overall_status = 0;
    
    /* Create the simple C source file */
    if (!create_simple_c_file("simple.c")) {
        return 1;
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION A: Sets state variables and succeeds
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * The -c flag ensures compilation stops before linking (successful)
     */
    printf("1. Invocation A: Setting state with successful compilation\n");
    const char *cmd_a = "gcc -save-temps -dumpdir ./test_artifacts "
                       "-dumpbase coverage_test -o test_output.o -c simple.c";
    run_gcc_command(cmd_a);
    printf("\n");
    
    /* Give time for cleanup to complete */
    sleep(1);
    
    /* INVOCATION B: Sets state variables and fails late
     * This sets different values for dumpdir, dumpbase, outbase
     * Uses invalid -march to cause backend failure after driver initialization
     */
    printf("2. Invocation B: Setting state with late failure\n");
    const char *cmd_b = "gcc -save-temps -dumpdir ./fail_artifacts "
                       "-dumpbase fail_test -o fail_output.o "
                       "-march=invalid-arch simple.c 2>/dev/null";
    run_gcc_command(cmd_b);
    printf("\n");
    
    sleep(1);
    
    /* INVOCATION C: Tests with -specs and -V flags
     * This may influence spec_machine and print_version
     */
    printf("3. Invocation C: Testing with -specs and -V flags\n");
    const char *cmd_c = "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5";
    run_gcc_command(cmd_c);
    printf("\n");
    
    sleep(1);
    
    /* INVOCATION D: Tests with -Werror to generate error exit status
     * This should set greatest_status to non-zero
     */
    printf("4. Invocation D: Testing with -Werror\n");
    /* First create a file with a warning */
    FILE *fp = fopen("warn.c", "w");
    if (fp) {
        fprintf(fp, "int main(void) {\n");
        fprintf(fp, "    int x;  /* Unused variable warning */\n");
        fprintf(fp, "    return 0;\n");
        fprintf(fp, "}\n");
        fclose(fp);
        
        const char *cmd_d = "gcc -Werror -c warn.c 2>/dev/null";
        run_gcc_command(cmd_d);
        remove("warn.c");
    }
    printf("\n");
    
    sleep(1);
    
    /* INVOCATION E: Tests dumpbase_ext and outbase with different extensions */
    printf("5. Invocation E: Testing dumpbase_ext and outbase\n");
    const char *cmd_e = "gcc -save-temps -dumpbase myprog.c -dumpbase-ext .ext "
                       "-o output.o -c simple.c";
    run_gcc_command(cmd_e);
    printf("\n");
    
    /* Clean up generated files */
    printf("Cleaning up test files...\n");
    cleanup_files();
    
    printf("\n=== Test completed ===\n");
    printf("The GCC driver's cleanup routine should have been executed multiple times,\n");
    printf("resetting the variables in the uncovered block:\n");
    printf("- is_cpp_driver, at_file_supplied, print_help_list, print_version\n");
    printf("- verbose_only_flag, print_subprocess_help, use_ld\n");
    printf("- report_times_to_file, target_system_root, target_system_root_changed\n");
    printf("- target_sysroot_suffix, target_sysroot_hdrs_suffix\n");
    printf("- save_temps_flag, save_temps_overrides_dumpdir\n");
    printf("- dumpdir_trailing_dash_added\n");
    printf("- dumpdir, dumpbase, dumpbase_ext, outbase (freed and set to NULL)\n");
    printf("- dumpdir_length, outbase_length (set to 0)\n");
    printf("- spec_machine (reset to DEFAULT_TARGET_MACHINE)\n");
    printf("- greatest_status (reset to 1)\n");
    
    return overall_status;
}
