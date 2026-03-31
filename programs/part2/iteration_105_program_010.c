#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TEMP_SOURCE "test_reset_source.c"
#define RESPONSE_FILE "test_reset_args.rsp"
#define OUTPUT_OBJ "test_reset_output.o"
#define OUTPUT_EXE "test_reset_output.exe"

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
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        perror("execl failed");
        exit(127);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    } else {
        perror("fork failed");
        return -1;
    }
}

void cleanup() {
    // Remove temporary files
    unlink(TEMP_SOURCE);
    unlink(RESPONSE_FILE);
    unlink(OUTPUT_OBJ);
    unlink(OUTPUT_EXE);
    unlink("test_reset_source.i");
    unlink("test_reset_source.s");
    unlink("test_reset_source.o");
}

int main(int argc, char **argv) {
    printf("=== Testing GCC driver reset logic ===\n");
    
    // Create test files
    create_test_source();
    create_response_file();
    
    int overall_status = 0;
    
    // 1. Invocation with -print-help-list (sets print_help_list)
    printf("\n--- Invocation 1: -print-help-list ---\n");
    run_gcc("-print-help-list 2>&1 | head -5");
    
    // 2. Invocation with --version (sets print_version)
    printf("\n--- Invocation 2: --version ---\n");
    run_gcc("--version");
    
    // 3. Invocation with -v (sets verbose_only_flag)
    printf("\n--- Invocation 3: -v ---\n");
    run_gcc("-v -E " TEMP_SOURCE " -o /dev/null");
    
    // 4. Invocation with -save-temps variants (sets save_temps_flag, dumpdir, etc.)
    printf("\n--- Invocation 4: -save-temps variants ---\n");
    run_gcc("-save-temps -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    run_gcc("-save-temps=cwd -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    run_gcc("-save-temps=obj -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 5. Invocation with response file (sets at_file_supplied)
    printf("\n--- Invocation 5: Response file ---\n");
    run_gcc("@" RESPONSE_FILE " -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 6. Invocation with -fuse-ld (sets use_ld)
    printf("\n--- Invocation 6: -fuse-ld ---\n");
    run_gcc("-fuse-ld=bfd -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    run_gcc("-fuse-ld=gold -c " TEMP_SOURCE " -o " OUTPUT_OBJ  " 2>/dev/null");
    
    // 7. Invocation with --sysroot (affects target_system_root)
    printf("\n--- Invocation 7: --sysroot ---\n");
    run_gcc("--sysroot=/ -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 8. Invocation with -ftime-report (sets report_times_to_file)
    printf("\n--- Invocation 8: -ftime-report ---\n");
    run_gcc("-ftime-report -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 9. Invocation that fails (sets greatest_status to non-1)
    printf("\n--- Invocation 9: Failing compilation ---\n");
    int fail_status = run_gcc("-c nonexistent_file.c -o " OUTPUT_OBJ " 2>/dev/null");
    printf("Failure exit status: %d\n", fail_status);
    
    // 10. Invocation with machine-specific options (affects spec_machine)
    printf("\n--- Invocation 10: Machine-specific options ---\n");
    run_gcc("-march=x86-64 -mtune=generic -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    run_gcc("-march=native -c " TEMP_SOURCE " -o " OUTPUT_OBJ " 2>/dev/null");
    
    // 11. Final successful compilation (ensures reset after failure)
    printf("\n--- Invocation 11: Final successful compilation ---\n");
    int final_status = run_gcc(TEMP_SOURCE " -o " OUTPUT_EXE);
    printf("Final exit status: %d\n", final_status);
    
    // 12. Multiple quick invocations to stress reset logic
    printf("\n--- Invocation 12: Stress test with rapid invocations ---\n");
    for (int i = 0; i < 5; i++) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "-v -save-temps -c " TEMP_SOURCE " -o test_iter_%d.o 2>/dev/null", i);
        run_gcc(cmd);
    }
    
    // Cleanup
    cleanup();
    
    printf("\n=== Test completed ===\n");
    return overall_status;
}
