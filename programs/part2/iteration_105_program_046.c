#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TEMP_SOURCE "test_reset_logic.c"
#define RESPONSE_FILE "args.rsp"
#define OUTPUT_OBJ "test_reset.o"
#define OUTPUT_EXE "test_reset.exe"

void create_temp_source() {
    FILE *f = fopen(TEMP_SOURCE, "w");
    if (!f) {
        perror("Failed to create temp source");
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
    remove(TEMP_SOURCE);
    remove(RESPONSE_FILE);
    remove(OUTPUT_OBJ);
    remove(OUTPUT_EXE);
    remove("test_reset_logic.i");
    remove("test_reset_logic.s");
    remove("test_reset_logic.o");
}

int main(int argc, char **argv) {
    printf("=== Testing GCC driver reset logic ===\n");
    
    // Create necessary files
    create_temp_source();
    create_response_file();
    
    int overall_status = 0;
    
    // 1. Invocation with -print-help-list (sets print_help_list)
    printf("\n--- Invocation 1: -print-help-list ---\n");
    int status1 = run_gcc("-print-help-list 2>&1 | head -5");
    printf("Exit status: %d\n", status1);
    
    // 2. Invocation with --version (sets print_version)
    printf("\n--- Invocation 2: --version ---\n");
    int status2 = run_gcc("--version 2>&1 | head -2");
    printf("Exit status: %d\n", status2);
    
    // 3. Invocation with -v (sets verbose_only_flag)
    printf("\n--- Invocation 3: -v (verbose) ---\n");
    int status3 = run_gcc("-v -E -x c /dev/null 2>&1 | tail -3");
    printf("Exit status: %d\n", status3);
    
    // 4. Invocation with -save-temps and response file
    //    (sets save_temps_flag, at_file_supplied, dumpdir/dumpbase variables)
    printf("\n--- Invocation 4: -save-temps with response file ---\n");
    int status4 = run_gcc("-save-temps=obj -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    printf("Exit status: %d\n", status4);
    
    // 5. Invocation with -fuse-ld and --sysroot
    //    (sets use_ld, target_system_root_changed)
    printf("\n--- Invocation 5: -fuse-ld and --sysroot ---\n");
    int status5 = run_gcc("-fuse-ld=bfd --sysroot=/  -c " TEMP_SOURCE " -o " OUTPUT_OBJ " 2>&1");
    printf("Exit status: %d\n", status5);
    
    // 6. Invocation with -ftime-report (sets report_times_to_file)
    printf("\n--- Invocation 6: -ftime-report ---\n");
    int status6 = run_gcc("-ftime-report -c " TEMP_SOURCE " -o " OUTPUT_OBJ " 2>&1 | grep -i time");
    printf("Exit status: %d\n", status6);
    
    // 7. Invocation with response file (@file syntax)
    //    (sets at_file_supplied = 1)
    printf("\n--- Invocation 7: Response file ---\n");
    int status7 = run_gcc("@" RESPONSE_FILE " -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    printf("Exit status: %d\n", status7);
    
    // 8. Failed invocation (sets greatest_status to non-1)
    printf("\n--- Invocation 8: Failed compilation ---\n");
    int status8 = run_gcc("-c nonexistent_file.c -o dummy.o 2>&1 | head -2");
    printf("Exit status: %d (should be non-zero)\n", status8);
    
    // 9. Successful compilation after failure (tests reset)
    printf("\n--- Invocation 9: Successful after failure ---\n");
    int status9 = run_gcc("-c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    printf("Exit status: %d (should be 0)\n", status9);
    
    // 10. Invocation with machine-specific options
    //     (attempts to set spec_machine)
    printf("\n--- Invocation 10: Machine-specific options ---\n");
    int status10 = run_gcc("-march=x86-64 -mtune=generic -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    printf("Exit status: %d\n", status10);
    
    // 11. Final linking invocation with multiple options
    printf("\n--- Invocation 11: Final linking ---\n");
    int status11 = run_gcc("-v -save-temps -ftime-report " TEMP_SOURCE " -o " OUTPUT_EXE " 2>&1 | tail -5");
    printf("Exit status: %d\n", status11);
    
    printf("\n=== Test completed ===\n");
    printf("Summary of exit statuses:\n");
    printf("  Invocation 8 (failure): %d\n", status8);
    printf("  Invocation 9 (success after failure): %d\n", status9);
    
    // Cleanup
    cleanup();
    
    return 0;
}
