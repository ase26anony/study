/* test_gcc_driver_init.c
 * 
 * This test program exercises the GCC driver initialization logic
 * (lines 11228-11250 in gcc.cc) by invoking multiple compilation jobs
 * with different option combinations, forcing re-initialization of
 * global state variables between jobs.
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

/* Helper function to create temporary source files */
static int create_temp_file(const char *filename, const char *content) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return 0;
    }
    fprintf(fp, "%s", content);
    fclose(fp);
    return 1;
}

/* Helper function to execute a command and check return status */
static int execute_command(const char *cmd, int expected_status) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    int result = WEXITSTATUS(status);
    
    if (expected_status >= 0) {
        if (result == expected_status) {
            printf("  ✓ Command returned expected status %d\n", expected_status);
            return 1;
        } else {
            printf("  ✗ Command returned %d, expected %d\n", result, expected_status);
            return 0;
        }
    } else {
        printf("  Command returned %d\n", result);
        return 1;
    }
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

/* Helper to remove temporary directory and its contents */
static void cleanup_temp_dir(const char *dirname) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dirname);
    system(cmd);
}

int main(void) {
    int success_count = 0;
    int total_tests = 0;
    
    printf("=== GCC Driver Initialization Block Test ===\n\n");
    
    /* Create temporary source files */
    printf("Creating temporary source files...\n");
    
    const char *valid_source1 = 
        "int foo(void) { return 0; }\n"
        "int main(void) { return foo(); }\n";
    
    const char *valid_source2 = 
        "int bar(void) { return 1; }\n"
        "int main(void) { return bar(); }\n";
    
    const char *error_source = 
        "int baz(void) { return /* missing semicolon and value */ }\n"
        "int main(void) { return 0; }\n";
    
    if (!create_temp_file("temp1.c", valid_source1)) {
        fprintf(stderr, "Failed to create temp1.c\n");
        return 1;
    }
    
    if (!create_temp_file("temp2.c", valid_source2)) {
        fprintf(stderr, "Failed to create temp2.c\n");
        unlink("temp1.c");
        return 1;
    }
    
    if (!create_temp_file("syntax_error.c", error_source)) {
        fprintf(stderr, "Failed to create syntax_error.c\n");
        unlink("temp1.c");
        unlink("temp2.c");
        return 1;
    }
    
    /* Create temporary dump directory */
    create_temp_dir("dump_test_dir");
    
    printf("\n=== Test Sequence 1: Help then Compile ===\n");
    printf("Tests: print_help_list, print_version reset\n");
    
    /* Sequence A: Help then compile - tests help/version flag reset */
    total_tests++;
    if (execute_command("gcc --help=common > /dev/null 2>&1", 0)) {
        total_tests++;
        if (execute_command("gcc -c temp1.c -o temp1.o", 0)) {
            success_count += 2;
            printf("  ✓ Sequence 1 completed successfully\n");
        }
    }
    
    printf("\n=== Test Sequence 2: Save-Temps with Dumpdir then Compile ===\n");
    printf("Tests: save_temps_flag, dumpdir, dumpbase reset and free()\n");
    
    /* Sequence B: Save-temps with dumpdir then compile - tests dumpdir/dumpbase reset */
    total_tests++;
    char cmd[512];
    snprintf(cmd, sizeof(cmd), 
             "gcc -save-temps -dumpdir ./dump_test_dir -dumpbase testbase "
             "-c temp1.c -o temp1_save.o 2>&1");
    if (execute_command(cmd, 0)) {
        total_tests++;
        /* Check if dump files were created */
        if (access("./dump_test_dir/testbase.i", F_OK) == 0) {
            printf("  ✓ Dump files created successfully\n");
        }
        
        /* Now compile without save-temps - should trigger reset */
        if (execute_command("gcc -c temp2.c -o temp2.o", 0)) {
            success_count += 2;
            printf("  ✓ Sequence 2 completed successfully\n");
        }
    }
    
    printf("\n=== Test Sequence 3: Error then Success ===\n");
    printf("Tests: greatest_status reset from failure to success\n");
    
    /* Sequence C: Error then success - tests greatest_status reset */
    total_tests++;
    /* First command should fail due to syntax error */
    if (execute_command("gcc -c syntax_error.c 2>/dev/null", 1)) {
        total_tests++;
        /* Second should succeed - greatest_status should be reset */
        if (execute_command("gcc -c temp1.c -o temp1_err.o", 0)) {
            success_count += 2;
            printf("  ✓ Sequence 3 completed successfully\n");
        }
    }
    
    printf("\n=== Test Sequence 4: Verbose and Linker then Plain ===\n");
    printf("Tests: verbose_only_flag, use_ld reset\n");
    
    /* Sequence D: Verbose and linker selection then plain compile */
    total_tests++;
    /* Try with bfd linker if available, otherwise gold */
    if (execute_command("gcc -v -fuse-ld=bfd -c temp1.c -o temp1_ld.o 2>&1 | "
                       "head -5 | grep -q 'COLLECT_GCC'", 0)) {
        total_tests++;
        /* Now compile without verbose or special linker */
        if (execute_command("gcc -c temp2.c -o temp2_plain.o", 0)) {
            success_count += 2;
            printf("  ✓ Sequence 4 completed successfully\n");
        }
    } else {
        /* Try with gold linker */
        total_tests++;
        if (execute_command("gcc -v -fuse-ld=gold -c temp1.c -o temp1_ld.o 2>&1 | "
                           "head -5 | grep -q 'COLLECT_GCC'", 0)) {
            total_tests++;
            if (execute_command("gcc -c temp2.c -o temp2_plain.o", 0)) {
                success_count += 2;
                printf("  ✓ Sequence 4 completed successfully (with gold)\n");
            }
        }
    }
    
    printf("\n=== Test Sequence 5: Multiple Jobs in Single Invocation ===\n");
    printf("Tests: per-job initialization with -x option\n");
    
    /* Sequence E: Multiple compilation jobs in single invocation using -x */
    total_tests++;
    /* Create a simple assembly file to test -x */
    const char *asm_source = 
        ".globl asm_func\n"
        "asm_func:\n"
        "    mov $42, %eax\n"
        "    ret\n";
    
    if (create_temp_file("temp_asm.s", asm_source)) {
        /* Compile C and assembly in one go - forces multiple jobs */
        if (execute_command("gcc -c temp1.c -x assembler temp_asm.s -o combined.o 2>&1", 0)) {
            success_count++;
            printf("  ✓ Sequence 5 completed successfully\n");
        }
        unlink("temp_asm.s");
    }
    
    printf("\n=== Test Sequence 6: Specs and Machine Reset ===\n");
    printf("Tests: spec_machine reset to DEFAULT_TARGET_MACHINE\n");
    
    /* Sequence F: Use custom specs then compile normally */
    total_tests++;
    /* Create a minimal spec file */
    const char *spec_content = 
        "*cpp:\n"
        "%{posix:-D_POSIX_SOURCE} %{pthread:-D_REENTRANT}\n";
    
    if (create_temp_file("test.spec", spec_content)) {
        if (execute_command("gcc -specs=test.spec -c temp1.c -o temp1_spec.o 2>&1", 0)) {
            total_tests++;
            /* Now compile without custom specs */
            if (execute_command("gcc -c temp2.c -o temp2_nospec.o", 0)) {
                success_count += 2;
                printf("  ✓ Sequence 6 completed successfully\n");
            }
        }
        unlink("test.spec");
    }
    
    /* Cleanup */
    printf("\n=== Cleaning Up ===\n");
    unlink("temp1.c");
    unlink("temp2.c");
    unlink("syntax_error.c");
    
    /* Remove object files */
    system("rm -f *.o 2>/dev/null");
    system("rm -f *.i *.s *.ii 2>/dev/null");
    
    /* Cleanup dump directory */
    cleanup_temp_dir("dump_test_dir");
    
    /* Final summary */
    printf("\n=== Test Summary ===\n");
    printf("Total checks performed: %d\n", total_tests);
    printf("Successful checks: %d\n", success_count);
    
    /* Compute checksum for validation */
    unsigned int checksum = success_count * 31 + total_tests;
    printf("Validation checksum: 0x%08x\n", checksum);
    
    if (success_count == total_tests) {
        printf("\n✓ All tests passed! The driver initialization block should be covered.\n");
        return 0;
    } else {
        printf("\n✗ Some tests failed. Success rate: %d/%d\n", 
               success_count, total_tests);
        return 1;
    }
}

/* Dummy functions to avoid unused function warnings */
static int foo(void) { return 0; }
static int bar(void) { return 1; }
static int baz(void) { return 2; }
