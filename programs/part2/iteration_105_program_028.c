#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TEMP_SOURCE "test_reset_logic.c"
#define RESPONSE_FILE "args.rsp"
#define OUTPUT_OBJ "test_reset.o"
#define OUTPUT_EXE "test_reset.exe"

void create_test_source(void) {
    FILE *f = fopen(TEMP_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(1);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

void create_response_file(void) {
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
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        perror("execl failed");
        exit(127);
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

void cleanup_files(void) {
    remove(TEMP_SOURCE);
    remove(RESPONSE_FILE);
    remove(OUTPUT_OBJ);
    remove(OUTPUT_EXE);
    // Clean up temporary files that might be created by -save-temps
    system("rm -f test_reset_logic.* test_reset.* *.o *.s *.i 2>/dev/null");
}

int main(int argc, char **argv) {
    printf("=== Testing GCC driver reset logic ===\n");
    
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
    printf("\n--- Invocation 3: Verbose with response file ---\n");
    run_gcc("-v @" RESPONSE_FILE " -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 4. Set save_temps_flag and related variables
    printf("\n--- Invocation 4: Save temps with different options ---\n");
    run_gcc("-save-temps=cwd -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 5. Set use_ld and target_system_root variables
    printf("\n--- Invocation 5: Linker and sysroot options ---\n");
    // Try different linkers if available
    run_gcc("-fuse-ld=bfd -c " TEMP_SOURCE " -o " OUTPUT_OBJ " 2>&1");
    run_gcc("-fuse-ld=gold -c " TEMP_SOURCE " -o " OUTPUT_OBJ " 2>&1");
    
    // 6. Cause a failure to set greatest_status != 1
    printf("\n--- Invocation 6: Force failure (non-existent file) ---\n");
    int fail_status = run_gcc("-c non_existent_file.c -o dummy.o 2>&1");
    printf("Failure exit status: %d\n", fail_status);
    
    // 7. Set report_times_to_file
    printf("\n--- Invocation 7: Time reporting ---\n");
    run_gcc("-ftime-report -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 8. Try to set spec_machine (architecture-specific)
    printf("\n--- Invocation 8: Machine/architecture options ---\n");
    run_gcc("-march=x86-64 -mtune=generic -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 9. Multiple options combined
    printf("\n--- Invocation 9: Combined options ---\n");
    run_gcc("-v -save-temps=obj -ftime-report -fuse-ld=bfd -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 10. Successful compilation after failure (tests reset of greatest_status)
    printf("\n--- Invocation 10: Successful compilation ---\n");
    int success_status = run_gcc("-c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    printf("Success exit status: %d\n", success_status);
    
    // 11. Test with output base options (affects dumpbase/outbase)
    printf("\n--- Invocation 11: Output base options ---\n");
    run_gcc("-dumpbase test.dump -dumpdir ./dumpdir/ -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 12. Test sysroot suffix options
    printf("\n--- Invocation 12: Sysroot suffix options ---\n");
    run_gcc("--sysroot=/usr -isysroot /usr/include -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // Clean up
    cleanup_files();
    
    printf("\n=== Test completed ===\n");
    return overall_status;
}
