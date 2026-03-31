/* test_gcc_driver_init.c
 * 
 * This program tests the GCC driver initialization logic by executing
 * multiple compilation jobs in sequence, forcing the driver to reset
 * various global and static variables between jobs.
 * 
 * The specific target is lines 11228-11250 in gcc.cc which reset:
 * - help/version flags (print_help_list, print_version)
 * - save-temps and dump directory flags
 * - verbose flag
 * - linker selection
 * - error status
 * - and other state variables
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Function prototypes for the dummy functions we'll create */
int foo(void) { return 0; }
int bar(void) { return 1; }
int baz(void) { return 2; }

/* Helper function to create a temporary file with given content */
static int create_temp_file(const char *filename, const char *content) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return 0;
    }
    fputs(content, fp);
    fclose(fp);
    return 1;
}

/* Helper function to remove a file if it exists */
static void cleanup_file(const char *filename) {
    if (access(filename, F_OK) == 0) {
        unlink(filename);
    }
}

/* Helper function to remove a directory if it exists */
static void cleanup_dir(const char *dirname) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dirname);
    system(cmd);
}

int main(void) {
    int overall_result = 0;
    int sequence_results[8] = {0}; /* Track results of each sequence */
    int seq_idx = 0;
    
    /* Create temporary source files */
    const char *temp1_content = 
        "int foo(void) { return 0; }\n"
        "int bar(void) { return 1; }\n"
        "int baz(void) { return 2; }\n";
    
    const char *temp2_content = 
        "int add(int a, int b) { return a + b; }\n"
        "int sub(int a, int b) { return a - b; }\n";
    
    const char *syntax_error_content = 
        "int bad_func(void) { return \n"  /* Missing expression after return */
        "int missing_brace(void) { return 0; \n" /* Missing closing brace */
        "int extra_stuff = ;\n"; /* Missing expression */
    
    /* Create the files */
    if (!create_temp_file("temp1.c", temp1_content)) {
        fprintf(stderr, "Failed to create temp1.c\n");
        return 1;
    }
    
    if (!create_temp_file("temp2.c", temp2_content)) {
        fprintf(stderr, "Failed to create temp2.c\n");
        cleanup_file("temp1.c");
        return 1;
    }
    
    if (!create_temp_file("syntax_error.c", syntax_error_content)) {
        fprintf(stderr, "Failed to create syntax_error.c\n");
        cleanup_file("temp1.c");
        cleanup_file("temp2.c");
        return 1;
    }
    
    /* Create dump directory */
    mkdir("./dump_test", 0755);
    
    printf("=== Testing GCC Driver Initialization Block ===\n");
    printf("Target: Lines 11228-11250 in gcc.cc\n\n");
    
    /* Sequence 1: Help flag then compilation
     * This sets print_help_list, then resets it for the next job */
    printf("Sequence 1: --help=common followed by compilation\n");
    int result1 = system("gcc --help=common > /dev/null 2>&1 && "
                         "gcc -c temp1.c -o temp1.o 2>/dev/null");
    sequence_results[seq_idx++] = (result1 == 0) ? 1 : 0;
    printf("Result: %s\n\n", (result1 == 0) ? "PASS" : "FAIL");
    
    /* Clean up temp1.o for next sequences */
    cleanup_file("temp1.o");
    
    /* Sequence 2: Version flag then compilation  
     * This sets print_version, then resets it */
    printf("Sequence 2: --version followed by compilation\n");
    int result2 = system("gcc --version > /dev/null 2>&1 && "
                         "gcc -c temp2.c -o temp2.o 2>/dev/null");
    sequence_results[seq_idx++] = (result2 == 0) ? 1 : 0;
    printf("Result: %s\n\n", (result2 == 0) ? "PASS" : "FAIL");
    
    cleanup_file("temp2.o");
    
    /* Sequence 3: Save-temps with dumpdir then plain compilation
     * This sets save_temps_flag, dumpdir, dumpbase, then frees them */
    printf("Sequence 3: -save-temps with -dumpdir then plain compile\n");
    int result3 = system("gcc -save-temps -dumpdir ./dump_test -dumpbase mydump "
                         "-c temp1.c -o temp1_save.o 2>/dev/null && "
                         "gcc -c temp2.c -o temp2_plain.o 2>/dev/null");
    sequence_results[seq_idx++] = (result3 == 0) ? 1 : 0;
    printf("Result: %s\n\n", (result3 == 0) ? "PASS" : "FAIL");
    
    /* Check if dump files were created */
    int dump_files_exist = (access("./dump_test/mydump.i", F_OK) == 0 ||
                           access("./dump_test/mydump.s", F_OK) == 0 ||
                           access("mydump.i", F_OK) == 0);
    printf("Dump files created: %s\n", dump_files_exist ? "YES" : "NO");
    
    cleanup_file("temp1_save.o");
    cleanup_file("temp2_plain.o");
    
    /* Sequence 4: Verbose flag then normal compilation
     * This sets verbose_only_flag, then resets it */
    printf("Sequence 4: -v (verbose) followed by normal compile\n");
    int result4 = system("gcc -v -c temp1.c -o temp1_v.o 2>&1 | "
                         "grep -q 'COLLECT_GCC_OPTIONS' && "
                         "gcc -c temp2.c -o temp2_norm.o 2>/dev/null");
    sequence_results[seq_idx++] = (result4 == 0) ? 1 : 0;
    printf("Result: %s\n\n", (result4 == 0) ? "PASS" : "FAIL");
    
    cleanup_file("temp1_v.o");
    cleanup_file("temp2_norm.o");
    
    /* Sequence 5: Linker selection then default
     * This sets use_ld, then resets to NULL */
    printf("Sequence 5: -fuse-ld=bfd then default linker\n");
    int result5 = system("gcc -fuse-ld=bfd -c temp1.c -o temp1_bfd.o 2>/dev/null && "
                         "gcc -c temp2.c -o temp2_def.o 2>/dev/null");
    sequence_results[seq_idx++] = (result5 == 0) ? 1 : 0;
    printf("Result: %s\n\n", (result5 == 0) ? "PASS" : "FAIL");
    
    cleanup_file("temp1_bfd.o");
    cleanup_file("temp2_def.o");
    
    /* Sequence 6: Error then success (tests greatest_status reset)
     * First compilation fails, second succeeds */
    printf("Sequence 6: Error compilation followed by success\n");
    int result6_fail = system("gcc -c syntax_error.c -o error.o 2>/dev/null");
    int result6_success = system("gcc -c temp1.c -o temp1_after_err.o 2>/dev/null");
    sequence_results[seq_idx++] = (result6_fail != 0 && result6_success == 0) ? 1 : 0;
    printf("First (error) result: %s\n", (result6_fail != 0) ? "FAIL as expected" : "UNEXPECTED PASS");
    printf("Second (success) result: %s\n", (result6_success == 0) ? "PASS" : "FAIL");
    printf("Overall: %s\n\n", (result6_fail != 0 && result6_success == 0) ? "PASS" : "FAIL");
    
    cleanup_file("error.o");
    cleanup_file("temp1_after_err.o");
    
    /* Sequence 7: Multiple options combined then reset
     * Tests multiple variables being set and reset together */
    printf("Sequence 7: Combined flags then clean compile\n");
    int result7 = system("gcc -v -save-temps -dumpdir ./dump_test2 -fuse-ld=bfd "
                         "-c temp1.c -o temp1_combo.o 2>/dev/null && "
                         "gcc -c temp2.c -o temp2_clean.o 2>/dev/null");
    sequence_results[seq_idx++] = (result7 == 0) ? 1 : 0;
    printf("Result: %s\n\n", (result7 == 0) ? "PASS" : "FAIL");
    
    cleanup_file("temp1_combo.o");
    cleanup_file("temp2_clean.o");
    cleanup_dir("./dump_test2");
    
    /* Sequence 8: Using -x to chain language-specific jobs
     * Tests job sequencing within single invocation */
    printf("Sequence 8: Multiple -x language specs in one invocation\n");
    const char *cmd8 = 
        "echo 'int x=1;' | gcc -x c - -x assembler - -x c - -c -o multi.o 2>/dev/null << 'EOF'\n"
        ".globl dummy\n"
        "dummy:\n"
        "  ret\n"
        "EOF\n"
        "int y=2;";
    FILE *fp8 = popen(cmd8, "r");
    int result8 = pclose(fp8);
    sequence_results[seq_idx++] = (result8 == 0) ? 1 : 0;
    printf("Result: %s\n\n", (result8 == 0) ? "PASS" : "FAIL");
    
    cleanup_file("multi.o");
    
    /* Calculate checksum based on sequence results */
    printf("=== Results Summary ===\n");
    int checksum = 0;
    for (int i = 0; i < seq_idx; i++) {
        printf("Sequence %d: %s\n", i+1, sequence_results[i] ? "PASS" : "FAIL");
        checksum = (checksum << 1) | sequence_results[i];
    }
    
    printf("\nChecksum (binary of pass/fail): 0x%02X\n", checksum);
    
    /* Cleanup temporary files */
    cleanup_file("temp1.c");
    cleanup_file("temp2.c");
    cleanup_file("syntax_error.c");
    cleanup_dir("./dump_test");
    
    /* Also clean up any stray dump files */
    cleanup_file("mydump.i");
    cleanup_file("mydump.s");
    cleanup_file("mydump.o");
    cleanup_file("temp1.i");
    cleanup_file("temp1.s");
    cleanup_file("temp2.i");
    cleanup_file("temp2.s");
    
    /* Use the dummy functions to avoid unused function warnings */
    (void)foo();
    (void)bar();
    (void)baz();
    
    printf("\nTest completed. To maximize coverage:\n");
    printf("1. Compile this test with: gcc -O0 -Wno-unused-function test_gcc_driver_init.c -o test_driver\n");
    printf("2. Run with: ./test_driver\n");
    printf("3. For coverage analysis, ensure gcc is built with coverage instrumentation\n");
    
    return overall_result;
}
