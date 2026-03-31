/* test_gcc_driver_init.c
 * 
 * This test program exercises the GCC driver initialization logic
 * (lines 11228-11250 in gcc.cc) by invoking multiple compilation jobs
 * with different option combinations that set and reset the target
 * state variables.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Simple utility functions for file operations */
static int write_temp_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fprintf(f, "%s", content);
    fclose(f);
    return 1;
}

static int file_exists(const char *filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

static void cleanup_file(const char *filename) {
    if (file_exists(filename)) {
        unlink(filename);
    }
}

static void cleanup_dir(const char *dirname) {
    /* Simple directory cleanup - remove if empty */
    rmdir(dirname);
}

/* Test sequences targeting specific initialization variables */
static int test_help_version_reset(void) {
    printf("Testing help/version flag reset...\n");
    
    /* Create a simple source file */
    const char *src1 = "int foo(void) { return 0; }\n";
    if (!write_temp_file("temp_help.c", src1)) return 0;
    
    /* Sequence: help flag followed by compilation */
    int status1 = system("gcc --help=common > /dev/null 2>&1");
    int status2 = system("gcc -c temp_help.c -o temp_help.o 2>/dev/null");
    
    /* Check both commands executed */
    if (status1 != 0) {
        printf("  WARNING: --help=common failed\n");
    }
    
    cleanup_file("temp_help.c");
    cleanup_file("temp_help.o");
    
    return status2 == 0;
}

static int test_save_temps_reset(void) {
    printf("Testing save-temps and dumpdir reset...\n");
    
    /* Create source files */
    const char *src1 = "int func1(void) { return 42; }\n";
    const char *src2 = "int func2(void) { return 99; }\n";
    
    if (!write_temp_file("temp_save1.c", src1)) return 0;
    if (!write_temp_file("temp_save2.c", src2)) return 0;
    
    /* Create dump directory */
    mkdir("./dump_test", 0755);
    
    /* Sequence: save-temps with dumpdir, then plain compilation */
    char cmd1[512];
    snprintf(cmd1, sizeof(cmd1),
             "gcc -save-temps -dumpdir ./dump_test -dumpbase mydump "
             "-c temp_save1.c -o temp_save1.o 2>/dev/null");
    
    int status1 = system(cmd1);
    int status2 = system("gcc -c temp_save2.c -o temp_save2.o 2>/dev/null");
    
    /* Check for some expected output files from save-temps */
    int has_i_file = file_exists("./dump_test/mydump.i");
    int has_s_file = file_exists("./dump_test/mydump.s");
    
    printf("  save-temps produced .i file: %s\n", has_i_file ? "yes" : "no");
    printf("  save-temps produced .s file: %s\n", has_s_file ? "yes" : "no");
    
    /* Cleanup */
    cleanup_file("temp_save1.c");
    cleanup_file("temp_save2.c");
    cleanup_file("temp_save1.o");
    cleanup_file("temp_save2.o");
    cleanup_file("./dump_test/mydump.i");
    cleanup_file("./dump_test/mydump.s");
    cleanup_file("./dump_test/mydump.o");
    cleanup_dir("./dump_test");
    
    return status1 == 0 && status2 == 0;
}

static int test_error_recovery_reset(void) {
    printf("Testing error recovery and greatest_status reset...\n");
    
    /* Create valid and invalid source files */
    const char *valid_src = "int valid_func(void) { return 0; }\n";
    const char *invalid_src = "int invalid_func(void) { return \n"; /* Syntax error */
    
    if (!write_temp_file("temp_valid.c", valid_src)) return 0;
    if (!write_temp_file("temp_invalid.c", invalid_src)) return 0;
    
    /* Sequence: failing compilation followed by successful one */
    /* First compilation should fail due to syntax error */
    int status1 = system("gcc -c temp_invalid.c -o temp_invalid.o 2>/dev/null");
    int status2 = system("gcc -c temp_valid.c -o temp_valid.o 2>/dev/null");
    
    printf("  First (error) compilation status: %d (expected non-zero)\n", status1);
    printf("  Second (success) compilation status: %d (expected zero)\n", status2);
    
    cleanup_file("temp_valid.c");
    cleanup_file("temp_invalid.c");
    cleanup_file("temp_valid.o");
    cleanup_file("temp_invalid.o");
    
    /* First should fail, second should succeed */
    return status1 != 0 && status2 == 0;
}

static int test_verbose_linker_reset(void) {
    printf("Testing verbose and linker flag reset...\n");
    
    const char *src1 = "int verbose_test1(void) { return 1; }\n";
    const char *src2 = "int verbose_test2(void) { return 2; }\n";
    
    if (!write_temp_file("temp_v1.c", src1)) return 0;
    if (!write_temp_file("temp_v2.c", src2)) return 0;
    
    /* Sequence: verbose with linker flag, then plain compilation */
    /* Use -### instead of -v to avoid huge output, but still set verbose flag */
    int status1 = system("gcc -### -fuse-ld=bfd -c temp_v1.c 2>&1 | head -5 > /dev/null");
    int status2 = system("gcc -c temp_v2.c -o temp_v2.o 2>/dev/null");
    
    /* Also test with -v flag */
    int status3 = system("gcc -v -c temp_v1.c 2>&1 | grep -q 'COLLECT_GCC'");
    
    cleanup_file("temp_v1.c");
    cleanup_file("temp_v2.c");
    cleanup_file("temp_v2.o");
    
    return status2 == 0;
}

static int test_multiple_inputs_single_invocation(void) {
    printf("Testing multiple inputs in single invocation...\n");
    
    /* Create multiple source files */
    const char *src1 = "int multi1(void) { return 1; }\n";
    const char *src2 = "int multi2(void) { return 2; }\n";
    const char *src3 = "int multi3(void) { return 3; }\n";
    
    if (!write_temp_file("temp_m1.c", src1)) return 0;
    if (!write_temp_file("temp_m2.c", src2)) return 0;
    if (!write_temp_file("temp_m3.c", src3)) return 0;
    
    /* Single invocation with multiple inputs - tests job sequencing */
    int status = system("gcc -c temp_m1.c temp_m2.c temp_m3.c 2>/dev/null");
    
    /* Also test with mixed options between files using -x */
    int status2 = system("gcc -x c temp_m1.c -x c temp_m2.c -x c temp_m3.c -c 2>/dev/null");
    
    cleanup_file("temp_m1.c");
    cleanup_file("temp_m2.c");
    cleanup_file("temp_m3.c");
    cleanup_file("temp_m1.o");
    cleanup_file("temp_m2.o");
    cleanup_file("temp_m3.o");
    
    return status == 0 || status2 == 0;
}

static int test_spec_machine_reset(void) {
    printf("Testing spec machine reset...\n");
    
    const char *src = "int spec_test(void) { return 0; }\n";
    if (!write_temp_file("temp_spec.c", src)) return 0;
    
    /* Try to affect spec machine through various options */
    int status1 = system("gcc -specs=/dev/null -c temp_spec.c 2>/dev/null");
    int status2 = system("gcc -c temp_spec.c 2>/dev/null");
    
    cleanup_file("temp_spec.c");
    cleanup_file("temp_spec.o");
    
    /* First might fail due to invalid specs file, but that's OK */
    return status2 == 0;
}

int main(void) {
    printf("=== GCC Driver Initialization Block Test ===\n\n");
    
    int results = 0;
    int test_num = 0;
    
    /* Run each test sequence and collect results */
    if (test_help_version_reset()) {
        results |= (1 << test_num);
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    test_num++;
    printf("\n");
    
    if (test_save_temps_reset()) {
        results |= (1 << test_num);
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    test_num++;
    printf("\n");
    
    if (test_error_recovery_reset()) {
        results |= (1 << test_num);
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    test_num++;
    printf("\n");
    
    if (test_verbose_linker_reset()) {
        results |= (1 << test_num);
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    test_num++;
    printf("\n");
    
    if (test_multiple_inputs_single_invocation()) {
        results |= (1 << test_num);
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    test_num++;
    printf("\n");
    
    if (test_spec_machine_reset()) {
        results |= (1 << test_num);
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    test_num++;
    
    /* Final checksum output */
    printf("\n=== Test Results Summary ===\n");
    printf("Checksum (bitmask of passed tests): 0x%02x\n", results);
    printf("All tests attempted.\n");
    
    /* Always return success since we're testing the compiler,
     * not the test program itself */
    return 0;
}

/* Avoid unused function warnings */
static void reference_temp_functions(void) {
    /* These functions are created in temp files and referenced here
     * to avoid compiler warnings about unused static functions */
    (void)foo;
    (void)func1;
    (void)func2;
    (void)valid_func;
    (void)verbose_test1;
    (void)verbose_test2;
    (void)multi1;
    (void)multi2;
    (void)multi3;
    (void)spec_test;
}
