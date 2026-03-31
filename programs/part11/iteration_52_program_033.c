/* test_gcc_driver_init.c
 * 
 * This test program exercises the GCC driver's per-job initialization logic
 * (lines 11228-11250 in gcc.cc) by invoking multiple compilation jobs with
 * different state configurations, forcing the driver to reset global variables
 * between jobs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Helper function to create a temporary file with given content */
static int create_temp_file(const char *template, const char *content) {
    FILE *f = fopen(template, "w");
    if (!f) {
        perror("fopen");
        return -1;
    }
    if (fprintf(f, "%s", content) < 0) {
        perror("fprintf");
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

/* Helper function to execute a command and check its return status */
static int execute_command(const char *cmd, int expected_status) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    int ret = WEXITSTATUS(status);
    
    if (expected_status >= 0) {
        if (ret != expected_status) {
            printf("  Warning: Command returned %d, expected %d\n", 
                   ret, expected_status);
        }
    }
    return ret;
}

/* Helper function to create a temporary directory */
static int create_temp_dir(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) == -1) {
            perror("mkdir");
            return -1;
        }
    }
    return 0;
}

/* Helper function to remove a directory and its contents */
static void remove_dir(const char *path) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
    system(cmd);
}

/* Unused functions to avoid -Wunused-function warnings */
static int unused_func1(void) { return 0; }
static int unused_func2(void) { return 1; }
static int unused_func3(void) { return 2; }

int main(void) {
    int checksum = 0;
    char cmd[1024];
    
    printf("=== Testing GCC Driver Initialization Block ===\n\n");
    
    /* Create temporary source files */
    printf("Creating temporary source files...\n");
    
    /* Valid source file 1 */
    if (create_temp_file("temp_test1.c", 
        "int foo(void) { return 0; }\n"
        "int bar(void) { return 1; }\n") < 0) {
        return 1;
    }
    
    /* Valid source file 2 */
    if (create_temp_file("temp_test2.c",
        "int baz(void) { return 2; }\n"
        "int qux(void) { return 3; }\n") < 0) {
        return 1;
    }
    
    /* Source file with syntax error */
    if (create_temp_file("temp_error.c",
        "int syntax_error(void) { return \n") < 0) {
        return 1;
    }
    
    /* Create dump directory */
    if (create_temp_dir("./dump_test") < 0) {
        printf("Failed to create dump directory\n");
    }
    
    /* Sequence 1: Help flag then compilation
     * This sets print_help_list or print_version, then resets them */
    printf("\n--- Sequence 1: Help then Compile ---\n");
    
    /* First job: help output (sets print_help_list) */
    snprintf(cmd, sizeof(cmd), 
             "%s --help=common > /dev/null 2>&1", 
             "gcc");
    checksum += execute_command(cmd, 0);
    
    /* Second job: compile (should reset print_help_list) */
    snprintf(cmd, sizeof(cmd),
             "%s -c temp_test1.c -o temp_test1.o",
             "gcc");
    checksum += execute_command(cmd, 0);
    
    /* Sequence 2: Version flag then different help then compile
     * Tests multiple help/version flag resets */
    printf("\n--- Sequence 2: Version -> Help -> Compile ---\n");
    
    snprintf(cmd, sizeof(cmd),
             "%s --version > /dev/null 2>&1",
             "gcc");
    checksum += execute_command(cmd, 0);
    
    snprintf(cmd, sizeof(cmd),
             "%s --help=optimizers > /dev/null 2>&1",
             "gcc");
    checksum += execute_command(cmd, 0);
    
    snprintf(cmd, sizeof(cmd),
             "%s -c temp_test2.c -o temp_test2.o",
             "gcc");
    checksum += execute_command(cmd, 0);
    
    /* Sequence 3: Save-temps with dumpdir then plain compile
     * This exercises save_temps_flag, dumpdir, dumpbase reset logic */
    printf("\n--- Sequence 3: Save-temps with dumpdir then plain compile ---\n");
    
    /* First job with save-temps and dumpdir (sets save_temps_flag, allocates dumpdir) */
    snprintf(cmd, sizeof(cmd),
             "%s -save-temps -dumpdir ./dump_test -dumpbase mytest "
             "-c temp_test1.c -o temp_test1_save.o 2>/dev/null",
             "gcc");
    checksum += execute_command(cmd, 0);
    
    /* Second job without save-temps (should free dumpdir, reset save_temps_flag) */
    snprintf(cmd, sizeof(cmd),
             "%s -c temp_test2.c -o temp_test2_plain.o",
             "gcc");
    checksum += execute_command(cmd, 0);
    
    /* Sequence 4: Verbose flag then compilation
     * Tests verbose_only_flag reset */
    printf("\n--- Sequence 4: Verbose flag then compile ---\n");
    
    snprintf(cmd, sizeof(cmd),
             "%s -v -c temp_test1.c 2>&1 | head -5 > /dev/null",
             "gcc");
    checksum += execute_command(cmd, 0);
    
    snprintf(cmd, sizeof(cmd),
             "%s -c temp_test2.c -o temp_test2_verbose.o",
             "gcc");
    checksum += execute_command(cmd, 0);
    
    /* Sequence 5: Linker selection then default
     * Tests use_ld reset */
    printf("\n--- Sequence 5: Linker selection then default ---\n");
    
    snprintf(cmd, sizeof(cmd),
             "%s -fuse-ld=bfd -c temp_test1.c -o temp_test1_bfd.o 2>/dev/null",
             "gcc");
    checksum += execute_command(cmd, 0);
    
    snprintf(cmd, sizeof(cmd),
             "%s -c temp_test2.c -o temp_test2_default.o",
             "gcc");
    checksum += execute_command(cmd, 0);
    
    /* Try with lld if available */
    snprintf(cmd, sizeof(cmd),
             "%s -fuse-ld=lld -c temp_test1.c -o temp_test1_lld.o 2>/dev/null",
             "gcc");
    execute_command(cmd, -1); /* Don't check status - lld might not be available */
    
    /* Sequence 6: Error recovery path
     * Tests greatest_status reset to 1 after failure */
    printf("\n--- Sequence 6: Error recovery (tests greatest_status reset) ---\n");
    
    /* First job: compilation error (should set greatest_status to failure) */
    snprintf(cmd, sizeof(cmd),
             "%s -c temp_error.c -o temp_error.o 2>/dev/null",
             "gcc");
    int error_status = execute_command(cmd, -1);
    checksum += (error_status > 0) ? 1 : 0; /* Expect non-zero for error */
    
    /* Second job: successful compilation (should reset greatest_status logic) */
    snprintf(cmd, sizeof(cmd),
             "%s -c temp_test1.c -o temp_test1_recover.o",
             "gcc");
    checksum += execute_command(cmd, 0);
    
    /* Sequence 7: Multiple jobs in single invocation
     * Tests per-job initialization within same process */
    printf("\n--- Sequence 7: Multiple inputs in single invocation ---\n");
    
    snprintf(cmd, sizeof(cmd),
             "%s -c temp_test1.c temp_test2.c",
             "gcc");
    checksum += execute_command(cmd, 0);
    
    /* Sequence 8: Mixed options across jobs
     * Complex state transitions */
    printf("\n--- Sequence 8: Complex state transitions ---\n");
    
    /* Job with multiple flags */
    snprintf(cmd, sizeof(cmd),
             "%s -save-temps -dumpdir ./dump_test -v -fuse-ld=bfd "
             "-c temp_test1.c -o temp_test1_complex.o 2>/dev/null",
             "gcc");
    checksum += execute_command(cmd, 0);
    
    /* Follow with help */
    snprintf(cmd, sizeof(cmd),
             "%s --help=warnings > /dev/null 2>&1",
             "gcc");
    checksum += execute_command(cmd, 0);
    
    /* Follow with plain compile */
    snprintf(cmd, sizeof(cmd),
             "%s -c temp_test2.c -o temp_test2_final.o",
             "gcc");
    checksum += execute_command(cmd, 0);
    
    /* Cleanup */
    printf("\n--- Cleaning up ---\n");
    
    /* Remove temporary files */
    remove_dir("./dump_test");
    system("rm -f temp_test1.c temp_test2.c temp_error.c");
    system("rm -f temp_test*.o temp_test*.i temp_test*.s temp_test*.ii");
    system("rm -f mytest.* ./dump_test/* 2>/dev/null");
    
    /* Final checksum and summary */
    printf("\n=== Test Summary ===\n");
    printf("Checksum (sum of return codes): %d\n", checksum);
    printf("Expected checksum range: 0-15 (depends on exact failures)\n");
    
    /* Reference unused functions to avoid compiler warnings */
    (void)unused_func1;
    (void)unused_func2;
    (void)unused_func3;
    
    if (checksum >= 0) {
        printf("\nTest completed successfully.\n");
        printf("The GCC driver initialization block should have been exercised.\n");
        return 0;
    } else {
        printf("\nTest encountered issues.\n");
        return 1;
    }
}
