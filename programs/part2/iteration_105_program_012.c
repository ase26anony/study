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

void cleanup_files(void) {
    unlink(TEMP_SOURCE);
    unlink(RESPONSE_FILE);
    unlink(OUTPUT_OBJ);
    unlink(OUTPUT_EXE);
    
    // Clean up save-temps files if they exist
    unlink("test_reset_logic.i");
    unlink("test_reset_logic.s");
    unlink("test_reset_logic.o");
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
    
    // 3. Invocation with verbose flag (sets verbose_only_flag)
    printf("\n--- Invocation 3: -v (verbose) ---\n");
    run_gcc("-v -E -dM - < /dev/null 2>&1 | head -10");
    
    // 4. Invocation with save-temps and response file
    //    Sets save_temps_flag, dumpdir/dumpbase variables, and at_file_supplied
    printf("\n--- Invocation 4: -save-temps with response file ---\n");
    run_gcc("-save-temps=obj -o " OUTPUT_OBJ " @" RESPONSE_FILE " " TEMP_SOURCE);
    
    // 5. Invocation with sysroot options
    //    Influences target_system_root and related variables
    printf("\n--- Invocation 5: --sysroot option ---\n");
    run_gcc("--sysroot=/ -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 6. Invocation with fuse-ld (sets use_ld)
    printf("\n--- Invocation 6: -fuse-ld option ---\n");
    run_gcc("-fuse-ld=bfd -c " TEMP_SOURCE " -o " OUTPUT_OBJ " 2>&1 | grep -i 'ld' | head -2");
    
    // 7. Invocation with time report (sets report_times_to_file)
    printf("\n--- Invocation 7: -ftime-report ---\n");
    run_gcc("-ftime-report -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 8. Invocation that fails (sets greatest_status to non-1)
    printf("\n--- Invocation 8: Failing compilation ---\n");
    int fail_status = run_gcc("-c non_existent_file.c -o fail.o 2>&1 | head -2");
    printf("Failed compilation exit status: %d\n", fail_status);
    
    // 9. Invocation with machine specification (attempts to set spec_machine)
    printf("\n--- Invocation 9: Machine specification ---\n");
    run_gcc("-march=x86-64 -mtune=generic -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 10. Final successful compilation to ensure reset after failure
    printf("\n--- Invocation 10: Final successful compilation ---\n");
    int final_status = run_gcc("-o " OUTPUT_EXE " " TEMP_SOURCE);
    printf("Final compilation exit status: %d\n", final_status);
    
    // 11. Test with dumpbase and outbase options
    printf("\n--- Invocation 11: dumpbase and outbase options ---\n");
    run_gcc("-dumpbase mydump -dumpdir ./dumpdir/ -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 12. Test with multiple options combined
    printf("\n--- Invocation 12: Combined options ---\n");
    run_gcc("-v -save-temps -ftime-report --sysroot=/ -fuse-ld=gold -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // Clean up
    cleanup_files();
    
    printf("\n=== Test completed ===\n");
    printf("Each invocation should have triggered driver state reset between calls.\n");
    printf("Check coverage to verify lines 11228-11250 in gcc.cc were executed.\n");
    
    return 0;
}
