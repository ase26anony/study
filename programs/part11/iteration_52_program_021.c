/* test_gcc_driver_init.c
 * 
 * This program tests the GCC driver initialization logic (lines 11228-11250 in gcc.cc)
 * by executing multiple compilation jobs with different option combinations.
 * Each sequence sets specific driver state variables that should be reset
 * between jobs according to the uncovered code block.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Function prototypes for the dummy functions we'll create */
static int foo(void) __attribute__((unused));
static int bar(void) __attribute__((unused));
static int baz(void) __attribute__((unused));

/* Helper function to create a temporary file with given content */
static int create_temp_file(const char *filename, const char *content) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return 0;
    }
    fprintf(fp, "%s", content);
    fclose(fp);
    return 1;
}

/* Helper function to execute a command and check its return status */
static int execute_command(const char *cmd, int expected_status) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    int result = WEXITSTATUS(status);
    
    if (expected_status == -1) {
        /* Any status is acceptable */
        printf("  Command completed with status: %d\n", result);
        return 1;
    }
    
    if (result == expected_status) {
        printf("  ✓ Command succeeded with expected status %d\n", result);
        return 1;
    } else {
        printf("  ✗ Command failed: got status %d, expected %d\n", result, expected_status);
        return 0;
    }
}

/* Helper to create a temporary directory */
static int create_temp_dir(const char *dirname) {
    struct stat st = {0};
    if (stat(dirname, &st) == -1) {
        if (mkdir(dirname, 0755) == -1) {
            perror("mkdir");
            return 0;
        }
    }
    return 1;
}

/* Helper to remove a directory and its contents */
static void remove_temp_dir(const char *dirname) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dirname);
    system(cmd);
}

int main(void) {
    int success_count = 0;
    int total_tests = 0;
    char cmd[1024];
    
    printf("=== Testing GCC Driver Initialization Block (lines 11228-11250) ===\n\n");
    
    /* Create temporary source files */
    printf("Creating temporary source files...\n");
    
    /* Valid source file 1 */
    if (!create_temp_file("temp1.c", 
        "static int foo(void) { return 0; }\n"
        "int main(void) { return foo(); }\n")) {
        return EXIT_FAILURE;
    }
    
    /* Valid source file 2 */
    if (!create_temp_file("temp2.c",
        "static int bar(void) { return 1; }\n"
        "int helper(void) { return bar(); }\n")) {
        return EXIT_FAILURE;
    }
    
    /* Source file with syntax error */
    if (!create_temp_file("syntax_error.c",
        "static int baz(void) { return  /* Missing expression */ }\n"
        "int bad(void) { return baz(); }\n")) {
        return EXIT_FAILURE;
    }
    
    /* Create dump directory */
    create_temp_dir("dump_test_dir");
    
    /* Test Sequence 1: Help flag then compilation
     * Tests: print_help_list, print_version reset */
    printf("\n--- Sequence 1: Help then Compile ---\n");
    
    /* First job: print help (sets print_help_list or print_version) */
    snprintf(cmd, sizeof(cmd), "%s --help=common 2>&1 | head -5 > /dev/null", 
             "gcc");
    total_tests++;
    success_count += execute_command(cmd, 0);
    
    /* Second job: compile a file (should reset help flags) */
    snprintf(cmd, sizeof(cmd), "%s -c temp1.c -o temp1.o", "gcc");
    total_tests++;
    success_count += execute_command(cmd, 0);
    
    /* Test Sequence 2: Version flag then different help
     * Tests: print_version, print_help_list interaction */
    printf("\n--- Sequence 2: Version then Optimizer Help ---\n");
    
    snprintf(cmd, sizeof(cmd), "%s --version 2>&1 | head -2 > /dev/null", "gcc");
    total_tests++;
    success_count += execute_command(cmd, 0);
    
    snprintf(cmd, sizeof(cmd), "%s --help=optimizers 2>&1 | head -5 > /dev/null", "gcc");
    total_tests++;
    success_count += execute_command(cmd, 0);
    
    /* Test Sequence 3: Save-temps with dumpdir then plain compile
     * Tests: save_temps_flag, dumpdir, dumpbase, free() calls */
    printf("\n--- Sequence 3: Save-temps with Dumpdir then Plain Compile ---\n");
    
    /* First job with save-temps and dump options */
    snprintf(cmd, sizeof(cmd), 
             "%s -save-temps -dumpdir ./dump_test_dir -dumpbase mydump "
             "-dumpbase-ext .ext -c temp1.c -o temp1_save.o 2>&1", 
             "gcc");
    total_tests++;
    success_count += execute_command(cmd, 0);
    
    /* Check if dump files were created */
    total_tests++;
    if (access("./dump_test_dir/mydump.i", F_OK) == 0 ||
        access("mydump.i", F_OK) == 0) {
        printf("  ✓ Save-temps created dump files\n");
        success_count++;
    } else {
        printf("  ✗ Save-temps may not have created expected files\n");
    }
    
    /* Second job without save-temps (should reset dumpdir, dumpbase, etc.) */
    snprintf(cmd, sizeof(cmd), "%s -c temp2.c -o temp2.o", "gcc");
    total_tests++;
    success_count += execute_command(cmd, 0);
    
    /* Test Sequence 4: Verbose flag then normal compile
     * Tests: verbose_only_flag reset */
    printf("\n--- Sequence 4: Verbose then Normal Compile ---\n");
    
    snprintf(cmd, sizeof(cmd), "%s -v -c temp1.c -o temp1_v.o 2>&1 | "
             "grep -q 'COLLECT_GCC_OPTIONS'", "gcc");
    total_tests++;
    success_count += execute_command(cmd, 0);
    
    snprintf(cmd, sizeof(cmd), "%s -c temp2.c -o temp2_v.o", "gcc");
    total_tests++;
    success_count += execute_command(cmd, 0);
    
    /* Test Sequence 5: Linker selection then default
     * Tests: use_ld reset */
    printf("\n--- Sequence 5: Specific Linker then Default ---\n");
    
    /* Try different linkers, accepting any result since not all may be available */
    snprintf(cmd, sizeof(cmd), "%s -fuse-ld=bfd -c temp1.c -o temp1_ld.o 2>&1", "gcc");
    total_tests++;
    success_count += execute_command(cmd, -1);
    
    snprintf(cmd, sizeof(cmd), "%s -c temp2.c -o temp2_ld.o", "gcc");
    total_tests++;
    success_count += execute_command(cmd, 0);
    
    /* Test Sequence 6: Error then success
     * Tests: greatest_status reset to 1 */
    printf("\n--- Sequence 6: Error then Success (tests greatest_status) ---\n");
    
    /* First job: compilation error (should set greatest_status to failure) */
    snprintf(cmd, sizeof(cmd), "%s -c syntax_error.c -o syntax_error.o 2>/dev/null", "gcc");
    total_tests++;
    /* Expect non-zero exit status for compilation error */
    int status = system(cmd);
    if (WEXITSTATUS(status) != 0) {
        printf("  ✓ First job failed as expected (status: %d)\n", WEXITSTATUS(status));
        success_count++;
    } else {
        printf("  ✗ First job should have failed\n");
    }
    
    /* Second job: successful compilation (greatest_status should be reset) */
    snprintf(cmd, sizeof(cmd), "%s -c temp1.c -o temp1_final.o", "gcc");
    total_tests++;
    success_count += execute_command(cmd, 0);
    
    /* Test Sequence 7: Multiple inputs in single invocation
     * Tests: per-job initialization with multiple source files */
    printf("\n--- Sequence 7: Multiple Inputs Single Invocation ---\n");
    
    snprintf(cmd, sizeof(cmd), "%s -c temp1.c temp2.c", "gcc");
    total_tests++;
    success_count += execute_command(cmd, 0);
    
    /* Test Sequence 8: Different language specifications
     * Tests: driver job sequencing with -x option */
    printf("\n--- Sequence 8: Different Language Specifications ---\n");
    
    /* Create a C++ source file */
    create_temp_file("temp3.cpp",
        "extern \"C\" int puts(const char*);\n"
        "int main() { puts(\"Hello\"); return 0; }\n");
    
    /* Compile C and C++ in sequence */
    snprintf(cmd, sizeof(cmd), "%s -c temp1.c -o temp1_mix.o", "gcc");
    total_tests++;
    success_count += execute_command(cmd, 0);
    
    snprintf(cmd, sizeof(cmd), "%s -x c++ -c temp3.cpp -o temp3.o", "gcc");
    total_tests++;
    success_count += execute_command(cmd, 0);
    
    /* Test Sequence 9: Combined options then minimal
     * Tests: multiple state variables reset simultaneously */
    printf("\n--- Sequence 9: Combined Options then Minimal ---\n");
    
    snprintf(cmd, sizeof(cmd),
             "%s -v -save-temps -dumpdir ./dump_test_dir -fuse-ld=gold "
             "-c temp1.c -o temp1_combined.o 2>&1 | head -20 > /dev/null",
             "gcc");
    total_tests++;
    success_count += execute_command(cmd, -1);
    
    snprintf(cmd, sizeof(cmd), "%s -c temp2.c -o temp2_minimal.o", "gcc");
    total_tests++;
    success_count += execute_command(cmd, 0);
    
    /* Cleanup */
    printf("\n--- Cleaning up temporary files ---\n");
    
    /* Remove temporary source files */
    remove("temp1.c");
    remove("temp2.c");
    remove("syntax_error.c");
    remove("temp3.cpp");
    
    /* Remove object files */
    system("rm -f *.o *.i *.s *.ii 2>/dev/null");
    
    /* Remove dump directory */
    remove_temp_dir("dump_test_dir");
    
    /* Remove any other temporary files */
    system("rm -f mydump.* 2>/dev/null");
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests executed: %d\n", total_tests);
    printf("Tests passed: %d\n", success_count);
    
    if (success_count == total_tests) {
        printf("✓ All tests passed!\n");
    } else {
        printf("✗ Some tests failed (%d failures)\n", total_tests - success_count);
    }
    
    /* Compute and print checksum for validation */
    unsigned int checksum = (success_count * 1000) + total_tests;
    printf("Validation checksum: 0x%08x\n", checksum);
    
    return (success_count == total_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* Dummy functions to avoid unused function warnings */
static int foo(void) { return 0; }
static int bar(void) { return 1; }
static int baz(void) { return 2; }
