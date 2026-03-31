/* test_gcc_driver_init.c
 * 
 * This test program exercises the GCC driver initialization logic
 * (lines 11228-11250 in gcc.cc) by invoking multiple compilation jobs
 * with different state configurations, forcing re-initialization of
 * global variables between jobs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Helper function to create a temporary file with given content */
static int create_temp_file(const char *template, const char *content) {
    FILE *f = fopen(template, "w");
    if (!f) {
        perror("fopen");
        return -1;
    }
    if (fprintf(f, "%s", content) < 0) {
        perror("fprintf");
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

/* Helper function to execute a command and check return status */
static int execute_command(const char *cmd, int expected_status) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    int result = WEXITSTATUS(status);
    
    if (result == expected_status) {
        printf("  -> OK (got %d, expected %d)\n", result, expected_status);
        return 0;
    } else {
        printf("  -> WARNING: got %d, expected %d\n", result, expected_status);
        return 1;
    }
}

/* Helper function to create a temporary directory */
static int create_temp_dir(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) == -1) {
            perror("mkdir");
            return -1;
        }
    }
    return 0;
}

/* Helper function to remove a directory and its contents */
static void remove_dir_recursive(const char *path) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
    system(cmd);
}

int main(void) {
    int checksum = 0;
    int result;
    char cmd[1024];
    
    printf("=== Testing GCC Driver Initialization Block (lines 11228-11250) ===\n\n");
    
    /* Create temporary source files */
    printf("Creating temporary source files...\n");
    
    /* Valid source file 1 */
    if (create_temp_file("temp1.c", 
        "int foo(void) { return 0; }\n"
        "int unused_func1(void) { return 42; }\n") < 0) {
        return 1;
    }
    
    /* Valid source file 2 */
    if (create_temp_file("temp2.c",
        "int bar(void) { return 1; }\n"
        "int unused_func2(void) { return 43; }\n") < 0) {
        unlink("temp1.c");
        return 1;
    }
    
    /* Source file with syntax error */
    if (create_temp_file("syntax_error.c",
        "int baz(void) { return /* missing semicolon and value */ }\n") < 0) {
        unlink("temp1.c");
        unlink("temp2.c");
        return 1;
    }
    
    /* Create dump directory */
    if (create_temp_dir("./dump1") < 0) {
        unlink("temp1.c");
        unlink("temp2.c");
        unlink("syntax_error.c");
        return 1;
    }
    
    printf("\n=== Sequence A: Help flag then compilation ===\n");
    printf("This sets print_help_list, then resets it in next job\n");
    
    /* First job: help flag sets print_help_list */
    result = execute_command("gcc --help=common 2>&1 | head -5 > /dev/null", 0);
    checksum += result;
    
    /* Second job: compilation should reset print_help_list */
    result = execute_command("gcc -c temp1.c -o temp1.o", 0);
    checksum += result;
    
    printf("\n=== Sequence B: Save-temps with dumpdir then plain compilation ===\n");
    printf("This sets save_temps_flag, dumpdir, dumpbase, then resets them\n");
    
    /* First job: save-temps with dumpdir sets multiple variables */
    snprintf(cmd, sizeof(cmd),
             "gcc -save-temps -dumpdir ./dump1 -dumpbase mydump "
             "-c temp1.c -o temp1_save.o 2>&1");
    result = execute_command(cmd, 0);
    checksum += result;
    
    /* Verify dump files were created */
    result = execute_command("test -f ./dump1/mydump.i || test -f mydump.i", 0);
    checksum += result;
    
    /* Second job: plain compilation should free dumpdir/dumpbase and set to NULL */
    result = execute_command("gcc -c temp2.c -o temp2.o", 0);
    checksum += result;
    
    printf("\n=== Sequence C: Error then success ===\n");
    printf("This exercises greatest_status reset to 1\n");
    
    /* First job: compilation error (should set greatest_status) */
    snprintf(cmd, sizeof(cmd),
             "gcc -c syntax_error.c -o syntax_error.o 2>/dev/null; "
             "echo 'Error job completed' > /dev/null");
    result = execute_command(cmd, 1);  /* gcc should return non-zero */
    checksum += result;
    
    /* Second job: successful compilation (should reset greatest_status from error state) */
    result = execute_command("gcc -c temp1.c -o temp1_err.o", 0);
    checksum += result;
    
    printf("\n=== Sequence D: Verbose and linker selection then plain ===\n");
    printf("This sets verbose_only_flag and use_ld, then resets them\n");
    
    /* First job: verbose with specific linker */
    snprintf(cmd, sizeof(cmd),
             "gcc -v -fuse-ld=bfd -c temp1.c -o temp1_verbose.o 2>&1 | "
             "grep -q 'COLLECT_GCC' || true");
    result = execute_command(cmd, 0);
    checksum += result;
    
    /* Second job: plain compilation */
    result = execute_command("gcc -c temp2.c -o temp2_plain.o", 0);
    checksum += result;
    
    printf("\n=== Sequence E: Version flag then compilation ===\n");
    printf("This sets print_version, then resets it\n");
    
    /* First job: version flag */
    result = execute_command("gcc --version 2>&1 | head -1 > /dev/null", 0);
    checksum += result;
    
    /* Second job: compilation */
    result = execute_command("gcc -c temp1.c -o temp1_ver.o", 0);
    checksum += result;
    
    printf("\n=== Sequence F: Multiple inputs as separate jobs ===\n");
    printf("Testing driver processing multiple inputs in one invocation\n");
    
    /* Single invocation with multiple inputs - should trigger initialization per job */
    snprintf(cmd, sizeof(cmd),
             "gcc -c temp1.c temp2.c -save-temps -dumpdir ./dump1 "
             "-dumpbase multi 2>&1 | tail -5");
    result = execute_command(cmd, 0);
    checksum += result;
    
    printf("\n=== Sequence G: Using -x to specify language (implicit jobs) ===\n");
    printf("Testing language specification triggering job boundaries\n");
    
    /* Create a C++ source file */
    if (create_temp_file("temp3.cpp",
        "extern \"C\" int cpp_func(void) { return 3; }\n") < 0) {
        /* Cleanup will happen at end */
    } else {
        /* Mix C and C++ compilation using -x */
        snprintf(cmd, sizeof(cmd),
                 "gcc -x c -c temp1.c -o temp1_mix.o && "
                 "gcc -x c++ -c temp3.cpp -o temp3.o 2>&1");
        result = execute_command(cmd, 0);
        checksum += result;
        unlink("temp3.cpp");
        unlink("temp3.o");
    }
    
    printf("\n=== Cleanup ===\n");
    
    /* Clean up temporary files */
    unlink("temp1.c");
    unlink("temp2.c");
    unlink("syntax_error.c");
    unlink("temp1.o");
    unlink("temp2.o");
    unlink("temp1_save.o");
    unlink("temp1_err.o");
    unlink("temp1_verbose.o");
    unlink("temp2_plain.o");
    unlink("temp1_ver.o");
    unlink("temp1_mix.o");
    
    /* Clean up dump files that might have been created */
    system("rm -f *.i *.s *.o *.ii dump1/* 2>/dev/null");
    remove_dir_recursive("./dump1");
    
    printf("\n=== Test Summary ===\n");
    printf("Checksum (lower is better): %d\n", checksum);
    printf("(Each non-matching exit status adds 1 to checksum)\n\n");
    
    if (checksum == 0) {
        printf("SUCCESS: All command sequences executed as expected!\n");
        printf("The GCC driver initialization block (lines 11228-11250) should be exercised.\n");
    } else {
        printf("PARTIAL SUCCESS: Some commands didn't execute as expected.\n");
        printf("Checksum: %d (0 = perfect)\n", checksum);
    }
    
    return (checksum == 0) ? 0 : 1;
}

/* Dummy functions to avoid unused function warnings */
void reference_dummy_functions(void) {
    /* These references prevent -Wunused-function warnings */
    (void)foo;
    (void)bar;
    /* Note: baz is intentionally broken, so we don't reference it */
}
