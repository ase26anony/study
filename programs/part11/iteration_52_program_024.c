/* test_gcc_driver_init.c
 * This program tests the GCC driver's per-job initialization logic
 * by executing multiple compilation jobs with varying state flags.
 * It specifically targets the reset of global variables in lines 11228-11250
 * of gcc.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Function prototypes for the dummy functions we'll create */
static int foo(void) { return 0; }
static int bar(void) { return 1; }
/* baz will be in a separate file with syntax error */

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
    
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    
    return strdup(template);
}

/* Helper to execute command and check status */
static int execute_and_check(const char* cmd, int expected_status) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    int result = WEXITSTATUS(status);
    
    if (expected_status >= 0 && result != expected_status) {
        printf("  Warning: Expected status %d, got %d\n", expected_status, result);
    }
    
    return result;
}

/* Helper to clean up files */
static void cleanup_file(const char* filename) {
    if (filename) {
        unlink(filename);
        free((void*)filename);
    }
}

int main(void) {
    int checksum = 0;
    int status;
    
    printf("=== Testing GCC Driver Initialization Block (lines 11228-11250) ===\n\n");
    
    /* Create temporary source files */
    char* temp1_c = create_temp_file(
        "int foo(void) { return 0; }\n"
        "int unused1(void) { return foo(); }\n",
        ".c");
    
    char* temp2_c = create_temp_file(
        "int bar(void) { return 1; }\n"
        "int unused2(void) { return bar(); }\n",
        ".c");
    
    char* error_c = create_temp_file(
        "int baz(void) { return  /* deliberate syntax error */ \n",
        ".c");
    
    if (!temp1_c || !temp2_c || !error_c) {
        fprintf(stderr, "Failed to create temporary files\n");
        return 1;
    }
    
    /* Create object file names */
    char temp1_o[256], temp2_o[256];
    snprintf(temp1_o, sizeof(temp1_o), "%s.o", temp1_c);
    snprintf(temp2_o, sizeof(temp2_o), "%s.o", temp2_c);
    
    /* Create dump directory */
    mkdir("./dump_test_dir", 0755);
    
    /* Sequence 1: Help flag then compilation
     * Tests: print_help_list, print_version reset */
    printf("\n--- Sequence 1: Help then Compile ---\n");
    char cmd1[512];
    snprintf(cmd1, sizeof(cmd1), 
             "gcc --help=common 2>&1 | head -5 > /dev/null");
    status = execute_and_check(cmd1, 0);
    checksum = (checksum * 31 + status) & 0xFF;
    
    snprintf(cmd1, sizeof(cmd1), "gcc -c %s -o %s", temp1_c, temp1_o);
    status = execute_and_check(cmd1, 0);
    checksum = (checksum * 31 + status) & 0xFF;
    
    /* Sequence 2: Version flag then different compilation
     * Tests: print_version reset */
    printf("\n--- Sequence 2: Version then Compile ---\n");
    snprintf(cmd1, sizeof(cmd1), "gcc --version 2>&1 | head -2 > /dev/null");
    status = execute_and_check(cmd1, 0);
    checksum = (checksum * 31 + status) & 0xFF;
    
    snprintf(cmd1, sizeof(cmd1), "gcc -c %s -o %s", temp2_c, temp2_o);
    status = execute_and_check(cmd1, 0);
    checksum = (checksum * 31 + status) & 0xFF;
    
    /* Sequence 3: Save-temps with dumpdir then plain compile
     * Tests: save_temps_flag, dumpdir, dumpbase reset and free() */
    printf("\n--- Sequence 3: Save-temps with dumpdir then Plain Compile ---\n");
    snprintf(cmd1, sizeof(cmd1),
             "gcc -save-temps -dumpdir ./dump_test_dir -dumpbase mydump "
             "-c %s -o %s 2>&1 | grep -v '^Note:' | head -5",
             temp1_c, temp1_o);
    status = execute_and_check(cmd1, 0);
    checksum = (checksum * 31 + status) & 0xFF;
    
    /* Clean up temp files from -save-temps */
    char* base = strrchr(temp1_c, '/');
    if (base) {
        char temp_files[3][256];
        snprintf(temp_files[0], sizeof(temp_files[0]), ".%s.i", base + 1);
        snprintf(temp_files[1], sizeof(temp_files[1]), ".%s.s", base + 1);
        snprintf(temp_files[2], sizeof(temp_files[2]), ".%s.o", base + 1);
        
        for (int i = 0; i < 3; i++) {
            unlink(temp_files[i]);
        }
    }
    
    /* Now plain compile to trigger reset */
    snprintf(cmd1, sizeof(cmd1), "gcc -c %s -o %s", temp2_c, temp2_o);
    status = execute_and_check(cmd1, 0);
    checksum = (checksum * 31 + status) & 0xFF;
    
    /* Sequence 4: Verbose flag then compilation
     * Tests: verbose_only_flag reset */
    printf("\n--- Sequence 4: Verbose then Compile ---\n");
    snprintf(cmd1, sizeof(cmd1), "gcc -v -c %s -o %s 2>&1 | "
             "grep -q 'COLLECT_GCC_OPTIONS'", temp1_c, temp1_o);
    status = execute_and_check(cmd1, 0);
    checksum = (checksum * 31 + status) & 0xFF;
    
    snprintf(cmd1, sizeof(cmd1), "gcc -c %s -o %s", temp2_c, temp2_o);
    status = execute_and_check(cmd1, 0);
    checksum = (checksum * 31 + status) & 0xFF;
    
    /* Sequence 5: Linker selection then compilation
     * Tests: use_ld reset */
    printf("\n--- Sequence 5: Linker selection then Compile ---\n");
    snprintf(cmd1, sizeof(cmd1), "gcc -fuse-ld=bfd -c %s -o %s 2>&1 | "
             "grep -v '^Note:'", temp1_c, temp1_o);
    status = execute_and_check(cmd1, 0);
    checksum = (checksum * 31 + status) & 0xFF;
    
    snprintf(cmd1, sizeof(cmd1), "gcc -c %s -o %s", temp2_c, temp2_o);
    status = execute_and_check(cmd1, 0);
    checksum = (checksum * 31 + status) & 0xFF;
    
    /* Sequence 6: Error then success compilation
     * Tests: greatest_status reset to 1 */
    printf("\n--- Sequence 6: Error then Success ---\n");
    snprintf(cmd1, sizeof(cmd1), "gcc -c %s 2>/dev/null", error_c);
    status = execute_and_check(cmd1, 1);  /* Expected to fail */
    checksum = (checksum * 31 + status) & 0xFF;
    
    snprintf(cmd1, sizeof(cmd1), "gcc -c %s -o %s", temp1_c, temp1_o);
    status = execute_and_check(cmd1, 0);
    checksum = (checksum * 31 + status) & 0xFF;
    
    /* Sequence 7: Multiple jobs in single invocation
     * Tests: per-job initialization with multiple inputs */
    printf("\n--- Sequence 7: Multiple Input Files ---\n");
    snprintf(cmd1, sizeof(cmd1), "gcc -c %s %s", temp1_c, temp2_c);
    status = execute_and_check(cmd1, 0);
    checksum = (checksum * 31 + status) & 0xFF;
    
    /* Sequence 8: Combined flags then reset
     * Tests: Multiple variables reset together */
    printf("\n--- Sequence 8: Combined Flags then Reset ---\n");
    snprintf(cmd1, sizeof(cmd1),
             "gcc -v --help=optimizers 2>&1 | head -3 > /dev/null");
    status = execute_and_check(cmd1, 0);
    checksum = (checksum * 31 + status) & 0xFF;
    
    snprintf(cmd1, sizeof(cmd1),
             "gcc -save-temps -dumpdir ./dump_test_dir -fuse-ld=bfd "
             "-c %s 2>&1 | grep -v '^Note:' | head -3",
             temp1_c);
    status = execute_and_check(cmd1, 0);
    checksum = (checksum * 31 + status) & 0xFF;
    
    /* Final plain compilation to ensure all reset */
    snprintf(cmd1, sizeof(cmd1), "gcc -c %s", temp2_c);
    status = execute_and_check(cmd1, 0);
    checksum = (checksum * 31 + status) & 0xFF;
    
    /* Cleanup */
    printf("\n--- Cleanup ---\n");
    cleanup_file(temp1_c);
    cleanup_file(temp2_c);
    cleanup_file(error_c);
    unlink(temp1_o);
    unlink(temp2_o);
    
    /* Clean up dump directory */
    system("rm -rf ./dump_test_dir");
    
    /* Clean up any remaining .i, .s files */
    system("rm -f ./*.i ./*.s ./*.o 2>/dev/null");
    
    /* Use the dummy functions to avoid unused function warnings */
    (void)foo();
    (void)bar();
    
    printf("\n=== Test Complete ===\n");
    printf("Final checksum: 0x%02X\n", checksum);
    printf("(All sequences executed. Check coverage of gcc.cc lines 11228-11250)\n");
    
    return 0;
}
