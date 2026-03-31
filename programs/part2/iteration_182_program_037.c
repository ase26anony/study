/* test_gcc_driver_cleanup.c
 * This test triggers the GCC driver's cleanup routine by invoking it
 * with various flags that set the state variables mentioned in the
 * uncovered lines (11228-11250 of gcc.cc).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Simple test C program that will be compiled */
const char *simple_c_content = 
"int main(void) {\n"
"    return 0;\n"
"}\n";

/* Another test with a deliberate error for failure testing */
const char *error_c_content = 
"int main(void) {\n"
"    /* Deliberate syntax error to cause compilation failure */\n"
"    return missing_variable;\n"
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
    if (status != -1 && WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

int main(void) {
    int overall_status = 0;
    
    /* Create test directories */
    create_dir_if_not_exists("test_artifacts");
    create_dir_if_not_exists("fail_artifacts");
    
    /* Write test source files */
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
    
    /* INVOCATION A: Sets state variables with successful compilation
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Uses -c to stop before linking (successful compilation) */
    printf("1. Testing successful compilation with state variables set:\n");
    int status_a = run_gcc_command(
        "gcc -save-temps -dumpdir ./test_artifacts "
        "-dumpbase coverage_test -o test_output.o -c simple.c"
    );
    printf("Exit status: %d\n\n", status_a);
    
    /* INVOCATION B: Sets state variables with compilation failure
     * Uses invalid architecture flag to cause backend failure
     * after driver initialization */
    printf("2. Testing failed compilation with state variables set:\n");
    int status_b = run_gcc_command(
        "gcc -save-temps -dumpdir ./fail_artifacts "
        "-dumpbase fail_test -o fail_output.o -march=invalid-arch simple.c 2>/dev/null"
    );
    printf("Exit status: %d\n\n", status_b);
    
    /* INVOCATION C: Tests with -specs and -V flags
     * Influences spec_machine and print_version */
    printf("3. Testing with -specs and -V flags:\n");
    int status_c = run_gcc_command(
        "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5"
    );
    printf("Exit status: %d\n\n", status_c);
    
    /* INVOCATION D: Tests with -Werror to create error exit status
     * Uses a file with an actual warning/error */
    printf("4. Testing with -Werror for error exit status:\n");
    int status_d = run_gcc_command(
        "gcc -Werror -save-temps -dumpdir ./test_artifacts "
        "-dumpbase werror_test -o werror_output.o error.c 2>/dev/null"
    );
    printf("Exit status: %d\n\n", status_d);
    
    /* INVOCATION E: Tests dumpbase_ext and outbase with different extensions */
    printf("5. Testing with dumpbase_ext and complex output names:\n");
    int status_e = run_gcc_command(
        "gcc -save-temps -dumpdir ./test_artifacts "
        "-dumpbase complex -dumpbase_ext .extra -o complex_output.obj -c simple.c"
    );
    printf("Exit status: %d\n\n", status_e);
    
    /* INVOCATION F: Tests verbose flag and print_help_list */
    printf("6. Testing verbose and help-related flags:\n");
    int status_f1 = run_gcc_command("gcc -v -c simple.c 2>&1 | head -10");
    int status_f2 = run_gcc_command("gcc --help=common 2>&1 | head -5");
    printf("Exit statuses: %d, %d\n\n", status_f1, status_f2);
    
    /* INVOCATION G: Multiple invocations to test state persistence
     * First sets variables, second uses different values */
    printf("7. Testing multiple sequential invocations:\n");
    int status_g1 = run_gcc_command(
        "gcc -save-temps=obj -dumpdir ./first -dumpbase first "
        "-o first.o -c simple.c"
    );
    int status_g2 = run_gcc_command(
        "gcc -save-temps=cwd -dumpdir ./second -dumpbase second "
        "-o second.o -c simple.c"
    );
    printf("Exit statuses: %d, %d\n\n", status_g1, status_g2);
    
    /* Clean up generated files */
    printf("Cleaning up test files...\n");
    
    /* Remove object files */
    remove_file_if_exists("test_output.o");
    remove_file_if_exists("fail_output.o");
    remove_file_if_exists("werror_output.o");
    remove_file_if_exists("complex_output.obj");
    remove_file_if_exists("first.o");
    remove_file_if_exists("second.o");
    
    /* Remove source files */
    remove_file_if_exists("simple.c");
    remove_file_if_exists("error.c");
    
    /* Remove temporary files created by -save-temps */
    remove_file_if_exists("simple.i");
    remove_file_if_exists("simple.s");
    remove_file_if_exists("error.i");
    remove_file_if_exists("error.s");
    remove_file_if_exists("coverage_test.i");
    remove_file_if_exists("coverage_test.s");
    remove_file_if_exists("fail_test.i");
    remove_file_if_exists("fail_test.s");
    remove_file_if_exists("werror_test.i");
    remove_file_if_exists("werror_test.s");
    remove_file_if_exists("complex.i");
    remove_file_if_exists("complex.s");
    remove_file_if_exists("first.i");
    remove_file_if_exists("first.s");
    remove_file_if_exists("second.i");
    remove_file_if_exists("second.s");
    
    /* Remove test directories */
    remove_dir_recursive("test_artifacts");
    remove_dir_recursive("fail_artifacts");
    remove_dir_recursive("first");
    remove_dir_recursive("second");
    
    printf("\n=== Test completed ===\n");
    printf("All GCC invocations attempted. Check coverage data for gcc.cc lines 11228-11250.\n");
    
    return overall_status;
}
