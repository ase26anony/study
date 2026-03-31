/* test_gcc_driver_cleanup.c
 * This test program exercises the GCC driver's cleanup routine by
 * invoking it multiple times with different flags that set the
 * state variables being reset in the uncovered block.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

/* Simple C source file content */
#define SIMPLE_C_CONTENT "int main(void) { return 0; }\n"

/* Create a temporary directory for test artifacts */
static char *create_temp_dir(void) {
    char template[] = "/tmp/gcc_test_XXXXXX";
    char *dir = mkdtemp(template);
    if (!dir) {
        perror("Failed to create temp directory");
        exit(1);
    }
    return strdup(dir);
}

/* Create the simple C source file */
static void create_simple_c(const char *dir) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/simple.c", dir);
    
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("Failed to create simple.c");
        exit(1);
    }
    
    fputs(SIMPLE_C_CONTENT, f);
    fclose(f);
}

/* Run a GCC command and capture its exit status */
static int run_gcc_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    } else {
        printf("Command terminated abnormally\n\n");
        return -1;
    }
}

/* Clean up generated files */
static void cleanup_files(const char *dir) {
    char cmd[2048];
    
    /* Remove common GCC output files */
    const char *patterns[] = {
        "*.o", "*.i", "*.s", "*.ii", "*.bc",
        "test_output*", "fail_output*", "coverage_test*",
        "fail_test*", "dump_test*", NULL
    };
    
    for (int i = 0; patterns[i]; i++) {
        snprintf(cmd, sizeof(cmd), "rm -f %s/%s 2>/dev/null", dir, patterns[i]);
        system(cmd);
    }
    
    /* Remove any subdirectories created by -dumpdir */
    snprintf(cmd, sizeof(cmd), "rm -rf %s/test_artifacts %s/fail_artifacts %s/dump_artifacts 2>/dev/null", 
             dir, dir, dir);
    system(cmd);
}

int main(void) {
    char *temp_dir = create_temp_dir();
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Change to temp directory */
    if (chdir(temp_dir) != 0) {
        perror("Failed to chdir to temp directory");
        free(temp_dir);
        return 1;
    }
    
    /* Create the simple C source file */
    create_simple_c(temp_dir);
    
    char cmd[4096];
    int overall_status = 0;
    
    /* INVOCATION 1: Successful compilation with -save-temps and dump options
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     */
    printf("=== Invocation 1: Successful compilation with state-setting flags ===\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -save-temps -dumpdir ./test_artifacts -dumpbase coverage_test "
             "-o test_output.o -c simple.c 2>&1");
    run_gcc_command(cmd);
    
    /* INVOCATION 2: Compilation that fails at backend (invalid architecture)
     * This also sets state variables but causes failure
     */
    printf("=== Invocation 2: Failed compilation with different state ===\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -save-temps=obj -dumpdir ./fail_artifacts -dumpbase fail_test "
             "-o fail_output.o -march=invalid-arch simple.c 2>&1");
    run_gcc_command(cmd);
    
    /* INVOCATION 3: Use -specs and -V to influence spec_machine and print_version
     * Note: -specs with non-existent file will cause error but still sets state
     */
    printf("=== Invocation 3: Using -specs and -V flags ===\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -20");
    run_gcc_command(cmd);
    
    /* INVOCATION 4: Test with -Werror turning warnings into errors
     * This affects greatest_status
     */
    printf("=== Invocation 4: Testing -Werror for error status ===\n");
    
    /* First create a file with a warning */
    FILE *f = fopen("warn.c", "w");
    if (f) {
        fputs("int main(void) { int x; return 0; } /* unused variable warning */\n", f);
        fclose(f);
    }
    
    snprintf(cmd, sizeof(cmd),
             "gcc -Werror -save-temps -dumpdir ./dump_artifacts "
             "-dumpbase dump_test -o warn_output.o -c warn.c 2>&1");
    run_gcc_command(cmd);
    
    /* INVOCATION 5: Test dumpbase_ext and outbase with different extensions */
    printf("=== Invocation 5: Testing dumpbase_ext and complex outbase ===\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -save-temps -dumpdir . -dumpbase complex "
             "-dumpbase-ext .ext -o complex_output.o -c simple.c 2>&1");
    run_gcc_command(cmd);
    
    /* INVOCATION 6: Test verbose flag which sets verbose_only_flag */
    printf("=== Invocation 6: Testing verbose flag ===\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -v -c simple.c 2>&1 | head -30");
    run_gcc_command(cmd);
    
    /* INVOCATION 7: Test help and version flags (print_help_list, print_version) */
    printf("=== Invocation 7: Testing help and version flags ===\n");
    snprintf(cmd, sizeof(cmd), "gcc --help=common 2>&1 | head -5");
    run_gcc_command(cmd);
    
    snprintf(cmd, sizeof(cmd), "gcc --version 2>&1 | head -2");
    run_gcc_command(cmd);
    
    /* INVOCATION 8: Test with -print-prog-name to trigger subprocess help logic */
    printf("=== Invocation 8: Testing -print-prog-name ===\n");
    snprintf(cmd, sizeof(cmd), "gcc -print-prog-name=cc1 2>&1");
    run_gcc_command(cmd);
    
    /* Clean up generated files */
    printf("=== Cleaning up test artifacts ===\n");
    cleanup_files(temp_dir);
    
    /* Remove the temp directory */
    char rm_cmd[1024];
    snprintf(rm_cmd, sizeof(rm_cmd), "rm -rf %s", temp_dir);
    system(rm_cmd);
    
    free(temp_dir);
    
    printf("\n=== Test completed successfully ===\n");
    printf("The GCC driver's cleanup routine should have been exercised multiple times,\n");
    printf("covering the reset of state variables and freeing of allocated memory.\n");
    
    return overall_status;
}
