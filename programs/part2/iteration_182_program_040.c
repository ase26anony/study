/* test_gcc_driver_cleanup.c
 * This test triggers the GCC driver's cleanup routine to cover lines 11228-11250 in gcc.cc
 * It invokes GCC multiple times with different flags to set the target variables.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define SIMPLE_C_CONTENT "int main(void) { return 0; }\n"
#define INVALID_C_CONTENT "int main(void) { return missing_variable; }\n"

/* Create a simple C source file */
static int create_source_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fputs(content, f);
    fclose(f);
    return 1;
}

/* Execute a GCC command and return its exit status */
static int run_gcc_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != -1 && WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    }
    return status;
}

/* Clean up generated files */
static void cleanup_files(void) {
    /* Remove source files */
    remove("simple.c");
    remove("invalid.c");
    
    /* Remove object files and temporary files */
    remove("test_output.o");
    remove("fail_output.o");
    remove("output_prog");
    
    /* Remove dump files and directories */
    system("rm -rf ./test_artifacts ./fail_artifacts ./temp_coverage");
    
    /* Remove save-temps files */
    remove("simple.i");
    remove("simple.s");
    remove("simple.o");
    remove("invalid.i");
    remove("invalid.s");
    remove("invalid.o");
    
    /* Remove dumpbase files */
    remove("coverage_test.*");
    remove("fail_test.*");
    remove("mydump.*");
}

int main(void) {
    int overall_status = 0;
    
    /* Create necessary directories */
    mkdir("./test_artifacts", 0755);
    mkdir("./fail_artifacts", 0755);
    mkdir("./temp_coverage", 0755);
    
    /* Create test source files */
    if (!create_source_file("simple.c", SIMPLE_C_CONTENT)) {
        fprintf(stderr, "Failed to create simple.c\n");
        return 1;
    }
    
    if (!create_source_file("invalid.c", INVALID_C_CONTENT)) {
        fprintf(stderr, "Failed to create invalid.c\n");
        return 1;
    }
    
    printf("=== GCC Driver Cleanup Coverage Test ===\n\n");
    
    /* Invocation 1: Successful compilation with -save-temps and dump options
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     */
    printf("1. Successful compilation with state variables set:\n");
    run_gcc_command("gcc -save-temps -dumpdir ./test_artifacts "
                    "-dumpbase coverage_test -o test_output.o -c simple.c");
    
    /* Invocation 2: Compilation with syntax error (fails in compiler proper)
     * This ensures cleanup runs after a compilation failure
     */
    printf("\n2. Compilation with syntax error (late failure):\n");
    run_gcc_command("gcc -save-temps -dumpdir ./fail_artifacts "
                    "-dumpbase fail_test -o fail_output.o -c invalid.c");
    
    /* Invocation 3: Use invalid architecture flag (backend failure)
     * This triggers cleanup after driver accepts the flag but backend rejects it
     */
    printf("\n3. Invalid architecture flag (backend failure):\n");
    run_gcc_command("gcc -save-temps -march=invalid-arch -o output_prog simple.c 2>/dev/null");
    
    /* Invocation 4: Link with non-existent library (linker failure)
     * Tests cleanup after linker error
     */
    printf("\n4. Linker error with non-existent library:\n");
    run_gcc_command("gcc -lnosuchlibrary999 -o output_prog simple.c 2>/dev/null");
    
    /* Invocation 5: Use -Werror to turn warnings into errors
     * Tests different failure mode
     */
    printf("\n5. Compilation with -Werror:\n");
    run_gcc_command("gcc -Werror -Wall -o output_prog simple.c 2>/dev/null");
    
    /* Invocation 6: Test with -specs= and -V flags
     * May influence spec_machine and print_version
     */
    printf("\n6. Testing with -specs and -V flags:\n");
    run_gcc_command("gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5");
    
    /* Invocation 7: Multiple dump options with different extensions
     * Tests dumpbase_ext and related variables
     */
    printf("\n7. Multiple dump options with extensions:\n");
    run_gcc_command("gcc -save-temps -dumpdir ./temp_coverage "
                    "-dumpbase mydump -dumpbase-ext .ext "
                    "-o output_prog simple.c 2>/dev/null");
    
    /* Invocation 8: Test verbose flag combinations
     * May influence verbose_only_flag
     */
    printf("\n8. Verbose flag combinations:\n");
    run_gcc_command("gcc -v -save-temps -dumpdir ./temp_coverage -o output_prog simple.c 2>/dev/null");
    
    /* Invocation 9: Test help and version flags
     * May influence print_help_list, print_version
     */
    printf("\n9. Help and version flags:\n");
    run_gcc_command("gcc --help");
    run_gcc_command("gcc --version");
    
    /* Invocation 10: Complex combination of all relevant flags
     * Maximizes state variable initialization
     */
    printf("\n10. Complex flag combination:\n");
    run_gcc_command("gcc -save-temps=obj -dumpdir ./temp_coverage "
                    "-dumpbase complex_test -dumpbase-ext .myext "
                    "-specs=/dev/null -v -Werror -Wall "
                    "-o final_output simple.c 2>/dev/null");
    
    /* Invocation 11: Test with -ftime-report and -fmem-report
     * May influence report_times_to_file usage
     */
    printf("\n11. Time and memory reporting flags:\n");
    run_gcc_command("gcc -ftime-report -fmem-report -o output_prog simple.c 2>/dev/null");
    
    /* Invocation 12: Test sysroot options
     * May influence target_system_root and related variables
     */
    printf("\n12. Sysroot options:\n");
    run_gcc_command("gcc --sysroot=/tmp -o output_prog simple.c 2>/dev/null");
    
    printf("\n=== All invocations completed ===\n");
    
    /* Clean up generated files */
    cleanup_files();
    
    return overall_status;
}
