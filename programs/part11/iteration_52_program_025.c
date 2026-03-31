/* test_gcc_driver_init.c
 * 
 * This program tests the GCC driver initialization logic (lines 11228-11250 in gcc.cc)
 * by executing multiple compilation jobs with varying state configurations.
 * Each sequence sets specific driver state variables, then subsequent jobs
 * should trigger the reset logic in the uncovered block.
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
static void create_temp_file(const char *filename, const char *content);
static int execute_command(const char *cmd);
static void cleanup_files(void);

/* Temporary file names */
#define TEMP1_C "temp_test1.c"
#define TEMP2_C "temp_test2.c"
#define TEMP_ERR_C "temp_error.c"
#define TEMP_O1 "temp_test1.o"
#define TEMP_O2 "temp_test2.o"
#define DUMP_DIR "./dump_test_dir"

/* Dummy functions to avoid unused function warnings */
static int foo(void) { return 0; }
static int bar(void) { return 1; }

/* Create a temporary file with given content */
static void create_temp_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create temp file");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "%s", content);
    fclose(f);
}

/* Execute a command and return its exit status */
static int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Clean up temporary files and directories */
static void cleanup_files(void) {
    remove(TEMP1_C);
    remove(TEMP2_C);
    remove(TEMP_ERR_C);
    remove(TEMP_O1);
    remove(TEMP_O2);
    
    /* Remove dump directory if it exists */
    system("rm -rf " DUMP_DIR);
    
    /* Clean up any other temp files that might have been created */
    system("rm -f *.s *.i *.o dump_test_* 2>/dev/null");
}

int main(void) {
    int checksum = 0;
    int result;
    
    printf("=== Testing GCC Driver Initialization Block (lines 11228-11250) ===\n\n");
    
    /* Create temporary source files */
    create_temp_file(TEMP1_C, 
        "int foo(void) { return 0; }\n"
        "int unused_func1(void) { return 2; }\n");
    
    create_temp_file(TEMP2_C,
        "int bar(void) { return 1; }\n"
        "int unused_func2(void) { return 3; }\n");
    
    /* Create a file with a syntax error */
    create_temp_file(TEMP_ERR_C,
        "int baz(void) { return \n"  /* Missing expression - syntax error */
        "int missing_semicolon() { return 0 }\n");
    
    /* Create dump directory */
    mkdir(DUMP_DIR, 0755);
    
    printf("--- Sequence 1: Help flag then compilation ---\n");
    printf("This sets print_help_list, then resets it for compilation job.\n");
    
    /* First job: --help sets print_help_list */
    /* Second job: compilation should reset print_help_list to 0 */
    result = execute_command("gcc --help=common > /dev/null 2>&1 && "
                            "gcc -c " TEMP1_C " -o " TEMP_O1 " 2>/dev/null");
    checksum = (checksum * 31 + result) & 0xFF;
    printf("Result: %d\n\n", result);
    
    printf("--- Sequence 2: Version flag then compilation ---\n");
    printf("This sets print_version, then resets it.\n");
    
    /* First job: --version sets print_version */
    /* Second job: compilation resets print_version to 0 */
    result = execute_command("gcc --version > /dev/null 2>&1 && "
                            "gcc -c " TEMP2_C " -o " TEMP_O2 " 2>/dev/null");
    checksum = (checksum * 31 + result) & 0xFF;
    printf("Result: %d\n\n", result);
    
    printf("--- Sequence 3: Save-temps with dumpdir then plain compilation ---\n");
    printf("This sets save_temps_flag, dumpdir, dumpbase, then frees them.\n");
    
    /* First job: -save-temps, -dumpdir, -dumpbase set various flags */
    /* Second job: plain compilation should free dumpdir/dumpbase and set to NULL */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "gcc -save-temps -dumpdir %s -dumpbase test_base -c %s -o %s 2>/dev/null && "
             "gcc -c %s 2>/dev/null",
             DUMP_DIR, TEMP1_C, TEMP_O1, TEMP2_C);
    result = execute_command(cmd);
    checksum = (checksum * 31 + result) & 0xFF;
    printf("Result: %d\n\n", result);
    
    printf("--- Sequence 4: Verbose flag then normal compilation ---\n");
    printf("This sets verbose_only_flag, then resets it.\n");
    
    /* First job: -v sets verbose_only_flag */
    /* Second job: normal compilation resets it */
    result = execute_command("gcc -v -c " TEMP1_C " 2>&1 | grep -q 'COLLECT_GCC' && "
                            "gcc -c " TEMP2_C " 2>/dev/null");
    checksum = (checksum * 31 + result) & 0xFF;
    printf("Result: %d\n\n", result);
    
    printf("--- Sequence 5: Linker selection then default ---\n");
    printf("This sets use_ld, then resets to NULL.\n");
    
    /* First job: -fuse-ld sets use_ld */
    /* Second job: no linker flag resets use_ld to NULL */
    result = execute_command("gcc -fuse-ld=bfd -c " TEMP1_C " -o " TEMP_O1 " 2>/dev/null && "
                            "gcc -c " TEMP2_C " 2>/dev/null");
    checksum = (checksum * 31 + result) & 0xFF;
    printf("Result: %d\n\n", result);
    
    printf("--- Sequence 6: Error then success (tests greatest_status reset) ---\n");
    printf("First job fails (syntax error), second succeeds.\n");
    
    /* First job: compilation with syntax error should fail */
    /* Second job: successful compilation - greatest_status should be reset */
    result = execute_command("gcc -c " TEMP_ERR_C " 2>/dev/null; "
                            "gcc -c " TEMP1_C " -o " TEMP_O1 " 2>/dev/null");
    checksum = (checksum * 31 + result) & 0xFF;
    printf("Result: %d\n\n", result);
    
    printf("--- Sequence 7: Multiple jobs in single invocation ---\n");
    printf("Tests per-job initialization with multiple source files.\n");
    
    /* Single gcc invocation with multiple source files creates multiple jobs */
    result = execute_command("gcc -c " TEMP1_C " " TEMP2_C " 2>/dev/null");
    checksum = (checksum * 31 + result) & 0xFF;
    printf("Result: %d\n\n", result);
    
    printf("--- Sequence 8: Mixed options across jobs ---\n");
    printf("Combines multiple state-changing options across jobs.\n");
    
    /* Chain of jobs with different option sets */
    snprintf(cmd, sizeof(cmd),
             "gcc --help=optimizers > /dev/null 2>&1 && "
             "gcc -save-temps -dumpdir %s -c %s 2>/dev/null && "
             "gcc -v -fuse-ld=bfd -c %s 2>&1 | grep -q 'COLLECT_GCC' && "
             "gcc -c %s 2>/dev/null",
             DUMP_DIR, TEMP1_C, TEMP2_C, TEMP1_C);
    result = execute_command(cmd);
    checksum = (checksum * 31 + result) & 0xFF;
    printf("Result: %d\n\n", result);
    
    printf("--- Sequence 9: Using -x to force multiple jobs ---\n");
    printf("-x language specification can create separate jobs.\n");
    
    /* Create a simple C++ source to test -x option */
    create_temp_file("temp_cpp.cpp", "int cpp_func() { return 42; }\n");
    
    /* Use -x to specify language, potentially creating separate jobs */
    result = execute_command("gcc -x c -c " TEMP1_C " -x c++ -c temp_cpp.cpp 2>/dev/null");
    checksum = (checksum * 31 + result) & 0xFF;
    remove("temp_cpp.cpp");
    printf("Result: %d\n\n", result);
    
    /* Clean up */
    cleanup_files();
    
    printf("=== Test Complete ===\n");
    printf("Final checksum: 0x%02X\n", checksum);
    
    /* Reference dummy functions to avoid unused warnings */
    (void)foo;
    (void)bar;
    
    return 0;
}
