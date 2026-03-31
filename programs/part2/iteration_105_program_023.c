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

/* Execute a command and return exit status */
int execute_command(const char *cmd) {
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
    char source_file[] = "/tmp/test_coverage_XXXXXX.c";
    char resp_file[] = "/tmp/test_coverage_XXXXXX.rsp";
    char output_file[] = "/tmp/test_coverage_XXXXXX.o";
    char exe_file[] = "/tmp/test_coverage_XXXXXX.exe";
    
    /* Create unique temporary filenames */
    int fd = mkstemps(source_file, 2);  /* .c extension */
    close(fd);
    
    strcpy(resp_file, source_file);
    resp_file[strlen(resp_file)-2] = '\0';  /* Remove .c */
    strcat(resp_file, ".rsp");
    
    strcpy(output_file, source_file);
    output_file[strlen(output_file)-2] = '\0';  /* Remove .c */
    strcat(output_file, ".o");
    
    strcpy(exe_file, source_file);
    exe_file[strlen(exe_file)-2] = '\0';  /* Remove .c */
    strcat(exe_file, ".exe");
    
    /* Create source and response files */
    create_source_file(source_file);
    create_response_file(resp_file);
    
    printf("=== Testing GCC Driver Reset Logic ===\n\n");
    
    /* Invocation 1: Set print_help_list */
    printf("1. Testing -print-help-list (sets print_help_list):\n");
    snprintf(cmd, sizeof(cmd), "gcc -print-help-list 2>&1 | head -5");
    execute_command(cmd);
    
    /* Invocation 2: Set print_version */
    printf("2. Testing --version (sets print_version):\n");
    snprintf(cmd, sizeof(cmd), "gcc --version");
    execute_command(cmd);
    
    /* Invocation 3: Set verbose_only_flag and use response file (at_file_supplied) */
    printf("3. Testing -v with response file (sets verbose_only_flag, at_file_supplied):\n");
    snprintf(cmd, sizeof(cmd), "gcc -v @%s -c %s -o %s 2>&1 | head -10", 
             resp_file, source_file, output_file);
    execute_command(cmd);
    
    /* Invocation 4: Set save_temps_flag and related dump variables */
    printf("4. Testing -save-temps variants (sets save_temps_flag, dumpdir/dumpbase):\n");
    snprintf(cmd, sizeof(cmd), "gcc -save-temps=cwd -c %s -o %s", 
             source_file, output_file);
    execute_command(cmd);
    
    /* Invocation 5: Set use_ld and report_times_to_file */
    printf("5. Testing -fuse-ld and -ftime-report (sets use_ld, report_times_to_file):\n");
    snprintf(cmd, sizeof(cmd), "gcc -fuse-ld=bfd -ftime-report -c %s -o %s 2>&1 | head -5", 
             source_file, output_file);
    execute_command(cmd);
    
    /* Invocation 6: Attempt to set spec_machine (may be target-specific) */
    printf("6. Testing -machine option (attempts to set spec_machine):\n");
    snprintf(cmd, sizeof(cmd), "gcc -dumpmachine");
    execute_command(cmd);
    
    /* Invocation 7: Cause failure to set greatest_status != 1 */
    printf("7. Testing with invalid option to cause failure (sets greatest_status != 1):\n");
    snprintf(cmd, sizeof(cmd), "gcc -invalid-option-xyz %s 2>&1 | head -3", source_file);
    execute_command(cmd);
    
    /* Invocation 8: Successful compilation after failure (tests reset) */
    printf("8. Testing successful compilation after failure (tests reset logic):\n");
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s", source_file, output_file);
    execute_command(cmd);
    
    /* Invocation 9: Test sysroot options (affects target_system_root) */
    printf("9. Testing sysroot options (affects target_system_root):\n");
    snprintf(cmd, sizeof(cmd), "gcc --sysroot=/ -c %s -o %s 2>&1 | head -3", 
             source_file, output_file);
    execute_command(cmd);
    
    /* Invocation 10: Complex combination */
    printf("10. Testing complex combination of options:\n");
    snprintf(cmd, sizeof(cmd), "gcc -v -save-temps=obj -ftime-report -fuse-ld=gold -c %s -o %s 2>&1 | head -8", 
             source_file, output_file);
    execute_command(cmd);
    
    /* Cleanup */
    unlink(source_file);
    unlink(resp_file);
    unlink(output_file);
    unlink(exe_file);
    
    /* Also clean up any save-temps files that might have been created */
    char temp_files[][50] = {"test_coverage.i", "test_coverage.s", "test_coverage.o"};
    for (int i = 0; i < 3; i++) {
        if (access(temp_files[i], F_OK) == 0) {
            unlink(temp_files[i]);
        }
    }
    
    printf("\n=== Test completed ===\n");
    printf("The GCC driver's reset logic should have been exercised multiple times.\n");
    printf("Check coverage for lines 11228-11250 in gcc.cc\n");
    
    return 0;
}
