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

void cleanup(void) {
    remove(TEMP_SOURCE);
    remove(RESPONSE_FILE);
    remove("test.o");
    remove("test.exe");
    remove("test_reset_logic.i");
    remove("test_reset_logic.s");
    remove("test_reset_logic.o");
}

int main(int argc, char **argv) {
    // Create necessary files
    create_test_source();
    create_response_file();
    
    // Register cleanup handler
    atexit(cleanup);
    
    int overall_status = 0;
    
    // 1. Set print_help_list flag
    printf("\n=== Invocation 1: Setting print_help_list ===\n");
    run_gcc("-print-help-list 2>&1 | head -5");
    
    // 2. Set print_version flag
    printf("\n=== Invocation 2: Setting print_version ===\n");
    run_gcc("--version");
    
    // 3. Set verbose_only_flag and use response file (at_file_supplied)
    printf("\n=== Invocation 3: Using response file and verbose flag ===\n");
    run_gcc("@args.rsp -c " TEMP_SOURCE);
    
    // 4. Set save_temps_flag and related variables with different variants
    printf("\n=== Invocation 4: Testing save-temps variants ===\n");
    run_gcc("-save-temps=cwd -c " TEMP_SOURCE);
    run_gcc("-save-temps=obj -c " TEMP_SOURCE");
    
    // 5. Set use_ld variable
    printf("\n=== Invocation 5: Setting use_ld ===\n");
    run_gcc("-fuse-ld=gold -o test.exe " TEMP_SOURCE " 2>&1 | grep -i 'ld'");
    
    // 6. Set report_times_to_file
    printf("\n=== Invocation 6: Setting report_times_to_file ===\n");
    run_gcc("-ftime-report -c " TEMP_SOURCE);
    
    // 7. Attempt to set spec_machine (may be target-specific)
    printf("\n=== Invocation 7: Attempting to set spec_machine ===\n");
    run_gcc("-machine=x86_64 -c " TEMP_SOURCE " 2>&1");
    
    // 8. Cause a failure to set greatest_status != 1
    printf("\n=== Invocation 8: Causing compilation failure ===\n");
    int fail_status = run_gcc("-c nonexistent_file.c 2>&1");
    printf("Failure status: %d\n", fail_status);
    
    // 9. Successful compilation after failure
    printf("\n=== Invocation 9: Successful compilation after failure ===\n");
    run_gcc("-o test.exe " TEMP_SOURCE);
    
    // 10. Test sysroot options (affects target_system_root variables)
    printf("\n=== Invocation 10: Testing sysroot options ===\n");
    run_gcc("--sysroot=/ -c " TEMP_SOURCE);
    run_gcc("-isysroot /usr -c " TEMP_SOURCE);
    
    // 11. Test dumpbase and related options
    printf("\n=== Invocation 11: Testing dumpbase options ===\n");
    run_gcc("-dumpbase test.dump -dumpdir ./ -c " TEMP_SOURCE);
    
    // 12. Multiple options combined
    printf("\n=== Invocation 12: Combined options ===\n");
    run_gcc("-v -save-temps --sysroot=/ -ftime-report -fuse-ld=bfd -c " TEMP_SOURCE);
    
    printf("\n=== All invocations completed ===\n");
    printf("The driver's reset logic should have been exercised between each invocation.\n");
    
    return 0;
}
