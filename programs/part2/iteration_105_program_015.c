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
    unlink(TEMP_SOURCE_FILE);
    unlink(RESPONSE_FILE);
    unlink(OUTPUT_OBJ);
    unlink(OUTPUT_EXE);
    
    // Clean up any save-temps files
    system("rm -f test_reset_source.* test_reset.* 2>/dev/null");
}

int main(int argc, char **argv) {
    printf("=== Testing GCC driver reset logic ===\n");
    
    // Create necessary files
    create_test_source();
    create_response_file();
    
    int overall_status = 0;
    
    // 1. Invocation with print-help-list (sets print_help_list)
    printf("\n--- Invocation 1: -print-help-list ---\n");
    run_gcc("-print-help-list 2>&1 | head -5");
    
    // 2. Invocation with version flag (sets print_version)
    printf("\n--- Invocation 2: --version ---\n");
    run_gcc("--version");
    
    // 3. Invocation with verbose flag (sets verbose_only_flag)
    printf("\n--- Invocation 3: -v (verbose) ---\n");
    run_gcc("-v -E -dM - < /dev/null 2>&1 | head -10");
    
    // 4. Invocation with save-temps and response file 
    // (sets save_temps_flag, at_file_supplied, dumpdir/dumpbase variables)
    printf("\n--- Invocation 4: -save-temps with response file ---\n");
    run_gcc("-save-temps -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    
    // 5. Invocation with sysroot options 
    // (affects target_system_root, target_system_root_changed)
    printf("\n--- Invocation 5: --sysroot option ---\n");
    run_gcc("--sysroot=/ -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ " 2>&1");
    
    // 6. Invocation with fuse-ld (sets use_ld)
    printf("\n--- Invocation 6: -fuse-ld option ---\n");
    run_gcc("-fuse-ld=bfd -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ " 2>&1");
    
    // 7. Invocation with time report (sets report_times_to_file)
    printf("\n--- Invocation 7: -ftime-report ---\n");
    run_gcc("-ftime-report -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ " 2>&1 | head -20");
    
    // 8. Invocation that will fail (non-existent file, affects greatest_status)
    printf("\n--- Invocation 8: Failing compilation ---\n");
    int fail_status = run_gcc("-c non_existent_file.c -o fail.o 2>&1");
    printf("Failed compilation exit status: %d\n", fail_status);
    
    // 9. Invocation with response file syntax (sets at_file_supplied)
    printf("\n--- Invocation 9: Using response file ---\n");
    run_gcc("@test_args.rsp -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ " 2>&1 | head -10");
    
    // 10. Invocation with machine-specific options (affects spec_machine)
    printf("\n--- Invocation 10: Machine-specific options ---\n");
    run_gcc("-march=x86-64 -mtune=generic -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    
    // 11. Invocation with dump options (affects dumpdir/dumpbase)
    printf("\n--- Invocation 11: Dump options ---\n");
    run_gcc("-fdump-tree-all -fdump-rtl-all -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ " 2>&1 | head -5");
    
    // 12. Final successful compilation to ensure reset after failures
    printf("\n--- Invocation 12: Final successful compilation ---\n");
    int final_status = run_gcc(TEMP_SOURCE_FILE " -o " OUTPUT_EXE);
    printf("Final compilation exit status: %d\n", final_status);
    
    // Cleanup
    cleanup_files();
    
    printf("\n=== Test completed ===\n");
    return overall_status;
}
