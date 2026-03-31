#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_CMD_LEN 4096

/* Create a minimal C source file */
void create_test_source(const char *filename) {
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

/* Execute a GCC command and return exit status */
int execute_gcc(const char *cmd) {
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
    char src_file[256];
    char resp_file[256];
    char output_file[256];
    char obj_file[256];
    
    snprintf(src_file, sizeof(src_file), "%s.c", base_name);
    snprintf(resp_file, sizeof(resp_file), "%s.rsp", base_name);
    snprintf(output_file, sizeof(output_file), "%s.exe", base_name);
    snprintf(obj_file, sizeof(obj_file), "%s.o", base_name);
    
    /* Create test source and response files */
    create_test_source(src_file);
    create_response_file(resp_file);
    
    printf("=== Testing GCC Driver Reset Logic ===\n\n");
    
    /* Invocation 1: Print help list - sets print_help_list */
    printf("1. Testing -print-help-list (sets print_help_list):\n");
    snprintf(cmd, sizeof(cmd), "gcc -print-help-list 2>&1 | head -5");
    execute_gcc(cmd);
    
    /* Invocation 2: Version - sets print_version */
    printf("2. Testing --version (sets print_version):\n");
    snprintf(cmd, sizeof(cmd), "gcc --version");
    execute_gcc(cmd);
    
    /* Invocation 3: Verbose only - sets verbose_only_flag */
    printf("3. Testing -v (sets verbose_only_flag):\n");
    snprintf(cmd, sizeof(cmd), "gcc -v %s 2>&1 | tail -5", src_file);
    execute_gcc(cmd);
    
    /* Invocation 4: Save temps with various options - sets save_temps_flag, dumpdir, etc. */
    printf("4. Testing -save-temps variants (sets save_temps_flag, dumpdir, dumpbase):\n");
    snprintf(cmd, sizeof(cmd), "gcc -save-temps=cwd -dumpdir=./dump -dumpbase=mydump -o %s %s", 
             output_file, src_file);
    execute_gcc(cmd);
    
    /* Invocation 5: Use response file - sets at_file_supplied */
    printf("5. Testing with response file (sets at_file_supplied):\n");
    snprintf(cmd, sizeof(cmd), "gcc @%s -o %s %s", resp_file, output_file, src_file);
    execute_gcc(cmd);
    
    /* Invocation 6: Set sysroot and related options - affects target_system_root */
    printf("6. Testing sysroot options (affects target_system_root):\n");
    snprintf(cmd, sizeof(cmd), "gcc --sysroot=/usr -isysroot=/usr/include -o %s %s", 
             output_file, src_file);
    execute_gcc(cmd);
    
    /* Invocation 7: Set linker - sets use_ld */
    printf("7. Testing -fuse-ld (sets use_ld):\n");
    snprintf(cmd, sizeof(cmd), "gcc -fuse-ld=bfd -o %s %s", output_file, src_file);
    execute_gcc(cmd);
    
    /* Invocation 8: Time report - sets report_times_to_file */
    printf("8. Testing -ftime-report (sets report_times_to_file):\n");
    snprintf(cmd, sizeof(cmd), "gcc -ftime-report -c %s -o %s", src_file, obj_file);
    execute_gcc(cmd);
    
    /* Invocation 9: Machine specification - attempts to set spec_machine */
    printf("9. Testing machine specification (attempts to set spec_machine):\n");
    snprintf(cmd, sizeof(cmd), "gcc -march=x86-64 -mtune=generic -o %s %s", 
             output_file, src_file);
    execute_gcc(cmd);
    
    /* Invocation 10: Force failure to set greatest_status != 1 */
    printf("10. Testing failure case (should set greatest_status != 1):\n");
    snprintf(cmd, sizeof(cmd), "gcc -invalid-option %s 2>&1 | head -3", src_file);
    execute_gcc(cmd);
    
    /* Invocation 11: Successful compilation after failure */
    printf("11. Testing successful compilation after failure (resets greatest_status):\n");
    snprintf(cmd, sizeof(cmd), "gcc -o %s %s", output_file, src_file);
    execute_gcc(cmd);
    
    /* Invocation 12: Complex combination of many options */
    printf("12. Testing complex combination (exercises multiple reset variables):\n");
    snprintf(cmd, sizeof(cmd), 
             "gcc -v -save-temps=obj -ftime-report -fuse-ld=gold --sysroot=/usr "
             "-dumpdir=./testdump -dumpbase=complex -o %s %s && "
             "gcc -v -c %s -o %s",  /* Two commands in sequence */
             output_file, src_file, src_file, obj_file);
    execute_gcc(cmd);
    
    /* Cleanup */
    printf("Cleaning up temporary files...\n");
    unlink(src_file);
    unlink(resp_file);
    unlink(output_file);
    unlink(obj_file);
    
    /* Also clean up any save-temps files that might have been created */
    char temp_files[][50] = {
        "test_coverage.i", "test_coverage.ii", "test_coverage.s",
        "test_coverage.o", "mydump.*", "./dump/*"
    };
    
    for (int i = 0; i < sizeof(temp_files)/sizeof(temp_files[0]); i++) {
        snprintf(cmd, sizeof(cmd), "rm -f %s 2>/dev/null", temp_files[i]);
        system(cmd);
    }
    
    printf("\n=== Test completed ===\n");
    printf("The GCC driver's reset logic should have been exercised multiple times.\n");
    printf("Check coverage reports to verify lines 11228-11250 in gcc.cc are now covered.\n");
    
    return 0;
}
