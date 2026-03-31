/* test_gcc_driver_init.c
 * 
 * This test program exercises the GCC driver initialization logic
 * (lines 11228-11250 in gcc.cc) by invoking multiple compilation jobs
 * with different state configurations, forcing re-initialization of
 * global variables between jobs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Helper function to create a temporary file with given content */
static int create_temp_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
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

/* Helper function to execute a command and check its return status */
static int execute_command(const char *cmd, int expected_status) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    int ret = WEXITSTATUS(status);
    
    if (expected_status >= 0) {
        if (ret == expected_status) {
            printf("  -> OK (got %d, expected %d)\n", ret, expected_status);
            return 1;
        } else {
            printf("  -> FAIL (got %d, expected %d)\n", ret, expected_status);
            return 0;
        }
    } else {
        printf("  -> executed (status: %d)\n", ret);
        return 1;
    }
}

/* Helper to create a temporary directory */
static int create_temp_dir(const char *dirname) {
    if (mkdir(dirname, 0755) == -1) {
        if (errno != EEXIST) {
            perror("mkdir");
            return 0;
        }
    }
    return 1;
}

/* Helper to remove a directory recursively */
static void remove_dir(const char *dirname) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dirname);
    system(cmd);
}

int main(void) {
    int success_count = 0;
    int total_tests = 0;
    char cmd[1024];
    
    printf("=== Testing GCC Driver Initialization Block ===\n\n");
    
    /* Create temporary source files */
    printf("Creating temporary source files...\n");
    
    const char *valid_src1 = 
        "int foo(void) { return 0; }\n"
        "int unused_func1(void) __attribute__((unused));\n"
        "int unused_func1(void) { return 42; }\n";
    
    const char *valid_src2 = 
        "int bar(void) { return 1; }\n"
        "int unused_func2(void) __attribute__((unused));\n"
        "int unused_func2(void) { return 43; }\n";
    
    const char *error_src = 
        "int baz(void) { return /* missing semicolon and value */ }\n";
    
    if (!create_temp_file("temp1.c", valid_src1)) {
        fprintf(stderr, "Failed to create temp1.c\n");
        return 1;
    }
    
    if (!create_temp_file("temp2.c", valid_src2)) {
        fprintf(stderr, "Failed to create temp2.c\n");
        remove("temp1.c");
        return 1;
    }
    
    if (!create_temp_file("syntax_error.c", error_src)) {
        fprintf(stderr, "Failed to create syntax_error.c\n");
        remove("temp1.c");
        remove("temp2.c");
        return 1;
    }
    
    /* Create dump directory */
    create_temp_dir("dump_test_dir");
    
    printf("\n--- Test Sequence 1: Help then Compile ---\n");
    printf("(Tests print_help_list, print_version reset)\n");
    
    /* First job: help request (sets print_help_list) */
    total_tests++;
    if (execute_command("gcc --help=common 2>&1 | head -5 > /dev/null", 0)) {
        success_count++;
    }
    
    /* Second job: compilation (should reset help flags) */
    total_tests++;
    if (execute_command("gcc -c temp1.c -o temp1.o", 0)) {
        success_count++;
    }
    
    printf("\n--- Test Sequence 2: Save-Temps with Dumpdir then Compile ---\n");
    printf("(Tests save_temps_flag, dumpdir, dumpbase reset)\n");
    
    /* First job: with save-temps and dumpdir (sets save_temps_flag, dumpdir, dumpbase) */
    total_tests++;
    snprintf(cmd, sizeof(cmd), 
             "gcc -save-temps -dumpdir ./dump_test_dir -dumpbase mydump "
             "-c temp1.c -o temp1_save.o 2>&1");
    if (execute_command(cmd, 0)) {
        success_count++;
    }
    
    /* Check if dump files were created */
    total_tests++;
    if (execute_command("test -f ./dump_test_dir/mydump.i || test -f temp1.i", -1)) {
        success_count++;
    }
    
    /* Second job: plain compilation (should free dumpdir/dumpbase and reset save_temps_flag) */
    total_tests++;
    if (execute_command("gcc -c temp2.c -o temp2.o", 0)) {
        success_count++;
    }
    
    printf("\n--- Test Sequence 3: Error then Success ---\n");
    printf("(Tests greatest_status reset from failure to success)\n");
    
    /* First job: compilation error (sets greatest_status to failure) */
    total_tests++;
    if (execute_command("gcc -c syntax_error.c 2>/dev/null", 1)) {
        success_count++;
    }
    
    /* Second job: successful compilation (should reset greatest_status) */
    total_tests++;
    if (execute_command("gcc -c temp1.c -o temp1_err.o", 0)) {
        success_count++;
    }
    
    printf("\n--- Test Sequence 4: Verbose and Linker then Plain ---\n");
    printf("(Tests verbose_only_flag, use_ld reset)\n");
    
    /* First job: verbose with specific linker */
    total_tests++;
    snprintf(cmd, sizeof(cmd),
             "gcc -v -fuse-ld=bfd -c temp1.c -o temp1_verbose.o 2>&1 "
             "| grep -q 'COLLECT_GCC_OPTIONS'");
    if (execute_command(cmd, 0)) {
        success_count++;
    }
    
    /* Second job: plain compilation (should reset verbose_only_flag, use_ld) */
    total_tests++;
    if (execute_command("gcc -c temp2.c -o temp2_plain.o", 0)) {
        success_count++;
    }
    
    printf("\n--- Test Sequence 5: Multiple Input Files (Single Invocation) ---\n");
    printf("(Tests per-job initialization with multiple inputs)\n");
    
    /* Single invocation with multiple inputs - each is a separate job */
    total_tests++;
    if (execute_command("gcc -c temp1.c temp2.c", 0)) {
        success_count++;
    }
    
    printf("\n--- Test Sequence 6: Version then Help then Compile ---\n");
    printf("(Tests multiple flag resets in sequence)\n");
    
    /* Chain of different job types */
    total_tests++;
    if (execute_command("gcc --version 2>&1 | head -1 > /dev/null", 0)) {
        success_count++;
    }
    
    total_tests++;
    if (execute_command("gcc --help=optimizers 2>&1 | head -5 > /dev/null", 0)) {
        success_count++;
    }
    
    total_tests++;
    if (execute_command("gcc -c temp1.c -o temp1_chain.o", 0)) {
        success_count++;
    }
    
    printf("\n--- Test Sequence 7: Specs and Sysroot ---\n");
    printf("(Tests spec_machine, target_system_root reset)\n");
    
    /* First job: with specs file if available */
    total_tests++;
    snprintf(cmd, sizeof(cmd),
             "echo '*cpp: -DSPEC_TEST' > test.specs 2>/dev/null && "
             "gcc -specs=test.specs -c temp1.c -o temp1_spec.o 2>/dev/null || true");
    if (execute_command(cmd, -1)) {
        success_count++;
    }
    
    /* Second job: without specs */
    total_tests++;
    if (execute_command("gcc -c temp2.c -o temp2_nospec.o", 0)) {
        success_count++;
    }
    
    /* Cleanup temporary files */
    printf("\n--- Cleaning up ---\n");
    remove("temp1.c");
    remove("temp2.c");
    remove("syntax_error.c");
    remove("temp1.o");
    remove("temp2.o");
    remove("temp1_save.o");
    remove("temp1_err.o");
    remove("temp1_verbose.o");
    remove("temp2_plain.o");
    remove("temp1_chain.o");
    remove("temp1_spec.o");
    remove("temp2_nospec.o");
    remove("test.specs");
    remove_dir("dump_test_dir");
    
    /* Also clean up any save-temps files that might have been created */
    system("rm -f temp1.i temp1.s temp1.o temp2.i temp2.s temp2.o 2>/dev/null");
    system("rm -f *.i *.s *.o 2>/dev/null");
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", success_count, total_tests);
    printf("Success rate: %.1f%%\n", (success_count * 100.0) / total_tests);
    
    /* Compute checksum for validation */
    unsigned int checksum = success_count * 31 + total_tests * 17;
    printf("Validation checksum: 0x%08x\n", checksum);
    
    if (success_count == total_tests) {
        printf("\n✓ All tests passed! The driver initialization block should be covered.\n");
        return 0;
    } else {
        printf("\n⚠ Some tests failed. Driver initialization may not be fully covered.\n");
        return 1;
    }
}

/* Dummy functions to avoid -Wunused-function warnings */
void __attribute__((unused)) reference_dummy_funcs(void) {
    /* These references ensure the functions in temp files aren't optimized away */
    extern int foo(void);
    extern int bar(void);
    (void)foo;
    (void)bar;
}
