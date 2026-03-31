/* driver_coverage_test.c
 * Test program to cover GCC driver initialization block (lines 11228-11250 in gcc.cc)
 * This program orchestrates multiple gcc invocations to trigger state resets.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Temporary file management */
static char temp_files[10][256];
static int temp_file_count = 0;

/* Create a temporary C source file with given content */
static char* create_temp_file(const char* content, const char* suffix) {
    if (temp_file_count >= 10) {
        fprintf(stderr, "Too many temporary files\n");
        return NULL;
    }
    
    char template[] = "/tmp/gcc_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    /* Write content */
    size_t len = strlen(content);
    if (write(fd, content, len) != (ssize_t)len) {
        close(fd);
        unlink(template);
        return NULL;
    }
    close(fd);
    
    /* Rename with proper suffix if needed */
    if (suffix) {
        char new_name[256];
        snprintf(new_name, sizeof(new_name), "%s%s", template, suffix);
        if (rename(template, new_name) == 0) {
            strcpy(template, new_name);
        }
    }
    
    strcpy(temp_files[temp_file_count], template);
    return temp_files[temp_file_count++];
}

/* Clean up all temporary files */
static void cleanup_temp_files(void) {
    for (int i = 0; i < temp_file_count; i++) {
        if (temp_files[i][0]) {
            unlink(temp_files[i]);
        }
    }
    temp_file_count = 0;
}

/* Execute a GCC command and check return status */
static int execute_gcc(const char* cmd, int expect_failure) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    
    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        if (expect_failure) {
            if (exit_status == 0) {
                printf("  WARNING: Expected failure but command succeeded\n");
                return 0;
            }
            printf("  Command failed as expected (status: %d)\n", exit_status);
            return 1;
        } else {
            if (exit_status != 0) {
                printf("  ERROR: Command failed (status: %d)\n", exit_status);
                return 0;
            }
            printf("  Command succeeded\n");
            return 1;
        }
    }
    
    printf("  ERROR: Command terminated abnormally\n");
    return 0;
}

/* Create dump directory for save-temps tests */
static int create_dump_dir(const char* dirname) {
    struct stat st;
    if (stat(dirname, &st) == 0) {
        /* Directory exists, clean it */
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", dirname);
        system(cmd);
    }
    return mkdir(dirname, 0755);
}

int main(void) {
    int success_count = 0;
    int total_tests = 0;
    
    printf("=== GCC Driver Initialization Coverage Test ===\n\n");
    
    /* Create temporary source files */
    char* valid1 = create_temp_file("int foo(void) { return 0; }\n", ".c");
    char* valid2 = create_temp_file("int bar(void) { return 1; }\n", ".c");
    char* error_file = create_temp_file("int baz(void) { return \n", ".c"); /* Syntax error */
    
    if (!valid1 || !valid2 || !error_file) {
        fprintf(stderr, "Failed to create temporary files\n");
        cleanup_temp_files();
        return 1;
    }
    
    /* Create object file names */
    char obj1[256], obj2[256];
    snprintf(obj1, sizeof(obj1), "%s.o", valid1);
    snprintf(obj2, sizeof(obj2), "%s.o", valid2);
    
    /* Test Sequence 1: Help/Version flags followed by compilation
     * This should set print_help_list or print_version, then reset them */
    printf("\n--- Test 1: Help flag then compilation ---\n");
    char cmd[1024];
    
    /* First job: print help */
    snprintf(cmd, sizeof(cmd), "gcc --help=common > /dev/null 2>&1");
    success_count += execute_gcc(cmd, 0);
    total_tests++;
    
    /* Second job: compile (should reset help flags) */
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s", valid1, obj1);
    success_count += execute_gcc(cmd, 0);
    total_tests++;
    
    /* Test Sequence 2: Version flag then compilation */
    printf("\n--- Test 2: Version flag then compilation ---\n");
    snprintf(cmd, sizeof(cmd), "gcc --version > /dev/null 2>&1");
    success_count += execute_gcc(cmd, 0);
    total_tests++;
    
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s", valid2, obj2);
    success_count += execute_gcc(cmd, 0);
    total_tests++;
    
    /* Test Sequence 3: Save-temps with dumpdir then plain compilation
     * This exercises save_temps_flag, dumpdir, dumpbase reset logic */
    printf("\n--- Test 3: Save-temps with dumpdir then plain compilation ---\n");
    
    /* Create dump directory */
    if (create_dump_dir("./dump_test") == 0) {
        /* First job: with save-temps and dumpdir */
        snprintf(cmd, sizeof(cmd), 
                 "gcc -save-temps -dumpdir ./dump_test -dumpbase mytest -c %s -o %s.savetemps",
                 valid1, obj1);
        success_count += execute_gcc(cmd, 0);
        total_tests++;
        
        /* Second job: plain compilation (should reset dumpdir/dumpbase) */
        snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s.plain", valid2, obj2);
        success_count += execute_gcc(cmd, 0);
        total_tests++;
        
        /* Clean up dump directory */
        system("rm -rf ./dump_test");
    } else {
        printf("  WARNING: Could not create dump directory, skipping test\n");
    }
    
    /* Test Sequence 4: Verbose flag then normal compilation */
    printf("\n--- Test 4: Verbose flag then normal compilation ---\n");
    snprintf(cmd, sizeof(cmd), "gcc -v -c %s -o %s.verbose 2>&1 | head -5 > /dev/null", 
             valid1, obj1);
    success_count += execute_gcc(cmd, 0);
    total_tests++;
    
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s.normal", valid2, obj2);
    success_count += execute_gcc(cmd, 0);
    total_tests++;
    
    /* Test Sequence 5: Linker selection then default */
    printf("\n--- Test 5: Linker selection then default ---\n");
    /* Try different linkers, but accept failure if not available */
    snprintf(cmd, sizeof(cmd), "gcc -fuse-ld=bfd -c %s -o %s.bfd 2>/dev/null", valid1, obj1);
    int bfd_result = system(cmd);
    if (WIFEXITED(bfd_result) && WEXITSTATUS(bfd_result) == 0) {
        printf("  bfd linker available\n");
        success_count++;
    } else {
        /* Try gold */
        snprintf(cmd, sizeof(cmd), "gcc -fuse-ld=gold -c %s -o %s.gold 2>/dev/null", valid1, obj1);
        int gold_result = system(cmd);
        if (WIFEXITED(gold_result) && WEXITSTATUS(gold_result) == 0) {
            printf("  gold linker available\n");
            success_count++;
        } else {
            printf("  WARNING: No alternative linkers available, skipping\n");
        }
    }
    total_tests++;
    
    /* Follow with default linker */
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s.default", valid2, obj2);
    success_count += execute_gcc(cmd, 0);
    total_tests++;
    
    /* Test Sequence 6: Error recovery (tests greatest_status reset) */
    printf("\n--- Test 6: Error recovery (greatest_status reset) ---\n");
    /* First: compilation that fails */
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o /tmp/bad.o 2>/dev/null", error_file);
    success_count += execute_gcc(cmd, 1); /* Expect failure */
    total_tests++;
    
    /* Second: successful compilation (should reset greatest_status) */
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s.recovered", valid1, obj1);
    success_count += execute_gcc(cmd, 0);
    total_tests++;
    
    /* Test Sequence 7: Multiple inputs in single invocation (multiple jobs) */
    printf("\n--- Test 7: Multiple inputs in single invocation ---\n");
    snprintf(cmd, sizeof(cmd), "gcc -c %s %s -o /tmp/multi.o 2>&1", valid1, valid2);
    /* This should fail because -o with multiple inputs needs different handling */
    success_count += execute_gcc(cmd, 1); /* Expect failure */
    total_tests++;
    
    /* But this should work: compile separately */
    snprintf(cmd, sizeof(cmd), "gcc -c %s && gcc -c %s", valid1, valid2);
    success_count += execute_gcc(cmd, 0);
    total_tests++;
    
    /* Test Sequence 8: Combined flags reset */
    printf("\n--- Test 8: Combined flags then minimal compilation ---\n");
    snprintf(cmd, sizeof(cmd), 
             "gcc -v --help=optimizers -save-temps -dumpbase combined -c %s -o /tmp/combined.o 2>/dev/null",
             valid1);
    success_count += execute_gcc(cmd, 0);
    total_tests++;
    
    /* Minimal compilation to trigger reset */
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o /tmp/minimal.o", valid2);
    success_count += execute_gcc(cmd, 0);
    total_tests++;
    
    /* Test Sequence 9: Using -x to specify language (creates separate jobs) */
    printf("\n--- Test 9: -x language specification ---\n");
    /* Create a C++ file */
    char* cpp_file = create_temp_file("int cpp_func() { return 42; }\n", ".cpp");
    if (cpp_file) {
        /* First compile C */
        snprintf(cmd, sizeof(cmd), "gcc -x c -c %s -o /tmp/c.o", valid1);
        success_count += execute_gcc(cmd, 0);
        total_tests++;
        
        /* Then compile C++ (if supported) */
        snprintf(cmd, sizeof(cmd), "gcc -x c++ -c %s -o /tmp/cpp.o 2>/dev/null", cpp_file);
        int cpp_result = system(cmd);
        if (WIFEXITED(cpp_result) && WEXITSTATUS(cpp_result) == 0) {
            printf("  C++ compilation succeeded\n");
            success_count++;
        } else {
            printf("  WARNING: C++ not supported, skipping\n");
        }
        total_tests++;
    }
    
    /* Cleanup */
    printf("\n--- Cleaning up ---\n");
    cleanup_temp_files();
    system("rm -f /tmp/*.o /tmp/*.i /tmp/*.s /tmp/*.ii 2>/dev/null");
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests executed: %d\n", total_tests);
    printf("Tests passed: %d\n", success_count);
    printf("Success rate: %.1f%%\n", (total_tests > 0) ? (100.0 * success_count / total_tests) : 0.0);
    
    /* Compute checksum for validation */
    unsigned int checksum = (success_count * 100) + total_tests;
    printf("Validation checksum: 0x%08x\n", checksum);
    
    return (success_count == total_tests) ? 0 : 1;
}

/* Unused functions to avoid -Wunused-function warnings */
static void __attribute__((unused)) use_functions(void) {
    /* Reference the functions from temp files to avoid warnings */
    extern int foo(void);
    extern int bar(void);
    (void)foo;
    (void)bar;
}
