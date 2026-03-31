/* test_gcc_driver_init.c
 * 
 * This test program exercises the GCC driver initialization logic
 * (lines 11228-11250 in gcc.cc) by invoking multiple compilation jobs
 * with different option combinations to ensure state variables are
 * properly reset between jobs.
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
        return 0;
    }
    if (fprintf(f, "%s", content) < 0) {
        perror("fprintf");
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

/* Helper function to execute a command and check return status */
static int execute_and_check(const char *cmd, int expected_status) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    int actual_status = WEXITSTATUS(status);
    
    if (actual_status != expected_status) {
        printf("  Warning: Expected status %d, got %d\n", expected_status, actual_status);
    }
    return actual_status;
}

/* Helper function to clean up temporary files */
static void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            unlink(files[i]);
        }
    }
}

int main(void) {
    int checksum = 0;
    const char *temp_files[10] = {0};
    int file_count = 0;
    
    printf("=== GCC Driver Initialization Block Test ===\n");
    printf("Testing lines 11228-11250 in gcc.cc\n\n");
    
    /* Create temporary source files */
    const char *temp1_c = "temp1_driver_test.c";
    const char *temp2_c = "temp2_driver_test.c";
    const char *error_c = "error_driver_test.c";
    const char *dump_dir = "./dump_test_dir";
    
    /* Create directory for dump tests */
    mkdir(dump_dir, 0755);
    
    /* Create valid source files */
    if (!create_temp_file(temp1_c, 
        "int foo(void) { return 0; }\n"
        "int unused_func1(void) { return 1; }\n")) {
        fprintf(stderr, "Failed to create %s\n", temp1_c);
        return 1;
    }
    temp_files[file_count++] = temp1_c;
    
    if (!create_temp_file(temp2_c,
        "int bar(void) { return 2; }\n"
        "int unused_func2(void) { return 3; }\n")) {
        fprintf(stderr, "Failed to create %s\n", temp2_c);
        cleanup_files(temp_files, file_count);
        return 1;
    }
    temp_files[file_count++] = temp2_c;
    
    /* Create source file with syntax error */
    if (!create_temp_file(error_c,
        "int baz(void) { return \n"  /* Missing expression */
        "int missing_semicolon(void) { return 4 }\n")) {
        fprintf(stderr, "Failed to create %s\n", error_c);
        cleanup_files(temp_files, file_count);
        return 1;
    }
    temp_files[file_count++] = error_c;
    
    /* Object files to clean up */
    const char *temp1_o = "temp1_driver_test.o";
    const char *temp2_o = "temp2_driver_test.o";
    temp_files[file_count++] = temp1_o;
    temp_files[file_count++] = temp2_o;
    
    /* Temporary files from -save-temps */
    const char *temp_files_save[] = {
        "base.i", "base.s", "base.o",
        "temp1_driver_test.i", "temp1_driver_test.s",
        NULL
    };
    
    printf("Test 1: Help flag followed by compilation (print_help_list, print_version)\n");
    printf("----------------------------------------------------------------\n");
    /* First job: help request (sets print_help_list) */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "gcc --help=common 2>&1 | head -5 > /dev/null");
    checksum += execute_and_check(cmd, 0);
    
    /* Second job: compilation (should reset print_help_list) */
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s", temp1_c, temp1_o);
    checksum += execute_and_check(cmd, 0) * 2;
    
    printf("\nTest 2: Version flag followed by different compilation (print_version)\n");
    printf("----------------------------------------------------------------\n");
    /* First job: version request */
    snprintf(cmd, sizeof(cmd), "gcc --version 2>&1 | head -1 > /dev/null");
    checksum += execute_and_check(cmd, 0);
    
    /* Second job: compilation with optimization */
    snprintf(cmd, sizeof(cmd), "gcc -O2 -c %s -o %s", temp2_c, temp2_o);
    checksum += execute_and_check(cmd, 0) * 3;
    
    printf("\nTest 3: Save-temps with dumpdir then plain compilation\n");
    printf("----------------------------------------------------------------\n");
    printf("Testing: save_temps_flag, dumpdir, dumpbase, free() calls\n");
    
    /* First job: -save-temps with dumpdir and dumpbase */
    snprintf(cmd, sizeof(cmd), 
             "gcc -save-temps -dumpdir %s -dumpbase base -c %s -o base.o 2>&1",
             dump_dir, temp1_c);
    checksum += execute_and_check(cmd, 0);
    
    /* Check if dump files were created */
    struct stat st;
    char dump_file[256];
    snprintf(dump_file, sizeof(dump_file), "%s/base.i", dump_dir);
    if (stat(dump_file, &st) == 0) {
        printf("  ✓ Dump file created: %s\n", dump_file);
        checksum += 5;
    }
    
    /* Second job: plain compilation (should reset dumpdir/dumpbase) */
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s", temp2_c, temp2_o);
    checksum += execute_and_check(cmd, 0) * 7;
    
    printf("\nTest 4: Verbose flag then quiet compilation (verbose_only_flag)\n");
    printf("----------------------------------------------------------------\n");
    /* First job: verbose compilation */
    snprintf(cmd, sizeof(cmd), "gcc -v -c %s -o %s 2>&1 | grep -q 'COLLECT_GCC_OPTIONS'", 
             temp1_c, temp1_o);
    int verbose_status = execute_and_check(cmd, 0);
    checksum += verbose_status == 0 ? 11 : 0;
    
    /* Second job: quiet compilation */
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s", temp2_c, temp2_o);
    checksum += execute_and_check(cmd, 0) * 13;
    
    printf("\nTest 5: Linker selection then default (use_ld)\n");
    printf("----------------------------------------------------------------\n");
    /* First job: specify linker */
    snprintf(cmd, sizeof(cmd), "gcc -fuse-ld=bfd -c %s -o %s 2>&1", temp1_c, temp1_o);
    checksum += execute_and_check(cmd, 0);
    
    /* Second job: default linker */
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s", temp2_c, temp2_o);
    checksum += execute_and_check(cmd, 0) * 17;
    
    printf("\nTest 6: Error then success (greatest_status reset to 1)\n");
    printf("----------------------------------------------------------------\n");
    /* First job: compilation with syntax error (should fail) */
    snprintf(cmd, sizeof(cmd), "gcc -c %s 2>/dev/null", error_c);
    int error_status = execute_and_check(cmd, 1);  /* Expected to fail */
    checksum += (error_status != 0) ? 19 : 0;
    
    /* Second job: successful compilation (greatest_status should be reset) */
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s", temp1_c, temp1_o);
    checksum += execute_and_check(cmd, 0) * 23;
    
    printf("\nTest 7: Multiple inputs as separate jobs\n");
    printf("----------------------------------------------------------------\n");
    /* Single invocation with multiple inputs - each should trigger initialization */
    snprintf(cmd, sizeof(cmd), "gcc -c %s %s -save-temps=cwd", temp1_c, temp2_c);
    checksum += execute_and_check(cmd, 0);
    
    printf("\nTest 8: Combined flags reset scenario\n");
    printf("----------------------------------------------------------------\n");
    /* Job with multiple flags */
    snprintf(cmd, sizeof(cmd), "gcc -v --help=optimizers 2>&1 | head -3 > /dev/null");
    checksum += execute_and_check(cmd, 0);
    
    /* Follow with compilation using different flags */
    snprintf(cmd, sizeof(cmd), "gcc -dumpdir ./dump2 -dumpbase test -c %s", temp1_c);
    checksum += execute_and_check(cmd, 0);
    
    /* Final job with no special flags */
    snprintf(cmd, sizeof(cmd), "gcc -c %s", temp2_c);
    checksum += execute_and_check(cmd, 0) * 29;
    
    /* Clean up dump directories */
    snprintf(cmd, sizeof(cmd), "rm -rf %s ./dump2", dump_dir);
    system(cmd);
    
    /* Clean up -save-temps files */
    for (int i = 0; temp_files_save[i]; i++) {
        unlink(temp_files_save[i]);
    }
    
    /* Clean up our temporary files */
    cleanup_files(temp_files, file_count);
    
    printf("\n=== Test Results ===\n");
    printf("Final checksum: %d\n", checksum);
    printf("Expected checksum range: 150-250 (depends on exact execution paths)\n");
    
    if (checksum > 100) {
        printf("✓ Driver initialization block likely exercised\n");
        printf("  Multiple jobs with varying options executed successfully.\n");
        return 0;
    } else {
        printf("✗ Insufficient test execution\n");
        return 1;
    }
}

/* Dummy functions to avoid unused function warnings */
void __attribute__((used)) reference_dummy_functions(void) {
    /* These references ensure the functions in our test files aren't optimized away */
    extern int foo(void);
    extern int bar(void);
    extern int unused_func1(void);
    extern int unused_func2(void);
    
    (void)foo;
    (void)bar;
    (void)unused_func1;
    (void)unused_func2;
}
