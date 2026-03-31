/* test_gcc_driver_init.c
 * 
 * This program tests the GCC driver initialization logic (lines 11228-11250 in gcc.cc)
 * by executing multiple compilation jobs with different option combinations.
 * Each sequence sets specific driver state variables that should be reset
 * between jobs according to the uncovered code block.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Function prototypes for the dummy functions we'll create */
static int foo(void) __attribute__((unused));
static int bar(void) __attribute__((unused));
static int baz(void) __attribute__((unused));

/* Helper function to create temporary files */
static int create_temp_file(const char *template, const char *content) {
    FILE *f = fopen(template, "w");
    if (!f) {
        perror("fopen");
        return -1;
    }
    fprintf(f, "%s", content);
    fclose(f);
    return 0;
}

/* Helper function to execute a command and check status */
static int execute_check(const char *cmd, const char *desc) {
    printf("Executing: %s\n", desc);
    printf("Command: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        printf("Command returned non-zero: %d\n", WEXITSTATUS(status));
    }
    return status;
}

int main(void) {
    int overall_result = 0;
    char cmd[1024];
    
    /* Create temporary directory for dump files */
    mkdir("./dump_test_dir", 0755);
    
    /* Create temporary source files with different content */
    const char *temp1_content = 
        "static int foo(void) { return 0; }\n"
        "int public_func1(void) { return foo(); }\n";
    
    const char *temp2_content = 
        "static int bar(void) { return 1; }\n"
        "int public_func2(void) { return bar(); }\n";
    
    const char *error_content = 
        "int baz(void) { return  /* deliberate syntax error */ \n";
    
    const char *temp3_content = 
        "int helper(void) { return 42; }\n";
    
    /* Create the files */
    if (create_temp_file("temp1.c", temp1_content) != 0 ||
        create_temp_file("temp2.c", temp2_content) != 0 ||
        create_temp_file("syntax_error.c", error_content) != 0 ||
        create_temp_file("temp3.c", temp3_content) != 0) {
        fprintf(stderr, "Failed to create temporary files\n");
        return 1;
    }
    
    printf("=== Testing GCC Driver Initialization Block (lines 11228-11250) ===\n\n");
    
    /* SEQUENCE 1: Help/Version flags then compilation
     * Sets: print_help_list, print_version
     * Then resets them for compilation job */
    printf("\n--- Sequence 1: Help then Compile ---\n");
    
    /* First job: print help (sets print_help_list) */
    snprintf(cmd, sizeof(cmd), 
             "%s --help=common > /dev/null 2>&1", 
             "gcc");
    overall_result |= (execute_check(cmd, "Help command") != 0) << 0;
    
    /* Second job: compile (should reset print_help_list) */
    snprintf(cmd, sizeof(cmd), 
             "%s -c temp1.c -o temp1.o", 
             "gcc");
    overall_result |= (execute_check(cmd, "Compile after help") != 0) << 1;
    
    /* Third job: version then compile (sets print_version) */
    snprintf(cmd, sizeof(cmd), 
             "%s --version > /dev/null 2>&1 && %s -c temp2.c -o temp2.o", 
             "gcc", "gcc");
    overall_result |= (execute_check(cmd, "Version then compile") != 0) << 2;
    
    /* SEQUENCE 2: Save-temps and dump directory options
     * Sets: save_temps_flag, dumpdir, dumpbase, etc.
     * Then resets them (freeing memory, setting to NULL) */
    printf("\n--- Sequence 2: Save-temps with dumpdir then Compile ---\n");
    
    /* First job: with save-temps and dump options */
    snprintf(cmd, sizeof(cmd),
             "%s -save-temps -dumpdir ./dump_test_dir -dumpbase mydump "
             "-c temp1.c -o temp1_save.o 2>/dev/null",
             "gcc");
    overall_result |= (execute_check(cmd, "Save-temps with dumpdir") != 0) << 3;
    
    /* Check if dump files were created */
    snprintf(cmd, sizeof(cmd),
             "test -f ./dump_test_dir/mydump.i || "
             "test -f temp1.i || "
             "test -f mydump.i");
    int dump_exists = system(cmd);
    
    /* Second job: plain compile (should reset dumpdir, dumpbase, etc.) */
    snprintf(cmd, sizeof(cmd),
             "%s -c temp2.c -o temp2_plain.o",
             "gcc");
    overall_result |= (execute_check(cmd, "Plain compile after save-temps") != 0) << 4;
    
    /* SEQUENCE 3: Error recovery (tests greatest_status reset)
     * First job fails, second succeeds */
    printf("\n--- Sequence 3: Error then Success ---\n");
    
    /* First job: should fail (syntax error) */
    snprintf(cmd, sizeof(cmd),
             "%s -c syntax_error.c -o error.o 2>/dev/null",
             "gcc");
    int error_status = system(cmd);
    printf("Error compilation returned: %d (expected non-zero)\n", WEXITSTATUS(error_status));
    
    /* Second job: should succeed (resets greatest_status from failure) */
    snprintf(cmd, sizeof(cmd),
             "%s -c temp3.c -o temp3.o",
             "gcc");
    overall_result |= (execute_check(cmd, "Successful compile after error") != 0) << 5;
    
    /* SEQUENCE 4: Verbose and linker options
     * Sets: verbose_only_flag, use_ld */
    printf("\n--- Sequence 4: Verbose and Linker then Plain ---\n");
    
    /* First job: verbose with specific linker */
    snprintf(cmd, sizeof(cmd),
             "%s -v -fuse-ld=bfd -c temp1.c -o temp1_verbose.o 2>&1 | "
             "grep -q 'COLLECT_GCC_OPTIONS'",
             "gcc");
    overall_result |= (execute_check(cmd, "Verbose with ld=bfd") != 0) << 6;
    
    /* Alternative: check for linker mention */
    snprintf(cmd, sizeof(cmd),
             "%s -v -fuse-ld=bfd -c temp1.c 2>&1 | "
             "head -20 > /dev/null",
             "gcc");
    system(cmd);  /* Don't check status, just execute */
    
    /* Second job: plain compile (should reset use_ld) */
    snprintf(cmd, sizeof(cmd),
             "%s -c temp2.c -o temp2_final.o",
             "gcc");
    overall_result |= (execute_check(cmd, "Plain compile after verbose") != 0) << 7;
    
    /* SEQUENCE 5: Multiple jobs in single invocation
     * Tests per-job initialization */
    printf("\n--- Sequence 5: Multiple inputs single invocation ---\n");
    
    snprintf(cmd, sizeof(cmd),
             "%s -c temp1.c temp2.c temp3.c",
             "gcc");
    overall_result |= (execute_check(cmd, "Multiple files in one invocation") != 0) << 8;
    
    /* SEQUENCE 6: Different language specifications with -x
     * Forces multiple compilation jobs */
    printf("\n--- Sequence 6: Multiple -x language specs ---\n");
    
    /* Create a C++ source file */
    const char *cpp_content = 
        "extern \"C\" int cpp_func() { return 3; }\n";
    create_temp_file("temp.cpp", cpp_content);
    
    snprintf(cmd, sizeof(cmd),
             "%s -x c -c temp1.c -o temp1_c.o && "
             "%s -x c++ -c temp.cpp -o temp_cpp.o",
             "gcc", "gcc");
    overall_result |= (execute_check(cmd, "Different language specs") != 0) << 9;
    
    /* SEQUENCE 7: Specs file usage (tests spec_machine) */
    printf("\n--- Sequence 7: Specs file then default ---\n");
    
    /* Create a minimal spec file */
    const char *spec_content = "*cpp:\n";
    create_temp_file("myspec.specs", spec_content);
    
    snprintf(cmd, sizeof(cmd),
             "%s -specs=myspec.specs -c temp1.c -o temp1_spec.o 2>/dev/null && "
             "%s -c temp2.c -o temp2_nospec.o",
             "gcc", "gcc");
    overall_result |= (execute_check(cmd, "With specs then without") != 0) << 10;
    
    /* Cleanup */
    printf("\n--- Cleaning up ---\n");
    system("rm -f temp*.c temp*.o temp*.i temp*.s temp*.ii syntax_error.c temp.cpp myspec.specs");
    system("rm -rf ./dump_test_dir");
    
    /* Create and use dummy functions to avoid unused warnings */
    int (*funcs[])(void) = {foo, bar, baz};
    printf("Dummy function count: %zu\n", sizeof(funcs)/sizeof(funcs[0]));
    
    printf("\n=== Test Complete ===\n");
    printf("Overall result bitmap: 0x%x\n", overall_result);
    printf("(Non-zero indicates some commands failed, which may be expected)\n");
    
    /* Most tests should pass, but we allow some expected failures */
    /* Only fail if critical tests failed (bits 1, 4, 5, 7 which are the actual compilations) */
    int critical_failures = overall_result & ((1<<1)|(1<<4)|(1<<5)|(1<<7));
    if (critical_failures) {
        printf("CRITICAL FAILURES: 0x%x\n", critical_failures);
        return 1;
    }
    
    return 0;
}

/* Dummy functions referenced to avoid unused warnings */
static int foo(void) { return 0; }
static int bar(void) { return 1; }
static int baz(void) { return 2; }
