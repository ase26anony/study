/* test_gcc_driver_cleanup.c
 * 
 * This test program exercises the GCC driver's cleanup routine by invoking
 * it multiple times with different command-line flags that set the state
 * variables being reset in the uncovered block (lines 11228-11250 of gcc.cc).
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

/* Create a temporary directory for test artifacts */
void create_temp_dir(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mkdir(path, 0700);
    }
}

/* Remove a directory and its contents */
void remove_dir(const char *path) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
    system(cmd);
}

/* Execute a GCC command and return its exit status */
int execute_gcc_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    return -1;
}

int main(void) {
    int overall_status = 0;
    
    /* Create directories for test artifacts */
    create_temp_dir("test_artifacts");
    create_temp_dir("fail_artifacts");
    
    /* Write the simple C source file */
    FILE *fp = fopen("simple.c", "w");
    if (!fp) {
        perror("Failed to create simple.c");
        return 1;
    }
    fputs(simple_c_content, fp);
    fclose(fp);
    
    /* Create a dummy specs file to test -specs flag */
    fp = fopen("dummy.specs", "w");
    if (fp) {
        fputs("*cpp:\n", fp);
        fclose(fp);
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION A: Sets state variables with successful compilation
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * The -c flag ensures compilation stops before linking (successful)
     */
    printf("Invocation A: Setting state with successful compilation\n");
    execute_gcc_command("gcc -save-temps -dumpdir ./test_artifacts "
                        "-dumpbase coverage_test -o test_output.o "
                        "-c simple.c 2>&1");
    
    /* INVOCATION B: Sets state but causes backend failure
     * Uses invalid architecture to trigger failure after driver initialization
     * This ensures cleanup runs with non-zero greatest_status
     */
    printf("Invocation B: Setting state with backend failure\n");
    execute_gcc_command("gcc -save-temps -dumpdir ./fail_artifacts "
                        "-dumpbase fail_test -o fail_output.o "
                        "-march=invalid-arch simple.c 2>&1");
    
    /* INVOCATION C: Tests different state variables
     * -specs influences spec_machine, -V sets print_version
     * This may not compile but triggers different initialization paths
     */
    printf("Invocation C: Testing specs and version flags\n");
    execute_gcc_command("gcc -specs=dummy.specs -V simple.c 2>&1");
    
    /* INVOCATION D: Tests -Werror turning warnings into errors
     * Uses a source file with a warning to trigger error exit status
     */
    printf("Invocation E: Testing with -Werror\n");
    execute_gcc_command("gcc -save-temps -dumpdir ./test_artifacts "
                        "-dumpbase werror_test -o werror_output.o "
                        "-Werror -Wunused-parameter simple.c 2>&1");
    
    /* INVOCATION F: Tests verbose flag and report times
     * -ftime-report sets report_times_to_file
     * -v sets verbose flag (influences verbose_only_flag)
     */
    printf("Invocation F: Testing verbose and timing flags\n");
    execute_gcc_command("gcc -save-temps -ftime-report -v "
                        "-dumpdir ./test_artifacts -dumpbase verbose_test "
                        "-o verbose_output.o -c simple.c 2>&1");
    
    /* INVOCATION G: Tests help flags
     * --help=common sets print_help_list
     */
    printf("Invocation G: Testing help flags\n");
    execute_gcc_command("gcc --help=common 2>&1");
    
    /* INVOCATION H: Tests with different output base variations
     * Multiple -dumpbase and -dumpdir combinations
     */
    printf("Invocation H: Testing various dumpbase/dumpdir combinations\n");
    execute_gcc_command("gcc -dumpbase base1 -dumpbase-ext .ext "
                        "-dumpdir dir1/ -o output1.o -c simple.c 2>&1");
    
    /* Clean up generated files */
    printf("Cleaning up test artifacts...\n");
    remove_dir("test_artifacts");
    remove_dir("fail_artifacts");
    
    /* Remove individual files */
    system("rm -f simple.c dummy.specs");
    system("rm -f test_output.* fail_output.* werror_output.* verbose_output.* output1.*");
    system("rm -f *.i *.s *.o 2>/dev/null");
    
    printf("\n=== Test completed ===\n");
    printf("The GCC driver's cleanup routine should have been executed multiple times,\n");
    printf("resetting the variables in the uncovered block (lines 11228-11250 of gcc.cc).\n");
    
    return overall_status;
}
