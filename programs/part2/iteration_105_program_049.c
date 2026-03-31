#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TEMP_SOURCE_FILE "test_reset_source.c"
#define RESPONSE_FILE "test_args.rsp"
#define OUTPUT_OBJ "test_reset.o"
#define OUTPUT_EXE "test_reset.exe"

void create_test_source(void) {
    FILE *f = fopen(TEMP_SOURCE_FILE, "w");
    if (!f) {
        perror("Failed to create test source file");
        exit(1);
    }
    fprintf(f, "int main(void) { return 0; }\n");
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

void cleanup_files(void) {
    remove(TEMP_SOURCE_FILE);
    remove(RESPONSE_FILE);
    remove(OUTPUT_OBJ);
    remove(OUTPUT_EXE);
    // Clean up any dump files that might have been created
    system("rm -f test_reset_source.* test_reset.* *.rsp *.o *.exe 2>/dev/null");
}

int main(int argc, char **argv) {
    printf("=== Testing GCC driver reset logic ===\n");
    
    // Create necessary files
    create_test_source();
    create_response_file();
    
    int overall_status = 0;
    
    // 1. Set print_help_list flag
    printf("\n--- Invocation 1: Setting print_help_list ---\n");
    run_gcc("-print-help-list 2>&1 | head -5");
    
    // 2. Set print_version flag
    printf("\n--- Invocation 2: Setting print_version ---\n");
    run_gcc("--version");
    
    // 3. Set verbose_only_flag and use response file (at_file_supplied)
    printf("\n--- Invocation 3: Using response file with verbose flag ---\n");
    run_gcc("-v @test_args.rsp -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    
    // 4. Set save_temps_flag and related variables with different modes
    printf("\n--- Invocation 4: Testing save-temps modes ---\n");
    run_gcc("-save-temps=cwd -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    run_gcc("-save-temps=obj -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    
    // 5. Set use_ld variable
    printf("\n--- Invocation 5: Setting use_ld ---\n");
    // Try different linkers (some may not be available)
    run_gcc("-fuse-ld=bfd -o " OUTPUT_EXE " " TEMP_SOURCE_FILE " 2>&1 | grep -i 'ld' || true");
    run_gcc("-fuse-ld=gold -o " OUTPUT_EXE " " TEMP_SOURCE_FILE " 2>&1 | grep -i 'ld' || true");
    
    // 6. Set report_times_to_file
    printf("\n--- Invocation 6: Setting time report flags ---\n");
    run_gcc("-ftime-report -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    run_gcc("-ftime-report-details -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    
    // 7. Set target_system_root variables (using dummy paths)
    printf("\n--- Invocation 7: Testing sysroot options ---\n");
    run_gcc("--sysroot=/dummy/sysroot -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ " 2>&1 | grep -i 'sysroot' || true");
    run_gcc("-isysroot /dummy/sysroot -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ " 2>&1 | grep -i 'sysroot' || true");
    
    // 8. Cause a failure to set greatest_status != 1
    printf("\n--- Invocation 8: Causing compilation failure ---\n");
    int fail_status = run_gcc("-c non_existent_file.c -o dummy.o 2>/dev/null");
    printf("Failure exit status: %d (should be non-zero)\n", fail_status);
    
    // 9. Follow with successful compilation to reset after failure
    printf("\n--- Invocation 9: Successful compilation after failure ---\n");
    run_gcc("-c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    
    // 10. Try to set spec_machine (architecture-specific options)
    printf("\n--- Invocation 10: Testing machine/architecture options ---\n");
    run_gcc("-march=x86-64 -mtune=generic -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    run_gcc("-march=native -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ " 2>&1 | grep -i 'march' || true");
    
    // 11. Combination test with many flags
    printf("\n--- Invocation 11: Combination test ---\n");
    char combo_cmd[1024];
    snprintf(combo_cmd, sizeof(combo_cmd),
             "-v -save-temps=obj -ftime-report --sysroot=/dummy -fuse-ld=bfd "
             "-c %s -o %s 2>&1 | tail -3",
             TEMP_SOURCE_FILE, OUTPUT_OBJ);
    run_gcc(combo_cmd);
    
    // 12. Test with different output base names (affects dumpbase/outbase)
    printf("\n--- Invocation 12: Testing dumpbase options ---\n");
    run_gcc("-dumpbase mytest.i -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    run_gcc("-dumpbase mytest -dumpbase-ext .c -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    
    printf("\n=== All invocations completed ===\n");
    printf("The driver's reset logic should have been exercised between each invocation.\n");
    
    // Cleanup
    cleanup_files();
    
    return overall_status;
}
