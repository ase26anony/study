/* test_gcc_driver_cleanup.c
 * This test exercises the GCC driver's cleanup routine by invoking it
 * with various flags that set internal state variables, then ensuring
 * the cleanup block is executed.
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

/* C source with syntax error (to trigger failure after driver init) */
const char *error_c_content = 
"int main(void) {\n"
"    missing_function(); /* Undefined function */\n"
"    return 0;\n"
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
    fprintf(f, "%s", content);
    fclose(f);
    return 0;
}

/* Execute a GCC command and capture its return code */
int run_gcc_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int ret = system(cmd);
    if (ret == -1) {
        perror("system");
        return -1;
    }
    return WEXITSTATUS(ret);
}

/* Clean up generated files */
void cleanup_files(void) {
    /* Remove generated source files */
    remove("simple.c");
    remove("error.c");
    
    /* Remove object files */
    remove("test_output.o");
    remove("fail_output.o");
    
    /* Remove dump files */
    remove("coverage_test.*");
    remove("fail_test.*");
    
    /* Remove temporary directories and their contents */
    system("rm -rf ./test_artifacts");
    system("rm -rf ./fail_artifacts");
    system("rm -rf ./temp");
    
    /* Remove any other potential artifacts */
    remove("output.o");
    remove("mydump.*");
}

int main(void) {
    int overall_result = 0;
    
    printf("=== GCC Driver Cleanup Coverage Test ===\n\n");
    
    /* Create temporary directories for dumpdir */
    if (create_temp_dir("./test_artifacts") < 0 ||
        create_temp_dir("./fail_artifacts") < 0 ||
        create_temp_dir("./temp") < 0) {
        return 1;
    }
    
    /* Write test source files */
    if (write_file("simple.c", simple_c_content) < 0 ||
        write_file("error.c", error_c_content) < 0) {
        return 1;
    }
    
    printf("1. Testing successful compilation with state variables set...\n");
    /* Invocation A: Sets state variables, successful compilation
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * The -c flag stops before linking, making this a successful compile
     */
    const char *cmd_a = "gcc -save-temps -dumpdir ./test_artifarts -dumpbase coverage_test "
                       "-o test_output.o -c simple.c 2>&1";
    int result_a = run_gcc_command(cmd_a);
    printf("Result: %d (0=success)\n\n", result_a);
    
    printf("2. Testing compilation failure after driver initialization...\n");
    /* Invocation B: Sets state variables, fails at backend
     * Uses invalid architecture to cause failure after driver init
     * Also tests different dumpdir/dumpbase values
     */
    const char *cmd_b = "gcc -save-temps -dumpdir ./fail_artifacts -dumpbase fail_test "
                       "-o fail_output.o -march=invalid-arch simple.c 2>&1";
    int result_b = run_gcc_command(cmd_b);
    printf("Result: %d (non-zero expected)\n\n", result_b);
    
    printf("3. Testing with -Werror to create error exit status...\n");
    /* Invocation C: Uses -Werror to generate error from warning
     * Tests greatest_status reset
     */
    const char *cmd_c = "gcc -Werror -Wall -dumpdir ./temp -dumpbase mydump "
                       "-o output.o -c error.c 2>&1";
    int result_c = run_gcc_command(cmd_c);
    printf("Result: %d (non-zero expected)\n\n", result_c);
    
    printf("4. Testing with -specs and -V flags...\n");
    /* Invocation D: Tests spec_machine and print_version
     * -V triggers version printing, -specs influences spec_machine
     */
    const char *cmd_d = "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5";
    int result_d = run_gcc_command(cmd_d);
    printf("Result: %d\n\n", result_d);
    
    printf("5. Testing multiple flags combination...\n");
    /* Invocation E: Comprehensive test with many flags
     * Tests save_temps_flag, dumpdir, dumpbase, outbase, etc.
     */
    const char *cmd_e = "gcc -save-temps=obj -dumpdir . -dumpbase combined "
                       "-dumpbase-ext .ext -o combined.o -c simple.c 2>&1";
    int result_e = run_gcc_command(cmd_e);
    printf("Result: %d\n\n", result_e);
    
    printf("6. Testing linker error path...\n");
    /* Invocation F: Causes linker error (undefined library)
     * Tests cleanup after linker failure
     */
    const char *cmd_f = "gcc -save-temps -dumpdir ./temp -dumpbase linktest "
                       "-o linktest -lnonexistentlibrary simple.c 2>&1";
    int result_f = run_gcc_command(cmd_f);
    printf("Result: %d (non-zero expected)\n\n", result_f);
    
    printf("7. Testing verbose flag...\n");
    /* Invocation G: Tests verbose_only_flag */
    const char *cmd_g = "gcc -v -c simple.c 2>&1 | grep -i 'gcc version'";
    int result_g = run_gcc_command(cmd_g);
    printf("Result: %d\n\n", result_g);
    
    /* Clean up generated files */
    printf("Cleaning up test artifacts...\n");
    cleanup_files();
    
    printf("\n=== Test Complete ===\n");
    printf("All GCC invocations attempted. Check coverage data for gcc.cc lines 11228-11250.\n");
    
    /* We return success regardless of GCC command results because
     * we want to execute the cleanup block even on compilation failures */
    return 0;
}
