#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TEMP_SOURCE "test_reset_logic.c"
#define RESPONSE_FILE "args.rsp"

void create_test_source() {
    FILE *f = fopen(TEMP_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(1);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

void create_response_file() {
    FILE *f = fopen(RESPONSE_FILE, "w");
    if (!f) {
        perror("Failed to create response file");
        exit(1);
    }
    // Options that set various driver state variables
    fprintf(f, "-v\n");
    fprintf(f, "-save-temps=obj\n");
    fprintf(f, "-ftime-report\n");
    fclose(f);
}

int run_gcc(const char *args) {
    printf("Running: gcc %s\n", args);
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "gcc %s", args);
        int ret = system(cmd);
        exit(WEXITSTATUS(ret));
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    } else {
        perror("fork failed");
        return -1;
    }
}

void cleanup() {
    // Clean up temporary files
    remove(TEMP_SOURCE);
    remove(RESPONSE_FILE);
    
    // Clean up potential output files from -save-temps
    remove("test.o");
    remove("test_reset_logic.o");
    remove("test_reset_logic.i");
    remove("test_reset_logic.s");
    remove("test.exe");
    remove("test");
    remove("a.out");
}

int main(int argc, char **argv) {
    printf("=== Testing GCC driver reset logic ===\n\n");
    
    // Create test files
    create_test_source();
    create_response_file();
    
    int overall_status = 0;
    
    // 1. Set print_help_list flag
    printf("\n--- Invocation 1: -print-help-list ---\n");
    run_gcc("-print-help-list 2>&1 | head -5");
    
    // 2. Set print_version flag
    printf("\n--- Invocation 2: --version ---\n");
    run_gcc("--version");
    
    // 3. Set verbose_only_flag and use response file (at_file_supplied)
    // Also trigger failure with non-existent file to affect greatest_status
    printf("\n--- Invocation 3: Verbose with response file and failure ---\n");
    int status3 = run_gcc("-v @args.rsp -o test.o nonexistent.c 2>&1 | tail -3");
    printf("Exit status: %d (should be non-zero)\n", status3);
    
    // 4. Set save_temps_flag and related variables
    printf("\n--- Invocation 4: -save-temps with various options ---\n");
    run_gcc("-save-temps -dumpdir=./dump -dumpbase=mydump -o test_reset_logic.o -c " TEMP_SOURCE);
    
    // 5. Set use_ld, sysroot, and time report
    printf("\n--- Invocation 5: Linker, sysroot and timing options ---\n");
    // Note: --sysroot path may not exist, but that's OK for testing driver logic
    run_gcc("-fuse-ld=gold --sysroot=/tmp/mock-sysroot -ftime-report -o test.exe " TEMP_SOURCE " 2>&1 | grep -E '(sysroot|ld|time)' | head -3");
    
    // 6. Attempt to set spec_machine (architecture-specific)
    printf("\n--- Invocation 6: Machine/architecture options ---\n");
    // Try various machine/arch options that might affect spec_machine
    run_gcc("-march=x86-64 -mtune=generic -c " TEMP_SOURCE);
    run_gcc("-march=armv7-a -c " TEMP_SOURCE " 2>&1 | grep -i 'note:' | head -2 || true");
    
    // 7. Test with save-temps variants
    printf("\n--- Invocation 7: Different save-temps modes ---\n");
    run_gcc("-save-temps=cwd -c " TEMP_SOURCE);
    run_gcc("-save-temps=obj -c " TEMP_SOURCE);
    
    // 8. Test with isysroot for target_sysroot_hdrs_suffix
    printf("\n--- Invocation 8: Header sysroot options ---\n");
    run_gcc("-isysroot /usr/include -c " TEMP_SOURCE " 2>&1 | grep -i sysroot | head -2 || true");
    
    // 9. Successful compilation to reset after previous failures
    printf("\n--- Invocation 9: Successful compilation (reset test) ---\n");
    int status9 = run_gcc("-c " TEMP_SOURCE);
    printf("Exit status: %d (should be 0)\n", status9);
    
    // 10. Final invocation to ensure state was reset
    printf("\n--- Invocation 10: Simple compilation (verify reset) ---\n");
    run_gcc("--version");
    
    printf("\n=== Test complete ===\n");
    
    // Cleanup
    cleanup();
    
    return 0;
}
