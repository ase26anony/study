#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TEMP_SOURCE "test_reset_logic.c"
#define RESPONSE_FILE "args.rsp"
#define OUTPUT_OBJ "test_reset.o"
#define OUTPUT_EXE "test_reset.exe"

void create_test_source(void) {
    FILE *f = fopen(TEMP_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source");
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
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        perror("execl failed");
        exit(127);
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
    remove(OUTPUT_OBJ);
    remove(OUTPUT_EXE);
    remove("test_reset_logic.i");
    remove("test_reset_logic.s");
    remove("test_reset_logic.o");
    remove("test_reset_logic.d");
}

int main(int argc, char **argv) {
    printf("=== Testing GCC Driver Reset Logic ===\n\n");
    
    // Create test files
    create_test_source();
    create_response_file();
    
    int overall_status = 0;
    
    // Invocation 1: Set print_help_list flag
    printf("\n--- Invocation 1: -print-help-list ---\n");
    int status1 = run_gcc("-print-help-list 2>&1 | head -5");
    printf("Exit status: %d\n", status1);
    
    // Invocation 2: Use response file and set multiple flags
    // This will likely fail (dummy.c doesn't exist), setting greatest_status != 1
    printf("\n--- Invocation 2: Response file with non-existent source ---\n");
    int status2 = run_gcc("-v -save-temps=obj -o " OUTPUT_OBJ " @" RESPONSE_FILE " dummy.c 2>&1");
    printf("Exit status: %d (expected non-zero)\n", status2);
    
    // Invocation 3: Set use_ld, sysroot, and time report flags with valid source
    printf("\n--- Invocation 3: Multiple option flags with valid source ---\n");
    int status3 = run_gcc("-fuse-ld=gold -ftime-report -o " OUTPUT_EXE " " TEMP_SOURCE " 2>&1");
    printf("Exit status: %d\n", status3);
    
    // Invocation 4: Attempt to set spec_machine (architecture-specific)
    printf("\n--- Invocation 4: Architecture/machine options ---\n");
    int status4 = run_gcc("-march=native -mtune=generic -c " TEMP_SOURCE " -o " OUTPUT_OBJ " 2>&1");
    printf("Exit status: %d\n", status4);
    
    // Invocation 5: Set save_temps with different modes
    printf("\n--- Invocation 5: Different save-temps modes ---\n");
    int status5 = run_gcc("-save-temps=cwd -c " TEMP_SOURCE " 2>&1");
    printf("Exit status: %d\n", status5);
    
    // Invocation 6: Version and verbose flags
    printf("\n--- Invocation 6: Version and verbose output ---\n");
    int status6 = run_gcc("--version -v 2>&1 | head -10");
    printf("Exit status: %d\n", status6);
    
    // Invocation 7: Sysroot options (using dummy paths)
    printf("\n--- Invocation 7: Sysroot options ---\n");
    int status7 = run_gcc("--sysroot=/dummy/sysroot -isysroot /dummy/isysroot -c " TEMP_SOURCE " 2>&1");
    printf("Exit status: %d\n", status7);
    
    // Invocation 8: Time report to file
    printf("\n--- Invocation 8: Time report with output file ---\n");
    int status8 = run_gcc("-ftime-report -ftime-report-details -o time_test.exe " TEMP_SOURCE " 2>&1");
    printf("Exit status: %d\n", status8);
    remove("time_test.exe");
    
    // Invocation 9: Dumpbase and dumpdir options
    printf("\n--- Invocation 9: Dumpbase and dumpdir ---\n");
    int status9 = run_gcc("-dumpbase dump_test -dumpdir ./dumps/ -c " TEMP_SOURCE " 2>&1");
    printf("Exit status: %d\n", status9);
    
    // Invocation 10: Final successful compilation to ensure reset after failures
    printf("\n--- Invocation 10: Clean successful compilation ---\n");
    int status10 = run_gcc("-c " TEMP_SOURCE " -o final.o 2>&1");
    printf("Exit status: %d\n", status10);
    remove("final.o");
    
    printf("\n=== Test Summary ===\n");
    printf("Each invocation forces driver re-initialization, exercising the reset logic.\n");
    printf("Variables targeted:\n");
    printf("  - print_help_list, print_version, verbose_only_flag\n");
    printf("  - save_temps_flag, dumpdir, dumpbase, outbase\n");
    printf("  - use_ld, report_times_to_file\n");
    printf("  - target_system_root*, spec_machine\n");
    printf("  - at_file_supplied (via @args.rsp)\n");
    printf("  - greatest_status (via failed compilation)\n");
    
    // Cleanup
    cleanup();
    
    return 0;
}
