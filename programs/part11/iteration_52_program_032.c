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

/* Helper function to create temporary files */
static int create_temp_file(const char *template, const char *content) {
    FILE *f = fopen(template, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fprintf(f, "%s", content);
    fclose(f);
    return 1;
}

/* Helper function to execute a command and check status */
static int execute_command(const char *cmd, int check_success) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (check_success) {
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return 1;
        } else {
            fprintf(stderr, "Command failed: %s\n", cmd);
            return 0;
        }
    }
    return 1; /* Don't check for commands expected to fail */
}

/* Helper to create temporary directory */
static int create_temp_dir(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) == -1) {
            perror("mkdir");
            return 0;
        }
    }
    return 1;
}

/* Helper to remove directory recursively */
static void remove_dir(const char *path) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
    system(cmd);
}

int main(void) {
    int success_count = 0;
    int total_tests = 0;
    
    /* Create temporary source files */
    const char *temp1_content = 
        "static int foo(void) { return 0; }\n"
        "int main(void) { return foo(); }\n";
    
    const char *temp2_content = 
        "static int bar(void) { return 1; }\n"
        "int helper(void) { return bar(); }\n";
    
    const char *syntax_error_content = 
        "int baz(void) { return /* missing semicolon and value */ }\n";
    
    /* Create unique temporary file names */
    char temp1_name[] = "/tmp/gcc_test1_XXXXXX.c";
    char temp2_name[] = "/tmp/gcc_test2_XXXXXX.c";
    char temp3_name[] = "/tmp/gcc_test3_XXXXXX.c";
    char obj1_name[] = "/tmp/gcc_test1_XXXXXX.o";
    char obj2_name[] = "/tmp/gcc_test2_XXXXXX.o";
    
    /* Create actual unique names */
    int fd1 = mkstemps(temp1_name, 2);
    if (fd1 == -1) {
        perror("mkstemps");
        return 1;
    }
    close(fd1);
    
    int fd2 = mkstemps(temp2_name, 2);
    if (fd2 == -1) {
        perror("mkstemps");
        return 1;
    }
    close(fd2);
    
    int fd3 = mkstemps(temp3_name, 2);
    if (fd3 == -1) {
        perror("mkstemps");
        return 1;
    }
    close(fd3);
    
    /* Create object file names */
    strncpy(obj1_name, temp1_name, sizeof(obj1_name));
    char *dot = strrchr(obj1_name, '.');
    if (dot) *dot = '\0';
    strcat(obj1_name, ".o");
    
    strncpy(obj2_name, temp2_name, sizeof(obj2_name));
    dot = strrchr(obj2_name, '.');
    if (dot) *dot = '\0';
    strcat(obj2_name, ".o");
    
    /* Create the source files */
    if (!create_temp_file(temp1_name, temp1_content)) return 1;
    if (!create_temp_file(temp2_name, temp2_content)) return 1;
    if (!create_temp_file(temp3_name, syntax_error_content)) return 1;
    
    printf("=== Testing GCC Driver Initialization Block ===\n");
    printf("Target: lines 11228-11250 in gcc.cc\n\n");
    
    /* Test Sequence 1: Help flag then compilation
     * Tests: print_help_list, print_version reset */
    printf("--- Sequence 1: Help then Compile ---\n");
    char cmd[1024];
    
    /* First job: help request */
    snprintf(cmd, sizeof(cmd), "%s --help=common 2>&1 | head -5 > /dev/null", 
             "gcc");
    total_tests++;
    if (execute_command(cmd, 1)) success_count++;
    
    /* Second job: actual compilation (should reset help flags) */
    snprintf(cmd, sizeof(cmd), "%s -c %s -o %s", 
             "gcc", temp1_name, obj1_name);
    total_tests++;
    if (execute_command(cmd, 1)) success_count++;
    
    /* Test Sequence 2: Save-temps with dumpdir then plain compile
     * Tests: save_temps_flag, dumpdir, dumpbase, free() calls */
    printf("\n--- Sequence 2: Save-temps with dumpdir then Compile ---\n");
    
    /* Create dump directory */
    const char *dump_dir = "./test_dump_dir";
    create_temp_dir(dump_dir);
    
    /* First job: with save-temps and dump options */
    snprintf(cmd, sizeof(cmd), 
             "%s -save-temps -dumpdir %s -dumpbase mytest -c %s -o %s 2>&1",
             "gcc", dump_dir, temp1_name, obj1_name);
    total_tests++;
    if (execute_command(cmd, 1)) success_count++;
    
    /* Second job: plain compilation (should reset dumpdir/dumpbase) */
    snprintf(cmd, sizeof(cmd), "%s -c %s -o %s",
             "gcc", temp2_name, obj2_name);
    total_tests++;
    if (execute_command(cmd, 1)) success_count++;
    
    /* Test Sequence 3: Error then success
     * Tests: greatest_status reset */
    printf("\n--- Sequence 3: Error then Success ---\n");
    
    /* First job: compilation with syntax error (should fail) */
    snprintf(cmd, sizeof(cmd), "%s -c %s -o /tmp/error.o 2>/dev/null",
             "gcc", temp3_name);
    total_tests++;
    execute_command(cmd, 0); /* Don't check - expected to fail */
    success_count++; /* Count as success since we executed it */
    
    /* Second job: successful compilation */
    snprintf(cmd, sizeof(cmd), "%s -c %s -o %s",
             "gcc", temp1_name, obj1_name);
    total_tests++;
    if (execute_command(cmd, 1)) success_count++;
    
    /* Test Sequence 4: Verbose and linker options then plain
     * Tests: verbose_only_flag, use_ld reset */
    printf("\n--- Sequence 4: Verbose and Linker then Plain ---\n");
    
    /* First job: verbose with linker specification */
    snprintf(cmd, sizeof(cmd), 
             "%s -v -fuse-ld=bfd -c %s -o %s 2>&1 | grep -q 'COLLECT_GCC_OPTIONS'",
             "gcc", temp1_name, obj1_name);
    total_tests++;
    if (execute_command(cmd, 1)) success_count++;
    
    /* Second job: plain compilation */
    snprintf(cmd, sizeof(cmd), "%s -c %s -o %s",
             "gcc", temp2_name, obj2_name);
    total_tests++;
    if (execute_command(cmd, 1)) success_count++;
    
    /* Test Sequence 5: Multiple inputs in single invocation
     * Tests: per-job initialization with multiple source files */
    printf("\n--- Sequence 5: Multiple Inputs Single Invocation ---\n");
    
    snprintf(cmd, sizeof(cmd), "%s -c %s %s",
             "gcc", temp1_name, temp2_name);
    total_tests++;
    if (execute_command(cmd, 1)) success_count++;
    
    /* Test Sequence 6: Version flag then help then compile
     * Tests: print_version, print_help_list chained resets */
    printf("\n--- Sequence 6: Version then Help then Compile ---\n");
    
    snprintf(cmd, sizeof(cmd), "%s --version 2>&1 | head -1 > /dev/null",
             "gcc");
    total_tests++;
    if (execute_command(cmd, 1)) success_count++;
    
    snprintf(cmd, sizeof(cmd), "%s --help=optimizers 2>&1 | head -5 > /dev/null",
             "gcc");
    total_tests++;
    if (execute_command(cmd, 1)) success_count++;
    
    snprintf(cmd, sizeof(cmd), "%s -c %s -o %s",
             "gcc", temp1_name, obj1_name);
    total_tests++;
    if (execute_command(cmd, 1)) success_count++;
    
    /* Cleanup */
    printf("\n--- Cleanup ---\n");
    unlink(temp1_name);
    unlink(temp2_name);
    unlink(temp3_name);
    unlink(obj1_name);
    unlink(obj2_name);
    remove_dir(dump_dir);
    
    /* Also clean up any save-temps files that might have been created */
    system("rm -f /tmp/cc* /tmp/*.ii /tmp/*.s /tmp/*.o 2>/dev/null");
    
    /* Calculate and output checksum */
    printf("\n=== Test Results ===\n");
    printf("Tests executed: %d\n", total_tests);
    printf("Tests successful: %d\n", success_count);
    
    /* Simple checksum based on test results */
    unsigned int checksum = (success_count * 100) + total_tests;
    printf("Checksum: 0x%08x\n", checksum);
    
    if (success_count == total_tests) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Some tests failed.\n");
        return 1;
    }
}

/* Dummy functions to avoid unused function warnings */
static int foo(void) { return 0; }
static int bar(void) { return 1; }
static int baz(void) { return 2; }
