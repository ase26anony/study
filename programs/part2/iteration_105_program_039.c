#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 1024

void create_temp_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (f) {
        fprintf(f, "%s", content);
        fclose(f);
    }
}

int run_gcc(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    printf("Exit status: %d\n\n", WEXITSTATUS(status));
    return status;
}

int main(int argc, char **argv) {
    char cmd[MAX_CMD_LEN];
    const char *temp_prefix = "gcc_reset_test_";
    
    // Create a minimal valid C source file
    const char *source_content = "int main() { return 0; }\n";
    create_temp_file("test_source.c", source_content);
    
    // Create a response file with various options
    const char *response_content = "-v\n-save-temps=obj\n-O2\n";
    create_temp_file("args.rsp", response_content);
    
    printf("=== Testing GCC Driver Reset Logic ===\n\n");
    
    // Invocation 1: Set print_help_list
    run_gcc("gcc -print-help-list 2>&1 | head -5");
    
    // Invocation 2: Set version flag
    run_gcc("gcc --version");
    
    // Invocation 3: Set verbose flag and use response file (triggers at_file_supplied)
    // This will likely fail because dummy.c doesn't exist, affecting greatest_status
    run_gcc("gcc @args.rsp dummy_nonexistent.c -o dummy.o 2>&1 | tail -3");
    
    // Invocation 4: Set save_temps_flag and related variables
    run_gcc("gcc -save-temps -dumpdir=./dump -dumpbase=test -dumpbase-ext=.c "
            "-o test_output test_source.c 2>&1 | grep -i 'save-temps'");
    
    // Invocation 5: Set use_ld and report_times_to_file
    run_gcc("gcc -fuse-ld=gold -ftime-report -c test_source.c 2>&1 | "
            "grep -E '(gold|Time)' | head -2");
    
    // Invocation 6: Set target_system_root related variables
    // Using -B option to add a (possibly non-existent) path to search
    run_gcc("gcc -B/tmp/nonexistent_path --sysroot=/ -c test_source.c 2>&1 | "
            "grep -i 'sysroot' | head -2");
    
    // Invocation 7: Try to set spec_machine (may be target-specific)
    // Different architectures to potentially affect spec_machine
    run_gcc("gcc -march=x86-64 -mtune=generic -c test_source.c 2>&1");
    run_gcc("gcc -march=armv8-a -c test_source.c 2>&1 | grep -i 'arm' | head -1");
    
    // Invocation 8: Combination of many flags
    run_gcc("gcc -v -save-temps=obj -ftime-report -fuse-ld=bfd "
            "-o final_output test_source.c 2>&1 | tail -5");
    
    // Invocation 9: Print subprocess help
    run_gcc("gcc -print-prog-name=cc1 2>&1");
    
    // Invocation 10: Verbose only flag
    run_gcc("gcc -### -E test_source.c 2>&1 | head -3");
    
    // Cleanup
    remove("test_source.c");
    remove("args.rsp");
    remove("test_output");
    remove("test_source.i");
    remove("test_source.s");
    remove("test_source.o");
    remove("final_output");
    
    printf("=== Test completed ===\n");
    printf("The GCC driver's reset logic should have been exercised multiple times.\n");
    printf("Check coverage data to verify lines 11228-11250 were executed.\n");
    
    return 0;
}
