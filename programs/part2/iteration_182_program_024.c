/* test_gcc_driver_cleanup.c
 * This test triggers the GCC driver's cleanup routine by invoking it
 * multiple times with different flags that set the state variables
 * mentioned in the uncovered lines (11228-11250 of gcc.cc).
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
void create_temp_dir(const char *dir_name) {
    struct stat st = {0};
    if (stat(dir_name, &st) == -1) {
        mkdir(dir_name, 0755);
    }
}

/* Remove a directory and its contents */
void remove_dir(const char *dir_name) {
    char command[256];
    snprintf(command, sizeof(command), "rm -rf %s", dir_name);
    system(command);
}

/* Execute a GCC command and return its exit status */
int execute_gcc_command(const char *command) {
    printf("Executing: %s\n", command);
    int status = system(command);
    return WEXITSTATUS(status);
}

int main(void) {
    int overall_status = 0;
    
    /* Create directories for test artifacts */
    create_temp_dir("test_artifacts");
    create_temp_dir("fail_artifacts");
    create_temp_dir("dump_test");
    
    /* Write the simple C source file */
    FILE *fp = fopen("simple.c", "w");
    if (!fp) {
        perror("Failed to create simple.c");
        return 1;
    }
    fputs(simple_c_content, fp);
    fclose(fp);
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION A: Sets state variables with successful compilation
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Uses -c flag to stop before linking (successful compilation) */
    printf("1. Invocation A: Setting state with successful compilation\n");
    const char *invocation_a = 
        "gcc -save-temps -dumpdir ./test_artifacts "
        "-dumpbase coverage_test -o test_output.o -c simple.c";
    int status_a = execute_gcc_command(invocation_a);
    printf("Exit status: %d\n\n", status_a);
    
    /* INVOCATION B: Sets state variables with compilation failure
     * Uses invalid architecture flag to cause backend failure
     * after driver initialization */
    printf("2. Invocation B: Setting state with compilation failure\n");
    const char *invocation_b = 
        "gcc -save-temps -dumpdir ./fail_artifacts "
        "-dumpbase fail_test -o fail_output.o "
        "-march=invalid-arch simple.c 2>/dev/null";
    int status_b = execute_gcc_command(invocation_b);
    printf("Exit status: %d\n\n", status_b);
    
    /* INVOCATION C: Tests with -specs= and -V flags
     * Influences spec_machine and print_version */
    printf("3. Invocation C: Testing with -specs and -V flags\n");
    const char *invocation_c = 
        "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5";
    int status_c = execute_gcc_command(invocation_c);
    printf("Exit status: %d\n\n", status_c);
    
    /* INVOCATION D: Tests -Werror turning warnings into errors
     * Creates a file with a warning to test error path */
    printf("4. Invocation D: Testing -Werror path\n");
    FILE *fp_warn = fopen("warn.c", "w");
    if (fp_warn) {
        fputs("int main(void) {\n", fp_warn);
        fputs("    int x; /* unused variable warning */\n", fp_warn);
        fputs("    return 0;\n", fp_warn);
        fputs("}\n", fp_warn);
        fclose(fp_warn);
    }
    
    const char *invocation_d = 
        "gcc -Werror -save-temps -dumpdir ./dump_test "
        "-dumpbase werror_test -o werror_output.o -c warn.c 2>/dev/null";
    int status_d = execute_gcc_command(invocation_d);
    printf("Exit status: %d\n\n", status_d);
    
    /* INVOCATION E: Tests multiple output-related flags
     * Exercises dumpbase_ext and outbase logic */
    printf("5. Invocation E: Testing output-related flags\n");
    const char *invocation_e = 
        "gcc -save-temps=obj -dumpdir . -dumpbase multi "
        "-dumpbase-ext .ext -o final_output.o -c simple.c";
    int status_e = execute_gcc_command(invocation_e);
    printf("Exit status: %d\n\n", status_e);
    
    /* INVOCATION F: Tests with verbose flag
     * Exercises verbose_only_flag */
    printf("6. Invocation F: Testing verbose flag\n");
    const char *invocation_f = 
        "gcc -v -c simple.c 2>&1 | grep -i 'gcc version'";
    int status_f = execute_gcc_command(invocation_f);
    printf("Exit status: %d\n\n", status_f);
    
    /* Cleanup generated files */
    printf("Cleaning up test files...\n");
    remove_dir("test_artifacts");
    remove_dir("fail_artifacts");
    remove_dir("dump_test");
    
    /* Remove individual files */
    system("rm -f simple.c warn.c test_output.o fail_output.o "
           "werror_output.o final_output.o "
           "*.i *.s *.o coverage_test.* fail_test.* "
           "werror_test.* multi.* 2>/dev/null");
    
    printf("\n=== Test completed ===\n");
    printf("All GCC invocations attempted. Check coverage data for gcc.cc\n");
    
    return overall_status;
}
