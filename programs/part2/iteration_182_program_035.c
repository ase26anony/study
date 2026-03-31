/* test_gcc_driver_cleanup.c
 * This test triggers the GCC driver's cleanup routine by invoking it
 * with various flags that set the state variables being reset in the
 * uncovered lines (11228-11250 of gcc.cc).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define SIMPLE_C_CONTENT "int main(void) { return 0; }\n"
#define INVALID_ARCH_C_CONTENT "int main(void) { return 0; }\n"

/* Create a simple C source file */
static int create_source_file(const char *filename, const char *content) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return 0;
    }
    fputs(content, fp);
    fclose(fp);
    return 1;
}

/* Execute a GCC command and capture its return code */
static int run_gcc_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int ret = system(cmd);
    printf("Return code: %d\n\n", ret);
    return ret;
}

/* Clean up generated files */
static void cleanup_files(void) {
    /* Remove source files */
    remove("simple.c");
    remove("invalid_arch.c");
    
    /* Remove object files */
    remove("test_output.o");
    remove("fail_output.o");
    
    /* Remove dump files */
    remove("coverage_test.*");
    remove("fail_test.*");
    
    /* Remove temporary files created by -save-temps */
    remove("simple.i");
    remove("simple.s");
    remove("invalid_arch.i");
    remove("invalid_arch.s");
    
    /* Remove output files */
    remove("output.exe");
    
    /* Remove dump directories */
    system("rm -rf ./test_artifacts");
    system("rm -rf ./fail_artifacts");
}

int main(void) {
    int overall_result = 0;
    
    /* Create necessary directories for dumpdir */
    mkdir("./test_artifacts", 0755);
    mkdir("./fail_artifacts", 0755);
    
    /* Create simple C source files */
    if (!create_source_file("simple.c", SIMPLE_C_CONTENT)) {
        fprintf(stderr, "Failed to create simple.c\n");
        return 1;
    }
    
    if (!create_source_file("invalid_arch.c", INVALID_ARCH_C_CONTENT)) {
        fprintf(stderr, "Failed to create invalid_arch.c\n");
        return 1;
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION A: Sets state variables with successful compilation
     * This sets:
     * - save_temps_flag via -save-temps
     * - dumpdir via -dumpdir ./test_artifacts
     * - dumpbase via -dumpbase coverage_test
     * - outbase via -o test_output.o
     * The -c flag ensures compilation stops before linking (successful)
     */
    printf("Invocation A: Successful compilation with state variables set\n");
    run_gcc_command("gcc -save-temps -dumpdir ./test_artifacts "
                    "-dumpbase coverage_test -o test_output.o -c simple.c");
    
    /* INVOCATION B: Sets state variables with compilation failure
     * Uses invalid architecture to cause backend failure after driver initialization
     * This ensures cleanup runs with failure status
     */
    printf("Invocation B: Failed compilation with different state variables\n");
    run_gcc_command("gcc -save-temps -dumpdir ./fail_artifacts "
                    "-dumpbase fail_test -o fail_output.o "
                    "-march=invalid-arch invalid_arch.c 2>&1 | head -20");
    
    /* INVOCATION C: Tests spec_machine and print_version
     * -specs= influences spec_machine
     * -V sets print_version flag
     */
    printf("Invocation C: Testing spec_machine and version printing\n");
    run_gcc_command("gcc -specs=nosuch.spec -V simple.c 2>&1 | head -10");
    
    /* INVOCATION D: Tests -Werror turning warnings into errors
     * This affects greatest_status with a different failure mode
     */
    printf("Invocation D: Testing -Werror for different exit status\n");
    run_gcc_command("gcc -Werror -Wall -o output.exe simple.c 2>&1");
    
    /* INVOCATION E: Tests dumpbase_ext and outbase_length
     * Using -dumpbase-ext to set dumpbase_ext
     */
    printf("Invocation E: Testing dumpbase_ext and outbase_length\n");
    run_gcc_command("gcc -dumpbase simple -dumpbase-ext .ext "
                    "-o long_output_name.o -c simple.c 2>&1");
    
    /* INVOCATION F: Tests target system root variables
     * Using --sysroot to influence target_system_root
     */
    printf("Invocation F: Testing target system root variables\n");
    run_gcc_command("gcc --sysroot=/nonexistent -c simple.c 2>&1 | head -5");
    
    /* INVOCATION G: Tests verbose flag and help
     * -v sets verbose_only_flag
     * --help sets print_help_list
     */
    printf("Invocation G: Testing verbose and help flags\n");
    run_gcc_command("gcc -v --help 2>&1 | head -5");
    
    /* INVOCATION H: Multiple flags combination
     * Tests interaction of multiple state variables
     */
    printf("Invocation H: Complex flag combination\n");
    run_gcc_command("gcc -save-temps=obj -dumpdir . -dumpbase complex "
                    "-dumpbase-ext .dump -specs=nosuch.spec -v "
                    "-o complex.o -c simple.c 2>&1 | head -10");
    
    /* Clean up generated files */
    printf("Cleaning up test files...\n");
    cleanup_files();
    
    printf("\n=== Test completed ===\n");
    printf("All GCC invocations attempted. Check coverage data for gcc.cc.\n");
    
    return overall_result;
}
