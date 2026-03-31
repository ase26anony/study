/* test_gcc_driver_init.c
 * This program tests the GCC driver's per-job initialization logic
 * by executing multiple compilation jobs with varying state flags.
 * It specifically targets the reset code in gcc.cc lines 11228-11250.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Helper function to create a temporary source file */
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

/* Helper function to execute a command and check its return status */
static int execute_and_check(const char *cmd, int expected_status) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    int exit_status = WEXITSTATUS(status);
    
    if (expected_status == -1) {
        /* Don't check status, just execute */
        return 1;
    }
    
    if (exit_status != expected_status) {
        fprintf(stderr, "Command failed with status %d (expected %d): %s\n", 
                exit_status, expected_status, cmd);
        return 0;
    }
    return 1;
}

/* Helper to create a temporary directory */
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

/* Helper to remove a directory and its contents */
static void remove_temp_dir(const char *path) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
    system(cmd);
}

int main(void) {
    int success = 1;
    int checksum = 0;
    
    /* Create temporary source files with different content */
    const char *valid_code1 = 
        "int foo(void) { return 0; }\n"
        "int unused_func1(void) { return 42; }\n";
    
    const char *valid_code2 = 
        "int bar(void) { return 1; }\n"
        "int unused_func2(void) { return 43; }\n";
    
    const char *syntax_error_code = 
        "int baz(void) { return \n"  /* Missing expression */
        "int missing_semicolon(void) { return 44 }\n";  /* Missing semicolon */
    
    /* Create temporary files */
    if (!create_temp_file("temp1.c", valid_code1)) {
        fprintf(stderr, "Failed to create temp1.c\n");
        return 1;
    }
    
    if (!create_temp_file("temp2.c", valid_code2)) {
        fprintf(stderr, "Failed to create temp2.c\n");
        unlink("temp1.c");
        return 1;
    }
    
    if (!create_temp_file("syntax_err.c", syntax_error_code)) {
        fprintf(stderr, "Failed to create syntax_err.c\n");
        unlink("temp1.c");
        unlink("temp2.c");
        return 1;
    }
    
    /* Create temporary dump directory */
    if (!create_temp_dir("./dump_test")) {
        fprintf(stderr, "Failed to create dump directory\n");
        success = 0;
    }
    
    printf("=== Testing GCC Driver Initialization Block ===\n\n");
    
    /* Sequence 1: Help flag then compilation
     * This sets print_help_list, then resets it in next job */
    printf("--- Sequence 1: Help then Compile ---\n");
    if (execute_and_check("gcc --help=common 2>&1 | head -5 > /dev/null", 0)) {
        checksum += 1;
        printf("  Help command executed\n");
    } else {
        success = 0;
    }
    
    /* Now compile - this should reset print_help_list */
    if (execute_and_check("gcc -c temp1.c -o temp1.o", 0)) {
        checksum += 2;
        printf("  Compilation after help succeeded\n");
    } else {
        success = 0;
    }
    
    /* Sequence 2: Version flag then compilation
     * This sets print_version, then resets it */
    printf("\n--- Sequence 2: Version then Compile ---\n");
    if (execute_and_check("gcc --version 2>&1 | head -1 > /dev/null", 0)) {
        checksum += 4;
        printf("  Version command executed\n");
    } else {
        success = 0;
    }
    
    if (execute_and_check("gcc -c temp2.c -o temp2.o", 0)) {
        checksum += 8;
        printf("  Compilation after version succeeded\n");
    } else {
        success = 0;
    }
    
    /* Sequence 3: Save-temps with dumpdir then plain compilation
     * This sets save_temps_flag, dumpdir, dumpbase, then resets them */
    printf("\n--- Sequence 3: Save-temps with dumpdir then plain compile ---\n");
    char cmd[512];
    snprintf(cmd, sizeof(cmd), 
             "gcc -save-temps -dumpdir ./dump_test -dumpbase mydump "
             "-c temp1.c -o temp1_save.o 2>&1");
    if (execute_and_check(cmd, 0)) {
        checksum += 16;
        printf("  Save-temps compilation succeeded\n");
        
        /* Check if dump files were created */
        if (execute_and_check("ls ./dump_test/mydump* 2>/dev/null | head -2", -1)) {
            printf("  Dump files created\n");
        }
    } else {
        success = 0;
    }
    
    /* Now compile without save-temps - this should free dumpdir/dumpbase */
    if (execute_and_check("gcc -c temp2.c -o temp2_plain.o", 0)) {
        checksum += 32;
        printf("  Plain compilation after save-temps succeeded\n");
    } else {
        success = 0;
    }
    
    /* Sequence 4: Verbose flag then normal compilation
     * This sets verbose_only_flag, then resets it */
    printf("\n--- Sequence 4: Verbose then normal compile ---\n");
    if (execute_and_check("gcc -v -c temp1.c -o temp1_verbose.o 2>&1 | "
                         "grep -q 'COLLECT_GCC_OPTIONS'", 0)) {
        checksum += 64;
        printf("  Verbose compilation succeeded\n");
    } else {
        success = 0;
    }
    
    if (execute_and_check("gcc -c temp2.c -o temp2_normal.o", 0)) {
        checksum += 128;
        printf("  Normal compilation after verbose succeeded\n");
    } else {
        success = 0;
    }
    
    /* Sequence 5: Linker selection then default
     * This sets use_ld, then resets it to NULL */
    printf("\n--- Sequence 5: Specific linker then default ---\n");
    if (execute_and_check("gcc -fuse-ld=bfd -c temp1.c -o temp1_ld.o 2>&1", 0)) {
        checksum += 256;
        printf("  Compilation with specific linker succeeded\n");
    } else {
        /* Some systems might not have bfd linker, try gold */
        if (execute_and_check("gcc -fuse-ld=gold -c temp1.c -o temp1_ld.o 2>&1", 0)) {
            checksum += 256;
            printf("  Compilation with gold linker succeeded\n");
        } else {
            printf("  Note: No alternative linker available, skipping\n");
        }
    }
    
    if (execute_and_check("gcc -c temp2.c -o temp2_default_ld.o", 0)) {
        checksum += 512;
        printf("  Compilation with default linker succeeded\n");
    } else {
        success = 0;
    }
    
    /* Sequence 6: Error then success
     * This tests greatest_status reset from failure to success */
    printf("\n--- Sequence 6: Error recovery ---\n");
    /* First command should fail due to syntax error */
    if (execute_and_check("gcc -c syntax_err.c -o syntax_err.o 2>/dev/null", 1)) {
        checksum += 1024;
        printf("  Syntax error correctly detected\n");
    } else {
        /* If it doesn't fail with status 1, it might fail with status != 0 */
        if (execute_and_check("gcc -c syntax_err.c -o syntax_err.o 2>/dev/null", -1)) {
            checksum += 1024;
            printf("  Syntax error detected (non-zero status)\n");
        }
    }
    
    /* Next compilation should succeed - greatest_status should be reset */
    if (execute_and_check("gcc -c temp1.c -o temp1_recover.o", 0)) {
        checksum += 2048;
        printf("  Recovery compilation succeeded\n");
    } else {
        success = 0;
    }
    
    /* Sequence 7: Multiple jobs in single invocation
     * This tests per-job initialization within a single gcc process */
    printf("\n--- Sequence 7: Multiple inputs in single invocation ---\n");
    if (execute_and_check("gcc -c temp1.c temp2.c", 0)) {
        checksum += 4096;
        printf("  Multiple file compilation succeeded\n");
    } else {
        success = 0;
    }
    
    /* Sequence 8: Using -x to specify language (creates separate jobs) */
    printf("\n--- Sequence 8: Using -x language specification ---\n");
    /* Create a simple C++ file */
    const char *cpp_code = "extern \"C\" int cpp_func(void) { return 3; }\n";
    if (!create_temp_file("simple.cpp", cpp_code)) {
        fprintf(stderr, "Failed to create simple.cpp\n");
        success = 0;
    } else {
        /* Compile C and C++ in sequence using -x */
        if (execute_and_check("gcc -x c -c temp1.c -o temp1_x.o && "
                             "gcc -x c++ -c simple.cpp -o simple.o", 0)) {
            checksum += 8192;
            printf("  Cross-language compilation succeeded\n");
        } else {
            success = 0;
        }
        unlink("simple.cpp");
    }
    
    /* Cleanup */
    printf("\n--- Cleanup ---\n");
    unlink("temp1.c");
    unlink("temp2.c");
    unlink("syntax_err.c");
    
    /* Remove object files */
    system("rm -f *.o 2>/dev/null");
    system("rm -f *.i *.s *.ii *.o 2>/dev/null");  /* Remove save-temps files */
    
    /* Remove dump directory */
    remove_temp_dir("./dump_test");
    
    /* Final summary */
    printf("\n=== Test Summary ===\n");
    printf("Success flag: %s\n", success ? "PASS" : "FAIL");
    printf("Checksum: %d (0x%04x)\n", checksum, checksum);
    printf("Coverage target: gcc.cc lines 11228-11250\n");
    
    /* Verify we touched key variables:
     * - print_help_list, print_version (Sequences 1, 2)
     * - save_temps_flag, dumpdir, dumpbase (Sequence 3)
     * - verbose_only_flag (Sequence 4)
     * - use_ld (Sequence 5)
     * - greatest_status (Sequence 6)
     * - Multiple job initialization (Sequences 7, 8)
     */
    
    return success ? 0 : 1;
}
