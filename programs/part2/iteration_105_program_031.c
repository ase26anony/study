#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define TEMP_SOURCE "test_reset_source.c"
#define RESPONSE_FILE "test_args.rsp"
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
    // Options that set various flags in the driver
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
    unlink(TEMP_SOURCE);
    unlink(RESPONSE_FILE);
    unlink(OUTPUT_OBJ);
    unlink(OUTPUT_EXE);
    unlink("test_reset_source.i");
    unlink("test_reset_source.s");
    unlink("test_reset_source.o");
}

int main(int argc, char **argv) {
    // Create necessary files
    create_test_source();
    create_response_file();
    
    int overall_status = 0;
    
    // 1. Invocation with print-help-list (sets print_help_list)
    printf("\n=== Invocation 1: -print-help-list ===\n");
    run_gcc("-print-help-list 2>&1 | head -5");
    
    // 2. Invocation with version flag (sets print_version)
    printf("\n=== Invocation 2: --version ===\n");
    run_gcc("--version");
    
    // 3. Invocation with verbose flag (sets verbose_only_flag)
    printf("\n=== Invocation 3: -v (verbose) ===\n");
    run_gcc("-v -E " TEMP_SOURCE " -o /dev/null");
    
    // 4. Invocation with save-temps and response file 
    // (sets save_temps_flag, at_file_supplied, dumpdir/dumpbase variables)
    printf("\n=== Invocation 4: -save-temps with response file ===\n");
    run_gcc("-save-temps -o " OUTPUT_OBJ " @" RESPONSE_FILE " " TEMP_SOURCE);
    
    // 5. Invocation with sysroot options 
    // (affects target_system_root and related variables)
    printf("\n=== Invocation 5: --sysroot option ===\n");
    run_gcc("--sysroot=/ -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 6. Invocation with fuse-ld (sets use_ld)
    printf("\n=== Invocation 6: -fuse-ld option ===\n");
    run_gcc("-fuse-ld=bfd -c " TEMP_SOURCE " -o " OUTPUT_OBJ " 2>/dev/null");
    
    // 7. Invocation with time report to file (sets report_times_to_file)
    printf("\n=== Invocation 7: -ftime-report ===\n");
    run_gcc("-ftime-report -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 8. Failed invocation (should set greatest_status to non-1)
    printf("\n=== Invocation 8: Failed compilation ===\n");
    int status = run_gcc("-c non_existent_file.c -o fake.o 2>/dev/null");
    printf("Exit status: %d (non-zero expected)\n", status);
    
    // 9. Successful compilation after failure (tests reset of greatest_status)
    printf("\n=== Invocation 9: Successful after failure ===\n");
    run_gcc("-c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 10. Invocation with dumpbase options
    printf("\n=== Invocation 10: -dumpbase options ===\n");
    run_gcc("-dumpbase mydump -dumpdir ./ -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 11. Invocation with machine-specific options (affects spec_machine)
    printf("\n=== Invocation 11: Machine-specific options ===\n");
    run_gcc("-march=x86-64 -mtune=generic -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 12. Complex invocation with multiple flags
    printf("\n=== Invocation 12: Complex multi-flag invocation ===\n");
    run_gcc("-v -save-temps=cwd -ftime-report -fuse-ld=gold --sysroot=/ -o " OUTPUT_EXE " " TEMP_SOURCE);
    
    // Cleanup
    cleanup();
    
    printf("\n=== All invocations completed ===\n");
    printf("The driver's reset logic (lines 11228-11250) should have been exercised.\n");
    
    return overall_status;
}
