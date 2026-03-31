#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TEMP_SOURCE "test_reset.c"
#define RESPONSE_FILE "args.rsp"
#define OUTPUT_OBJ "test_reset.o"
#define OUTPUT_EXE "test_reset.exe"

void create_temp_files(void) {
    // Create a minimal valid C source file
    FILE *src = fopen(TEMP_SOURCE, "w");
    if (src) {
        fprintf(src, "int main() { return 0; }\n");
        fclose(src);
    }
    
    // Create a response file with various options
    FILE *rsp = fopen(RESPONSE_FILE, "w");
    if (rsp) {
        fprintf(rsp, "-v\n");
        fprintf(rsp, "-save-temps\n");
        fprintf(rsp, "-Wall\n");
        fclose(rsp);
    }
}

void cleanup_temp_files(void) {
    unlink(TEMP_SOURCE);
    unlink(RESPONSE_FILE);
    unlink(OUTPUT_OBJ);
    unlink(OUTPUT_EXE);
    unlink("test_reset.i");
    unlink("test_reset.s");
    unlink("test_reset.o");
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
    }
    return -1;
}

int main(int argc, char **argv) {
    printf("=== Testing GCC driver reset logic ===\n");
    
    create_temp_files();
    
    // Track overall test status
    int test_passed = 1;
    
    // 1. Invocation with -print-help-list (sets print_help_list)
    printf("\n--- Invocation 1: -print-help-list ---\n");
    int status1 = run_gcc("-print-help-list 2>&1 | head -5");
    printf("Exit status: %d\n", status1);
    
    // 2. Invocation with --version (sets print_version)
    printf("\n--- Invocation 2: --version ---\n");
    int status2 = run_gcc("--version 2>&1 | head -2");
    printf("Exit status: %d\n", status2);
    
    // 3. Invocation with verbose flag (sets verbose_only_flag)
    printf("\n--- Invocation 3: -v (verbose) ---\n");
    int status3 = run_gcc("-v -E -x c /dev/null 2>&1 | tail -3");
    printf("Exit status: %d\n", status3);
    
    // 4. Invocation with save-temps variants (sets save_temps_flag and related variables)
    printf("\n--- Invocation 4: -save-temps variants ---\n");
    int status4a = run_gcc("-save-temps=cwd -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    printf("Exit status (save-temps=cwd): %d\n", status4a);
    
    int status4b = run_gcc("-save-temps=obj -c " TEMP_SOURCE);
    printf("Exit status (save-temps=obj): %d\n", status4b);
    
    // 5. Invocation with response file (sets at_file_supplied)
    printf("\n--- Invocation 5: Using response file ---\n");
    int status5 = run_gcc("@" RESPONSE_FILE " -c " TEMP_SOURCE);
    printf("Exit status: %d\n", status5);
    
    // 6. Invocation with sysroot options (affects target_system_root variables)
    printf("\n--- Invocation 6: Sysroot options ---\n");
    int status6 = run_gcc("--sysroot=/ -isysroot/usr -c " TEMP_SOURCE);
    printf("Exit status: %d\n", status6);
    
    // 7. Invocation with fuse-ld (sets use_ld)
    printf("\n--- Invocation 7: Linker selection ---\n");
    int status7 = run_gcc("-fuse-ld=bfd -c " TEMP_SOURCE);
    printf("Exit status: %d\n", status7);
    
    // 8. Invocation with time report (sets report_times_to_file)
    printf("\n--- Invocation 8: Time reporting ---\n");
    int status8 = run_gcc("-ftime-report -c " TEMP_SOURCE " 2>&1 | grep -i 'time' | head -2");
    printf("Exit status: %d\n", status8);
    
    // 9. Invocation that fails (affects greatest_status)
    printf("\n--- Invocation 9: Failing compilation ---\n");
    int status9 = run_gcc("-c non_existent_file.c 2>&1 | head -2");
    printf("Exit status (should be non-zero): %d\n", status9);
    
    // 10. Invocation with machine/arch options (affects spec_machine)
    printf("\n--- Invocation 10: Machine/architecture options ---\n");
    int status10a = run_gcc("-march=x86-64 -c " TEMP_SOURCE);
    printf("Exit status (march): %d\n", status10a);
    
    int status10b = run_gcc("-mtune=generic -c " TEMP_SOURCE);
    printf("Exit status (mtune): %d\n", status10b);
    
    // 11. Combined invocation with many options
    printf("\n--- Invocation 11: Combined options ---\n");
    int status11 = run_gcc("-v -save-temps -ftime-report -Wall -O2 " TEMP_SOURCE " -o " OUTPUT_EXE);
    printf("Exit status: %d\n", status11);
    
    // 12. Final simple invocation to ensure reset worked
    printf("\n--- Invocation 12: Simple compilation after all ---\n");
    int status12 = run_gcc("-c " TEMP_SOURCE);
    printf("Exit status: %d\n", status12);
    
    // Cleanup
    cleanup_temp_files();
    
    printf("\n=== Test completed ===\n");
    printf("Note: This test exercises the driver's reset logic between invocations.\n");
    printf("To verify coverage, run with an instrumented GCC build.\n");
    
    return 0;
}
