/* test_gcc_driver_cleanup.c
 * 
 * This test program exercises the GCC driver's cleanup routine by invoking
 * it multiple times with different command-line options that set the state
 * variables mentioned in the uncovered lines (11228-11250 of gcc.cc).
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

/* Create a temporary directory for test artifacts */
int create_temp_dir(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) != 0) {
            perror("mkdir");
            return 0;
        }
    }
    return 1;
}

/* Remove a directory and its contents */
void remove_dir(const char *path) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
    system(cmd);
}

/* Write the simple C source file */
int write_simple_c(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fputs(simple_c_content, f);
    fclose(f);
    return 1;
}

/* Execute a GCC command and capture its return code */
int execute_gcc(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int ret = system(cmd);
    if (WIFEXITED(ret)) {
        printf("Exit code: %d\n\n", WEXITSTATUS(ret));
        return WEXITSTATUS(ret);
    }
    printf("Command terminated abnormally\n\n");
    return -1;
}

int main(void) {
    int overall_result = 0;
    
    /* Create test directories */
    if (!create_temp_dir("test_artifacts")) return 1;
    if (!create_temp_dir("fail_artifacts")) return 1;
    
    /* Write the simple C source file */
    if (!write_simple_c("simple.c")) return 1;
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* Invocation A: Sets state variables with successful compilation
     * This sets:
     *   - save_temps_flag via -save-temps
     *   - dumpdir via -dumpdir
     *   - dumpbase via -dumpbase  
     *   - outbase via -o
     *   - Uses -c to stop before linking (successful compilation)
     */
    printf("Invocation A: Setting state with successful compilation\n");
    execute_gcc("gcc -save-temps -dumpdir ./test_artifacts -dumpbase coverage_test -o test_output.o -c simple.c");
    
    /* Invocation B: Sets state but causes backend failure
     * Uses invalid architecture to trigger failure after driver initialization
     * This ensures cleanup runs with error state
     */
    printf("Invocation B: Setting state with backend failure\n");
    execute_gcc("gcc -save-temps -dumpdir ./fail_artifacts -dumpbase fail_test -o fail_output.o -march=invalid-arch simple.c 2>/dev/null");
    
    /* Invocation C: Tests spec_machine and print_version
     * This may influence spec_machine and print_version variables
     */
    printf("Invocation C: Testing specs and version\n");
    execute_gcc("gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5");
    
    /* Invocation D: Tests with -Werror to generate different failure mode
     * Creates a file with a warning, then compiles with -Werror
     */
    printf("Invocation D: Testing -Werror failure path\n");
    FILE *f = fopen("warning.c", "w");
    if (f) {
        fputs("int main(void) { int x; return 0; } /* unused variable warning */\n", f);
        fclose(f);
        execute_gcc("gcc -Werror -save-temps -dumpdir ./test_artifacts -dumpbase werror_test -o werror_output.o warning.c 2>/dev/null");
    }
    
    /* Invocation E: Tests dumpbase_ext and outbase with different extensions
     * Uses -dumpbase-ext to set dumpbase_ext
     */
    printf("Invocation E: Testing dumpbase_ext\n");
    execute_gcc("gcc -save-temps -dumpdir ./test_artifacts -dumpbase ext_test -dumpbase-ext .extra -o ext_output.o -c simple.c");
    
    /* Invocation F: Tests verbose flag and help
     * This may set verbose_only_flag and print_help_list
     */
    printf("Invocation F: Testing verbose and help flags\n");
    execute_gcc("gcc -v -c simple.c 2>&1 | head -10");
    execute_gcc("gcc --help=common 2>&1 | head -5");
    
    /* Invocation G: Tests with -ftime-report to potentially set report_times_to_file */
    printf("Invocation G: Testing time reporting\n");
    execute_gcc("gcc -ftime-report -c simple.c 2>&1 | head -5");
    
    /* Invocation H: Multiple flags combination
     * Tests many variables at once
     */
    printf("Invocation H: Comprehensive flag combination\n");
    execute_gcc("gcc -save-temps=obj -dumpdir ./test_artifacts -dumpbase comprehensive -dumpbase-ext .test -o comp_output.o -v -c simple.c 2>&1 | tail -10");
    
    /* Clean up generated files */
    printf("Cleaning up test files...\n");
    remove_dir("test_artifacts");
    remove_dir("fail_artifacts");
    system("rm -f simple.c warning.c test_output.o fail_output.o werror_output.o ext_output.o comp_output.o");
    system("rm -f *.i *.s *.o 2>/dev/null");  /* Clean any stray save-temps files */
    
    printf("\n=== Test completed ===\n");
    printf("The GCC driver's cleanup routine should have been executed multiple times,\n");
    printf("resetting the variables: is_cpp_driver, at_file_supplied, print_help_list,\n");
    printf("print_version, verbose_only_flag, print_subprocess_help, use_ld,\n");
    printf("report_times_to_file, target_system_root, target_system_root_changed,\n");
    printf("target_sysroot_suffix, target_sysroot_hdrs_suffix, save_temps_flag,\n");
    printf("save_temps_overrides_dumpdir, dumpdir_trailing_dash_added,\n");
    printf("dumpdir, dumpbase, dumpbase_ext, outbase, dumpdir_length,\n");
    printf("outbase_length, spec_machine, and greatest_status.\n");
    
    return overall_result;
}
