/* test_gcc_driver_cleanup.c
 * 
 * This test program exercises the GCC driver's cleanup routine by invoking
 * it multiple times with different command-line arguments that set the
 * state variables mentioned in the uncovered lines.
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
        return -1;
    }
    fprintf(fp, "%s", simple_c_content);
    fclose(fp);
    return 0;
}

/* Remove a file if it exists */
void remove_file(const char *filename) {
    if (access(filename, F_OK) == 0) {
        unlink(filename);
    }
}

/* Remove a directory and its contents */
void remove_directory(const char *dirname) {
    char command[256];
    snprintf(command, sizeof(command), "rm -rf %s", dirname);
    system(command);
}

/* Execute a GCC command and return the exit status */
int execute_gcc_command(const char *command) {
    printf("Executing: %s\n", command);
    int status = system(command);
    if (status == -1) {
        perror("system() failed");
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
    if (create_simple_c_file("simple.c") != 0) {
        return 1;
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION A: Sets state variables and succeeds with -c (compile only)
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Should trigger cleanup with successful compilation
     */
    printf("1. Testing successful compilation with state variables set:\n");
    int status_a = execute_gcc_command(
        "gcc -save-temps -dumpdir ./test_artifacts -dumpbase coverage_test "
        "-o test_output.o -c simple.c 2>&1"
    );
    printf("Exit status: %d\n\n", status_a);
    
    /* INVOCATION B: Sets state variables and fails with invalid architecture
     * This ensures cleanup runs even on failure after argument parsing
     * Uses different dumpdir/dumpbase values to allocate new memory
     */
    printf("2. Testing failed compilation with state variables set:\n");
    int status_b = execute_gcc_command(
        "gcc -save-temps -dumpdir ./fail_artifacts -dumpbase fail_test "
        "-o fail_output.o -march=invalid-arch simple.c 2>&1"
    );
    printf("Exit status: %d\n\n", status_b);
    
    /* INVOCATION C: Tests with -specs and -V flags
     * May influence spec_machine and print_version
     * Note: -specs with non-existent file should cause error
     */
    printf("3. Testing with -specs and -V flags:\n");
    int status_c = execute_gcc_command(
        "gcc -specs=nosuch.spec -V simple.c 2>&1"
    );
    printf("Exit status: %d\n\n", status_c);
    
    /* INVOCATION D: Tests -Werror turning warnings into errors
     * Creates a file with a warning to trigger -Werror
     */
    printf("4. Testing -Werror with warning-containing source:\n");
    
    /* Create a file with an unused variable warning */
    FILE *fp = fopen("warning.c", "w");
    if (fp) {
        fprintf(fp, "int main(void) {\n");
        fprintf(fp, "    int unused = 42;  /* Trigger -Wunused-variable */\n");
        fprintf(fp, "    return 0;\n");
        fprintf(fp, "}\n");
        fclose(fp);
        
        int status_d = execute_gcc_command(
            "gcc -Werror -Wunused-variable -o warning_output warning.c 2>&1"
        );
        printf("Exit status: %d\n\n", status_d);
        
        remove_file("warning.c");
        remove_file("warning_output");
    }
    
    /* INVOCATION E: Tests dumpbase_ext and outbase_length
     * Uses -dumpbase with extension specification
     */
    printf("5. Testing dumpbase_ext and complex output names:\n");
    int status_e = execute_gcc_command(
        "gcc -dumpbase myprog.c -dumpbase-ext .ext "
        "-o long_output_name.o -c simple.c 2>&1"
    );
    printf("Exit status: %d\n\n", status_e);
    
    /* INVOCATION F: Tests verbose flag and report times
     * Uses -ftime-report to potentially set report_times_to_file
     */
    printf("6. Testing verbose and timing flags:\n");
    int status_f = execute_gcc_command(
        "gcc -v -ftime-report -o verbose_output simple.c 2>&1 | head -20"
    );
    printf("Exit status: %d\n\n", status_f);
    
    /* Clean up generated files */
    printf("Cleaning up test artifacts...\n");
    
    /* Remove object files */
    remove_file("test_output.o");
    remove_file("fail_output.o");
    remove_file("verbose_output");
    remove_file("long_output_name.o");
    
    /* Remove simple.c */
    remove_file("simple.c");
    
    /* Remove save-temps intermediate files */
    remove_file("simple.i");
    remove_file("simple.s");
    remove_file("coverage_test.i");
    remove_file("coverage_test.s");
    remove_file("fail_test.i");
    remove_file("fail_test.s");
    remove_file("myprog.c.i");
    remove_file("myprog.c.s");
    
    /* Remove directories */
    remove_directory("test_artifacts");
    remove_directory("fail_artifacts");
    
    printf("\n=== Test completed ===\n");
    printf("All GCC invocations attempted. Check coverage data for gcc.cc\n");
    
    return overall_status;
}
