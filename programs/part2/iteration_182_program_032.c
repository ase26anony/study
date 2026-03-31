/**
 * Test program to cover GCC driver cleanup lines in gcc.cc
 * This program invokes the GCC driver with various flags to ensure
 * the cleanup routine (lines 11228-11250) is executed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define SIMPLE_C_FILE "simple_test.c"
#define OUTPUT_DIR "test_coverage_dir"

/**
 * Creates a simple valid C source file for compilation tests
 */
static void create_simple_c_file(void) {
    FILE *f = fopen(SIMPLE_C_FILE, "w");
    if (!f) {
        perror("Failed to create test C file");
        exit(1);
    }
    
    fprintf(f, "/* Simple test file for GCC driver coverage */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main(void) {\n");
    fprintf(f, "    printf(\"Hello from test program\\n\");\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
    printf("Created test file: %s\n", SIMPLE_C_FILE);
}

/**
 * Creates a C file with a deliberate syntax error for failure testing
 */
static void create_error_c_file(void) {
    const char *filename = "error_test.c";
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create error test C file");
        exit(1);
    }
    
    fprintf(f, "/* Test file with syntax error */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main(void) {\n");
    fprintf(f, "    printf(\"This has a syntax error\\n\")\n");  // Missing semicolon
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
    printf("Created error test file: %s\n", filename);
}

/**
 * Creates a directory for test artifacts
 */
static void create_test_directory(void) {
    struct stat st = {0};
    if (stat(OUTPUT_DIR, &st) == -1) {
        if (mkdir(OUTPUT_DIR, 0755) != 0) {
            perror("Failed to create test directory");
            exit(1);
        }
        printf("Created test directory: %s\n", OUTPUT_DIR);
    }
}

/**
 * Executes a GCC command and returns the exit status
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
 * Cleans up generated files
 */
static void cleanup_generated_files(void) {
    // Remove generated source files
    remove(SIMPLE_C_FILE);
    remove("error_test.c");
    
    // Remove common output files
    remove("test_output.o");
    remove("test_output");
    remove("fail_output.o");
    remove("fail_output");
    remove("coverage_test.o");
    remove("coverage_test");
    
    // Remove save-temps files
    remove("simple_test.i");
    remove("simple_test.s");
    remove("error_test.i");
    remove("error_test.s");
    
    // Remove dump files
    remove("mydump.*");
    remove("coverage_test.*");
    remove("fail_test.*");
    
    // Remove directory if empty
    rmdir(OUTPUT_DIR);
    
    printf("\nCleaned up generated files\n");
}

int main(void) {
    printf("=== GCC Driver Cleanup Coverage Test ===\n");
    
    // Create test environment
    create_test_directory();
    create_simple_c_file();
    create_error_c_file();
    
    // Build GCC command prefix - using the current gcc in PATH
    const char *gcc = "gcc";
    
    // Test 1: Successful compilation with -save-temps and dump options
    // This sets save_temps_flag, allocates dumpdir, dumpbase, and outbase
    char cmd1[1024];
    snprintf(cmd1, sizeof(cmd1),
             "%s -save-temps -dumpdir ./%s -dumpbase mydump "
             "-o test_output.o -c %s",
             gcc, OUTPUT_DIR, SIMPLE_C_FILE);
    
    execute_gcc_command("Test 1: Successful compilation with save-temps and dump options", cmd1);
    
    // Test 2: Compilation with different dump options and output base
    // Tests different combinations of dumpdir, dumpbase, and outbase
    char cmd2[1024];
    snprintf(cmd2, sizeof(cmd2),
             "%s -dumpdir ./%s -dumpbase coverage_test "
             "-dumpbase-ext .ext -o coverage_test.o -c %s",
             gcc, OUTPUT_DIR, SIMPLE_C_FILE);
    
    execute_gcc_command("Test 2: Compilation with extended dump options", cmd2);
    
    // Test 3: Failed compilation with invalid architecture
    // This causes backend failure after driver initialization
    // Still triggers cleanup with allocated state
    char cmd3[1024];
    snprintf(cmd3, sizeof(cmd3),
             "%s -save-temps -dumpdir ./%s -dumpbase fail_test "
             "-o fail_output.o -march=invalid-arch %s 2>/dev/null",
             gcc, OUTPUT_DIR, SIMPLE_C_FILE);
    
    execute_gcc_command("Test 3: Failed compilation with invalid architecture", cmd3);
    
    // Test 4: Compilation with syntax error (late failure)
    // Driver parses args successfully, but compiler proper fails
    char cmd4[1024];
    snprintf(cmd4, sizeof(cmd4),
             "%s -save-temps -dumpdir ./%s -dumpbase syntax_fail "
             "-o syntax_output.o error_test.c 2>/dev/null",
             gcc, OUTPUT_DIR);
    
    execute_gcc_command("Test 4: Compilation with syntax error", cmd4);
    
    // Test 5: Version printing and specs (influences spec_machine and print_version)
    char cmd5[1024];
    snprintf(cmd5, sizeof(cmd5),
             "%s -V %s 2>&1 | head -5",  // Limit output
             gcc, SIMPLE_C_FILE);
    
    execute_gcc_command("Test 5: Version printing", cmd5);
    
    // Test 6: Help listing (influences print_help_list)
    char cmd6[1024];
    snprintf(cmd6, sizeof(cmd6),
             "%s --help=common 2>&1 | head -10",  // Limit output
             gcc);
    
    execute_gcc_command("Test 6: Help listing", cmd6);
    
    // Test 7: Warning as error (different failure mode)
    char cmd7[1024];
    snprintf(cmd7, sizeof(cmd7),
             "%s -Werror -Wall -Wextra -o warn_test %s 2>/dev/null",
             gcc, SIMPLE_C_FILE);
    
    execute_gcc_command("Test 7: Warnings as errors", cmd7);
    
    // Test 8: Linker error (undefined library)
    // Driver accepts the flag but linker fails
    char cmd8[1024];
    snprintf(cmd8, sizeof(cmd8),
             "%s -lnonexistentlibrary -o link_test %s 2>/dev/null",
             gcc, SIMPLE_C_FILE);
    
    execute_gcc_command("Test 8: Linker error with undefined library", cmd8);
    
    // Test 9: Multiple dumpdir and outbase combinations
    char cmd9[1024];
    snprintf(cmd9, sizeof(cmd9),
             "%s -dumpdir ./%s/dump1 -dumpbase multi_test1 "
             "-o %s/output1.o -c %s",
             gcc, OUTPUT_DIR, OUTPUT_DIR, SIMPLE_C_FILE);
    
    execute_gcc_command("Test 9: Multiple output combinations", cmd9);
    
    // Test 10: Full compilation with all state-setting flags
    char cmd10[1024];
    snprintf(cmd10, sizeof(cmd10),
             "%s -save-temps=obj -dumpdir ./%s -dumpbase full_test "
             "-dumpbase-ext .full -specs=/dev/null -v -o %s/full_output %s 2>/dev/null",
             gcc, OUTPUT_DIR, OUTPUT_DIR, SIMPLE_C_FILE);
    
    execute_gcc_command("Test 10: Full compilation with all flags", cmd10);
    
    // Clean up
    cleanup_generated_files();
    
    printf("\n=== All tests completed ===\n");
    printf("The GCC driver cleanup routine should have been executed multiple times,\n");
    printf("covering the reset of variables and freeing of allocated memory.\n");
    
    return 0;
}
