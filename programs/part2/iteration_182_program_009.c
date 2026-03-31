/* test_gcc_driver_cleanup.c
 * This test triggers the GCC driver's cleanup routine by invoking it
 * with various command-line options that set the state variables
 * mentioned in the uncovered lines (11228-11250 of gcc.cc).
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

/* Remove the temporary directory and its contents */
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
    
    /* Invocation 1: Sets state variables with successful compilation
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Uses -c to stop before linking (successful compilation) */
    char cmd1[512];
    snprintf(cmd1, sizeof(cmd1),
             "gcc -save-temps -dumpdir %s -dumpbase coverage_test "
             "-o %s/test_output.o -c %s",
             TEMP_DIR, TEMP_DIR, simple_c_path);
    
    int status1 = run_gcc_command(cmd1);
    printf("Invocation 1 returned: %d\n", status1);
    
    /* Invocation 2: Sets state variables with compilation failure
     * Uses invalid architecture to cause backend failure after driver initialization */
    char cmd2[512];
    snprintf(cmd2, sizeof(cmd2),
             "gcc -save-temps=obj -dumpdir %s/fail_dir -dumpbase fail_test "
             "-o %s/fail_output.o -march=invalid-arch %s 2>/dev/null",
             TEMP_DIR, TEMP_DIR, simple_c_path);
    
    int status2 = run_gcc_command(cmd2);
    printf("Invocation 2 returned: %d\n", status2);
    
    /* Invocation 3: Uses different specs and version flag
     * This may influence spec_machine and print_version */
    char cmd3[512];
    snprintf(cmd3, sizeof(cmd3),
             "gcc -specs=nosuch.spec -V %s 2>&1 | head -5",
             simple_c_path);
    
    int status3 = run_gcc_command(cmd3);
    printf("Invocation 3 returned: %d\n", status3);
    
    /* Invocation 4: Tests with -Werror turning warnings into errors
     * Uses an undefined function to generate a warning/error */
    char warning_c_path[256];
    snprintf(warning_c_path, sizeof(warning_c_path), "%s/warning.c", TEMP_DIR);
    FILE *f = fopen(warning_c_path, "w");
    if (f) {
        fputs("int main(void) { undefined_function(); return 0; }\n", f);
        fclose(f);
        
        char cmd4[512];
        snprintf(cmd4, sizeof(cmd4),
                 "gcc -Werror -save-temps=cwd -dumpbase warn_test "
                 "-o %s/warn_output.o %s 2>/dev/null",
                 TEMP_DIR, warning_c_path);
        
        int status4 = run_gcc_command(cmd4);
        printf("Invocation 4 returned: %d\n", status4);
    }
    
    /* Invocation 5: Tests with verbose flag and custom output base */
    char cmd5[512];
    snprintf(cmd5, sizeof(cmd5),
             "gcc -v -dumpbase myprog -dumpbase-ext .ext "
             "-o %s/final_output %s 2>&1 | tail -3",
             TEMP_DIR, simple_c_path);
    
    int status5 = run_gcc_command(cmd5);
    printf("Invocation 5 returned: %d\n", status5);
    
    /* Invocation 6: Tests with time report flag */
    char cmd6[512];
    snprintf(cmd6, sizeof(cmd6),
             "gcc -ftime-report -o %s/time_output %s 2>&1 | head -10",
             TEMP_DIR, simple_c_path);
    
    int status6 = run_gcc_command(cmd6);
    printf("Invocation 6 returned: %d\n", status6);
    
    /* Clean up generated files */
    cleanup_temp_dir();
    
    printf("\nAll GCC invocations attempted.\n");
    printf("Coverage should now include the cleanup block in gcc.cc\n");
    
    return overall_result;
}
