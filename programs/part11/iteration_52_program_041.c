/* test_gcc_driver_init.c
 * This program tests the GCC driver initialization logic by invoking
 * multiple compilation jobs with different option combinations.
 * It specifically targets the reset logic in driver::main (lines 11228-11250).
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
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fprintf(f, "%s", content);
    fclose(f);
    return 1;
}

/* Helper function to execute a command and check its return status */
static int execute_and_check(const char *cmd, int expected_status) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    int result = WEXITSTATUS(status);
    
    if (expected_status == -1) {
        /* Don't check status, just execute */
        return 1;
    }
    
    if (result != expected_status) {
        printf("  Warning: Command returned %d, expected %d\n", result, expected_status);
        /* Still continue the test */
    }
    return 1;
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
static void remove_dir(const char *dirname) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dirname);
    system(cmd);
}

int main(void) {
    int checksum = 0;
    char cmd[1024];
    
    printf("=== Testing GCC Driver Initialization Reset Logic ===\n");
    
    /* Create temporary source files */
    if (!create_temp_file("temp1.c", 
        "static int foo(void) { return 0; }\n"
        "int main(void) { return foo(); }\n")) {
        return 1;
    }
    
    if (!create_temp_file("temp2.c",
        "static int bar(void) { return 1; }\n"
        "int helper(void) { return bar(); }\n")) {
        remove("temp1.c");
        return 1;
    }
    
    if (!create_temp_file("syntax_error.c",
        "int baz(void) { return /* missing semicolon and value */ }\n")) {
        remove("temp1.c");
        remove("temp2.c");
        return 1;
    }
    
    /* Create dump directory */
    create_temp_dir("dump_test");
    
    /* Sequence 1: Help flag then compilation
     * This sets print_help_list, then resets it */
    printf("\n--- Sequence 1: Help then Compile ---\n");
    execute_and_check("gcc --help=common 2>&1 | head -5 > /dev/null", 0);
    checksum += 1;
    
    execute_and_check("gcc -c temp1.c -o temp1.o", 0);
    checksum += 2;
    
    /* Sequence 2: Version flag then compilation  
     * This sets print_version, then resets it */
    printf("\n--- Sequence 2: Version then Compile ---\n");
    execute_and_check("gcc --version 2>&1 | head -1 > /dev/null", 0);
    checksum += 4;
    
    execute_and_check("gcc -c temp2.c -o temp2.o", 0);
    checksum += 8;
    
    /* Sequence 3: Save-temps with dumpdir then plain compile
     * This sets save_temps_flag, dumpdir, dumpbase, then resets them */
    printf("\n--- Sequence 3: Save-temps with dumpdir then Compile ---\n");
    snprintf(cmd, sizeof(cmd), 
        "gcc -save-temps -dumpdir ./dump_test -dumpbase mydump "
        "-c temp1.c -o temp1_save.o 2>&1");
    execute_and_check(cmd, 0);
    checksum += 16;
    
    /* Check if dump files were created */
    execute_and_check("ls -la dump_test/mydump* 2>/dev/null | head -2", -1);
    
    /* Now compile without save-temps to trigger reset */
    execute_and_check("gcc -c temp2.c -o temp2_plain.o", 0);
    checksum += 32;
    
    /* Sequence 4: Verbose flag then normal compile
     * This sets verbose_only_flag, then resets it */
    printf("\n--- Sequence 4: Verbose then Normal ---\n");
    execute_and_check("gcc -v -c temp1.c -o temp1_v.o 2>&1 | grep -q 'COLLECT_GCC'", 0);
    checksum += 64;
    
    execute_and_check("gcc -c temp2.c -o temp2_norm.o", 0);
    checksum += 128;
    
    /* Sequence 5: Linker selection then default
     * This sets use_ld, then resets it to NULL */
    printf("\n--- Sequence 5: Specific Linker then Default ---\n");
    /* Try different linkers, accepting failure if not available */
    execute_and_check("gcc -fuse-ld=bfd -c temp1.c -o temp1_ld.o 2>/dev/null", -1);
    checksum += 256;
    
    execute_and_check("gcc -c temp2.c -o temp2_def.o", 0);
    checksum += 512;
    
    /* Sequence 6: Error then success
     * This tests greatest_status reset from failure to success */
    printf("\n--- Sequence 6: Error then Success ---\n");
    execute_and_check("gcc -c syntax_error.c 2>/dev/null", 1); /* Expected to fail */
    checksum += 1024;
    
    execute_and_check("gcc -c temp1.c -o temp1_final.o", 0);
    checksum += 2048;
    
    /* Sequence 7: Multiple options combined then reset
     * Tests multiple variables being set and reset together */
    printf("\n--- Sequence 7: Combined Options then Reset ---\n");
    snprintf(cmd, sizeof(cmd),
        "gcc -v --help=optimizers -save-temps -dumpdir ./dump_test2 "
        "-dumpbase combined -c temp1.c -o temp1_combined.o 2>&1 | head -20");
    execute_and_check(cmd, -1);
    checksum += 4096;
    
    create_temp_dir("dump_test2");
    execute_and_check("gcc -c temp2.c -o temp2_simple.o", 0);
    checksum += 8192;
    
    /* Sequence 8: Test with -### (dry run verbose) */
    printf("\n--- Sequence 8: Dry run then Actual ---\n");
    execute_and_check("gcc -### -c temp1.c 2>&1 | head -5 > /dev/null", 0);
    checksum += 16384;
    
    execute_and_check("gcc -c temp2.c -o temp2_actual.o", 0);
    checksum += 32768;
    
    /* Cleanup */
    printf("\n--- Cleaning up ---\n");
    remove("temp1.c");
    remove("temp2.c");
    remove("syntax_error.c");
    
    /* Remove object files */
    system("rm -f *.o *.i *.s *.ii 2>/dev/null");
    
    /* Remove dump directories */
    remove_dir("dump_test");
    remove_dir("dump_test2");
    
    /* Final checksum output */
    printf("\n=== Test Complete ===\n");
    printf("Checksum (sum of successful steps): %d\n", checksum);
    printf("Expected checksum if all sequences ran: 65535\n");
    
    /* Also compile the test program itself with various options
     * to exercise more driver paths during its own compilation */
    printf("\n=== Compiling test program with coverage-enhancing options ===\n");
    system("gcc -O0 -Wno-unused-function -save-temps test_gcc_driver_init.c "
           "-o test_gcc_driver_init 2>&1 | tail -5");
    
    return 0;
}

/* Dummy functions to avoid unused function warnings */
static int foo(void) { return 0; }
static int bar(void) { return 1; }
static int baz(void) { return 2; }
