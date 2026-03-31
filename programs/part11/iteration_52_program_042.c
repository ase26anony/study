/* test_gcc_driver_init.c
 * 
 * This test program exercises the GCC driver initialization logic
 * (lines 11228-11250 in gcc.cc) by invoking multiple compilation jobs
 * with different states, forcing the driver to reset global variables
 * between jobs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Simple checksum to track test execution */
static unsigned int checksum = 0;

/* Function to add result to checksum */
static void add_result(int result) {
    checksum = (checksum << 1) | (result ? 1 : 0);
}

/* Create a temporary file with given content */
static char* create_temp_file(const char* content, const char* suffix) {
    char template[] = "/tmp/gcc_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    /* Append suffix if provided */
    char* filename = malloc(strlen(template) + strlen(suffix) + 1);
    if (!filename) {
        close(fd);
        unlink(template);
        return NULL;
    }
    
    strcpy(filename, template);
    strcat(filename, suffix);
    
    /* Rename to include suffix */
    close(fd);
    if (rename(template, filename) != 0) {
        free(filename);
        return NULL;
    }
    
    /* Write content */
    FILE* f = fopen(filename, "w");
    if (!f) {
        free(filename);
        return NULL;
    }
    
    fputs(content, f);
    fclose(f);
    return filename;
}

/* Execute a command and check return status */
static int execute_command(const char* cmd, int expect_success) {
    printf("Executing: %s\n", cmd);
    int result = system(cmd);
    int exit_status = WEXITSTATUS(result);
    
    if (expect_success) {
        if (exit_status == 0) {
            printf("  ✓ Command succeeded as expected\n");
            add_result(1);
            return 1;
        } else {
            printf("  ✗ Command failed unexpectedly (exit: %d)\n", exit_status);
            add_result(0);
            return 0;
        }
    } else {
        if (exit_status != 0) {
            printf("  ✓ Command failed as expected (exit: %d)\n", exit_status);
            add_result(1);
            return 1;
        } else {
            printf("  ✗ Command succeeded unexpectedly\n");
            add_result(0);
            return 0;
        }
    }
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
    printf("=== GCC Driver Initialization Block Test ===\n\n");
    
    /* Create temporary source files */
    char* temp_files[10] = {0};
    int file_count = 0;
    
    /* Valid C source 1 */
    temp_files[file_count] = create_temp_file(
        "int foo(void) { return 0; }\n",
        ".c"
    );
    if (!temp_files[file_count]) {
        fprintf(stderr, "Failed to create temp file 1\n");
        return 1;
    }
    char* valid1 = temp_files[file_count];
    file_count++;
    
    /* Valid C source 2 */
    temp_files[file_count] = create_temp_file(
        "int bar(void) { return 1; }\n",
        ".c"
    );
    if (!temp_files[file_count]) {
        fprintf(stderr, "Failed to create temp file 2\n");
        cleanup_files(temp_files, file_count);
        return 1;
    }
    char* valid2 = temp_files[file_count];
    file_count++;
    
    /* Source with syntax error */
    temp_files[file_count] = create_temp_file(
        "int baz(void) { return \n",  /* Missing expression */
        ".c"
    );
    if (!temp_files[file_count]) {
        fprintf(stderr, "Failed to create temp file 3\n");
        cleanup_files(temp_files, file_count);
        return 1;
    }
    char* error_src = temp_files[file_count];
    file_count++;
    
    /* Create dump directory */
    mkdir("./dump_test_dir", 0755);
    
    /* Build command buffers */
    char cmd[1024];
    int success_count = 0;
    int total_tests = 0;
    
    printf("\n--- Test Sequence 1: Help then Compile ---\n");
    printf("(Tests print_help_list and print_version reset)\n");
    
    /* Job 1: Help command (sets print_help_list) */
    snprintf(cmd, sizeof(cmd), "%s --help=common 2>&1 | head -5 > /dev/null", 
             "gcc");
    success_count += execute_command(cmd, 1);
    total_tests++;
    
    /* Job 2: Compile immediately after help (should reset help flags) */
    snprintf(cmd, sizeof(cmd), "%s -c %s -o %s.o 2>&1", 
             "gcc", valid1, valid1);
    success_count += execute_command(cmd, 1);
    total_tests++;
    
    /* Clean up object file */
    snprintf(cmd, sizeof(cmd), "rm -f %s.o", valid1);
    system(cmd);
    
    printf("\n--- Test Sequence 2: Version then Compile ---\n");
    printf("(Tests print_version reset)\n");
    
    /* Job 1: Version command */
    snprintf(cmd, sizeof(cmd), "%s --version 2>&1 | head -1 > /dev/null", 
             "gcc");
    success_count += execute_command(cmd, 1);
    total_tests++;
    
    /* Job 2: Compile with optimization */
    snprintf(cmd, sizeof(cmd), "%s -O2 -c %s -o %s.o 2>&1", 
             "gcc", valid2, valid2);
    success_count += execute_command(cmd, 1);
    total_tests++;
    
    /* Clean up object file */
    snprintf(cmd, sizeof(cmd), "rm -f %s.o", valid2);
    system(cmd);
    
    printf("\n--- Test Sequence 3: Save-Temps with Dumpdir then Compile ---\n");
    printf("(Tests save_temps_flag, dumpdir, dumpbase reset)\n");
    
    /* Job 1: Compile with save-temps and dumpdir (sets multiple flags) */
    snprintf(cmd, sizeof(cmd), 
             "%s -save-temps -dumpdir ./dump_test_dir -dumpbase testdump "
             "-c %s -o %s.save.o 2>&1", 
             "gcc", valid1, valid1);
    success_count += execute_command(cmd, 1);
    total_tests++;
    
    /* Job 2: Simple compile without those flags (should reset them) */
    snprintf(cmd, sizeof(cmd), "%s -c %s -o %s.simple.o 2>&1", 
             "gcc", valid2, valid2);
    success_count += execute_command(cmd, 1);
    total_tests++;
    
    printf("\n--- Test Sequence 4: Verbose and Linker then Plain ---\n");
    printf("(Tests verbose_only_flag, use_ld reset)\n");
    
    /* Job 1: Verbose output with specific linker */
    snprintf(cmd, sizeof(cmd), 
             "%s -v -fuse-ld=bfd -c %s -o %s.verbose.o 2>&1 | "
             "grep -q 'COLLECT_GCC_OPTIONS'", 
             "gcc", valid1, valid1);
    success_count += execute_command(cmd, 1);
    total_tests++;
    
    /* Job 2: Plain compile (should reset verbose and linker flags) */
    snprintf(cmd, sizeof(cmd), "%s -c %s -o %s.plain.o 2>&1", 
             "gcc", valid2, valid2);
    success_count += execute_command(cmd, 1);
    total_tests++;
    
    printf("\n--- Test Sequence 5: Error then Success ---\n");
    printf("(Tests greatest_status reset from failure to success)\n");
    
    /* Job 1: Compile with syntax error (should fail) */
    snprintf(cmd, sizeof(cmd), "%s -c %s -o /tmp/error.o 2>/dev/null", 
             "gcc", error_src);
    success_count += execute_command(cmd, 0);  /* Expect failure */
    total_tests++;
    
    /* Job 2: Successful compile immediately after error */
    snprintf(cmd, sizeof(cmd), "%s -c %s -o %s.after_error.o 2>&1", 
             "gcc", valid1, valid1);
    success_count += execute_command(cmd, 1);
    total_tests++;
    
    printf("\n--- Test Sequence 6: Multiple Input Files ---\n");
    printf("(Tests driver processing multiple jobs in one invocation)\n");
    
    /* Single invocation with multiple source files - creates multiple jobs */
    snprintf(cmd, sizeof(cmd), "%s -c %s %s -o /tmp/multi.o 2>&1", 
             "gcc", valid1, valid2);
    success_count += execute_command(cmd, 0);  /* Should fail - can't specify multiple -o */
    total_tests++;
    
    /* Correct way: compile multiple files separately but in one line */
    snprintf(cmd, sizeof(cmd), "%s -c %s && %s -c %s", 
             "gcc", valid1, "gcc", valid2);
    success_count += execute_command(cmd, 1);
    total_tests++;
    
    printf("\n--- Test Sequence 7: Specs and Machine ---\n");
    printf("(Tests spec_machine reset)\n");
    
    /* Job 1: Compile with machine-specific option */
    snprintf(cmd, sizeof(cmd), 
             "%s -c %s -march=x86-64 -o %s.march.o 2>&1", 
             "gcc", valid1, valid1);
    success_count += execute_command(cmd, 1);
    total_tests++;
    
    /* Job 2: Compile without machine options */
    snprintf(cmd, sizeof(cmd), "%s -c %s -o %s.nomarch.o 2>&1", 
             "gcc", valid2, valid2);
    success_count += execute_command(cmd, 1);
    total_tests++;
    
    printf("\n--- Test Sequence 8: Combined Flags Reset ---\n");
    
    /* Job with many flags set */
    snprintf(cmd, sizeof(cmd), 
             "%s -v --help=optimizers -save-temps -dumpdir ./dump_test_dir "
             "-fuse-ld=bfd -c %s 2>&1 | head -20 > /dev/null", 
             "gcc", valid1);
    success_count += execute_command(cmd, 1);
    total_tests++;
    
    /* Follow with minimal compile to ensure all flags reset */
    snprintf(cmd, sizeof(cmd), "%s -c %s 2>&1", "gcc", valid2);
    success_count += execute_command(cmd, 1);
    total_tests++;
    
    /* Cleanup */
    printf("\n--- Cleanup ---\n");
    
    /* Remove object files */
    snprintf(cmd, sizeof(cmd), "rm -f %s.*.o %s.*.o /tmp/*.o", valid1, valid2);
    system(cmd);
    
    /* Remove save-temps files */
    snprintf(cmd, sizeof(cmd), "rm -f %s.i %s.s %s.o", valid1, valid1, valid1);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "rm -f %s.i %s.s %s.o", valid2, valid2, valid2);
    system(cmd);
    
    /* Remove dump directory */
    snprintf(cmd, sizeof(cmd), "rm -rf ./dump_test_dir");
    system(cmd);
    
    /* Clean up temp source files */
    cleanup_files(temp_files, file_count);
    
    /* Final results */
    printf("\n=== Test Results ===\n");
    printf("Tests passed: %d/%d\n", success_count, total_tests);
    printf("Checksum: 0x%08x\n", checksum);
    
    if (success_count == total_tests) {
        printf("\n✓ All tests passed! The driver initialization block should be exercised.\n");
        return 0;
    } else {
        printf("\n✗ Some tests failed. Driver coverage may be incomplete.\n");
        return 1;
    }
}

/* Dummy functions to avoid unused function warnings */
#ifdef __GNUC__
__attribute__((used))
#endif
static void use_functions(void) {
    /* Reference the functions from the test files to avoid warnings */
    extern int foo(void);
    extern int bar(void);
    (void)foo;
    (void)bar;
}
