#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_CMD_LEN 4096

/* Helper function to execute a command and return its exit status */
static int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Helper to create a temporary file with given content */
static int create_temp_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return -1;
    }
    fputs(content, f);
    fclose(f);
    return 0;
}

int main(int argc, char **argv) {
    char cmd[MAX_CMD_LEN];
    int status;
    const char *base_name = "test_gcc_reset";
    
    /* Create a minimal valid C source file */
    const char *source_content = 
        "int main(void) {\n"
        "    return 0;\n"
        "}\n";
    
    if (create_temp_file("test_source.c", source_content) < 0) {
        fprintf(stderr, "Failed to create source file\n");
        return 1;
    }
    
    /* Create a response file with various options */
    const char *response_content = 
        "-v\n"
        "-save-temps=obj\n"
        "-Wall\n"
        "-Wextra\n";
    
    if (create_temp_file("args.rsp", response_content) < 0) {
        fprintf(stderr, "Failed to create response file\n");
        unlink("test_source.c");
        return 1;
    }
    
    printf("=== Testing GCC driver reset logic ===\n\n");
    
    /* Invocation 1: Set print_help_list */
    printf("1. Testing -print-help-list (sets print_help_list)\n");
    snprintf(cmd, sizeof(cmd), "gcc -print-help-list 2>&1 | head -5");
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 2: Set print_version */
    printf("2. Testing --version (sets print_version)\n");
    snprintf(cmd, sizeof(cmd), "gcc --version");
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 3: Set verbose_only_flag and use response file (at_file_supplied) */
    printf("3. Testing -v with response file (sets verbose_only_flag, at_file_supplied)\n");
    snprintf(cmd, sizeof(cmd), "gcc -v @args.rsp test_source.c -o test_output1 2>&1 | tail -3");
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 4: Set save_temps_flag and related variables */
    printf("4. Testing -save-temps variants (sets save_temps_flag, dumpdir, etc.)\n");
    snprintf(cmd, sizeof(cmd), "gcc -save-temps=cwd test_source.c -o test_output2");
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 5: Set use_ld and report_times_to_file */
    printf("5. Testing -fuse-ld and -ftime-report (sets use_ld, report_times_to_file)\n");
    snprintf(cmd, sizeof(cmd), "gcc -fuse-ld=bfd -ftime-report test_source.c -o test_output3 2>&1 | grep -i 'time'");
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 6: Attempt to set target_system_root (may need sudo or specific path) */
    printf("6. Testing --sysroot (affects target_system_root)\n");
    snprintf(cmd, sizeof(cmd), "gcc --sysroot=/ test_source.c -o test_output4 2>&1 | head -2");
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 7: Cause compilation failure to set greatest_status != 1 */
    printf("7. Testing with invalid file (should fail, sets greatest_status != 1)\n");
    snprintf(cmd, sizeof(cmd), "gcc -c non_existent_file.c -o test_output5 2>&1 | head -2");
    status = execute_command(cmd);
    printf("Exit status: %d\n\n", status);
    
    /* Invocation 8: Successful compilation after failure (tests reset) */
    printf("8. Testing successful compilation after failure (tests reset of greatest_status)\n");
    snprintf(cmd, sizeof(cmd), "gcc -c test_source.c -o test_output6.o");
    status = execute_command(cmd);
    printf("Exit status: %d\n\n", status);
    
    /* Invocation 9: Test spec_machine variations via -march/-mtune */
    printf("9. Testing -march variants (affects spec_machine)\n");
    const char *arches[] = {"x86-64", "native", "haswell", "skylake"};
    for (int i = 0; i < 4; i++) {
        snprintf(cmd, sizeof(cmd), "gcc -march=%s -c test_source.c -o test_output7_%d.o 2>&1 | head -1", 
                 arches[i], i);
        execute_command(cmd);
    }
    printf("\n");
    
    /* Invocation 10: Complex combination of many options */
    printf("10. Testing complex option combination\n");
    snprintf(cmd, sizeof(cmd), 
             "gcc -v -save-temps=obj -ftime-report -fuse-ld=gold -Wall "
             "-march=x86-64 -mtune=generic test_source.c -o test_output8 2>&1 | tail -5");
    execute_command(cmd);
    
    /* Cleanup */
    printf("\n=== Cleaning up temporary files ===\n");
    unlink("test_source.c");
    unlink("args.rsp");
    
    /* Remove generated files if they exist */
    char *files_to_remove[] = {
        "test_output1", "test_output2", "test_output3", "test_output4",
        "test_output5", "test_output6.o", "test_output8",
        "test_source.i", "test_source.s", "test_source.o",
        "test_output1.i", "test_output1.s", "test_output1.o",
        "test_output2.i", "test_output2.s", "test_output2.o",
        "test_output3.i", "test_output3.s", "test_output3.o",
        NULL
    };
    
    for (int i = 0; files_to_remove[i] != NULL; i++) {
        if (access(files_to_remove[i], F_OK) == 0) {
            unlink(files_to_remove[i]);
            printf("Removed: %s\n", files_to_remove[i]);
        }
    }
    
    /* Remove numbered output files */
    for (int i = 0; i < 8; i++) {
        snprintf(cmd, sizeof(cmd), "test_output7_%d.o", i);
        if (access(cmd, F_OK) == 0) {
            unlink(cmd);
        }
    }
    
    printf("\n=== Test completed ===\n");
    return 0;
}
