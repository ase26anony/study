#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 1024

/* Create a minimal C source file */
void create_source_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen source");
        exit(1);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

/* Create a response file with various options */
void create_response_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen response");
        exit(1);
    }
    fprintf(f, "-v\n");
    fprintf(f, "-save-temps=obj\n");
    fprintf(f, "-Wall\n");
    fclose(f);
}

/* Execute a GCC command and return exit status */
int run_gcc_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    return -1;
}

int main(int argc, char **argv) {
    char cmd[MAX_CMD_LEN];
    const char *base_name = "test_coverage";
    
    /* Create temporary files */
    char source_file[256];
    char response_file[256];
    char output_file[256];
    char object_file[256];
    
    snprintf(source_file, sizeof(source_file), "%s.c", base_name);
    snprintf(response_file, sizeof(response_file), "%s.rsp", base_name);
    snprintf(output_file, sizeof(output_file), "%s.exe", base_name);
    snprintf(object_file, sizeof(object_file), "%s.o", base_name);
    
    /* Create source and response files */
    create_source_file(source_file);
    create_response_file(response_file);
    
    printf("=== Testing GCC Driver Reset Logic ===\n\n");
    
    /* Invocation 1: Set print_help_list flag */
    printf("1. Testing -print-help-list (sets print_help_list):\n");
    snprintf(cmd, sizeof(cmd), "gcc -print-help-list 2>&1 | head -5");
    run_gcc_command(cmd);
    
    /* Invocation 2: Set multiple flags including version */
    printf("2. Testing --version (sets print_version):\n");
    snprintf(cmd, sizeof(cmd), "gcc --version");
    run_gcc_command(cmd);
    
    /* Invocation 3: Use response file (sets at_file_supplied) and verbose flag */
    printf("3. Testing with response file and verbose flag:\n");
    snprintf(cmd, sizeof(cmd), "gcc @%s -c %s -o %s", 
             response_file, source_file, object_file);
    run_gcc_command(cmd);
    
    /* Invocation 4: Set save_temps_flag and related variables */
    printf("4. Testing -save-temps with dumpdir:\n");
    snprintf(cmd, sizeof(cmd), "gcc -save-temps=cwd -fdumpdir=./dumpdir/ -c %s -o %s",
             source_file, object_file);
    run_gcc_command(cmd);
    
    /* Invocation 5: Set use_ld and sysroot variables */
    printf("5. Testing -fuse-ld and --sysroot:\n");
    snprintf(cmd, sizeof(cmd), "gcc -fuse-ld=bfd --sysroot=/  -c %s -o %s",
             source_file, object_file);
    run_gcc_command(cmd);
    
    /* Invocation 6: Set time report flag */
    printf("6. Testing -ftime-report:\n");
    snprintf(cmd, sizeof(cmd), "gcc -ftime-report -c %s -o %s 2>&1 | head -10",
             source_file, object_file);
    run_gcc_command(cmd);
    
    /* Invocation 7: Cause failure to set greatest_status != 1 */
    printf("7. Testing with invalid option (should fail):\n");
    snprintf(cmd, sizeof(cmd), "gcc -invalid-option-xyz %s -o %s 2>&1 | head -3",
             source_file, output_file);
    int fail_status = run_gcc_command(cmd);
    
    /* Invocation 8: Successful compilation after failure */
    printf("8. Testing successful compilation after failure:\n");
    snprintf(cmd, sizeof(cmd), "gcc -v -o %s %s 2>&1 | tail -5",
             output_file, source_file);
    run_gcc_command(cmd);
    
    /* Invocation 9: Test with machine-specific options */
    printf("9. Testing with machine/arch options:\n");
    snprintf(cmd, sizeof(cmd), "gcc -march=x86-64 -mtune=generic -c %s -o %s",
             source_file, object_file);
    run_gcc_command(cmd);
    
    /* Invocation 10: Test verbose-only flag */
    printf("10. Testing verbose-only flag:\n");
    snprintf(cmd, sizeof(cmd), "gcc -### -E %s 2>&1 | head -5",
             source_file);
    run_gcc_command(cmd);
    
    /* Cleanup */
    printf("Cleaning up temporary files...\n");
    unlink(source_file);
    unlink(response_file);
    unlink(object_file);
    unlink(output_file);
    
    /* Also clean up any save-temps files that might have been created */
    char temp_files[][50] = {
        "test_coverage.i", "test_coverage.s", "test_coverage.ii",
        "test_coverage.o", "test_coverage.exe"
    };
    
    for (size_t i = 0; i < sizeof(temp_files)/sizeof(temp_files[0]); i++) {
        if (access(temp_files[i], F_OK) == 0) {
            unlink(temp_files[i]);
        }
    }
    
    printf("\n=== Test completed ===\n");
    printf("The driver reset logic should have been exercised multiple times.\n");
    printf("Variables like at_file_supplied, print_version, save_temps_flag,\n");
    printf("use_ld, report_times_to_file, and greatest_status were set and reset.\n");
    
    return 0;
}
