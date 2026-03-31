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
    fprintf(f, "-save-temps=obj\n");
    fprintf(f, "-ftime-report\n");
    fclose(f);
}

void cleanup_files(void) {
    unlink(TEMP_SOURCE);
    unlink(RESPONSE_FILE);
    // Clean up temporary files that might be created by -save-temps
    system("rm -f test_reset_logic.i test_reset_logic.s test_reset_logic.o "
           "test_reset_logic.exe test.o test.exe a.out 2>/dev/null");
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
    printf("=== Testing GCC driver reset logic ===\n\n");
    
    // Create necessary files
    create_test_source();
    create_response_file();
    
    int overall_status = 0;
    
    // Invocation 1: Print help list (sets print_help_list)
    printf("\n--- Invocation 1: -print-help-list ---\n");
    run_gcc("-print-help-list 2>&1 | head -5");
    
    // Invocation 2: Version (sets print_version)
    printf("\n--- Invocation 2: --version ---\n");
    run_gcc("--version");
    
    // Invocation 3: Verbose with response file (sets verbose_only_flag, at_file_supplied)
    printf("\n--- Invocation 3: Verbose with response file ---\n");
    run_gcc("-v @args.rsp -c " TEMP_SOURCE);
    
    // Invocation 4: Save temps variants (sets save_temps_flag, dumpdir, etc.)
    printf("\n--- Invocation 4: Save temps variants ---\n");
    run_gcc("-save-temps=cwd -c " TEMP_SOURCE);
    run_gcc("-save-temps=obj -dumpdir=./dump -dumpbase=test -c " TEMP_SOURCE);
    
    // Invocation 5: Sysroot options (affects target_system_root)
    printf("\n--- Invocation 5: Sysroot options ---\n");
    run_gcc("--sysroot=/usr -c " TEMP_SOURCE);
    run_gcc("-isysroot /usr -c " TEMP_SOURCE);
    
    // Invocation 6: Linker specification (sets use_ld)
    printf("\n--- Invocation 6: Linker specification ---\n");
    run_gcc("-fuse-ld=bfd -c " TEMP_SOURCE);
    run_gcc("-fuse-ld=gold -c " TEMP_SOURCE  " 2>/dev/null"); // May fail if gold not available
    
    // Invocation 7: Time report (sets report_times_to_file)
    printf("\n--- Invocation 7: Time report ---\n");
    run_gcc("-ftime-report -c " TEMP_SOURCE);
    run_gcc("-ftime-report-details -c " TEMP_SOURCE  " 2>/dev/null");
    
    // Invocation 8: Machine specification (sets spec_machine)
    printf("\n--- Invocation 8: Machine specification ---\n");
    run_gcc("-march=x86-64 -mtune=generic -c " TEMP_SOURCE);
    run_gcc("-march=native -c " TEMP_SOURCE  " 2>/dev/null");
    
    // Invocation 9: Force failure (affects greatest_status)
    printf("\n--- Invocation 9: Force failure ---\n");
    int fail_status = run_gcc("-c nonexistent_file.c 2>/dev/null");
    printf("Failure status: %d\n", fail_status);
    
    // Invocation 10: Successful compilation after failure
    printf("\n--- Invocation 10: Successful compilation after failure ---\n");
    int success_status = run_gcc("-c " TEMP_SOURCE " -o test_final.o");
    printf("Success status: %d\n", success_status);
    
    // Invocation 11: Complex combination
    printf("\n--- Invocation 11: Complex combination ---\n");
    run_gcc("-v -save-temps=obj -ftime-report -fuse-ld=bfd --sysroot=/ -c " TEMP_SOURCE);
    
    // Invocation 12: With output redirection
    printf("\n--- Invocation 12: With output files ---\n");
    run_gcc("-c " TEMP_SOURCE " -o output.o");
    run_gcc(TEMP_SOURCE " -o output.exe");
    
    // Cleanup
    cleanup_files();
    
    printf("\n=== Test completed ===\n");
    return overall_status;
}
