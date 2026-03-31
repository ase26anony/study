/* test_gcc_driver_cleanup.c
 * This test triggers the GCC driver's cleanup routine with various
 * state variables initialized to ensure coverage of the reset/free operations.
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
    fputs(simple_c_content, fp);
    fclose(fp);
    return 0;
}

/* Run a GCC command and capture its exit status */
int run_gcc_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Clean up generated files */
void cleanup_files(void) {
    /* Remove generated files from successful compilation */
    remove("simple.o");
    remove("simple.i");
    remove("simple.s");
    remove("test_output.o");
    remove("test_output.i");
    remove("test_output.s");
    remove("fail_output.o");
    remove("fail_output.i");
    remove("fail_output.s");
    
    /* Remove dump files */
    remove("coverage_test.*");
    remove("fail_test.*");
    
    /* Remove directories */
    rmdir("test_artifacts");
    rmdir("fail_artifacts");
    rmdir("temp");
    
    /* Remove source file */
    remove("simple.c");
}

int main(void) {
    int overall_status = 0;
    
    /* Create test directories */
    mkdir("test_artifacts", 0755);
    mkdir("fail_artifacts", 0755);
    mkdir("temp", 0755);
    
    /* Create the simple C source file */
    if (create_simple_c_file("simple.c") != 0) {
        return 1;
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION A: Sets state with -save-temps, dumpdir, dumpbase, and succeeds */
    printf("1. Testing successful compilation with state variables set:\n");
    const char *cmd_a = "gcc -save-temps -dumpdir ./test_artifacts "
                        "-dumpbase coverage_test -o test_output.o -c simple.c";
    run_gcc_command(cmd_a);
    
    /* INVOCATION B: Sets state but fails with invalid architecture */
    printf("2. Testing failed compilation with state variables set:\n");
    const char *cmd_b = "gcc -save-temps -dumpdir ./fail_artifacts "
                        "-dumpbase fail_test -o fail_output.o "
                        "-march=invalid-arch -c simple.c 2>/dev/null";
    run_gcc_command(cmd_b);
    
    /* INVOCATION C: Tests with -specs and -V flags */
    printf("3. Testing with -specs and -V flags:\n");
    const char *cmd_c = "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5";
    run_gcc_command(cmd_c);
    
    /* INVOCATION D: Tests -Werror turning warnings into errors */
    printf("4. Testing -Werror with a warning:\n");
    /* Create a file with a potential warning */
    FILE *fp = fopen("warn.c", "w");
    if (fp) {
        fputs("int main(void) {\n", fp);
        fputs("    int x;\n", fp);  /* Uninitialized variable warning */
        fputs("    return x;\n", fp);
        fputs("}\n", fp);
        fclose(fp);
        
        const char *cmd_d = "gcc -Werror -save-temps -dumpdir ./temp "
                            "-dumpbase warn_test -o warn_output.o -c warn.c 2>/dev/null";
        run_gcc_command(cmd_d);
        remove("warn.c");
    }
    
    /* INVOCATION E: Tests multiple dumpbase extensions */
    printf("5. Testing with dumpbase extensions:\n");
    const char *cmd_e = "gcc -save-temps -dumpdir . -dumpbase main "
                        "-dumpbase-ext .c -o simple.o -c simple.c";
    run_gcc_command(cmd_e);
    
    /* INVOCATION F: Tests verbose flag */
    printf("6. Testing verbose flag:\n");
    const char *cmd_f = "gcc -v -save-temps -o verbose_output.o -c simple.c 2>&1 | head -10";
    run_gcc_command(cmd_f);
    
    /* INVOCATION G: Tests help flags */
    printf("7. Testing help flags:\n");
    const char *cmd_g = "gcc --help=common 2>&1 | head -5";
    run_gcc_command(cmd_g);
    
    /* Clean up generated files */
    printf("Cleaning up test files...\n");
    cleanup_files();
    
    printf("\n=== Test completed ===\n");
    printf("The GCC driver's cleanup routine should have been executed multiple times,\n");
    printf("resetting variables like:\n");
    printf("  - save_temps_flag\n");
    printf("  - dumpdir, dumpbase, dumpbase_ext, outbase (freed and set to NULL)\n");
    printf("  - spec_machine\n");
    printf("  - greatest_status\n");
    
    return overall_status;
}
