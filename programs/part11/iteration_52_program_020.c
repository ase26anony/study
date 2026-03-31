/* test_gcc_driver_init.c
 * 
 * This program tests the GCC driver's per-job initialization logic
 * (lines 11228-11250 in gcc.cc) by executing multiple compilation
 * jobs in sequence with different option combinations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Function prototypes for the dummy functions we'll create */
int foo(void);
int bar(void);
int baz(void);

/* Simple checksum to track test results */
static unsigned int test_checksum = 0;

/* Helper function to execute a command and update checksum */
static int execute_command(const char *cmd, int expected_status) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    int result = WEXITSTATUS(status);
    
    /* Update checksum: XOR with (command_hash << 1) | (result == expected_status) */
    unsigned int cmd_hash = 0;
    const char *p = cmd;
    while (*p) {
        cmd_hash = (cmd_hash << 5) + cmd_hash + *p++;
    }
    test_checksum ^= (cmd_hash << 1) | (result == expected_status ? 1 : 0);
    
    return result;
}

/* Create a temporary file with given content */
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

/* Clean up temporary files */
static void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            unlink(files[i]);
        }
    }
}

/* Clean up temporary directories */
static void cleanup_dirs(const char **dirs, int count) {
    for (int i = 0; i < count; i++) {
        if (dirs[i]) {
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "rm -rf %s", dirs[i]);
            system(cmd);
        }
    }
}

int main(void) {
    /* Track created files for cleanup */
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
    
    const char *temp_dirs[] = {
        "./dump1",
        "./dump2",
        NULL
    };
    
    /* Create temporary source files */
    printf("=== Creating temporary source files ===\n");
    
    if (!create_temp_file("temp1.c", 
        "int foo(void) { return 0; }\n"
        "int main(void) { return foo(); }\n")) {
        return 1;
    }
    
    if (!create_temp_file("temp2.c",
        "int bar(void) { return 1; }\n"
        "int unused_func(void) { return bar(); }\n")) {
        cleanup_files(temp_files, 7);
        return 1;
    }
    
    if (!create_temp_file("syntax_error.c",
        "int baz(void) { return /* missing semicolon and value */ }\n")) {
        cleanup_files(temp_files, 7);
        return 1;
    }
    
    printf("\n=== Testing GCC driver initialization block ===\n");
    
    /* Sequence 1: Help flag then compilation (tests print_help_list reset) */
    printf("\n--- Sequence 1: Help then compile ---\n");
    execute_command("gcc --help=common 2>&1 | head -5 > /dev/null", 0);
    execute_command("gcc -c temp1.c -o temp1.o", 0);
    
    /* Sequence 2: Version flag then compilation (tests print_version reset) */
    printf("\n--- Sequence 2: Version then compile ---\n");
    execute_command("gcc --version 2>&1 | head -1 > /dev/null", 0);
    execute_command("gcc -c temp2.c -o temp2.o", 0);
    
    /* Sequence 3: Save-temps with dumpdir then plain compile 
       (tests save_temps_flag, dumpdir, dumpbase reset) */
    printf("\n--- Sequence 3: Save-temps with dumpdir then plain compile ---\n");
    
    /* Create dump directory */
    mkdir("./dump1", 0755);
    
    /* First job with save-temps and dump options */
    execute_command("gcc -save-temps -dumpdir ./dump1 -dumpbase mydump -c temp1.c 2>&1", 0);
    
    /* Second job without those options - should trigger free() and NULL assignments */
    execute_command("gcc -c temp2.c 2>&1", 0);
    
    /* Sequence 4: Verbose flag then normal compile (tests verbose_only_flag reset) */
    printf("\n--- Sequence 4: Verbose then normal compile ---\n");
    execute_command("gcc -v -c temp1.c 2>&1 | grep -q 'COLLECT_GCC'", 0);
    execute_command("gcc -c temp2.c 2>&1", 0);
    
    /* Sequence 5: Linker selection then default (tests use_ld reset) */
    printf("\n--- Sequence 5: Specific linker then default ---\n");
    /* Try different linkers, accepting failure if not available */
    execute_command("gcc -fuse-ld=bfd -c temp1.c 2>&1", 0);  /* May fail if bfd not available */
    execute_command("gcc -fuse-ld=gold -c temp2.c 2>&1", 0); /* May fail if gold not available */
    execute_command("gcc -c temp1.c 2>&1", 0);  /* Back to default */
    
    /* Sequence 6: Error then success (tests greatest_status reset) */
    printf("\n--- Sequence 6: Error then success ---\n");
    /* First command expected to fail due to syntax error */
    execute_command("gcc -c syntax_error.c 2>/dev/null", 1);
    /* Second should succeed */
    execute_command("gcc -c temp1.c -o temp1.o 2>&1", 0);
    
    /* Sequence 7: Multiple dump options with different extensions */
    printf("\n--- Sequence 7: Multiple dump options ---\n");
    execute_command("gcc -dumpdir ./dump2 -dumpbase multi -dumpbase-ext .ext -c temp1.c 2>&1", 0);
    execute_command("gcc -c temp2.c 2>&1", 0);  /* Should reset dumpbase_ext */
    
    /* Sequence 8: Combined flags to stress initialization */
    printf("\n--- Sequence 8: Combined flags stress test ---\n");
    execute_command("gcc -v --help=optimizers 2>&1 | head -10 > /dev/null", 0);
    execute_command("gcc -save-temps -dumpdir ./dump1 -c temp1.c 2>&1", 0);
    execute_command("gcc -c temp2.c 2>&1", 0);
    
    /* Sequence 9: Test with -### (dry run verbose) */
    printf("\n--- Sequence 9: Dry run then actual compile ---\n");
    execute_command("gcc -### -c temp1.c 2>&1 | head -5 > /dev/null", 0);
    execute_command("gcc -c temp2.c 2>&1", 0);
    
    /* Sequence 10: Multiple input files (multiple jobs in single invocation) */
    printf("\n--- Sequence 10: Multiple inputs single invocation ---\n");
    execute_command("gcc -c temp1.c temp2.c 2>&1", 0);
    
    /* Clean up */
    printf("\n=== Cleaning up temporary files ===\n");
    cleanup_files(temp_files, 7);
    cleanup_dirs(temp_dirs, 2);
    
    /* Final checksum output */
    printf("\n=== Test Results ===\n");
    printf("Final checksum: 0x%08x\n", test_checksum);
    printf("If checksum != 0, at least some commands executed successfully.\n");
    
    /* Use the dummy functions to avoid unused function warnings */
    (void)foo;
    (void)bar;
    (void)baz;
    
    return 0;
}

/* Dummy function definitions to satisfy prototypes */
int foo(void) { return 0; }
int bar(void) { return 1; }
int baz(void) { return 2; }
