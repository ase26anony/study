#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TEMP_SOURCE "test_reset.c"
#define RESPONSE_FILE "args.rsp"
#define OUTPUT_OBJ "test_reset.o"
#define OUTPUT_EXE "test_reset.exe"

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
        exit(1);
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
    // Remove temporary files
    unlink(TEMP_SOURCE);
    unlink(RESPONSE_FILE);
    unlink(OUTPUT_OBJ);
    unlink(OUTPUT_EXE);
    unlink("test_reset.i");
    unlink("test_reset.s");
    unlink("test_reset.dSYM");  // For macOS
    system("rm -f *.o *.i *.s *.time *.exe *.out 2>/dev/null");
}

int main(int argc, char **argv) {
    printf("=== Testing GCC Driver Reset Logic ===\n\n");
    
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
    printf("\n--- Invocation 3: -v (verbose) ---\n");
    run_gcc("-v -E - < /dev/null 2>&1 | head -10");
    
    // 4. Invocation with response file (sets at_file_supplied)
    printf("\n--- Invocation 4: Using response file ---\n");
    run_gcc("@args.rsp -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 5. Invocation with -save-temps variants (sets save_temps_flag, dumpdir, etc.)
    printf("\n--- Invocation 5: -save-temps variants ---\n");
    run_gcc("-save-temps=cwd -c " TEMP_SOURCE " -o temp.o");
    run_gcc("-save-temps=obj -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 6. Invocation with sysroot options (affects target_system_root)
    printf("\n--- Invocation 6: Sysroot options ---\n");
    run_gcc("--sysroot=/ -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    run_gcc("-isysroot /usr -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 7. Invocation with -fuse-ld (sets use_ld)
    printf("\n--- Invocation 7: Linker selection ---\n");
    run_gcc("-fuse-ld=bfd -c " TEMP_SOURCE " -o " OUTPUT_OBJ " 2>&1 | grep -i 'ld' || true");
    run_gcc("-fuse-ld=gold -c " TEMP_SOURCE " -o " OUTPUT_OBJ " 2>&1 | grep -i 'ld' || true");
    
    // 8. Invocation with time reporting (sets report_times_to_file)
    printf("\n--- Invocation 8: Time reporting ---\n");
    run_gcc("-ftime-report -c " TEMP_SOURCE " -o " OUTPUT_OBJ " 2>&1 | tail -5");
    run_gcc("-ftime-report-details -c " TEMP_SOURCE " -o " OUTPUT_OBJ " 2>&1 | tail -5");
    
    // 9. Invocation that fails (sets greatest_status to non-1)
    printf("\n--- Invocation 9: Failing compilation ---\n");
    int fail_status = run_gcc("-c non_existent_file.c -o fake.o 2>&1 | head -3");
    printf("Failure exit status: %d\n", fail_status);
    
    // 10. Invocation with machine/arch options (affects spec_machine)
    printf("\n--- Invocation 10: Machine/architecture options ---\n");
    run_gcc("-march=x86-64 -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    run_gcc("-mtune=generic -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 11. Final successful invocation (ensures reset after failure)
    printf("\n--- Invocation 11: Final successful compilation ---\n");
    int final_status = run_gcc("-O2 -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    printf("Final exit status: %d\n", final_status);
    
    // 12. Link invocation with multiple options
    printf("\n--- Invocation 12: Linking with multiple options ---\n");
    run_gcc("-v -save-temps -ftime-report " TEMP_SOURCE " -o " OUTPUT_EXE);
    
    // 13. Test dumpbase/dumpdir options explicitly
    printf("\n--- Invocation 13: Explicit dumpbase/dumpdir ---\n");
    run_gcc("-dumpbase mydump -dumpdir ./dumps/ -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 14. Test with wrapper script to ensure fresh process each time
    printf("\n--- Invocation 14: Using wrapper for fresh process ---\n");
    system("echo '#!/bin/sh\nexec gcc \"$@\"' > /tmp/gcc_wrapper.sh");
    system("chmod +x /tmp/gcc_wrapper.sh");
    run_gcc("-wrapper /tmp/gcc_wrapper.sh -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    printf("\n=== All invocations completed ===\n");
    
    // Cleanup
    cleanup();
    
    return 0;
}
