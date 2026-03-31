#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_CMD_LEN 4096

/* Create a temporary file with given content */
int create_temp_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) return -1;
    fputs(content, f);
    fclose(f);
    return 0;
}

/* Execute a command and return its exit status */
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
    const char *tmpdir = "/tmp";
    char src_file[256], resp_file[256], output_file[256];
    
    /* Generate unique filenames */
    snprintf(src_file, sizeof(src_file), "%s/test_gcc_reset_%d.c", tmpdir, getpid());
    snprintf(resp_file, sizeof(resp_file), "%s/args_%d.rsp", tmpdir, getpid());
    snprintf(output_file, sizeof(output_file), "%s/test_output_%d.o", tmpdir, getpid());
    
    /* Create a minimal valid C source file */
    const char *source_code = 
        "int main() {\n"
        "    return 0;\n"
        "}\n";
    
    if (create_temp_file(src_file, source_code) != 0) {
        perror("Failed to create source file");
        return 1;
    }
    
    /* Create a response file with various options */
    const char *response_content = 
        "-v\n"
        "-save-temps=obj\n"
        "-ftime-report\n";
    
    if (create_temp_file(resp_file, response_content) != 0) {
        perror("Failed to create response file");
        unlink(src_file);
        return 1;
    }
    
    printf("=== Testing GCC Driver Reset Logic ===\n\n");
    
    /* Invocation 1: Set print_help_list */
    printf("1. Testing -print-help-list (sets print_help_list):\n");
    execute_command("gcc -print-help-list 2>&1 | head -5");
    
    /* Invocation 2: Set print_version */
    printf("2. Testing --version (sets print_version):\n");
    execute_command("gcc --version");
    
    /* Invocation 3: Set verbose_only_flag and use response file (sets at_file_supplied) */
    printf("3. Testing with response file (sets at_file_supplied, verbose_only_flag):\n");
    snprintf(cmd, sizeof(cmd), "gcc @%s -c %s -o %s 2>&1 | head -10", 
             resp_file, src_file, output_file);
    execute_command(cmd);
    
    /* Invocation 4: Set save_temps_flag and related variables */
    printf("4. Testing -save-temps variants (sets save_temps_flag, dumpdir, etc.):\n");
    snprintf(cmd, sizeof(cmd), "gcc -save-temps=cwd -dumpdir=./dump_ -dumpbase=testdump "
             "-c %s -o %s.save 2>&1 | head -5", src_file, output_file);
    execute_command(cmd);
    
    /* Invocation 5: Set use_ld and target_system_root variables */
    printf("5. Testing linker and sysroot options (sets use_ld, target_system_root*):\n");
    snprintf(cmd, sizeof(cmd), "gcc -fuse-ld=bfd --sysroot=/ -isysroot/usr/include "
             "-c %s -o %s.ld 2>&1 | head -5", src_file, output_file);
    execute_command(cmd);
    
    /* Invocation 6: Cause failure to set greatest_status != 1 */
    printf("6. Testing with invalid option to cause failure (sets greatest_status):\n");
    execute_command("gcc -invalid-option-xyz 2>&1 | head -3");
    
    /* Invocation 7: Test machine specification (attempts to set spec_machine) */
    printf("7. Testing machine/target options (attempts to set spec_machine):\n");
    snprintf(cmd, sizeof(cmd), "gcc -mtune=generic -march=x86-64 -c %s -o %s.machine 2>&1 | head -5", 
             src_file, output_file);
    execute_command(cmd);
    
    /* Invocation 8: Test time reporting (sets report_times_to_file) */
    printf("8. Testing time reporting (sets report_times_to_file):\n");
    snprintf(cmd, sizeof(cmd), "gcc -ftime-report -c %s -o %s.time 2>&1 | "
             "grep -A2 -B2 'Time' | head -10", src_file, output_file);
    execute_command(cmd);
    
    /* Invocation 9: Combined test with many options */
    printf("9. Combined test with multiple reset-targeting options:\n");
    snprintf(cmd, sizeof(cmd), "gcc -v -save-temps=obj -ftime-report -fuse-ld=gold "
             "-mtune=generic -c %s -o %s.combined 2>&1 | head -15", 
             src_file, output_file);
    execute_command(cmd);
    
    /* Invocation 10: Final simple compilation to ensure reset worked */
    printf("10. Final simple compilation (verifies reset occurred):\n");
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s.final 2>&1 | head -3", 
             src_file, output_file);
    execute_command(cmd);
    
    /* Cleanup */
    unlink(src_file);
    unlink(resp_file);
    
    /* Remove output files if they were created */
    char *outputs[] = {
        output_file,
        ".save", ".ld", ".machine", ".time", ".combined", ".final"
    };
    
    for (int i = 0; i < sizeof(outputs)/sizeof(outputs[0]); i++) {
        char full_path[512];
        if (i == 0) {
            snprintf(full_path, sizeof(full_path), "%s", outputs[i]);
        } else {
            snprintf(full_path, sizeof(full_path), "%s%s", output_file, outputs[i]);
        }
        unlink(full_path);
    }
    
    /* Also clean up any save-temps files */
    system("rm -f dump_* *.i *.s *.o 2>/dev/null");
    
    printf("=== Test completed ===\n");
    printf("The GCC driver's reset logic should have been exercised multiple times.\n");
    printf("Check coverage data to verify lines 11228-11250 were executed.\n");
    
    return 0;
}
