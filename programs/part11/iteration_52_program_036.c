/* test_gcc_driver_init.c
 * Test program to cover GCC driver initialization block (lines 11228-11250 in gcc.cc)
 * This program orchestrates multiple GCC invocations with different state configurations
 * to ensure the driver reset logic is exercised between jobs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Simple functions to avoid unused function warnings */
static int foo(void) { return 0; }
static int bar(void) { return 1; }

/* Create a temporary C source file with given content */
static int create_temp_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create temp file");
        return 0;
    }
    fprintf(f, "%s", content);
    fclose(f);
    return 1;
}

/* Execute a GCC command and check return status */
static int run_gcc_command(const char *cmd, int expected_nonzero) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    int result = WEXITSTATUS(status);
    
    if (expected_nonzero) {
        return result != 0;  /* Return 1 if command failed as expected */
    } else {
        return result == 0;  /* Return 1 if command succeeded */
    }
}

/* Clean up temporary files */
static void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            unlink(files[i]);
        }
    }
}

int main(void) {
    int checksum = 0;
    const char *temp_files[10] = {NULL};
    int file_count = 0;
    
    /* Create temporary source files */
    const char *valid_c_code = "int foo(void) { return 0; }\n";
    const char *valid_c_code2 = "int bar(void) { return 1; }\n";
    const char *syntax_error_code = "int baz(void) { return \n";  /* Missing expression */
    
    temp_files[file_count++] = "temp1.c";
    temp_files[file_count++] = "temp2.c";
    temp_files[file_count++] = "syntax_error.c";
    temp_files[file_count++] = "temp1.o";
    temp_files[file_count++] = "temp2.o";
    temp_files[file_count++] = "temp1.i";
    temp_files[file_count++] = "temp1.s";
    
    if (!create_temp_file("temp1.c", valid_c_code) ||
        !create_temp_file("temp2.c", valid_c_code2) ||
        !create_temp_file("syntax_error.c", syntax_error_code)) {
        cleanup_files(temp_files, file_count);
        return 1;
    }
    
    /* Create dump directory */
    mkdir("./dump1", 0755);
    
    printf("=== Testing GCC Driver Initialization Reset Logic ===\n\n");
    
    /* Sequence 1: Help flag then compilation (tests print_help_list reset) */
    printf("--- Sequence 1: Help then Compile ---\n");
    checksum += run_gcc_command("gcc --help=common 2>&1 | head -5 > /dev/null", 0);
    checksum += run_gcc_command("gcc -c temp1.c -o temp1.o", 0);
    printf("Sequence 1 result: %d\n\n", checksum);
    
    /* Sequence 2: Version flag then compilation (tests print_version reset) */
    printf("--- Sequence 2: Version then Compile ---\n");
    checksum += run_gcc_command("gcc --version 2>&1 | head -1 > /dev/null", 0);
    checksum += run_gcc_command("gcc -c temp2.c -o temp2.o", 0);
    printf("Sequence 2 result: %d\n\n", checksum);
    
    /* Sequence 3: Save-temps with dumpdir then plain compile 
       (tests save_temps_flag, dumpdir, dumpbase reset) */
    printf("--- Sequence 3: Save-temps with dumpdir then Plain Compile ---\n");
    checksum += run_gcc_command("gcc -save-temps -dumpdir ./dump1 -dumpbase base -c temp1.c -o temp1.o 2>/dev/null", 0);
    checksum += run_gcc_command("gcc -c temp2.c -o temp2.o 2>/dev/null", 0);
    printf("Sequence 3 result: %d\n\n", checksum);
    
    /* Sequence 4: Verbose flag then quiet compile (tests verbose_only_flag reset) */
    printf("--- Sequence 4: Verbose then Quiet Compile ---\n");
    checksum += run_gcc_command("gcc -v -c temp1.c 2>&1 | grep -q 'COLLECT_GCC_OPTIONS'", 0);
    checksum += run_gcc_command("gcc -c temp2.c 2>/dev/null", 0);
    printf("Sequence 4 result: %d\n\n", checksum);
    
    /* Sequence 5: Specific linker then default (tests use_ld reset) */
    printf("--- Sequence 5: Specific Linker then Default ---\n");
    /* Try different linkers, accepting failure if not available */
    checksum += run_gcc_command("gcc -fuse-ld=bfd -c temp1.c 2>/dev/null", 1);  /* May fail if bfd not available */
    checksum += run_gcc_command("gcc -c temp2.c 2>/dev/null", 0);
    printf("Sequence 5 result: %d\n\n", checksum);
    
    /* Sequence 6: Error then success (tests greatest_status reset) */
    printf("--- Sequence 6: Error then Success ---\n");
    checksum += run_gcc_command("gcc -c syntax_error.c 2>/dev/null", 1);  /* Should fail */
    checksum += run_gcc_command("gcc -c temp1.c -o temp1.o", 0);  /* Should succeed */
    printf("Sequence 6 result: %d\n\n", checksum);
    
    /* Sequence 7: Multiple input files in single invocation (tests per-job initialization) */
    printf("--- Sequence 7: Multiple Input Files ---\n");
    checksum += run_gcc_command("gcc -c temp1.c temp2.c 2>/dev/null", 0);
    printf("Sequence 7 result: %d\n\n", checksum);
    
    /* Sequence 8: Using -x to specify language (tests driver job sequencing) */
    printf("--- Sequence 8: Using -x Language Specification ---\n");
    checksum += run_gcc_command("echo 'int x=1;' | gcc -x c -c - -o temp1.o 2>/dev/null", 0);
    checksum += run_gcc_command("gcc -c temp2.c 2>/dev/null", 0);
    printf("Sequence 8 result: %d\n\n", checksum);
    
    /* Sequence 9: Compiler self-test with different optimization levels */
    printf("--- Sequence 9: Different Optimization Levels ---\n");
    checksum += run_gcc_command("gcc -O0 -c temp1.c -o temp1.o 2>/dev/null", 0);
    checksum += run_gcc_command("gcc -O2 -c temp2.c -o temp2.o 2>/dev/null", 0);
    checksum += run_gcc_command("gcc -Os -c temp1.c -o temp1.o 2>/dev/null", 0);
    printf("Sequence 9 result: %d\n\n", checksum);
    
    /* Clean up */
    printf("--- Cleaning up ---\n");
    cleanup_files(temp_files, file_count);
    
    /* Remove dump directory if empty */
    rmdir("./dump1");
    
    /* Final checksum and verification */
    printf("\n=== Final Results ===\n");
    printf("Total checksum: %d\n", checksum);
    printf("Expected minimum checksum (all basic sequences succeed): 12\n");
    
    /* Reference the functions to avoid unused warnings */
    (void)foo();
    (void)bar();
    
    if (checksum >= 12) {
        printf("SUCCESS: Driver initialization reset logic likely exercised\n");
        return 0;
    } else {
        printf("WARNING: Some sequences may have failed\n");
        return 1;
    }
}
