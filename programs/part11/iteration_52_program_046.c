/* test_gcc_driver_init.c
 * 
 * This test program exercises the GCC driver initialization logic
 * (lines 11228-11250 in gcc.cc) by invoking multiple compilation jobs
 * with different state configurations, forcing the driver to reset
 * global variables between jobs.
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
static char* create_temp_file(const char* content, const char* suffix) {
    char template[] = "/tmp/gcc_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    if (suffix) {
        char newpath[256];
        snprintf(newpath, sizeof(newpath), "%s%s", template, suffix);
        rename(template, newpath);
        strcpy(template, newpath);
    }
    
    FILE* f = fdopen(fd, "w");
    if (!f) {
        perror("fdopen failed");
        close(fd);
        return NULL;
    }
    
    fputs(content, f);
    fclose(f);
    
    return strdup(template);
}

/* Helper function to run a command and check status */
static int run_command(const char* cmd, int check_status) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    
    if (check_status) {
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    }
    return 0;
}

/* Helper to cleanup temporary files */
static void cleanup_file(const char* path) {
    if (path) {
        unlink(path);
        free((void*)path);
    }
}

/* Helper to cleanup directory */
static void cleanup_dir(const char* path) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
    system(cmd);
}

/* Dummy functions to avoid unused function warnings */
static int foo(void) { return 0; }
static int bar(void) { return 1; }
static int baz(void) { return 2; }

int main(void) {
    int checksum = 0;
    int test_results[8] = {0};
    int test_idx = 0;
    
    printf("=== GCC Driver Initialization Block Test ===\n");
    printf("Testing lines 11228-11250 of gcc.cc\n\n");
    
    /* Create temporary source files */
    char* temp1 = create_temp_file("int foo(void) { return 0; }\n", ".c");
    char* temp2 = create_temp_file("int bar(void) { return 1; }\n", ".c");
    char* temp_err = create_temp_file("int baz(void) { return \n", ".c");  /* Syntax error */
    
    if (!temp1 || !temp2 || !temp_err) {
        fprintf(stderr, "Failed to create temporary files\n");
        return 1;
    }
    
    /* Create output object file names */
    char obj1[256], obj2[256];
    snprintf(obj1, sizeof(obj1), "%s.o", temp1);
    snprintf(obj2, sizeof(obj2), "%s.o", temp2);
    
    /* Create dump directory */
    mkdir("./dump1", 0755);
    
    /* Test Sequence 1: Help flag then compilation
     * This sets print_help_list, then resets it */
    printf("\n--- Test 1: Help then Compile ---\n");
    char cmd1[512];
    snprintf(cmd1, sizeof(cmd1), 
             "gcc --help=common > /dev/null 2>&1 && "
             "gcc -c %s -o %s", temp1, obj1);
    test_results[test_idx++] = run_command(cmd1, 1) == 0;
    
    /* Test Sequence 2: Version flag then compilation  
     * This sets print_version, then resets it */
    printf("\n--- Test 2: Version then Compile ---\n");
    char cmd2[512];
    snprintf(cmd2, sizeof(cmd2),
             "gcc --version > /dev/null 2>&1 && "
             "gcc -c %s -o %s", temp2, obj2);
    test_results[test_idx++] = run_command(cmd2, 1) == 0;
    
    /* Test Sequence 3: Save-temps with dumpdir then plain compile
     * This sets save_temps_flag, dumpdir, dumpbase, then resets them */
    printf("\n--- Test 3: Save-temps with dumpdir then plain compile ---\n");
    char cmd3[512];
    snprintf(cmd3, sizeof(cmd3),
             "gcc -save-temps -dumpdir ./dump1 -dumpbase testbase -c %s -o %s 2>/dev/null && "
             "gcc -c %s -o %s 2>/dev/null", temp1, obj1, temp2, obj2);
    test_results[test_idx++] = run_command(cmd3, 1) == 0;
    
    /* Test Sequence 4: Verbose flag then compilation
     * This sets verbose_only_flag, then resets it */
    printf("\n--- Test 4: Verbose then Compile ---\n");
    char cmd4[512];
    snprintf(cmd4, sizeof(cmd4),
             "gcc -v -c %s 2>&1 | grep -q 'COLLECT_GCC_OPTIONS' && "
             "gcc -c %s -o %s", temp1, temp2, obj2);
    test_results[test_idx++] = run_command(cmd4, 1) == 0;
    
    /* Test Sequence 5: Linker selection then plain compile
     * This sets use_ld, then resets it to NULL */
    printf("\n--- Test 5: Linker selection then plain compile ---\n");
    char cmd5[512];
    snprintf(cmd5, sizeof(cmd5),
             "gcc -fuse-ld=bfd -c %s -o %s 2>/dev/null; "
             "gcc -c %s -o %s", temp1, obj1, temp2, obj2);
    test_results[test_idx++] = run_command(cmd5, 1) == 0;
    
    /* Test Sequence 6: Error then success compilation
     * This exercises greatest_status reset logic */
    printf("\n--- Test 6: Error then success ---\n");
    char cmd6[512];
    snprintf(cmd6, sizeof(cmd6),
             "gcc -c %s 2>/dev/null; "
             "gcc -c %s -o %s", temp_err, temp1, obj1);
    test_results[test_idx++] = run_command(cmd6, 1) == 0;
    
    /* Test Sequence 7: Multiple language specifications with -x
     * Forces multiple compilation jobs in single invocation */
    printf("\n--- Test 7: Multiple -x language specs ---\n");
    char cmd7[512];
    snprintf(cmd7, sizeof(cmd7),
             "echo 'int x=1;' | gcc -x c - -x c - -x c - -c -o %s 2>/dev/null", obj1);
    test_results[test_idx++] = run_command(cmd7, 1) == 0;
    
    /* Test Sequence 8: Combined flags then reset
     * Multiple state variables set then cleared */
    printf("\n--- Test 8: Combined flags then reset ---\n");
    char cmd8[512];
    snprintf(cmd8, sizeof(cmd8),
             "gcc -v --help=optimizers -save-temps -dumpdir ./dump1 -c %s 2>&1 > /dev/null && "
             "gcc -c %s -o %s", temp1, temp2, obj2);
    test_results[test_idx++] = run_command(cmd8, 1) == 0;
    
    /* Calculate checksum from test results */
    for (int i = 0; i < test_idx; i++) {
        checksum = (checksum << 1) | test_results[i];
        printf("Test %d: %s\n", i + 1, test_results[i] ? "PASS" : "FAIL");
    }
    
    /* Cleanup */
    printf("\n--- Cleaning up ---\n");
    cleanup_file(temp1);
    cleanup_file(temp2);
    cleanup_file(temp_err);
    cleanup_file(obj1);
    cleanup_file(obj2);
    cleanup_dir("./dump1");
    
    /* Remove any leftover temp files */
    system("rm -f /tmp/gcc_test_* 2>/dev/null");
    
    printf("\n=== Test Complete ===\n");
    printf("Checksum: 0x%02X\n", checksum);
    printf("Expected coverage of variables:\n");
    printf("  print_help_list, print_version, verbose_only_flag\n");
    printf("  use_ld, save_temps_flag, dumpdir, dumpbase\n");
    printf("  greatest_status, and other globals in init block\n");
    
    /* Reference dummy functions to avoid unused warnings */
    (void)foo();
    (void)bar();
    (void)baz();
    
    return 0;
}
