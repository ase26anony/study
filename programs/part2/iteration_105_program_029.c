#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_CMD_LEN 1024

/* Helper function to execute a command and return exit status */
static int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Create a minimal C source file */
static void create_source_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create source file");
        exit(1);
    }
    fprintf(f, "int main(void) { return 0; }\n");
    fclose(f);
}

/* Create a response file with various options */
static void create_response_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create response file");
        exit(1);
    }
    /* Options that set various driver state variables */
    fprintf(f, "-v\n");
    fprintf(f, "-save-temps=obj\n");
    fprintf(f, "-Wall\n");
    fprintf(f, "-Wextra\n");
    fclose(f);
}

int main(int argc, char **argv) {
    char cmd[MAX_CMD_LEN];
    const char *base_name = "test_gcc_reset";
    const char *source_file = "test_source.c";
    const char *response_file = "args.rsp";
    const char *output_obj = "test_output.o";
    const char *output_exe = "test_output.exe";
    
    /* Clean up any existing files from previous runs */
    unlink(source_file);
    unlink(response_file);
    unlink(output_obj);
    unlink(output_exe);
    unlink("test_source.i");
    unlink("test_source.s");
    unlink("test_source.o");
    
    /* Create test files */
    create_source_file(source_file);
    create_response_file(response_file);
    
    printf("=== Testing GCC Driver Reset Logic ===\n\n");
    
    /* Invocation 1: Set print_help_list flag */
    printf("1. Setting print_help_list flag...\n");
    snprintf(cmd, sizeof(cmd), "gcc -print-help-list 2>&1 | head -5");
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 2: Set print_version flag */
    printf("2. Setting print_version flag...\n");
    snprintf(cmd, sizeof(cmd), "gcc --version 2>&1 | head -2");
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 3: Set verbose_only_flag and use response file (at_file_supplied) */
    printf("3. Setting verbose flag with response file (at_file_supplied)...\n");
    snprintf(cmd, sizeof(cmd), "gcc -v -c %s @%s 2>&1 | head -10", source_file, response_file);
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 4: Set save_temps_flag and related dumpdir/dumpbase variables */
    printf("4. Setting save_temps flag with various options...\n");
    snprintf(cmd, sizeof(cmd), "gcc -save-temps=cwd -dumpdir=./dump -dumpbase=testdump -o %s %s 2>&1 | head -5", 
             output_exe, source_file);
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 5: Set use_ld variable */
    printf("5. Setting use_ld variable...\n");
    snprintf(cmd, sizeof(cmd), "gcc -fuse-ld=bfd -o %s %s 2>&1 | head -5", output_exe, source_file);
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 6: Set report_times_to_file variable */
    printf("6. Setting time report flag...\n");
    snprintf(cmd, sizeof(cmd), "gcc -ftime-report -c %s -o %s 2>&1 | head -20", source_file, output_obj);
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 7: Attempt to set target_system_root (may need root path) */
    printf("7. Attempting to set sysroot...\n");
    snprintf(cmd, sizeof(cmd), "gcc --sysroot=/ -c %s -o %s 2>&1 | head -5", source_file, output_obj);
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 8: Cause a failure to set greatest_status != 1 */
    printf("8. Causing compilation failure (to set greatest_status != 1)...\n");
    snprintf(cmd, sizeof(cmd), "gcc -c non_existent_file.c -o fake.o 2>&1 | head -5");
    int fail_status = execute_command(cmd);
    printf("Failure exit status: %d\n\n", fail_status);
    
    /* Invocation 9: Successful compilation after failure (should reset greatest_status) */
    printf("9. Successful compilation after failure...\n");
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s 2>&1", source_file, output_obj);
    int success_status = execute_command(cmd);
    printf("Success exit status: %d\n\n", success_status);
    
    /* Invocation 10: Try to set spec_machine via -march/-mtune options */
    printf("10. Setting machine-specific options...\n");
    snprintf(cmd, sizeof(cmd), "gcc -march=x86-64 -mtune=generic -c %s -o %s 2>&1 | head -5", 
             source_file, output_obj);
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 11: Combination of many flags at once */
    printf("11. Combination of many state-setting options...\n");
    snprintf(cmd, sizeof(cmd), "gcc -v -save-temps -ftime-report -fuse-ld=gold -Wall -c %s -o %s 2>&1 | head -15", 
             source_file, output_obj);
    execute_command(cmd);
    printf("\n");
    
    /* Cleanup */
    printf("Cleaning up temporary files...\n");
    unlink(source_file);
    unlink(response_file);
    unlink(output_obj);
    unlink(output_exe);
    unlink("test_source.i");
    unlink("test_source.s");
    unlink("test_source.o");
    unlink("testdump.i");
    unlink("testdump.s");
    unlink("testdump.o");
    
    printf("\n=== Test completed ===\n");
    printf("Each GCC invocation should have triggered the reset logic in driver::finalize\n");
    
    return 0;
}
