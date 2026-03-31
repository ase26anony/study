/* test_gcc_driver_cleanup.c
 * This test triggers the GCC driver's cleanup routine by invoking it
 * with various flags that set the state variables being reset.
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

/* C source file with a deliberate error (for failure testing) */
const char *error_c_content = 
"int main(void) {\n"
"    /* Deliberate error: missing semicolon */\n"
"    return 0\n"
"}\n";

/* Create a temporary directory for test artifacts */
int create_temp_dir(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) != 0) {
            perror("mkdir");
            return -1;
        }
    }
    return 0;
}

/* Write a file with given content */
int write_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return -1;
    }
    fputs(content, f);
    fclose(f);
    return 0;
}

/* Execute a GCC command and capture its return code */
int run_gcc_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int ret = system(cmd);
    if (ret != -1) {
        if (WIFEXITED(ret)) {
            printf("Exit code: %d\n", WEXITSTATUS(ret));
        }
    }
    return ret;
}

/* Clean up generated files */
void cleanup_files(void) {
    /* Remove generated object files */
    system("rm -f test_output.o fail_output.o output.o 2>/dev/null");
    
    /* Remove dump files and directories */
    system("rm -rf ./test_artifacts ./fail_artifacts ./temp 2>/dev/null");
    
    /* Remove save-temps files */
    system("rm -f simple.i simple.s simple.o 2>/dev/null");
    system("rm -f error.i error.s error.o 2>/dev/null");
    
    /* Remove dumpbase files */
    system("rm -f coverage_test.* fail_test.* mydump.* 2>/dev/null");
    
    /* Remove source files */
    system("rm -f simple.c error.c 2>/dev/null");
}

int main(void) {
    int overall_result = 0;
    
    /* Clean up any existing files first */
    cleanup_files();
    
    /* Create temporary directories for dumpdir */
    create_temp_dir("./test_artifacts");
    create_temp_dir("./fail_artifacts");
    create_temp_dir("./temp");
    
    /* Write test source files */
    if (write_file("simple.c", simple_c_content) != 0) {
        fprintf(stderr, "Failed to write simple.c\n");
        return 1;
    }
    
    if (write_file("error.c", error_c_content) != 0) {
        fprintf(stderr, "Failed to write error.c\n");
        return 1;
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION A: Sets state variables and succeeds
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Should trigger cleanup with successful compilation
     */
    printf("1. Invocation A: Successful compilation with state variables set\n");
    const char *cmd_a = "gcc -save-temps -dumpdir ./test_artifacts "
                       "-dumpbase coverage_test -o test_output.o -c simple.c";
    run_gcc_command(cmd_a);
    printf("\n");
    
    /* INVOCATION B: Sets state variables and fails (backend error)
     * Uses invalid architecture flag to cause failure after driver initialization
     * This ensures cleanup runs with non-zero greatest_status
     */
    printf("2. Invocation B: Failed compilation with different state variables\n");
    const char *cmd_b = "gcc -save-temps -dumpdir ./fail_artifacts "
                       "-dumpbase fail_test -o fail_output.o "
                       "-march=invalid-arch -mtune=invalid-tune simple.c 2>/dev/null";
    run_gcc_command(cmd_b);
    printf("\n");
    
    /* INVOCATION C: Tests with -specs= and -V flags
     * These may influence spec_machine and print_version
     * The non-existent spec file will cause an error
     */
    printf("3. Invocation C: Testing spec_machine and version flags\n");
    const char *cmd_c = "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5";
    run_gcc_command(cmd_c);
    printf("\n");
    
    /* INVOCATION D: Tests -Werror turning warnings into errors
     * Uses a source file with a warning that becomes an error
     */
    printf("4. Invocation D: Testing -Werror path\n");
    const char *cmd_d = "gcc -Werror -Wall -o output.o -c simple.c 2>/dev/null";
    run_gcc_command(cmd_d);
    printf("\n");
    
    /* INVOCATION E: Tests syntax error that's caught late
     * Uses the error.c file with a missing semicolon
     * This fails in the compiler proper, not the driver
     */
    printf("5. Invocation E: Testing late compilation failure\n");
    const char *cmd_e = "gcc -save-temps -dumpdir ./temp -dumpbase mydump "
                       "-o error_output.o error.c 2>/dev/null";
    run_gcc_command(cmd_e);
    printf("\n");
    
    /* INVOCATION F: Tests linker error path
     * Requests a non-existent library
     */
    printf("6. Invocation F: Testing linker error path\n");
    const char *cmd_f = "gcc simple.c -lnonexistentlibrary 2>/dev/null";
    run_gcc_command(cmd_f);
    printf("\n");
    
    /* INVOCATION G: Tests multiple flag combinations
     * Combines several flags that affect the target variables
     */
    printf("7. Invocation G: Testing combined flags\n");
    const char *cmd_g = "gcc -save-temps=obj -dumpdir . -dumpbase combined "
                       "-specs=/dev/null -ftime-report -o combined.o -c simple.c 2>&1";
    run_gcc_command(cmd_g);
    printf("\n");
    
    /* INVOCATION H: Tests verbose flag (affects verbose_only_flag)
     */
    printf("8. Invocation H: Testing verbose output\n");
    const char *cmd_h = "gcc -v -c simple.c 2>&1 | tail -3";
    run_gcc_command(cmd_h);
    printf("\n");
    
    /* Clean up generated files */
    printf("Cleaning up test files...\n");
    cleanup_files();
    
    printf("\n=== Test completed ===\n");
    printf("All GCC invocations attempted. Check coverage data for gcc.cc cleanup block.\n");
    
    return overall_result;
}
