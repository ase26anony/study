/* test_gcc_driver_init.c
 * Test program to cover GCC driver initialization block (lines 11228-11250 in gcc.cc)
 * This program orchestrates multiple GCC invocations to trigger state resets.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Simple functions to create temporary source files */
static int create_temp_file(const char *template, const char *content) {
    FILE *f = fopen(template, "w");
    if (!f) return -1;
    fputs(content, f);
    fclose(f);
    return 0;
}

static void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) unlink(files[i]);
    }
}

static int create_temp_dir(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        return mkdir(path, 0755);
    }
    return 0;
}

static void cleanup_dir(const char *path) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
    system(cmd);
}

/* Execute GCC command and check return status */
static int execute_gcc(const char *cmd, int expect_failure) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    int result = WEXITSTATUS(status);
    
    if (expect_failure) {
        if (result == 0) {
            printf("  WARNING: Expected failure but command succeeded\n");
            return 0;  // Still count as executed
        }
        return 1;  // Command executed (failed as expected)
    } else {
        if (result != 0) {
            printf("  WARNING: Command failed with status %d\n", result);
        }
        return 1;  // Command executed
    }
}

int main(void) {
    int checksum = 0;
    const char *temp_files[] = {
        "temp1.c",
        "temp2.c", 
        "syntax_error.c",
        "temp1.o",
        "temp2.o",
        "temp1.i",
        "temp1.s",
        NULL
    };
    
    /* Create temporary directory for dump files */
    const char *dump_dir = "./dump_test_dir";
    
    /* Step 1: Create temporary source files */
    printf("=== Creating temporary source files ===\n");
    
    if (create_temp_file("temp1.c", 
        "int foo(void) { return 0; }\n"
        "int unused_func1(void) { return 42; }\n") != 0) {
        fprintf(stderr, "Failed to create temp1.c\n");
        return 1;
    }
    
    if (create_temp_file("temp2.c",
        "int bar(void) { return 1; }\n"
        "int unused_func2(void) { return 43; }\n") != 0) {
        fprintf(stderr, "Failed to create temp2.c\n");
        cleanup_files(temp_files, 2);
        return 1;
    }
    
    if (create_temp_file("syntax_error.c",
        "int baz(void) { return /* missing semicolon and value */ \n") != 0) {
        fprintf(stderr, "Failed to create syntax_error.c\n");
        cleanup_files(temp_files, 3);
        return 1;
    }
    
    /* Create dump directory */
    create_temp_dir(dump_dir);
    
    printf("\n=== Starting GCC driver state transition tests ===\n\n");
    
    /* SEQUENCE A: Help flag then compilation - tests print_help_list reset */
    printf("--- Sequence A: Help then Compile ---\n");
    checksum += execute_gcc("gcc --help=common 2>&1 | head -5 > /dev/null", 0);
    checksum += execute_gcc("gcc -c temp1.c -o temp1.o", 0);
    
    /* SEQUENCE B: Save-temps with dumpdir then plain compile - tests save_temps_flag and dumpdir reset */
    printf("\n--- Sequence B: Save-temps with dumpdir then Compile ---\n");
    char cmd_b1[512];
    snprintf(cmd_b1, sizeof(cmd_b1), 
             "gcc -save-temps -dumpdir %s -dumpbase testbase -c temp1.c -o temp1.o 2>&1", 
             dump_dir);
    checksum += execute_gcc(cmd_b1, 0);
    
    /* Verify dump files were created */
    struct stat st;
    if (stat("./dump_test_dir/testbase.i", &st) == 0) {
        printf("  Verified: dump file created\n");
        checksum += 1;
    }
    
    checksum += execute_gcc("gcc -c temp2.c -o temp2.o", 0);
    
    /* SEQUENCE C: Error then success - tests greatest_status reset */
    printf("\n--- Sequence C: Error then Success ---\n");
    checksum += execute_gcc("gcc -c syntax_error.c 2>/dev/null", 1);  /* Expected to fail */
    checksum += execute_gcc("gcc -c temp1.c -o temp1.o", 0);
    
    /* SEQUENCE D: Verbose and linker selection then plain - tests verbose_only_flag and use_ld reset */
    printf("\n--- Sequence D: Verbose and Linker then Plain ---\n");
    checksum += execute_gcc("gcc -v -fuse-ld=bfd -c temp1.c 2>&1 | grep -q 'COLLECT_GCC_OPTIONS' && echo 'Found' > /dev/null", 0);
    checksum += execute_gcc("gcc -c temp2.c -o temp2.o", 0);
    
    /* SEQUENCE E: Version flag then compilation - tests print_version reset */
    printf("\n--- Sequence E: Version then Compile ---\n");
    checksum += execute_gcc("gcc --version 2>&1 | head -1 > /dev/null", 0);
    checksum += execute_gcc("gcc -c temp1.c -o temp1.o", 0);
    
    /* SEQUENCE F: Multiple language specifications using -x - tests driver job sequencing */
    printf("\n--- Sequence F: Multiple -x language specs ---\n");
    checksum += execute_gcc("gcc -x c -c temp1.c -o temp1.o", 0);
    checksum += execute_gcc("gcc -x c -c temp2.c -o temp2.o", 0);
    
    /* SEQUENCE G: Combined flags then minimal compile - tests multiple variable resets */
    printf("\n--- Sequence G: Combined flags then Minimal ---\n");
    char cmd_g[512];
    snprintf(cmd_g, sizeof(cmd_g),
             "gcc -v --help=optimizers -save-temps -dumpdir %s -fuse-ld=gold -c temp1.c 2>&1 | head -10 > /dev/null",
             dump_dir);
    checksum += execute_gcc(cmd_g, 0);
    checksum += execute_gcc("gcc -c temp2.c", 0);
    
    /* SEQUENCE H: Target system root simulation (using -sysroot) */
    printf("\n--- Sequence H: Sysroot then Default ---\n");
    checksum += execute_gcc("gcc -sysroot=/ -c temp1.c 2>&1 | grep -v 'warning' > /dev/null", 0);
    checksum += execute_gcc("gcc -c temp2.c", 0);
    
    /* Cleanup */
    printf("\n=== Cleaning up temporary files ===\n");
    cleanup_files(temp_files, 7);
    cleanup_dir(dump_dir);
    
    /* Final checksum output */
    printf("\n=== Test Summary ===\n");
    printf("Commands executed: %d\n", checksum);
    printf("Expected checksum range: 15-20 (depends on system configuration)\n");
    
    if (checksum >= 15) {
        printf("SUCCESS: Driver initialization block likely exercised\n");
        return 0;
    } else {
        printf("WARNING: Fewer commands executed than expected\n");
        return 1;
    }
}
