#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TEMP_SOURCE "test_reset_logic.c"
#define RESPONSE_FILE "args.rsp"

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
    fprintf(f, "-v\n");
    fprintf(f, "-save-temps\n");
    fprintf(f, "-o test_output.o\n");
    fclose(f);
}

void cleanup_files(void) {
    remove(TEMP_SOURCE);
    remove(RESPONSE_FILE);
    remove("test_output.o");
    remove("test_output.exe");
    remove("test_final.o");
    remove("test_reset_logic.i");
    remove("test_reset_logic.s");
    remove("test_reset_logic.o");
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

int main(int argc, char **argv) {
    printf("=== Testing GCC driver reset logic ===\n");
    
    // Create necessary files
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
    
    // 4. Invocation with -save-temps and response file
    //    (sets save_temps_flag, at_file_supplied, dumpdir/dumpbase variables)
    printf("\n--- Invocation 4: -save-temps with response file ---\n");
    run_gcc("@args.rsp " TEMP_SOURCE);
    
    // 5. Invocation with -fuse-ld (sets use_ld)
    printf("\n--- Invocation 5: -fuse-ld ---\n");
    run_gcc("-fuse-ld=bfd -c " TEMP_SOURCE " -o test_final.o 2>&1");
    
    // 6. Invocation with --sysroot (influences target_system_root)
    printf("\n--- Invocation 6: --sysroot ---\n");
    run_gcc("--sysroot=/ -c " TEMP_SOURCE " -o test_final.o 2>&1");
    
    // 7. Invocation with -ftime-report (sets report_times_to_file)
    printf("\n--- Invocation 7: -ftime-report ---\n");
    run_gcc("-ftime-report -c " TEMP_SOURCE " -o test_final.o 2>&1");
    
    // 8. Invocation that fails (sets greatest_status to non-1)
    printf("\n--- Invocation 8: Failing compilation ---\n");
    int fail_status = run_gcc("-c non_existent_file.c -o test_final.o 2>&1");
    printf("Failure status: %d (should be non-zero)\n", fail_status);
    
    // 9. Invocation with machine-specific options (attempts to set spec_machine)
    printf("\n--- Invocation 9: Machine-specific options ---\n");
    run_gcc("-march=x86-64 -mtune=generic -c " TEMP_SOURCE " -o test_final.o");
    
    // 10. Final successful compilation (ensures reset after failure)
    printf("\n--- Invocation 10: Final successful compilation ---\n");
    int final_status = run_gcc("-c " TEMP_SOURCE " -o test_final.o");
    printf("Final status: %d (should be 0)\n", final_status);
    
    // 11. Test with -save-temps variants
    printf("\n--- Invocation 11: -save-temps variants ---\n");
    run_gcc("-save-temps=cwd -c " TEMP_SOURCE);
    run_gcc("-save-temps=obj -c " TEMP_SOURCE " -o test_final.o");
    
    // 12. Test with verbose only flag
    printf("\n--- Invocation 12: Verbose only ---\n");
    run_gcc("-### " TEMP_SOURCE " 2>&1 | head -3");
    
    // 13. Test with dumpbase options
    printf("\n--- Invocation 13: dumpbase options ---\n");
    run_gcc("-dumpbase test.dump -dumpbase-ext .ext -c " TEMP_SOURCE);
    
    printf("\n=== All invocations completed ===\n");
    
    // Cleanup
    cleanup_files();
    
    return 0;
}
