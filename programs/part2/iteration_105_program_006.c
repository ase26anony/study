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

/* Helper to create a temporary file with given content */
static void create_temp_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (f) {
        fprintf(f, "%s", content);
        fclose(f);
    }
}

int main(int argc, char **argv) {
    char cmd[MAX_CMD_LEN];
    int status;
    
    /* Create a minimal valid C source file */
    const char *source_content = 
        "int main() {\n"
        "    return 0;\n"
        "}\n";
    
    create_temp_file("test_source.c", source_content);
    
    /* Create a response file with various options */
    const char *response_content = 
        "-v\n"
        "-save-temps=obj\n"
        "-Wall\n";
    
    create_temp_file("args.rsp", response_content);
    
    /* Create a dummy file that doesn't exist for failure case */
    const char *dummy_content = 
        "invalid C code\n"
        "this will cause compilation to fail\n";
    
    create_temp_file("dummy_fail.c", dummy_content);
    
    printf("=== Starting GCC driver reset logic test ===\n\n");
    
    /* Invocation 1: Set print_help_list flag */
    printf("--- Invocation 1: -print-help-list ---\n");
    snprintf(cmd, sizeof(cmd), "gcc -print-help-list 2>&1 | head -5");
    status = execute_command(cmd);
    printf("Exit status: %d\n\n", status);
    
    /* Invocation 2: Use response file and multiple flags, will fail */
    printf("--- Invocation 2: Response file with failure ---\n");
    snprintf(cmd, sizeof(cmd), "gcc -v -save-temps=obj -o test.o @args.rsp dummy_fail.c 2>&1 | head -10");
    status = execute_command(cmd);
    printf("Exit status: %d (should be non-zero)\n\n", status);
    
    /* Invocation 3: Set multiple flags including sysroot and time report */
    printf("--- Invocation 3: Multiple flags with valid source ---\n");
    snprintf(cmd, sizeof(cmd), "gcc -fuse-ld=gold -ftime-report -o test.exe test_source.c 2>&1 | tail -20");
    status = execute_command(cmd);
    printf("Exit status: %d\n\n", status);
    
    /* Invocation 4: Attempt to set spec_machine (architecture flags) */
    printf("--- Invocation 4: Architecture/machine flags ---\n");
    snprintf(cmd, sizeof(cmd), "gcc -march=x86-64 -mtune=generic -c test_source.c -o test.o 2>&1");
    status = execute_command(cmd);
    printf("Exit status: %d\n\n", status);
    
    /* Invocation 5: Version flag */
    printf("--- Invocation 5: Version flag ---\n");
    snprintf(cmd, sizeof(cmd), "gcc --version 2>&1 | head -2");
    status = execute_command(cmd);
    printf("Exit status: %d\n\n", status);
    
    /* Invocation 6: Save temps with different options */
    printf("--- Invocation 6: Different save-temps options ---\n");
    snprintf(cmd, sizeof(cmd), "gcc -save-temps=cwd -c test_source.c 2>&1");
    status = execute_command(cmd);
    printf("Exit status: %d\n\n", status);
    
    /* Invocation 7: Sysroot options */
    printf("--- Invocation 7: Sysroot options ---\n");
    snprintf(cmd, sizeof(cmd), "gcc --sysroot=/ -isysroot=/usr/include -c test_source.c 2>&1");
    status = execute_command(cmd);
    printf("Exit status: %d\n\n", status);
    
    /* Invocation 8: Verbose only */
    printf("--- Invocation 8: Verbose flag ---\n");
    snprintf(cmd, sizeof(cmd), "gcc -v -c test_source.c 2>&1 | grep -i 'gcc version'");
    status = execute_command(cmd);
    printf("Exit status: %d\n\n", status);
    
    /* Invocation 9: Combination that should succeed */
    printf("--- Invocation 9: Final successful compilation ---\n");
    snprintf(cmd, sizeof(cmd), "gcc -O2 -o final_test test_source.c 2>&1");
    status = execute_command(cmd);
    printf("Exit status: %d (should be 0)\n\n", status);
    
    /* Cleanup */
    printf("=== Cleaning up temporary files ===\n");
    remove("test_source.c");
    remove("args.rsp");
    remove("dummy_fail.c");
    remove("test.o");
    remove("test.exe");
    remove("final_test");
    remove("test_source.i");
    remove("test_source.s");
    
    printf("Test completed. Check coverage of gcc.cc lines 11228-11250.\n");
    
    return 0;
}
