/* test_gcc_driver_cleanup.c
 * This test program invokes the GCC driver with various flags to ensure
 * the cleanup routine in gcc.cc is executed, covering lines 11228-11250.
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

/* Run a GCC command and return its exit status */
int run_gcc_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    printf("Command failed to execute properly\n\n");
    return -1;
}

/* Clean up generated files */
void cleanup_generated_files(void) {
    /* Remove temporary files and directories */
    system("rm -f simple.c simple.o simple.i simple.s");
    system("rm -f test_output.o test_output.i test_output.s");
    system("rm -f fail_output.o fail_output.i fail_output.s");
    system("rm -f output.o output.i output.s");
    system("rm -rf ./test_artifacts ./fail_artifacts ./temp");
    system("rm -f *.gcda *.gcno 2>/dev/null");
}

int main(void) {
    int overall_result = 0;
    
    /* Create the simple C source file */
    if (!create_simple_c_file("simple.c")) {
        return 1;
    }
    
    printf("=== GCC Driver Cleanup Coverage Test ===\n\n");
    
    /* Invocation A: Sets state with -save-temps, -dumpdir, -dumpbase, -o, and succeeds */
    printf("Test A: Successful compilation with state-setting flags\n");
    printf("--------------------------------------------------------\n");
    run_gcc_command("gcc -save-temps -dumpdir ./test_artifacts -dumpbase coverage_test "
                    "-o test_output.o -c simple.c");
    
    /* Invocation B: Sets state but fails with invalid architecture */
    printf("Test B: Failed compilation with invalid architecture\n");
    printf("------------------------------------------------------\n");
    run_gcc_command("gcc -save-temps -dumpdir ./fail_artifacts -dumpbase fail_test "
                    "-o fail_output.o -march=invalid-arch -c simple.c 2>/dev/null");
    
    /* Invocation C: Uses -specs flag (should fail as file doesn't exist) */
    printf("Test C: Compilation with non-existent spec file\n");
    printf("-----------------------------------------------\n");
    run_gcc_command("gcc -specs=nosuch.spec -c simple.c 2>/dev/null");
    
    /* Invocation D: Version printing flag */
    printf("Test D: Version printing (triggers print_version)\n");
    printf("--------------------------------------------------\n");
    run_gcc_command("gcc -V 2>&1 | head -5");
    
    /* Invocation E: Help flag (triggers print_help_list) */
    printf("Test E: Help flag\n");
    printf("-----------------\n");
    run_gcc_command("gcc --help 2>&1 | head -10");
    
    /* Invocation F: Verbose flag */
    printf("Test F: Verbose output\n");
    printf("----------------------\n");
    run_gcc_command("gcc -v -c simple.c 2>&1 | tail -5");
    
    /* Invocation G: Multiple dumpbase extensions */
    printf("Test G: Multiple dumpbase extensions\n");
    printf("------------------------------------\n");
    run_gcc_command("gcc -save-temps -dumpdir ./temp -dumpbase mydump "
                    "-dumpbase-ext .ext -o output.o -c simple.c");
    
    /* Invocation H: Compilation with -Werror on code with warnings */
    const char *warning_c_content = 
    "int main(void) {\n"
    "    int x;\n"  /* Unused variable warning */
    "    return 0;\n"
    "}\n";
    
    FILE *fp = fopen("warning.c", "w");
    if (fp) {
        fputs(warning_c_content, fp);
        fclose(fp);
        
        printf("Test H: Warnings as errors\n");
        printf("--------------------------\n");
        run_gcc_command("gcc -Werror -c warning.c 2>&1");
        
        system("rm -f warning.c warning.o");
    }
    
    /* Invocation I: Linker error (undefined reference) */
    printf("Test I: Linker error\n");
    printf("--------------------\n");
    run_gcc_command("gcc -lnonexistentlibrary simple.c 2>&1 | tail -3");
    
    /* Invocation J: Reset all flags with minimal compilation */
    printf("Test J: Minimal successful compilation (resets state)\n");
    printf("-----------------------------------------------------\n");
    run_gcc_command("gcc -c simple.c");
    
    /* Clean up generated files */
    printf("Cleaning up generated files...\n");
    cleanup_generated_files();
    
    printf("\n=== Test completed ===\n");
    printf("The GCC driver's cleanup routine should have been executed multiple times,\n");
    printf("covering the reset of variables like:\n");
    printf("  - is_cpp_driver, at_file_supplied, print_help_list\n");
    printf("  - save_temps_flag, dumpdir, dumpbase, dumpbase_ext, outbase\n");
    printf("  - spec_machine, greatest_status\n");
    
    return overall_result;
}
