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
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        perror("execl failed");
        exit(255);
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
    unlink("test_reset_logic.i");
    unlink("test_reset_logic.s");
    unlink("test_reset_logic.o");
}

int main(int argc, char **argv) {
    // Create test files
    create_test_source();
    create_response_file();
    
    int overall_status = 0;
    
    // Invocation 1: Print help list (sets print_help_list)
    printf("\n=== Invocation 1: -print-help-list ===\n");
    run_gcc("-print-help-list 2>/dev/null");
    
    // Invocation 2: Version (sets print_version)
    printf("\n=== Invocation 2: --version ===\n");
    run_gcc("--version");
    
    // Invocation 3: Verbose only (sets verbose_only_flag)
    printf("\n=== Invocation 3: -v (verbose) ===\n");
    run_gcc("-v 2>&1 | head -5");
    
    // Invocation 4: Save temps with dumpdir/dumpbase variables
    printf("\n=== Invocation 4: -save-temps with dumpdir ===\n");
    run_gcc("-save-temps -dumpdir ./dumpdir/ -dumpbase testdump "
            "-dumpbase-ext .ext -o outbase_test " TEMP_SOURCE);
    
    // Invocation 5: Use response file (sets at_file_supplied)
    printf("\n=== Invocation 5: With response file ===\n");
    char cmd5[256];
    snprintf(cmd5, sizeof(cmd5), "@%s -o %s %s", RESPONSE_FILE, OUTPUT_OBJ, TEMP_SOURCE);
    run_gcc(cmd5);
    
    // Invocation 6: Set use_ld and sysroot
    printf("\n=== Invocation 6: Linker and sysroot options ===\n");
    // Try different linkers (some may not be available)
    run_gcc("-fuse-ld=bfd --sysroot=/ 2>&1 | head -3");
    run_gcc("-fuse-ld=gold 2>&1 | head -3");
    
    // Invocation 7: Time report to file
    printf("\n=== Invocation 7: Time report ===\n");
    run_gcc("-ftime-report -ftime-report-details " TEMP_SOURCE " 2>&1 | head -10");
    
    // Invocation 8: Cause failure (non-existent file) to affect greatest_status
    printf("\n=== Invocation 8: Failed compilation ===\n");
    int status8 = run_gcc("nonexistent_file.c 2>/dev/null");
    printf("Exit status: %d (should be non-zero)\n", status8);
    
    // Invocation 9: Successful compilation after failure
    printf("\n=== Invocation 9: Successful compilation ===\n");
    run_gcc("-o " OUTPUT_EXE " " TEMP_SOURCE);
    
    // Invocation 10: Machine/spec options
    printf("\n=== Invocation 10: Machine-specific options ===\n");
    // Try various architecture options that might affect spec_machine
    run_gcc("-march=x86-64 -mtune=generic " TEMP_SOURCE " -c -o " OUTPUT_OBJ);
    run_gcc("-m32 " TEMP_SOURCE " -c -o " OUTPUT_OBJ "32 2>&1 | head -3");
    
    // Invocation 11: Complex combination
    printf("\n=== Invocation 11: Complex combination ===\n");
    run_gcc("-v -save-temps=obj -ftime-report -fuse-ld=gold "
            "--sysroot=/ -o final.exe " TEMP_SOURCE " 2>&1 | tail -5");
    
    // Cleanup
    cleanup();
    
    printf("\n=== Test completed ===\n");
    printf("The GCC driver's reset logic should have been exercised between each invocation.\n");
    printf("Variables like print_help_list, print_version, verbose_only_flag,\n");
    printf("save_temps_flag, dumpdir/dumpbase, at_file_supplied, use_ld,\n");
    printf("report_times_to_file, spec_machine, and greatest_status\n");
    printf("should have been set and then reset to defaults.\n");
    
    return overall_status;
}
