/* test_gcc_driver_cleanup.c
 * 
 * This test program exercises the GCC driver's cleanup routine by invoking
 * it multiple times with different command-line flags that set the state
 * variables mentioned in the uncovered lines.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

/* Simple C source file content */
const char *simple_c_content = 
"int main(void) {\n"
"    return 0;\n"
"}\n";

/* Create a simple C source file */
int create_simple_c_file(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create simple.c");
        return 0;
    }
    fputs(simple_c_content, fp);
    fclose(fp);
    return 1;
}

/* Run a GCC command and capture its return code */
int run_gcc_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (status == -1) {
        perror("system() failed");
        return -1;
    }
    return WEXITSTATUS(status);
}

/* Clean up generated files */
void cleanup_files(void) {
    /* Remove generated files from successful compilation */
    remove("simple.o");
    remove("simple.i");
    remove("simple.s");
    remove("test_output.o");
    remove("test_output.i");
    remove("test_output.s");
    remove("fail_output.o");
    remove("fail_output.i");
    remove("fail_output.s");
    remove("simple");
    remove("test_output");
    remove("fail_output");
    
    /* Remove dumpdir directories */
    system("rm -rf ./test_artifacts ./fail_artifacts ./temp");
}

int main(void) {
    int overall_result = 0;
    
    /* Create the simple C source file */
    if (!create_simple_c_file("simple.c")) {
        return 1;
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* INVOCATION 1: Set state variables with successful compilation
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Uses -c flag to stop before linking (successful compilation) */
    printf("1. Testing with -save-temps, -dumpdir, -dumpbase, -o (successful):\n");
    char cmd1[1024];
    snprintf(cmd1, sizeof(cmd1),
             "gcc -save-temps -dumpdir ./test_artifacts -dumpbase coverage_test "
             "-o test_output.o -c simple.c 2>&1");
    int result1 = run_gcc_command(cmd1);
    printf("Exit code: %d\n\n", result1);
    
    /* INVOCATION 2: Set state variables with compilation failure
     * Uses invalid architecture to cause backend failure after driver init */
    printf("2. Testing with -save-temps and invalid -march (should fail):\n");
    char cmd2[1024];
    snprintf(cmd2, sizeof(cmd2),
             "gcc -save-temps -dumpdir ./fail_artifacts -dumpbase fail_test "
             "-o fail_output.o -march=invalid-arch simple.c 2>&1");
    int result2 = run_gcc_command(cmd2);
    printf("Exit code: %d\n\n", result2);
    
    /* INVOCATION 3: Test with -specs and -V flags
     * This influences spec_machine and print_version */
    printf("3. Testing with -specs and -V flags:\n");
    char cmd3[1024];
    snprintf(cmd3, sizeof(cmd3),
             "gcc -specs=nosuch.spec -V simple.c 2>&1");
    int result3 = run_gcc_command(cmd3);
    printf("Exit code: %d\n\n", result3);
    
    /* INVOCATION 4: Test with -Werror to generate error exit status
     * This should affect greatest_status */
    printf("4. Testing with -Werror (warning turned to error):\n");
    
    /* First create a file with a warning */
    FILE *fp = fopen("warn.c", "w");
    if (fp) {
        fputs("int main(void) {\n", fp);
        fputs("    int x;  /* unused variable warning */\n", fp);
        fputs("    return 0;\n", fp);
        fputs("}\n", fp);
        fclose(fp);
        
        char cmd4[1024];
        snprintf(cmd4, sizeof(cmd4),
                 "gcc -Werror -c warn.c 2>&1");
        int result4 = run_gcc_command(cmd4);
        printf("Exit code: %d\n\n", result4);
        
        remove("warn.c");
        remove("warn.o");
    }
    
    /* INVOCATION 5: Test with verbose flag and output redirection
     * This sets verbose_only_flag and uses dumpdir */
    printf("5. Testing with -v and dumpdir:\n");
    char cmd5[1024];
    snprintf(cmd5, sizeof(cmd5),
             "gcc -v -dumpdir ./temp -o simple simple.c 2>&1 | head -20");
    int result5 = run_gcc_command(cmd5);
    printf("Exit code: %d\n\n", result5);
    
    /* INVOCATION 6: Test help and version flags
     * These set print_help_list and print_version */
    printf("6. Testing --help and --version flags:\n");
    char cmd6[1024];
    snprintf(cmd6, sizeof(cmd6),
             "gcc --help 2>&1 | head -5");
    int result6 = run_gcc_command(cmd6);
    printf("Exit code: %d\n", result6);
    
    char cmd7[1024];
    snprintf(cmd7, sizeof(cmd7),
             "gcc --version 2>&1 | head -2");
    int result7 = run_gcc_command(cmd7);
    printf("Exit code: %d\n\n", result7);
    
    /* INVOCATION 7: Test with multiple output-related flags
     * Exercises dumpbase_ext and outbase logic */
    printf("7. Testing with multiple output flags:\n");
    char cmd8[1024];
    snprintf(cmd8, sizeof(cmd8),
             "gcc -save-temps=obj -dumpbase myprog.c -dumpbase-ext .ext "
             "-o final_output simple.c 2>&1");
    int result8 = run_gcc_command(cmd8);
    printf("Exit code: %d\n\n", result8);
    
    /* Clean up generated files */
    printf("Cleaning up generated files...\n");
    cleanup_files();
    remove("simple.c");
    remove("final_output");
    remove("myprog.c.ext.*");
    
    printf("\n=== Test completed ===\n");
    printf("All GCC invocations attempted. Check coverage data for gcc.cc\n");
    
    return overall_result;
}
