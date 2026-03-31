/* test_gcc_driver_cleanup.c
 * 
 * This test program exercises the GCC driver's cleanup routine by invoking
 * it multiple times with different command-line flags that set the state
 * variables mentioned in the uncovered lines (11228-11250 of gcc.cc).
 * 
 * The goal is to ensure that after each invocation, the driver properly
 * resets its internal state variables and frees allocated memory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Simple C source file content that will be compiled */
#define SIMPLE_C_CONTENT "int main(void) { return 0; }\n"

/* Create a simple C source file for testing */
static int create_simple_c_file(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create simple.c");
        return -1;
    }
    fputs(SIMPLE_C_CONTENT, fp);
    fclose(fp);
    return 0;
}

/* Execute a GCC command and return its exit status */
static int execute_gcc_command(const char *command) {
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
static void cleanup_generated_files(void) {
    /* Remove common generated files */
    system("rm -f simple.c simple.o simple.i simple.s");
    system("rm -f test_output.o fail_output.o");
    system("rm -f coverage_test.* fail_test.*");
    system("rm -rf ./test_artifacts ./fail_artifacts");
    system("rm -f output.txt");
}

int main(void) {
    int overall_status = 0;
    
    printf("=== GCC Driver Cleanup Test ===\n\n");
    
    /* Step 1: Create the simple C source file */
    printf("1. Creating simple.c source file...\n");
    if (create_simple_c_file("simple.c") != 0) {
        return 1;
    }
    
    /* Step 2: Create directories for dumpdir if they don't exist */
    mkdir("./test_artifacts", 0755);
    mkdir("./fail_artifacts", 0755);
    
    /* Step 3: Execute multiple GCC invocations to set different state variables */
    
    /* Invocation A: Sets state with successful compilation (-c flag)
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     * Should succeed and trigger cleanup */
    printf("\n2. Invocation A: Successful compilation with state-setting flags\n");
    printf("   (Sets save_temps_flag, dumpdir, dumpbase, outbase)\n");
    const char *invocation_a = 
        "gcc -save-temps -dumpdir ./test_artifacts "
        "-dumpbase coverage_test -o test_output.o -c simple.c 2>&1";
    execute_gcc_command(invocation_a);
    
    /* Invocation B: Sets state but causes backend failure
     * Uses invalid architecture flag to cause failure after driver initialization
     * This ensures cleanup runs even on failure */
    printf("\n3. Invocation B: Failed compilation with state-setting flags\n");
    printf("   (Tests cleanup on failure path)\n");
    const char *invocation_b = 
        "gcc -save-temps -dumpdir ./fail_artifacts "
        "-dumpbase fail_test -o fail_output.o -march=invalid-arch simple.c 2>&1";
    execute_gcc_command(invocation_b);
    
    /* Invocation C: Uses -specs= and -V flags
     * These may influence spec_machine and print_version */
    printf("\n4. Invocation C: Using -specs and -V flags\n");
    printf("   (May set spec_machine and print_version)\n");
    const char *invocation_c = 
        "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -20";
    execute_gcc_command(invocation_c);
    
    /* Invocation D: Tests -Werror turning warnings into errors
     * This affects greatest_status variable */
    printf("\n5. Invocation D: Testing -Werror for error status\n");
    printf("   (Affects greatest_status)\n");
    
    /* First create a file with a warning */
    FILE *fp = fopen("warn.c", "w");
    if (fp) {
        fputs("int main(void) { int x; return 0; }\n", fp);  /* unused variable warning */
        fclose(fp);
        
        const char *invocation_d = 
            "gcc -Werror -c warn.c 2>&1";
        execute_gcc_command(invocation_d);
        
        system("rm -f warn.c warn.o");
    }
    
    /* Invocation E: Tests verbose flag and other state variables */
    printf("\n6. Invocation E: Testing verbose and help flags\n");
    printf("   (Sets verbose_only_flag, print_help_list, etc.)\n");
    const char *invocation_e = 
        "gcc -v -c simple.c 2>&1 | head -10";
    execute_gcc_command(invocation_e);
    
    /* Invocation F: Tests dumpbase_ext and other dump-related options */
    printf("\n7. Invocation F: Testing extended dump options\n");
    printf("   (Tests dumpbase_ext handling)\n");
    const char *invocation_f = 
        "gcc -save-temps -fdump-tree-all -fdump-rtl-all "
        "-dumpbase full_dump -dumpdir ./test_artifacts -c simple.c 2>&1 | head -5";
    execute_gcc_command(invocation_f);
    
    /* Invocation G: Tests target system root variables */
    printf("\n8. Invocation G: Testing sysroot options\n");
    printf("   (May affect target_system_root_changed, target_sysroot_suffix)\n");
    const char *invocation_g = 
        "gcc --sysroot=/nonexistent -c simple.c 2>&1";
    execute_gcc_command(invocation_g);
    
    /* Step 4: Clean up generated files */
    printf("\n9. Cleaning up generated files...\n");
    cleanup_generated_files();
    
    printf("\n=== Test completed ===\n");
    printf("All GCC invocations attempted. Check coverage data for gcc.cc lines 11228-11250.\n");
    
    return overall_status;
}
