#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TEMP_SOURCE "test_reset_source.c"
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
    // Options that will set various flags in the driver
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
        
        // Use system() in child to simplify
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

void cleanup(void) {
    // Remove temporary files
    remove(TEMP_SOURCE);
    remove(RESPONSE_FILE);
    remove(OUTPUT_OBJ);
    remove(OUTPUT_EXE);
    
    // Also remove any .i, .s, .o files created by -save-temps
    system("rm -f test_reset_source.i test_reset_source.s test_reset_source.o 2>/dev/null");
    system("rm -f *.i *.s *.o *.time 2>/dev/null 2>&1");
}

int main(int argc, char **argv) {
    printf("=== GCC Driver Reset Logic Test ===\n\n");
    
    // Create test files
    create_test_source();
    create_response_file();
    
    int overall_status = 0;
    
    // Test 1: Print help list (sets print_help_list)
    printf("\n--- Test 1: -print-help-list ---\n");
    run_gcc("-print-help-list 2>&1 | head -5");
    
    // Test 2: Version (sets print_version)
    printf("\n--- Test 2: --version ---\n");
    run_gcc("--version");
    
    // Test 3: Verbose flag (sets verbose_only_flag)
    printf("\n--- Test 3: -v flag ---\n");
    run_gcc("-v -E " TEMP_SOURCE " -o /dev/null 2>&1 | head -10");
    
    // Test 4: Save temps with various options (sets save_temps_flag, dumpdir, etc.)
    printf("\n--- Test 4: -save-temps variants ---\n");
    run_gcc("-save-temps -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    run_gcc("-save-temps=cwd -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    run_gcc("-save-temps=obj -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // Test 5: Use response file (sets at_file_supplied)
    printf("\n--- Test 5: Response file (@file) ---\n");
    run_gcc("@" RESPONSE_FILE " -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // Test 6: Different linker (sets use_ld)
    printf("\n--- Test 6: Different linkers ---\n");
    run_gcc("-fuse-ld=bfd -v " TEMP_SOURCE " -o " OUTPUT_EXE " 2>&1 | grep -i 'collect2'");
    run_gcc("-fuse-ld=gold -v " TEMP_SOURCE " -o " OUTPUT_EXE " 2>&1 | grep -i 'collect2'");
    
    // Test 7: Sysroot options (affects target_system_root)
    printf("\n--- Test 7: Sysroot options ---\n");
    run_gcc("--sysroot=/ -v " TEMP_SOURCE " -o " OUTPUT_EXE " 2>&1 | grep -i 'sysroot'");
    run_gcc("-isysroot /usr/include -v " TEMP_SOURCE " -o " OUTPUT_EXE " 2>&1 | grep -i 'sysroot'");
    
    // Test 8: Time report (sets report_times_to_file)
    printf("\n--- Test 8: Time reporting ---\n");
    run_gcc("-ftime-report -c " TEMP_SOURCE " -o " OUTPUT_OBJ " 2>&1 | tail -5");
    
    // Test 9: Machine/spec options (affects spec_machine)
    printf("\n--- Test 9: Machine/architecture options ---\n");
    run_gcc("-march=x86-64 -mtune=generic -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    run_gcc("-m32 -c " TEMP_SOURCE " -o " OUTPUT_OBJ " 2>&1 | head -5");
    
    // Test 10: Force failure to set greatest_status != 1
    printf("\n--- Test 10: Force failure (invalid option) ---\n");
    int fail_status = run_gcc("-invalid-option-that-does-not-exist 2>&1 | head -3");
    printf("Failure exit status: %d (should be non-zero)\n", fail_status);
    
    // Test 11: Successful compilation after failure
    printf("\n--- Test 11: Success after failure ---\n");
    int success_status = run_gcc("-c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    printf("Success exit status: %d (should be 0)\n", success_status);
    
    // Test 12: Combination of many flags
    printf("\n--- Test 12: Combination test ---\n");
    char combo_cmd[1024];
    snprintf(combo_cmd, sizeof(combo_cmd),
             "-v -save-temps=obj -ftime-report -fuse-ld=bfd --sysroot=/ "
             "-march=x86-64 -c %s -o %s 2>&1 | head -20",
             TEMP_SOURCE, OUTPUT_OBJ);
    run_gcc(combo_cmd);
    
    printf("\n=== All tests completed ===\n");
    
    // Cleanup
    cleanup();
    
    return overall_status;
}
