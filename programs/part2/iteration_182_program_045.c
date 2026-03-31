/* test_gcc_driver_cleanup.c
 * 
 * This test program exercises the GCC driver's cleanup routine by invoking
 * it multiple times with different command-line arguments that set the
 * state variables mentioned in the uncovered lines (11228-11250 of gcc.cc).
 * The goal is to ensure these variables are allocated/set and then properly
 * reset during cleanup.
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

/* Run a GCC command and capture its exit status */
int run_gcc_command(const char *command) {
    printf("Executing: %s\n", command);
    int status = system(command);
    
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    } else {
        printf("Command terminated abnormally\n");
    }
    
    return status;
}

/* Clean up generated files */
void cleanup_generated_files(void) {
    /* Remove temporary files that might have been created */
    system("rm -f simple.c simple.o simple.i simple.s");
    system("rm -f test_output.o test_output.i test_output.s");
    system("rm -f fail_output.o fail_output.i fail_output.s");
    system("rm -f output.o output.i output.s");
    system("rm -f *.gcda *.gcno");
    
    /* Remove temporary directories */
    system("rm -rf ./test_artifacts ./fail_artifacts ./temp");
}

int main(void) {
    int overall_result = 0;
    
    /* Create the simple C source file */
    if (!create_simple_c_file("simple.c")) {
        return 1;
    }
    
    printf("=== Testing GCC Driver Cleanup Routine ===\n\n");
    
    /* Invocation 1: Sets state variables and succeeds with -c (compile only)
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     */
    printf("--- Invocation 1: Successful compilation with state variables ---\n");
    run_gcc_command("gcc -save-temps -dumpdir ./test_artifacts "
                    "-dumpbase coverage_test -o test_output.o -c simple.c");
    printf("\n");
    
    /* Invocation 2: Sets different state variables and fails late
     * Uses invalid architecture to cause backend failure after driver init
     * This exercises cleanup with failure status
     */
    printf("--- Invocation 2: Late failure with different state variables ---\n");
    run_gcc_command("gcc -save-temps -dumpdir ./fail_artifacts "
                    "-dumpbase fail_test -o fail_output.o "
                    "-march=invalid-arch -mtune=invalid-tune simple.c 2>/dev/null");
    printf("\n");
    
    /* Invocation 3: Tests spec_machine and print_version
     * The -V flag triggers print_version, and -specs= affects spec_machine
     */
    printf("--- Invocation 3: Testing version and specs ---\n");
    run_gcc_command("gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5");
    printf("\n");
    
    /* Invocation 4: Tests dumpbase_ext and outbase with different extensions */
    printf("--- Invocation 4: Testing dumpbase_ext ---\n");
    run_gcc_command("gcc -save-temps=obj -dumpdir ./temp -dumpbase myprog "
                    "-dumpbase-ext .ext -o output.o -c simple.c");
    printf("\n");
    
    /* Invocation 5: Tests -Werror turning warnings into errors
     * This affects greatest_status
     */
    printf("--- Invocation 5: Testing -Werror for error status ---\n");
    
    /* Create a file with a potential warning */
    FILE *fp = fopen("warn.c", "w");
    if (fp) {
        fputs("int main(void) {\n", fp);
        fputs("    int x;\n", fp);  /* Unused variable warning */
        fputs("    return 0;\n", fp);
        fputs("}\n", fp);
        fclose(fp);
        
        run_gcc_command("gcc -Werror -o warn.o -c warn.c 2>&1");
        system("rm -f warn.c warn.o");
    }
    printf("\n");
    
    /* Invocation 6: Tests verbose flag and report times */
    printf("--- Invocation 6: Testing verbose and timing ---\n");
    run_gcc_command("gcc -ftime-report -c simple.c 2>&1 | head -10");
    printf("\n");
    
    /* Invocation 7: Tests target system root variables */
    printf("--- Invocation 7: Testing sysroot options ---\n");
    run_gcc_command("gcc --sysroot=/tmp -c simple.c 2>&1");
    printf("\n");
    
    /* Invocation 8: Tests help and version flags */
    printf("--- Invocation 8: Testing help and version print flags ---\n");
    run_gcc_command("gcc --help | head -5");
    run_gcc_command("gcc --version");
    printf("\n");
    
    /* Clean up generated files */
    printf("--- Cleaning up generated files ---\n");
    cleanup_generated_files();
    
    printf("\n=== Test completed ===\n");
    printf("All GCC invocations attempted. Check coverage data for gcc.cc.\n");
    
    return overall_result;
}
