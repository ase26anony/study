/* test_gcc_driver_cleanup.c
 * 
 * This test program exercises the GCC driver's cleanup routine by invoking
 * it with various command-line flags that set the state variables being
 * reset in the uncovered block (lines 11228-11250 of gcc.cc).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Simple C source file content */
static const char simple_c_content[] = 
"int main(void) {\n"
"    return 0;\n"
"}\n";

/* Create a simple C source file */
static int create_simple_c_file(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return 0;
    }
    fputs(simple_c_content, fp);
    fclose(fp);
    return 1;
}

/* Run a GCC command and capture its return status */
static int run_gcc_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    printf("Command terminated abnormally\n\n");
    return -1;
}

/* Clean up generated files */
static void cleanup_files(void) {
    /* Remove generated source file */
    unlink("simple.c");
    
    /* Remove object files */
    unlink("test_output.o");
    unlink("fail_output.o");
    unlink("output.o");
    
    /* Remove dump files */
    unlink("coverage_test.*");
    unlink("fail_test.*");
    unlink("mydump.*");
    
    /* Remove temporary files from -save-temps */
    unlink("simple.i");
    unlink("simple.s");
    unlink("test_output.i");
    unlink("test_output.s");
    unlink("fail_output.i");
    unlink("fail_output.s");
    
    /* Remove directories */
    rmdir("test_artifacts");
    rmdir("fail_artifacts");
    rmdir("./temp");
}

int main(void) {
    int overall_status = 0;
    
    /* Create the simple C source file */
    if (!create_simple_c_file("simple.c")) {
        fprintf(stderr, "Failed to create simple.c\n");
        return 1;
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION A: Sets state variables and succeeds
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * The -c flag ensures compilation succeeds (creates object file)
     */
    printf("1. Testing successful compilation with state variables set:\n");
    run_gcc_command("gcc -save-temps -dumpdir ./test_artifacts "
                    "-dumpbase coverage_test -o test_output.o -c simple.c");
    
    /* INVOCATION B: Sets state variables and fails late
     * This sets different values for dumpdir, dumpbase, outbase
     * Uses invalid architecture to cause backend failure
     */
    printf("2. Testing failed compilation with state variables set:\n");
    run_gcc_command("gcc -save-temps -dumpdir ./fail_artifacts "
                    "-dumpbase fail_test -o fail_output.o "
                    "-march=invalid-arch simple.c 2>/dev/null");
    
    /* INVOCATION C: Tests different specs and version printing
     * This may influence spec_machine and print_version
     */
    printf("3. Testing with specs and version flag:\n");
    run_gcc_command("gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5");
    
    /* INVOCATION D: Tests dumpbase_ext and outbase with different extensions */
    printf("4. Testing with dumpbase_ext and explicit output:\n");
    run_gcc_command("gcc -dumpbase mydump -dumpbase-ext .ext "
                    "-o output.o -c simple.c");
    
    /* INVOCATION E: Tests verbose flag and report times */
    printf("5. Testing verbose and time reporting:\n");
    run_gcc_command("gcc -ftime-report -c simple.c 2>&1 | head -10");
    
    /* INVOCATION F: Tests with -Werror to generate error status */
    printf("6. Testing with -Werror to generate non-zero exit status:\n");
    /* First create a file with a warning */
    FILE *fp = fopen("warn.c", "w");
    if (fp) {
        fputs("int main(void) { int x; return 0; } /* unused variable */\n", fp);
        fclose(fp);
        run_gcc_command("gcc -Werror -c warn.c 2>/dev/null");
        unlink("warn.c");
        unlink("warn.o");
    }
    
    /* INVOCATION G: Tests target system root flags */
    printf("7. Testing with sysroot flags:\n");
    run_gcc_command("gcc --sysroot=/nonexistent -c simple.c 2>/dev/null");
    
    /* INVOCATION H: Tests help and version flags that set print_* variables */
    printf("8. Testing help and version flags:\n");
    run_gcc_command("gcc --help | head -5");
    run_gcc_command("gcc --version | head -2");
    
    /* Clean up generated files */
    printf("Cleaning up generated files...\n");
    cleanup_files();
    
    printf("\n=== Test completed ===\n");
    printf("The GCC driver's cleanup routine should have been executed\n");
    printf("multiple times with different state variable configurations.\n");
    
    return overall_status;
}
