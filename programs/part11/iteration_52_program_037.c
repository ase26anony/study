/* test_gcc_driver_init.c
 * 
 * This test program exercises the driver initialization logic in gcc.cc
 * (lines 11228-11250) by invoking the GCC driver with multiple compilation
 * jobs that set and reset various global state variables.
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

/* Create a temporary file with given content */
static char* create_temp_file(const char* prefix, const char* content) {
    char template[256];
    snprintf(template, sizeof(template), "/tmp/%s_XXXXXX", prefix);
    
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    if (write(fd, content, strlen(content)) != (ssize_t)strlen(content)) {
        perror("write failed");
        close(fd);
        return NULL;
    }
    
    close(fd);
    
    char* filename = strdup(template);
    if (!filename) {
        perror("strdup failed");
        return NULL;
    }
    
    return filename;
}

/* Execute a GCC command and check return status */
static int execute_gcc_command(const char* cmd, int expected_status) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    
    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        printf("  Exit status: %d (expected: %d)\n", exit_status, expected_status);
        
        /* For help/version commands, GCC returns 0 even though they're not "compilations" */
        if (strstr(cmd, "--help") || strstr(cmd, "--version")) {
            return exit_status == 0 ? 1 : 0;
        }
        
        if (expected_status == -1) {
            /* Don't check specific status */
            return 1;
        }
        
        return exit_status == expected_status ? 1 : 0;
    }
    
    return 0;
}

/* Clean up temporary files */
static void cleanup_files(char** files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            unlink(files[i]);
            free(files[i]);
        }
    }
}

int main(void) {
    int success_count = 0;
    int total_tests = 0;
    char* temp_files[10] = {0};
    int file_count = 0;
    
    printf("=== Testing GCC Driver Initialization Block (lines 11228-11250) ===\n\n");
    
    /* Create temporary source files */
    char* valid1 = create_temp_file("test1", "int foo(void) { return 0; }\n");
    char* valid2 = create_temp_file("test2", "int bar(void) { return 1; }\n");
    char* error_file = create_temp_file("error", "int baz(void) { return \n");  /* Syntax error */
    
    if (!valid1 || !valid2 || !error_file) {
        fprintf(stderr, "Failed to create temporary files\n");
        return 1;
    }
    
    temp_files[file_count++] = valid1;
    temp_files[file_count++] = valid2;
    temp_files[file_count++] = error_file;
    
    /* Create output object files */
    char obj1[256], obj2[256];
    snprintf(obj1, sizeof(obj1), "/tmp/test1_%d.o", getpid());
    snprintf(obj2, sizeof(obj2), "/tmp/test2_%d.o", getpid());
    
    /* Create dump directory */
    char dumpdir[256];
    snprintf(dumpdir, sizeof(dumpdir), "/tmp/dumpdir_%d", getpid());
    mkdir(dumpdir, 0755);
    
    /* TEST 1: Help flag then compilation - tests print_help_list reset */
    printf("\n--- Test 1: Help flag followed by compilation ---\n");
    char cmd1[512];
    snprintf(cmd1, sizeof(cmd1), "gcc --help=common 2>&1 | head -5 > /dev/null");
    success_count += execute_gcc_command(cmd1, 0);
    total_tests++;
    
    char cmd2[512];
    snprintf(cmd2, sizeof(cmd2), "gcc -c %s -o %s", valid1, obj1);
    success_count += execute_gcc_command(cmd2, 0);
    total_tests++;
    
    /* TEST 2: Version flag then compilation - tests print_version reset */
    printf("\n--- Test 2: Version flag followed by compilation ---\n");
    char cmd3[512];
    snprintf(cmd3, sizeof(cmd3), "gcc --version 2>&1 | head -1 > /dev/null");
    success_count += execute_gcc_command(cmd3, 0);
    total_tests++;
    
    char cmd4[512];
    snprintf(cmd4, sizeof(cmd4), "gcc -c %s -o %s", valid2, obj2);
    success_count += execute_gcc_command(cmd4, 0);
    total_tests++;
    
    /* TEST 3: Save-temps with dumpdir then plain compilation - tests save_temps_flag, dumpdir reset */
    printf("\n--- Test 3: Save-temps with dumpdir followed by plain compilation ---\n");
    char cmd5[512];
    snprintf(cmd5, sizeof(cmd5), "gcc -save-temps -dumpdir %s -dumpbase testdump -c %s -o %s 2>&1", 
             dumpdir, valid1, obj1);
    success_count += execute_gcc_command(cmd5, 0);
    total_tests++;
    
    /* Check if dump files were created */
    char dumpfile[512];
    snprintf(dumpfile, sizeof(dumpfile), "%s/testdump.i", dumpdir);
    struct stat st;
    int dump_created = stat(dumpfile, &st) == 0;
    printf("  Dump file created: %s\n", dump_created ? "YES" : "NO");
    
    /* Now compile without save-temps */
    char cmd6[512];
    snprintf(cmd6, sizeof(cmd6), "gcc -c %s -o %s", valid2, obj2);
    success_count += execute_gcc_command(cmd6, 0);
    total_tests++;
    
    /* TEST 4: Verbose flag then normal compilation - tests verbose_only_flag reset */
    printf("\n--- Test 4: Verbose flag followed by normal compilation ---\n");
    char cmd7[512];
    snprintf(cmd7, sizeof(cmd7), "gcc -v -c %s -o %s 2>&1 | grep -q 'COLLECT_GCC_OPTIONS'", valid1, obj1);
    success_count += execute_gcc_command(cmd7, 0);
    total_tests++;
    
    char cmd8[512];
    snprintf(cmd8, sizeof(cmd8), "gcc -c %s -o %s", valid2, obj2);
    success_count += execute_gcc_command(cmd8, 0);
    total_tests++;
    
    /* TEST 5: Linker selection then default - tests use_ld reset */
    printf("\n--- Test 5: Specific linker followed by default ---\n");
    char cmd9[512];
    snprintf(cmd9, sizeof(cmd9), "gcc -fuse-ld=bfd -c %s -o %s 2>&1", valid1, obj1);
    success_count += execute_gcc_command(cmd9, 0);
    total_tests++;
    
    char cmd10[512];
    snprintf(cmd10, sizeof(cmd10), "gcc -c %s -o %s", valid2, obj2);
    success_count += execute_gcc_command(cmd10, 0);
    total_tests++;
    
    /* TEST 6: Error then success - tests greatest_status reset */
    printf("\n--- Test 6: Failed compilation followed by successful one ---\n");
    char cmd11[512];
    snprintf(cmd11, sizeof(cmd11), "gcc -c %s -o /tmp/error.o 2>/dev/null", error_file);
    success_count += execute_gcc_command(cmd11, 1);  /* Should fail */
    total_tests++;
    
    char cmd12[512];
    snprintf(cmd12, sizeof(cmd12), "gcc -c %s -o %s", valid1, obj1);
    success_count += execute_gcc_command(cmd12, 0);
    total_tests++;
    
    /* TEST 7: Multiple source files in one invocation - tests per-job initialization */
    printf("\n--- Test 7: Multiple source files (multiple jobs in one invocation) ---\n");
    char cmd13[512];
    snprintf(cmd13, sizeof(cmd13), "gcc -c %s %s -o /tmp/multi.o", valid1, valid2);
    success_count += execute_gcc_command(cmd13, 0);
    total_tests++;
    
    /* TEST 8: Combined flags then minimal compilation */
    printf("\n--- Test 8: Combined flags then minimal compilation ---\n");
    char cmd14[512];
    snprintf(cmd14, sizeof(cmd14), "gcc -v --help=optimizers -save-temps -dumpdir %s 2>&1 | head -10 > /dev/null", dumpdir);
    success_count += execute_gcc_command(cmd14, 0);
    total_tests++;
    
    char cmd15[512];
    snprintf(cmd15, sizeof(cmd15), "gcc -c %s", valid1);
    success_count += execute_gcc_command(cmd15, 0);
    total_tests++;
    
    /* Cleanup */
    printf("\n--- Cleanup ---\n");
    unlink(obj1);
    unlink(obj2);
    unlink("/tmp/multi.o");
    unlink("/tmp/error.o");
    
    /* Remove dump directory contents */
    char cleanup_cmd[512];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s/* 2>/dev/null", dumpdir);
    system(cleanup_cmd);
    rmdir(dumpdir);
    
    cleanup_files(temp_files, file_count);
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests executed: %d\n", total_tests);
    printf("Tests successful: %d\n", success_count);
    
    /* Calculate checksum for validation */
    unsigned int checksum = 0;
    for (int i = 0; i < total_tests; i++) {
        checksum = (checksum * 31 + (success_count > i ? 1 : 0)) & 0xFFFF;
    }
    
    printf("Validation checksum: 0x%04X\n", checksum);
    
    /* Reference functions to avoid unused warnings */
    (void)foo();
    (void)bar();
    
    return success_count == total_tests ? 0 : 1;
}
