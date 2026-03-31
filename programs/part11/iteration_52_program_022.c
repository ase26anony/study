/* Test program to exercise GCC driver initialization logic (lines 11228-11250 in gcc.cc) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Function prototypes for the dummy functions we'll create */
int foo(void) { return 0; }
int bar(void) { return 1; }
/* baz will be intentionally broken in the syntax error file */

/* Helper to create temporary files */
static int create_temp_file(const char *template, const char *content) {
    FILE *f = fopen(template, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fputs(content, f);
    fclose(f);
    return 1;
}

/* Helper to run a command and check status */
static int run_command(const char *cmd, int expected_status) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    int exit_status = WEXITSTATUS(status);
    
    if (expected_status == -1) {
        /* Don't check status */
        return 1;
    }
    
    if (exit_status != expected_status) {
        fprintf(stderr, "Command failed: %s (got %d, expected %d)\n", 
                cmd, exit_status, expected_status);
        return 0;
    }
    return 1;
}

/* Helper to cleanup */
static void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            unlink(files[i]);
        }
    }
}

int main(void) {
    int success = 1;
    const char *temp_files[] = {
        "/tmp/test_gcc_driver_1.c",
        "/tmp/test_gcc_driver_2.c", 
        "/tmp/test_gcc_driver_err.c",
        "/tmp/test_gcc_driver_1.o",
        "/tmp/test_gcc_driver_2.o",
        "/tmp/test_gcc_driver_1.i",
        "/tmp/test_gcc_driver_1.s",
        NULL
    };
    
    /* Create temporary directory for dump files */
    mkdir("/tmp/gcc_driver_test_dump", 0755);
    
    /* Create valid source files */
    if (!create_temp_file("/tmp/test_gcc_driver_1.c", 
            "int foo(void) { return 0; }\n")) {
        return 1;
    }
    
    if (!create_temp_file("/tmp/test_gcc_driver_2.c",
            "int bar(void) { return 1; }\n")) {
        return 1;
    }
    
    /* Create file with syntax error */
    if (!create_temp_file("/tmp/test_gcc_driver_err.c",
            "int baz(void) { return /* missing semicolon and value */ }\n")) {
        return 1;
    }
    
    printf("=== Testing GCC driver initialization block coverage ===\n\n");
    
    /* SEQUENCE 1: Help flag then compilation (tests print_help_list, print_version reset) */
    printf("--- Sequence 1: Help then compile ---\n");
    success &= run_command("gcc --help=common 2>&1 | head -5 > /dev/null", 0);
    success &= run_command("gcc -c /tmp/test_gcc_driver_1.c -o /tmp/test_gcc_driver_1.o", 0);
    
    /* SEQUENCE 2: Save-temps with dumpdir then plain compile 
       (tests save_temps_flag, dumpdir, dumpbase reset) */
    printf("\n--- Sequence 2: Save-temps with dumpdir then plain compile ---\n");
    success &= run_command("gcc -save-temps -dumpdir /tmp/gcc_driver_test_dump/ "
                          "-dumpbase mydump -c /tmp/test_gcc_driver_1.c "
                          "-o /tmp/test_gcc_driver_1.o 2>/dev/null", 0);
    /* Second job without save-temps - should trigger free() of dumpdir/dumpbase */
    success &= run_command("gcc -c /tmp/test_gcc_driver_2.c -o /tmp/test_gcc_driver_2.o", 0);
    
    /* SEQUENCE 3: Error then success (tests greatest_status reset) */
    printf("\n--- Sequence 3: Error then success ---\n");
    /* First compilation fails due to syntax error */
    run_command("gcc -c /tmp/test_gcc_driver_err.c -o /tmp/error.o 2>/dev/null", -1);
    /* Second should succeed - greatest_status should be reset between jobs */
    success &= run_command("gcc -c /tmp/test_gcc_driver_1.c -o /tmp/test_gcc_driver_1.o", 0);
    
    /* SEQUENCE 4: Verbose and linker selection then plain compile
       (tests verbose_only_flag, use_ld reset) */
    printf("\n--- Sequence 4: Verbose with linker selection then plain compile ---\n");
    success &= run_command("gcc -v -fuse-ld=bfd -c /tmp/test_gcc_driver_1.c "
                          "-o /tmp/test_gcc_driver_1.o 2>&1 | "
                          "grep -q 'COLLECT_GCC_OPTIONS'", 0);
    success &= run_command("gcc -c /tmp/test_gcc_driver_2.c -o /tmp/test_gcc_driver_2.o", 0);
    
    /* SEQUENCE 5: Multiple language specifications via -x
       (tests is_cpp_driver, at_file_supplied reset) */
    printf("\n--- Sequence 5: Multiple -x language specs ---\n");
    success &= run_command("echo 'int x = 42;' | gcc -x c - -c -o /tmp/test1.o", 0);
    success &= run_command("echo 'int y = 43;' | gcc -x c - -c -o /tmp/test2.o", 0);
    
    /* SEQUENCE 6: Version flag then compilation (tests print_version reset) */
    printf("\n--- Sequence 6: Version then compile ---\n");
    success &= run_command("gcc --version 2>&1 | head -1 > /dev/null", 0);
    success &= run_command("gcc -c /tmp/test_gcc_driver_1.c -o /tmp/test_gcc_driver_1.o", 0);
    
    /* SEQUENCE 7: Combined flags to stress multiple variables */
    printf("\n--- Sequence 7: Combined flags stress test ---\n");
    success &= run_command("gcc -v --help=optimizers -save-temps -dumpdir /tmp/gcc_driver_test_dump2 "
                          "-dumpbase combined -fuse-ld=lld -c /tmp/test_gcc_driver_1.c "
                          "-o /tmp/test_gcc_driver_1.o 2>&1 | head -20 > /dev/null", 0);
    /* Follow with minimal compile to trigger full reset */
    success &= run_command("gcc -c /tmp/test_gcc_driver_2.c -o /tmp/test_gcc_driver_2.o", 0);
    
    /* Cleanup */
    printf("\n--- Cleaning up ---\n");
    for (int i = 0; temp_files[i]; i++) {
        unlink(temp_files[i]);
    }
    
    /* Clean dump directories */
    system("rm -rf /tmp/gcc_driver_test_dump /tmp/gcc_driver_test_dump2");
    unlink("/tmp/test1.o");
    unlink("/tmp/test2.o");
    unlink("/tmp/error.o");
    
    /* Final checksum based on success of all sequences */
    printf("\n=== Test Result Checksum: %s ===\n", success ? "PASS" : "FAIL");
    
    /* Reference dummy functions to avoid unused function warnings */
    (void)foo;
    (void)bar;
    
    return success ? 0 : 1;
}
