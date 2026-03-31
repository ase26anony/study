/**
 * test_gcc_driver_cleanup.c
 * 
 * This test program exercises the GCC driver's cleanup routine by invoking
 * the driver with different command-line options that set the state variables
 * being reset in the uncovered block (lines 11228-11250 of gcc.cc).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define SIMPLE_C_FILE "simple_test_file.c"
#define TEMP_DIR "test_temp_dir"

/**
 * Creates a minimal valid C source file for compilation tests.
 */
static void create_test_source_file(void) {
    FILE *f = fopen(SIMPLE_C_FILE, "w");
    if (!f) {
        perror("Failed to create test source file");
        exit(1);
    }
    
    fprintf(f, "/* Simple test file for GCC driver coverage */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main(void) {\n");
    fprintf(f, "    printf(\"Hello from test program\\n\");\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
    printf("Created test source file: %s\n", SIMPLE_C_FILE);
}

/**
 * Creates a temporary directory for test artifacts.
 */
static void create_temp_directory(void) {
    if (mkdir(TEMP_DIR, 0755) != 0) {
        // Directory might already exist, that's OK
        if (access(TEMP_DIR, F_OK) != 0) {
            perror("Failed to create temp directory");
            exit(1);
        }
    }
    printf("Created/verified temp directory: %s\n", TEMP_DIR);
}

/**
 * Executes a GCC command and returns the exit status.
 */
static int execute_gcc_command(const char *description, const char *command) {
    printf("\n=== %s ===\n", description);
    printf("Command: %s\n", command);
    
    int status = system(command);
    
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    } else {
        printf("Command terminated abnormally\n");
    }
    
    return status;
}

/**
 * Cleans up generated files and directories.
 */
static void cleanup_test_artifacts(void) {
    // Remove the test source file
    if (remove(SIMPLE_C_FILE) == 0) {
        printf("Removed test source file: %s\n", SIMPLE_C_FILE);
    }
    
    // Remove generated files in temp directory
    const char *files_to_remove[] = {
        "test_output.o",
        "test_output.i",
        "test_output.s",
        "fail_output.o",
        "fail_output.i",
        "fail_output.s",
        "coverage_test.*",
        "fail_test.*",
        "simple_test_file.o",
        "simple_test_file.i",
        "simple_test_file.s",
        NULL
    };
    
    for (int i = 0; files_to_remove[i] != NULL; i++) {
        if (remove(files_to_remove[i]) == 0) {
            printf("Removed: %s\n", files_to_remove[i]);
        }
    }
    
    // Try to remove temp directory (might fail if not empty)
    if (rmdir(TEMP_DIR) == 0) {
        printf("Removed temp directory: %s\n", TEMP_DIR);
    }
}

int main(void) {
    printf("=== GCC Driver Cleanup Coverage Test ===\n");
    
    // Create test artifacts
    create_test_source_file();
    create_temp_directory();
    
    int overall_status = 0;
    
    /**
     * INVOCATION 1: Successful compilation with state variables set
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Should trigger cleanup after successful compilation
     */
    char cmd1[1024];
    snprintf(cmd1, sizeof(cmd1),
             "gcc -save-temps -dumpdir ./%s -dumpbase coverage_test "
             "-o test_output.o -c %s",
             TEMP_DIR, SIMPLE_C_FILE);
    
    execute_gcc_command("Invocation 1: Successful compilation with state variables", cmd1);
    
    /**
     * INVOCATION 2: Compilation that fails after driver initialization
     * Uses invalid architecture flag to cause backend failure
     * Still sets state variables and triggers cleanup
     */
    char cmd2[1024];
    snprintf(cmd2, sizeof(cmd2),
             "gcc -save-temps -dumpdir ./%s -dumpbase fail_test "
             "-o fail_output.o -march=invalid-arch %s 2>/dev/null",
             TEMP_DIR, SIMPLE_C_FILE);
    
    execute_gcc_command("Invocation 2: Failed compilation (invalid arch)", cmd2);
    
    /**
     * INVOCATION 3: Use -specs= to influence spec_machine
     * Also uses -V for version printing (sets print_version)
     * Non-existent spec file causes error but driver still initializes
     */
    char cmd3[1024];
    snprintf(cmd3, sizeof(cmd3),
             "gcc -specs=nosuch.spec -V %s 2>&1 | head -5",
             SIMPLE_C_FILE);
    
    execute_gcc_command("Invocation 3: Specs and version printing", cmd3);
    
    /**
     * INVOCATION 4: Test with -Werror to create different failure mode
     * Also tests dumpbase_ext and outbase with different extensions
     */
    char cmd4[1024];
    snprintf(cmd4, sizeof(cmd4),
             "gcc -save-temps=obj -dumpdir ./%s -dumpbase extended_test "
             "-dumpbase-ext .extra -o different_output.o "
             "-Werror -Wall %s 2>&1 | head -10",
             TEMP_DIR, SIMPLE_C_FILE);
    
    execute_gcc_command("Invocation 4: Werror and extended options", cmd4);
    
    /**
     * INVOCATION 5: Test verbose flag (verbose_only_flag)
     * Combined with other state-setting options
     */
    char cmd5[1024];
    snprintf(cmd5, sizeof(cmd5),
             "gcc -v -save-temps -dumpdir ./%s -o verbose_output.o %s 2>&1 | "
             "grep -E '(COLLECT_GCC|drivers)' | head -5",
             TEMP_DIR, SIMPLE_C_FILE);
    
    execute_gcc_command("Invocation 5: Verbose mode with state", cmd5);
    
    /**
     * INVOCATION 6: Test help flags (print_help_list, print_subprocess_help)
     * These set flags that should be reset in cleanup
     */
    char cmd6[1024];
    snprintf(cmd6, sizeof(cmd6),
             "gcc --help=common 2>&1 | head -3");
    
    execute_gcc_command("Invocation 6: Help flag testing", cmd6);
    
    /**
     * INVOCATION 7: Test target system root variables
     * Uses --sysroot to influence target_system_root
     */
    char cmd7[1024];
    snprintf(cmd7, sizeof(cmd7),
             "gcc --sysroot=/nonexistent -o sysroot_test.o -c %s 2>&1 | "
             "head -5",
             SIMPLE_C_FILE);
    
    execute_gcc_command("Invocation 7: Sysroot testing", cmd7);
    
    /**
     * INVOCATION 8: Test with -ftime-report (influences report_times_to_file)
     * Combined with -o to set outbase
     */
    char cmd8[1024];
    snprintf(cmd8, sizeof(cmd8),
             "gcc -ftime-report -o timereport.o -c %s 2>&1 | "
             "grep -i 'time' | head -3",
             SIMPLE_C_FILE);
    
    execute_gcc_command("Invocation 8: Time reporting", cmd8);
    
    /**
     * INVOCATION 9: Test linker-specific options (use_ld)
     * Uses -fuse-ld to set use_ld variable
     */
    char cmd9[1024];
    snprintf(cmd9, sizeof(cmd9),
             "gcc -fuse-ld=bfd -o linker_test %s 2>&1 | head -5",
             SIMPLE_C_FILE);
    
    execute_gcc_command("Invocation 9: Linker selection", cmd9);
    
    /**
     * INVOCATION 10: Final test with multiple state variables
     * Combines many options to ensure comprehensive coverage
     */
    char cmd10[1024];
    snprintf(cmd10, sizeof(cmd10),
             "gcc -save-temps -dumpdir ./%s -dumpbase final_test "
             "-dumpbase-ext .final -o final_output.o "
             "-v -Werror -Wall --help=common --version "
             "-specs=nosuch.spec -march=x86-64 %s 2>&1 | "
             "head -20",
             TEMP_DIR, SIMPLE_C_FILE);
    
    execute_gcc_command("Invocation 10: Comprehensive state test", cmd10);
    
    // Clean up
    printf("\n=== Cleaning up test artifacts ===\n");
    cleanup_test_artifacts();
    
    printf("\n=== Test completed successfully ===\n");
    printf("The GCC driver's cleanup routine should have been executed multiple times,\n");
    printf("covering the reset of state variables and memory freeing in gcc.cc lines 11228-11250.\n");
    
    return overall_status;
}
