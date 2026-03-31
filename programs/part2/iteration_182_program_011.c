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

#define SIMPLE_C_CONTENT "int main(void) { return 0; }\n"
#define TEMP_DIR "./gcc_cleanup_test_dir"

/* Create a temporary directory for test artifacts */
static int create_temp_dir(void) {
    struct stat st = {0};
    if (stat(TEMP_DIR, &st) == -1) {
        if (mkdir(TEMP_DIR, 0755) != 0) {
            perror("mkdir");
            return -1;
        }
    }
    return 0;
}

/* Remove temporary directory and its contents */
static void cleanup_temp_dir(void) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", TEMP_DIR);
    system(cmd);
}

/* Write a simple C source file */
static int write_simple_c_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return -1;
    }
    fputs(SIMPLE_C_CONTENT, f);
    fclose(f);
    return 0;
}

/* Execute a GCC command and capture its return status */
static int run_gcc_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status == -1) {
        perror("system");
        return -1;
    }
    return WEXITSTATUS(status);
}

int main(void) {
    int overall_result = 0;
    
    /* Create temporary directory for test artifacts */
    if (create_temp_dir() != 0) {
        return 1;
    }
    
    /* Write the simple C source file */
    char simple_c_path[256];
    snprintf(simple_c_path, sizeof(simple_c_path), "%s/simple.c", TEMP_DIR);
    if (write_simple_c_file(simple_c_path) != 0) {
        cleanup_temp_dir();
        return 1;
    }
    
    /* Build GCC command strings */
    char cmd[1024];
    
    /* Invocation A: Sets state with -save-temps, -dumpdir, -dumpbase, -o
     * Successful compilation (-c flag stops before linking) */
    snprintf(cmd, sizeof(cmd),
             "gcc -save-temps -dumpdir %s/dump -dumpbase coverage_test "
             "-o %s/test_output.o -c %s",
             TEMP_DIR, TEMP_DIR, simple_c_path);
    printf("\n=== Invocation A: Setting state with successful compilation ===\n");
    int status_a = run_gcc_command(cmd);
    printf("Exit status: %d\n", status_a);
    
    /* Invocation B: Sets state but causes backend failure
     * Using invalid architecture flag to trigger failure after driver initialization */
    snprintf(cmd, sizeof(cmd),
             "gcc -save-temps=obj -dumpdir %s/fail_dump -dumpbase fail_test "
             "-o %s/fail_output.o -march=invalid-arch-nonexistent %s 2>/dev/null",
             TEMP_DIR, TEMP_DIR, simple_c_path);
    printf("\n=== Invocation B: Setting state with backend failure ===\n");
    int status_b = run_gcc_command(cmd);
    printf("Exit status: %d\n", status_b);
    
    /* Invocation C: Uses -specs and -V to influence spec_machine and print_version
     * The non-existent spec file will cause an error */
    snprintf(cmd, sizeof(cmd),
             "gcc -specs=%s/nosuch.spec -V %s 2>&1 | head -5",
             TEMP_DIR, simple_c_path);
    printf("\n=== Invocation C: Using -specs and -V flags ===\n");
    int status_c = run_gcc_command(cmd);
    printf("Exit status: %d\n", status_c);
    
    /* Invocation D: Trigger warning error with -Werror
     * This creates a different failure mode for greatest_status */
    /* First create a file with a warning */
    char warn_c_path[256];
    snprintf(warn_c_path, sizeof(warn_c_path), "%s/warn.c", TEMP_DIR);
    FILE *f = fopen(warn_c_path, "w");
    if (f) {
        fputs("int main(void) { int x; return 0; }\n", f); /* unused variable warning */
        fclose(f);
    }
    
    snprintf(cmd, sizeof(cmd),
             "gcc -Werror -save-temps -dumpdir %s/werror_dump "
             "-dumpbase werror_test -o %s/werror_output.o %s 2>/dev/null",
             TEMP_DIR, TEMP_DIR, warn_c_path);
    printf("\n=== Invocation D: Triggering -Werror failure ===\n");
    int status_d = run_gcc_command(cmd);
    printf("Exit status: %d\n", status_d);
    
    /* Invocation E: Test with verbose flag to set verbose_only_flag */
    snprintf(cmd, sizeof(cmd),
             "gcc -v -save-temps -dumpdir %s/verbose_dump "
             "-dumpbase verbose_test -o %s/verbose_output.o -c %s 2>&1 | head -10",
             TEMP_DIR, TEMP_DIR, simple_c_path);
    printf("\n=== Invocation E: Testing with verbose flag ===\n");
    int status_e = run_gcc_command(cmd);
    printf("Exit status: %d\n", status_e);
    
    /* Invocation F: Test help flags to set print_help_list */
    snprintf(cmd, sizeof(cmd),
             "gcc --help=common 2>&1 | head -5");
    printf("\n=== Invocation F: Testing help flags ===\n");
    int status_f = run_gcc_command(cmd);
    printf("Exit status: %d\n", status_f);
    
    /* Invocation G: Test version printing to set print_version */
    snprintf(cmd, sizeof(cmd),
             "gcc --version 2>&1 | head -2");
    printf("\n=== Invocation G: Testing version flag ===\n");
    int status_g = run_gcc_command(cmd);
    printf("Exit status: %d\n", status_g);
    
    /* Clean up generated files */
    printf("\n=== Cleaning up test artifacts ===\n");
    cleanup_temp_dir();
    
    printf("\n=== Test completed ===\n");
    printf("All GCC invocations attempted. Coverage data should be generated.\n");
    printf("Individual exit statuses: A=%d, B=%d, C=%d, D=%d, E=%d, F=%d, G=%d\n",
           status_a, status_b, status_c, status_d, status_e, status_f, status_g);
    
    return overall_result;
}
