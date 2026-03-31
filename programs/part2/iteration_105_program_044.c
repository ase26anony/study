#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_CMD_LEN 4096
#define TEMP_DIR "/tmp/gcc_test_XXXXXX"

// Create a minimal C source file
void create_source_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (f) {
        fprintf(f, "int main() { return 0; }\n");
        fclose(f);
    }
}

// Create a response file with various options
void create_response_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (f) {
        fprintf(f, "-v\n");
        fprintf(f, "-save-temps=obj\n");
        fprintf(f, "-ftime-report\n");
        fprintf(f, "-fuse-ld=gold\n");
        fclose(f);
    }
}

// Execute a command and return exit status
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
    char temp_dir[256];
    char source_file[512];
    char response_file[512];
    char output_file[512];
    char cmd[MAX_CMD_LEN];
    int overall_status = 0;
    
    // Create temporary directory
    strcpy(temp_dir, TEMP_DIR);
    if (mkdtemp(temp_dir) == NULL) {
        perror("mkdtemp failed");
        return 1;
    }
    
    // Create file paths
    snprintf(source_file, sizeof(source_file), "%s/test.c", temp_dir);
    snprintf(response_file, sizeof(response_file), "%s/args.rsp", temp_dir);
    snprintf(output_file, sizeof(output_file), "%s/test.exe", temp_dir);
    
    // Create test files
    create_source_file(source_file);
    create_response_file(response_file);
    
    printf("=== Testing GCC Driver Reset Logic ===\n\n");
    
    // Invocation 1: Set print_help_list
    printf("1. Testing -print-help-list (sets print_help_list):\n");
    execute_command("gcc -print-help-list 2>&1 | head -5");
    
    // Invocation 2: Set print_version
    printf("2. Testing --version (sets print_version):\n");
    execute_command("gcc --version");
    
    // Invocation 3: Set verbose_only_flag and use response file (at_file_supplied)
    printf("3. Testing verbose + response file (sets verbose_only_flag, at_file_supplied):\n");
    snprintf(cmd, sizeof(cmd), "gcc -v @%s -c %s -o %s.o 2>&1 | head -10", 
             response_file, source_file, temp_dir);
    execute_command(cmd);
    
    // Invocation 4: Set save_temps_flag and related variables
    printf("4. Testing -save-temps variants (sets save_temps_flag, dumpdir, etc.):\n");
    snprintf(cmd, sizeof(cmd), "gcc -save-temps=cwd -dumpdir %s/ -dumpbase test -o %s %s 2>&1 | head -5",
             temp_dir, output_file, source_file);
    execute_command(cmd);
    
    // Invocation 5: Set use_ld and report_times_to_file
    printf("5. Testing -fuse-ld and -ftime-report (sets use_ld, report_times_to_file):\n");
    snprintf(cmd, sizeof(cmd), "gcc -fuse-ld=bfd -ftime-report -o %s_bfd %s 2>&1 | head -15",
             temp_dir, source_file);
    execute_command(cmd);
    
    // Invocation 6: Attempt to set target system root (may require root or specific path)
    printf("6. Testing --sysroot (influences target_system_root):\n");
    // Try with a dummy sysroot - this will likely fail but still exercise the code
    snprintf(cmd, sizeof(cmd), "gcc --sysroot=/nonexistent/path -c %s -o %s/nonexist.o 2>&1 | head -5",
             source_file, temp_dir);
    execute_command(cmd);
    
    // Invocation 7: Cause compilation failure to set greatest_status != 1
    printf("7. Testing compilation failure (sets greatest_status != 1):\n");
    execute_command("gcc -c /nonexistent/file.c -o /tmp/nonexist.o 2>&1 | head -5");
    
    // Invocation 8: Successful compilation after failure (should reset greatest_status)
    printf("8. Testing successful compilation after failure (resets state):\n");
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s/final.o", source_file, temp_dir);
    execute_command(cmd);
    
    // Invocation 9: Test machine-specific options (attempt to set spec_machine)
    printf("9. Testing machine-specific options (influences spec_machine):\n");
    // Try various architecture options
    const char *archs[] = {"-march=x86-64", "-march=native", "-mtune=generic"};
    for (int i = 0; i < 3; i++) {
        snprintf(cmd, sizeof(cmd), "gcc %s -c %s -o %s/arch%d.o 2>&1 | head -3",
                 archs[i], source_file, temp_dir, i);
        execute_command(cmd);
    }
    
    // Invocation 10: Test print_subprocess_help
    printf("10. Testing subprocess help (sets print_subprocess_help):\n");
    execute_command("gcc -### -E - 2>&1 | head -5");
    
    // Cleanup
    printf("Cleaning up temporary files...\n");
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    system(cmd);
    
    printf("\n=== Test Complete ===\n");
    printf("The GCC driver's reset logic should have been exercised multiple times.\n");
    printf("Check coverage data to verify lines 11228-11250 were executed.\n");
    
    return overall_status;
}
