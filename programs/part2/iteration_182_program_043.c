/* test_gcc_driver_cleanup.c
 * This test program exercises the GCC driver's cleanup routine by invoking
 * it multiple times with different flags that set the state variables
 * mentioned in the uncovered lines (11228-11250 of gcc.cc).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

/* Simple C source file content */
#define SIMPLE_C_CONTENT "int main(void) { return 0; }"

/* C source file with a deliberate error for failure testing */
#define ERROR_C_CONTENT "int main(void) { return undefined_variable; }"

/* Create a temporary directory for test artifacts */
static int create_temp_dir(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        return mkdir(path, 0755);
    }
    return 0;
}

/* Remove a directory and its contents */
static int remove_dir(const char *path) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
    return system(cmd);
}

/* Execute a GCC command and return its exit status */
static int run_gcc(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    printf("Command failed to execute properly\n\n");
    return -1;
}

/* Write a file with given content */
static int write_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to open file for writing");
        return -1;
    }
    fputs(content, f);
    fclose(f);
    return 0;
}

int main(void) {
    int overall_result = 0;
    
    /* Create temporary directories for test artifacts */
    create_temp_dir("test_artifacts");
    create_temp_dir("fail_artifacts");
    create_temp_dir("dump_test");
    
    /* Write test source files */
    if (write_file("simple.c", SIMPLE_C_CONTENT) < 0) {
        return 1;
    }
    if (write_file("error.c", ERROR_C_CONTENT) < 0) {
        return 1;
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION A: Sets state variables with successful compilation
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Uses -c flag to stop before linking (successful compilation)
     */
    printf("Invocation A: Setting state with successful compilation\n");
    printf("--------------------------------------------------------\n");
    run_gcc("gcc -save-temps -dumpdir ./test_artifacts -dumpbase coverage_test "
            "-o test_output.o -c simple.c");
    
    /* INVOCATION B: Sets state but causes compilation failure
     * Uses invalid architecture flag to trigger backend failure
     * after driver initialization
     */
    printf("Invocation B: Setting state with compilation failure\n");
    printf("----------------------------------------------------\n");
    run_gcc("gcc -save-temps -dumpdir ./fail_artifacts -dumpbase fail_test "
            "-o fail_output.o -march=invalid-arch simple.c 2>/dev/null");
    
    /* INVOCATION C: Tests with -specs flag and version printing
     * This may influence spec_machine and print_version
     */
    printf("Invocation C: Testing with specs and version flags\n");
    printf("---------------------------------------------------\n");
    run_gcc("gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5");
    
    /* INVOCATION D: Tests -dumpbase with extension
     * Exercises dumpbase_ext variable
     */
    printf("Invocation D: Testing dumpbase with extension\n");
    printf("---------------------------------------------\n");
    run_gcc("gcc -dumpbase myprog.c -dumpbase-ext .ext -c simple.c");
    
    /* INVOCATION E: Tests -dumpdir with trailing slash
     * Exercises dumpdir_trailing_dash_added logic
     */
    printf("Invocation E: Testing dumpdir with trailing slash\n");
    printf("-----------------------------------------------\n");
    run_gcc("gcc -dumpdir ./dump_test/ -dumpbase dump_test -c simple.c");
    
    /* INVOCATION F: Tests save-temps=obj
     * Different value for save_temps_flag
     */
    printf("Invocation F: Testing save-temps=obj\n");
    printf("------------------------------------\n");
    run_gcc("gcc -save-temps=obj -dumpdir ./test_artifacts "
            "-o obj_output.o -c simple.c");
    
    /* INVOCATION G: Tests with -Werror to create error exit status
     * Exercises greatest_status reset logic
     */
    printf("Invocation G: Testing with -Werror for error status\n");
    printf("---------------------------------------------------\n");
    run_gcc("gcc -Werror -Wall -c simple.c 2>/dev/null");
    
    /* INVOCATION H: Tests compilation with actual error
     * Uses error.c with undefined variable
     */
    printf("Invocation H: Testing with compilation error\n");
    printf("-------------------------------------------\n");
    run_gcc("gcc -c error.c 2>/dev/null");
    
    /* INVOCATION I: Tests linking failure
     * Requests non-existent library
     */
    printf("Invocation I: Testing linking failure\n");
    printf("-------------------------------------\n");
    run_gcc("gcc simple.c -lnonexistentlibrary 2>/dev/null");
    
    /* INVOCATION J: Tests verbose flag
     * Exercises verbose_only_flag
     */
    printf("Invocation J: Testing verbose flag\n");
    printf("----------------------------------\n");
    run_gcc("gcc -### -c simple.c 2>&1 | head -3");
    
    /* Clean up generated files */
    printf("Cleaning up test artifacts...\n");
    system("rm -f simple.c error.c");
    system("rm -f *.o *.i *.s *.ii *.bc");
    system("rm -f test_output.* fail_output.* obj_output.*");
    remove_dir("test_artifacts");
    remove_dir("fail_artifacts");
    remove_dir("dump_test");
    
    printf("\n=== Test completed ===\n");
    printf("The GCC driver's cleanup routine should have been exercised multiple times.\n");
    printf("Check coverage data for gcc.cc lines 11228-11250.\n");
    
    return overall_result;
}
