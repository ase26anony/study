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
        perror("Failed to create source file");
        exit(1);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

/* Create a response file with various options */
void create_response_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create response file");
        exit(1);
    }
    fprintf(f, "-v\n");
    fprintf(f, "-save-temps=obj\n");
    fprintf(f, "-ftime-report\n");
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
    printf("Command failed to execute properly\n\n");
    return -1;
}

int main(int argc, char **argv) {
    char cmd[MAX_CMD_LEN];
    const char *base_name = "test_coverage";
    
    /* Create temporary files */
    char source_file[] = "/tmp/test_coverage_XXXXXX.c";
    char response_file[] = "/tmp/test_coverage_XXXXXX.rsp";
    char output_file[] = "/tmp/test_coverage_XXXXXX.o";
    char exe_file[] = "/tmp/test_coverage_XXXXXX.exe";
    
    /* Create unique temporary filenames */
    int fd = mkstemps(source_file, 2);  /* .c extension is 2 chars */
    close(fd);
    
    strcpy(response_file, source_file);
    strcpy(response_file + strlen(response_file) - 2, ".rsp");
    
    strcpy(output_file, source_file);
    strcpy(output_file + strlen(output_file) - 2, ".o");
    
    strcpy(exe_file, source_file);
    strcpy(exe_file + strlen(exe_file) - 2, ".exe");
    
    /* Create source and response files */
    create_source_file(source_file);
    create_response_file(response_file);
    
    printf("=== Testing GCC Driver Reset Logic ===\n\n");
    
    /* Invocation 1: Set print_help_list flag */
    printf("1. Testing -print-help-list (sets print_help_list):\n");
    snprintf(cmd, sizeof(cmd), "gcc -print-help-list 2>&1 | head -5");
    execute_command(cmd);
    
    /* Invocation 2: Set version flag */
    printf("2. Testing --version (sets print_version):\n");
    snprintf(cmd, sizeof(cmd), "gcc --version");
    execute_command(cmd);
    
    /* Invocation 3: Set verbose flag and use response file (sets at_file_supplied) */
    printf("3. Testing -v with response file (sets verbose_only_flag, at_file_supplied):\n");
    snprintf(cmd, sizeof(cmd), "gcc -v @%s -o %s %s 2>&1 | head -10", 
             response_file, output_file, source_file);
    execute_command(cmd);
    
    /* Invocation 4: Set save_temps flags and dumpdir variables */
    printf("4. Testing -save-temps variants (sets save_temps_flag, dumpdir, etc.):\n");
    snprintf(cmd, sizeof(cmd), "gcc -save-temps=cwd -dumpdir=./dump_ -dumpbase=testbase "
             "-o %s %s 2>&1 | head -5", output_file, source_file);
    execute_command(cmd);
    
    /* Invocation 5: Set sysroot and related variables */
    printf("5. Testing --sysroot (sets target_system_root_changed):\n");
    snprintf(cmd, sizeof(cmd), "gcc --sysroot=/tmp -isysroot /tmp -o %s %s 2>&1 | head -5", 
             exe_file, source_file);
    execute_command(cmd);
    
    /* Invocation 6: Set use_ld variable */
    printf("6. Testing -fuse-ld (sets use_ld):\n");
    snprintf(cmd, sizeof(cmd), "gcc -fuse-ld=bfd -o %s %s 2>&1 | head -5", 
             exe_file, source_file);
    execute_command(cmd);
    
    /* Invocation 7: Set report_times_to_file */
    printf("7. Testing -ftime-report (sets report_times_to_file):\n");
    snprintf(cmd, sizeof(cmd), "gcc -ftime-report -o %s %s 2>&1 | tail -10", 
             exe_file, source_file);
    execute_command(cmd);
    
    /* Invocation 8: Try to set spec_machine (architecture-specific) */
    printf("8. Testing -march/-mtune (may affect spec_machine):\n");
    snprintf(cmd, sizeof(cmd), "gcc -march=x86-64 -mtune=generic -c %s -o %s 2>&1 | head -5", 
             source_file, output_file);
    execute_command(cmd);
    
    /* Invocation 9: Force failure to set greatest_status != 1 */
    printf("9. Testing failure case (should set greatest_status != 1):\n");
    snprintf(cmd, sizeof(cmd), "gcc -invalid-option-xyz %s 2>&1 | head -5", source_file);
    execute_command(cmd);
    
    /* Invocation 10: Successful compilation after failure (tests reset) */
    printf("10. Testing successful compilation after failure (tests reset logic):\n");
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s", source_file, output_file);
    execute_command(cmd);
    
    /* Invocation 11: Multiple options combined */
    printf("11. Testing combined options (exercises multiple resets):\n");
    snprintf(cmd, sizeof(cmd), "gcc -v -save-temps=obj -ftime-report -fuse-ld=gold "
             "--sysroot=/ -o %s %s 2>&1 | tail -5", exe_file, source_file);
    execute_command(cmd);
    
    /* Clean up */
    unlink(source_file);
    unlink(response_file);
    unlink(output_file);
    unlink(exe_file);
    
    printf("=== Test completed ===\n");
    printf("The GCC driver's reset logic should have been exercised multiple times.\n");
    printf("Check coverage data to verify lines 11228-11250 were executed.\n");
    
    return 0;
}
