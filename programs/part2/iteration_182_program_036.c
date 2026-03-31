/* test_gcc_driver_cleanup.c
 * This test exercises the GCC driver's cleanup routine by invoking it
 * with various command-line options that set the state variables
 * mentioned in the uncovered lines (11228-11250 of gcc.cc).
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
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fprintf(f, "%s", SIMPLE_C_CONTENT);
    fclose(f);
    return 1;
}

/* Remove a file if it exists */
static void remove_file(const char *filename) {
    if (access(filename, F_OK) == 0) {
        unlink(filename);
    }
}

/* Remove a directory and its contents */
static void remove_directory(const char *dirname) {
    char command[256];
    snprintf(command, sizeof(command), "rm -rf %s", dirname);
    system(command);
}

/* Execute a GCC command and return its exit status */
static int execute_gcc_command(const char *command) {
    printf("Executing: %s\n", command);
    int status = system(command);
    if (status == -1) {
        perror("system");
        return -1;
    }
    return WEXITSTATUS(status);
}

int main(void) {
    int overall_status = 0;
    
    /* Create directories for test artifacts */
    mkdir("test_artifacts", 0755);
    mkdir("fail_artifacts", 0755);
    
    /* Create the simple C source file */
    if (!create_simple_c_file("simple.c")) {
        fprintf(stderr, "Failed to create simple.c\n");
        return 1;
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* Invocation A: Sets state variables with successful compilation
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Uses -c to stop before linking (successful compilation)
     */
    printf("--- Invocation A: Successful compilation with state variables set ---\n");
    const char *invocation_a = 
        "gcc -save-temps -dumpdir ./test_artifacts -dumpbase coverage_test "
        "-o test_output.o -c simple.c";
    int status_a = execute_gcc_command(invocation_a);
    printf("Exit status: %d\n\n", status_a);
    
    /* Invocation B: Sets state variables but causes backend failure
     * Uses invalid architecture to trigger failure after driver initialization
     * This ensures cleanup runs with non-zero exit status
     */
    printf("--- Invocation B: Failed compilation with different state variables ---\n");
    const char *invocation_b = 
        "gcc -save-temps -dumpdir ./fail_artifacts -dumpbase fail_test "
        "-o fail_output.o -march=invalid-arch simple.c 2>/dev/null";
    int status_b = execute_gcc_command(invocation_b);
    printf("Exit status: %d\n\n", status_b);
    
    /* Invocation C: Tests spec_machine and print_version
     * Uses -specs= and -V to influence driver state
     * The non-existent spec file will cause an error
     */
    printf("--- Invocation C: Testing spec_machine and version printing ---\n");
    const char *invocation_c = 
        "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5";
    int status_c = execute_gcc_command(invocation_c);
    printf("Exit status: %d\n\n", status_c);
    
    /* Invocation D: Tests -Werror turning warnings into errors
     * Uses an undefined function to generate a warning/error
     */
    printf("--- Invocation D: Testing -Werror for different failure mode ---\n");
    const char *invocation_d = 
        "echo 'int main(void) { undefined_function(); return 0; }' | "
        "gcc -save-temps -dumpdir ./werror_artifacts -dumpbase werror_test "
        "-o werror_output.o -Werror -x c - 2>/dev/null";
    int status_d = execute_gcc_command(invocation_d);
    printf("Exit status: %d\n\n", status_d);
    
    /* Invocation E: Tests dumpbase_ext and outbase with different extensions */
    printf("--- Invocation E: Testing dumpbase_ext and complex outbase ---\n");
    const char *invocation_e = 
        "gcc -save-temps -dumpdir ./ext_artifacts -dumpbase ext_test.dump "
        "-dumpbase-ext .extra -o complex_output.simple.o -c simple.c";
    int status_e = execute_gcc_command(invocation_e);
    printf("Exit status: %d\n\n", status_e);
    
    /* Invocation F: Tests verbose flag and report times */
    printf("--- Invocation F: Testing verbose flag and timing ---\n");
    const char *invocation_f = 
        "gcc -save-temps=obj -ftime-report -o time_output simple.c 2>&1 | "
        "grep -i 'time' | head -3";
    int status_f = execute_gcc_command(invocation_f);
    printf("Exit status: %d\n\n", status_f);
    
    /* Clean up generated files */
    printf("--- Cleaning up test artifacts ---\n");
    
    /* Remove object files */
    remove_file("test_output.o");
    remove_file("fail_output.o");
    remove_file("werror_output.o");
    remove_file("complex_output.simple.o");
    remove_file("time_output");
    
    /* Remove temporary files from -save-temps */
    remove_file("simple.i");
    remove_file("simple.s");
    remove_file("simple.o");
    remove_file("test_output.i");
    remove_file("test_output.s");
    remove_file("ext_test.dump.i");
    remove_file("ext_test.dump.s");
    remove_file("ext_test.dump.o");
    
    /* Remove test directories */
    remove_directory("test_artifacts");
    remove_directory("fail_artifacts");
    remove_directory("werror_artifacts");
    remove_directory("ext_artifacts");
    
    /* Remove source file */
    remove_file("simple.c");
    
    /* Remove any other potential temporary files */
    remove_file("a.out");
    
    printf("\n=== Test completed ===\n");
    printf("All GCC invocations attempted. Check coverage data for gcc.cc lines 11228-11250.\n");
    
    return overall_status;
}
