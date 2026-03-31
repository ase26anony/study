/* test_gcc_driver_cleanup.c
 * This test triggers the GCC driver's cleanup routine by invoking it
 * with various command-line flags that set the state variables being reset.
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

/* C source file with syntax error (for failure case) */
const char *error_c_content = 
"int main(void) {\n"
"    missing_semicolon\n"
"    return 0;\n"
"}\n";

/* Create a directory if it doesn't exist */
static int create_dir_if_not_exists(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        return mkdir(path, 0755);
    }
    return 0;
}

/* Remove a file if it exists */
static void remove_file_if_exists(const char *path) {
    if (access(path, F_OK) != -1) {
        unlink(path);
    }
}

/* Remove a directory and its contents */
static void remove_dir_recursive(const char *path) {
    char command[256];
    snprintf(command, sizeof(command), "rm -rf %s", path);
    system(command);
}

/* Run a GCC command and return the exit status */
static int run_gcc_command(const char *command) {
    printf("Running: %s\n", command);
    int status = system(command);
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    return -1;
}

int main(void) {
    int overall_status = 0;
    
    /* Create test directories */
    create_dir_if_not_exists("test_artifacts");
    create_dir_if_not_exists("fail_artifacts");
    
    /* Write simple C source files */
    FILE *fp = fopen("simple.c", "w");
    if (!fp) {
        perror("Failed to create simple.c");
        return 1;
    }
    fputs(simple_c_content, fp);
    fclose(fp);
    
    fp = fopen("error.c", "w");
    if (!fp) {
        perror("Failed to create error.c");
        return 1;
    }
    fputs(error_c_content, fp);
    fclose(fp);
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* Invocation A: Sets state with -save-temps, -dumpdir, -dumpbase, -o
     * Successful compilation (-c flag stops before linking)
     * This should set: save_temps_flag, dumpdir, dumpbase, outbase
     */
    printf("Invocation A: Successful compilation with state variables set\n");
    run_gcc_command("gcc -save-temps -dumpdir ./test_artifacts -dumpbase coverage_test "
                    "-o test_output.o -c simple.c 2>&1");
    
    /* Invocation B: Sets state but causes backend failure
     * Using invalid architecture flag to trigger failure after driver initialization
     * This ensures cleanup runs with non-zero exit status
     */
    printf("Invocation B: Compilation with invalid architecture (should fail)\n");
    run_gcc_command("gcc -save-temps -dumpdir ./fail_artifacts -dumpbase fail_test "
                    "-o fail_output.o -march=invalid-arch simple.c 2>&1");
    
    /* Invocation C: Uses -specs= and -V flags
     * These may influence spec_machine and print_version
     * The non-existent spec file will cause an error
     */
    printf("Invocation C: Using -specs and -V flags\n");
    run_gcc_command("gcc -specs=nosuch.spec -V simple.c 2>&1");
    
    /* Invocation D: Test with -Werror turning warnings into errors
     * This produces a different failure mode
     */
    printf("Invocation D: Using -Werror with a warning\n");
    run_gcc_command("gcc -Werror -Wunused-parameter -o warn_output error.c 2>&1");
    
    /* Invocation E: Test dumpbase_ext and outbase with different extensions */
    printf("Invocation E: Testing dumpbase_ext and outbase with extensions\n");
    run_gcc_command("gcc -save-temps -dumpdir ./test_artifacts -dumpbase myprog "
                    "-dumpbase-ext .ext -o myoutput.exe simple.c 2>&1");
    
    /* Invocation F: Test with verbose flag (verbose_only_flag) */
    printf("Invocation F: Testing with verbose flag\n");
    run_gcc_command("gcc -v -c simple.c 2>&1");
    
    /* Invocation G: Test with --help flag (print_help_list) */
    printf("Invocation G: Testing help flag\n");
    run_gcc_command("gcc --help 2>&1 | head -5");
    
    /* Invocation H: Test with --version flag (print_version) */
    printf("Invocation H: Testing version flag\n");
    run_gcc_command("gcc --version 2>&1");
    
    /* Invocation I: Test with -ftime-report (report_times_to_file) */
    printf("Invocation I: Testing time report flag\n");
    run_gcc_command("gcc -ftime-report -c simple.c 2>&1 | head -10");
    
    /* Invocation J: Test with -fuse-ld= (use_ld) */
    printf("Invocation J: Testing linker selection flag\n");
    run_gcc_command("gcc -fuse-ld=bfd -o ld_output simple.c 2>&1");
    
    /* Invocation K: Test with --sysroot (target_system_root) */
    printf("Invocation K: Testing sysroot flag\n");
    run_gcc_command("gcc --sysroot=/tmp/nonexistent -c simple.c 2>&1");
    
    /* Invocation L: Test save-temps overrides */
    printf("Invocation L: Testing save-temps with dumpdir\n");
    run_gcc_command("gcc -save-temps=obj -dumpdir ./override -c simple.c 2>&1");
    
    /* Clean up generated files */
    printf("\n=== Cleaning up test artifacts ===\n");
    
    /* Remove object files */
    remove_file_if_exists("test_output.o");
    remove_file_if_exists("fail_output.o");
    remove_file_if_exists("warn_output");
    remove_file_if_exists("myoutput.exe");
    remove_file_if_exists("ld_output");
    remove_file_if_exists("simple.o");
    
    /* Remove source files */
    remove_file_if_exists("simple.c");
    remove_file_if_exists("error.c");
    
    /* Remove save-temps intermediate files */
    remove_file_if_exists("simple.i");
    remove_file_if_exists("simple.s");
    remove_file_if_exists("error.i");
    remove_file_if_exists("error.s");
    
    /* Remove directories */
    remove_dir_recursive("test_artifacts");
    remove_dir_recursive("fail_artifacts");
    remove_dir_recursive("override");
    
    printf("\nTest completed. The GCC driver's cleanup routine should have been executed.\n");
    printf("Check coverage data for gcc.cc lines 11228-11250.\n");
    
    return overall_status;
}
