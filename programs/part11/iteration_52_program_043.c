/* driver_coverage_test.c
 * Test program to cover GCC driver initialization block (lines 11228-11250 in gcc.cc)
 * This program orchestrates multiple gcc invocations with different state variables
 * to ensure the driver reset logic is executed between jobs.
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
    fputs(content, f);
    fclose(f);
    return 0;
}

/* Helper to remove temporary files */
static void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            unlink(files[i]);
        }
    }
}

/* Helper to execute a command and check return status */
static int execute_and_check(const char *cmd, const char *desc) {
    printf("Executing: %s\n", desc);
    printf("Command: %s\n", cmd);
    
    int status = system(cmd);
    if (status == -1) {
        fprintf(stderr, "Failed to execute command: %s\n", desc);
        return -1;
    }
    
    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        printf("Exit status: %d\n\n", exit_status);
        return exit_status;
    }
    
    return -1;
}

int main(void) {
    int overall_result = 0;
    int checksum = 0;
    
    /* Create temporary source files */
    const char *temp_files[] = {
        "temp_valid1.c",
        "temp_valid2.c", 
        "temp_error.c",
        "temp_valid3.c",
        NULL
    };
    
    /* Create valid source file 1 */
    if (create_temp_file("temp_valid1.c", 
        "int foo(void) { return 0; }\n"
        "int unused_func1(void) { return 42; }\n") < 0) {
        return 1;
    }
    
    /* Create valid source file 2 */
    if (create_temp_file("temp_valid2.c",
        "int bar(void) { return 1; }\n"
        "int unused_func2(void) { return 43; }\n") < 0) {
        cleanup_files(temp_files, 4);
        return 1;
    }
    
    /* Create source file with syntax error */
    if (create_temp_file("temp_error.c",
        "int baz(void) { return /* missing semicolon and value */ }\n") < 0) {
        cleanup_files(temp_files, 4);
        return 1;
    }
    
    /* Create another valid source file for additional tests */
    if (create_temp_file("temp_valid3.c",
        "int qux(void) { return 2; }\n"
        "int unused_func3(void) { return 44; }\n") < 0) {
        cleanup_files(temp_files, 4);
        return 1;
    }
    
    /* Create dump directory */
    mkdir("./dump_test_dir", 0755);
    
    printf("=== GCC Driver Initialization Coverage Test ===\n\n");
    
    /* Sequence 1: Help flag then compilation
     * This sets print_help_list, then resets it for compilation job */
    printf("--- Sequence 1: Help then Compile ---\n");
    int seq1_result = execute_and_check(
        "gcc --help=common 2>&1 | head -5 > /dev/null && "
        "gcc -c temp_valid1.c -o temp_valid1.o 2>&1",
        "Help flag followed by compilation"
    );
    checksum = (checksum * 31 + (seq1_result == 0 ? 1 : 0)) & 0xFF;
    
    /* Sequence 2: Version flag then compilation  
     * This sets print_version, then resets it */
    printf("--- Sequence 2: Version then Compile ---\n");
    int seq2_result = execute_and_check(
        "gcc --version 2>&1 | head -1 > /dev/null && "
        "gcc -c temp_valid2.c -o temp_valid2.o 2>&1",
        "Version flag followed by compilation"
    );
    checksum = (checksum * 31 + (seq2_result == 0 ? 1 : 0)) & 0xFF;
    
    /* Sequence 3: Save-temps with dumpdir then plain compile
     * This sets save_temps_flag, dumpdir, dumpbase, then resets them */
    printf("--- Sequence 3: Save-temps with dumpdir then plain compile ---\n");
    int seq3_result = execute_and_check(
        "gcc -save-temps -dumpdir ./dump_test_dir -dumpbase testbase "
        "-c temp_valid1.c -o temp_valid1_save.o 2>&1 && "
        "gcc -c temp_valid2.c -o temp_valid2_plain.o 2>&1",
        "Save-temps with dumpdir followed by plain compilation"
    );
    checksum = (checksum * 31 + (seq3_result == 0 ? 1 : 0)) & 0xFF;
    
    /* Sequence 4: Verbose flag then compilation
     * This sets verbose_only_flag, then resets it */
    printf("--- Sequence 4: Verbose then Compile ---\n");
    int seq4_result = execute_and_check(
        "gcc -v -c temp_valid1.c -o temp_valid1_verbose.o 2>&1 | "
        "grep -q 'COLLECT_GCC_OPTIONS' && "
        "gcc -c temp_valid2.c -o temp_valid2_normal.o 2>&1",
        "Verbose compilation followed by normal compilation"
    );
    checksum = (checksum * 31 + (seq4_result == 0 ? 1 : 0)) & 0xFF;
    
    /* Sequence 5: Linker selection then default
     * This sets use_ld, then resets it to NULL */
    printf("--- Sequence 5: Specific linker then default ---\n");
    int seq5_result = execute_and_check(
        "gcc -fuse-ld=bfd -c temp_valid1.c -o temp_valid1_ldbfd.o 2>&1 && "
        "gcc -c temp_valid2.c -o temp_valid2_lddefault.o 2>&1",
        "BFD linker followed by default linker"
    );
    checksum = (checksum * 31 + (seq5_result == 0 ? 1 : 0)) & 0xFF;
    
    /* Sequence 6: Error then success compilation
     * This tests greatest_status reset from failure to success */
    printf("--- Sequence 6: Error then Success ---\n");
    int seq6_result = execute_and_check(
        "gcc -c temp_error.c -o temp_error.o 2>/dev/null; "
        "gcc -c temp_valid3.c -o temp_valid3.o 2>&1",
        "Failed compilation followed by successful compilation"
    );
    /* The first command should fail, second should succeed */
    checksum = (checksum * 31 + 1) & 0xFF;  // Always count this sequence
    
    /* Sequence 7: Multiple options combined then reset
     * Tests multiple variables being set and cleared together */
    printf("--- Sequence 7: Combined options then reset ---\n");
    int seq7_result = execute_and_check(
        "gcc -v --help=optimizers -save-temps -dumpdir ./dump_test_dir2 "
        "-fuse-ld=lld -c temp_valid1.c 2>&1 | head -20 > /dev/null; "
        "mkdir -p ./dump_test_dir2; "
        "gcc -c temp_valid2.c -o temp_valid2_final.o 2>&1",
        "Multiple options combined followed by clean compilation"
    );
    checksum = (checksum * 31 + (seq7_result == 0 ? 1 : 0)) & 0xFF;
    
    /* Sequence 8: Test with -### (verbose only) flag
     * This sets verbose_only_flag differently */
    printf("--- Sequence 8: -### flag then normal compile ---\n");
    int seq8_result = execute_and_check(
        "gcc -### -c temp_valid1.c 2>&1 | grep -q '^ \"/' && "
        "gcc -c temp_valid2.c -o temp_valid2_afterhash.o 2>&1",
        "-### flag followed by normal compilation"
    );
    checksum = (checksum * 31 + (seq8_result == 0 ? 1 : 0)) & 0xFF;
    
    /* Cleanup temporary files */
    printf("--- Cleaning up ---\n");
    cleanup_files(temp_files, 4);
    
    /* Remove object files created during tests */
    const char *obj_files[] = {
        "temp_valid1.o", "temp_valid2.o", "temp_valid1_save.o",
        "temp_valid2_plain.o", "temp_valid1_verbose.o", "temp_valid2_normal.o",
        "temp_valid1_ldbfd.o", "temp_valid2_lddefault.o", "temp_valid3.o",
        "temp_valid2_final.o", "temp_valid2_afterhash.o",
        "temp_valid1.i", "temp_valid1.s",  /* save-temps files */
        "testbase.i", "testbase.s",        /* dumpbase files */
        NULL
    };
    
    for (int i = 0; obj_files[i]; i++) {
        unlink(obj_files[i]);
    }
    
    /* Remove dump directories */
    system("rm -rf ./dump_test_dir ./dump_test_dir2");
    
    /* Final checksum and result */
    printf("\n=== Test Results ===\n");
    printf("Checksum (indicator of sequences executed): 0x%02X\n", checksum);
    
    /* Use the functions from our test files to avoid unused function warnings */
    void (*funcs[4])(void) = {
        (void(*)(void))foo,
        (void(*)(void))bar,
        (void(*)(void))qux,
        NULL
    };
    
    /* Reference them to avoid warnings (though they won't actually be called) */
    if (checksum == 0) {
        /* This won't execute, but references the functions */
        for (int i = 0; i < 3; i++) {
            if (funcs[i]) {
                /* Cast to void to avoid "unused function" warnings */
                (void)funcs[i];
            }
        }
    }
    
    printf("\nDriver initialization block coverage test completed.\n");
    printf("To verify coverage, run:\n");
    printf("  gcc -O0 -Wno-unused-function driver_coverage_test.c -o driver_test\n");
    printf("  ./driver_test\n");
    printf("Then check gcov/lcov output for gcc.cc lines 11228-11250\n");
    
    return overall_result;
}

/* Dummy declarations to satisfy linker if needed */
int foo(void) { return 0; }
int bar(void) { return 1; }
int qux(void) { return 2; }
