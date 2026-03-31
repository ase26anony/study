#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define TEMP_SOURCE_FILE "test_reset_source.c"
#define RESPONSE_FILE "test_reset_args.rsp"
#define OUTPUT_OBJ "test_reset_output.o"
#define OUTPUT_EXE "test_reset_output.exe"

void create_test_files(void) {
    // Create a minimal valid C source file
    FILE *src = fopen(TEMP_SOURCE_FILE, "w");
    if (src) {
        fprintf(src, "int main() { return 0; }\n");
        fclose(src);
    }
    
    // Create a response file with various options
    FILE *rsp = fopen(RESPONSE_FILE, "w");
    if (rsp) {
        fprintf(rsp, "-v\n");
        fprintf(rsp, "-save-temps=obj\n");
        fprintf(rsp, "-ftime-report\n");
        fclose(rsp);
    }
}

void cleanup_test_files(void) {
    // Remove temporary files
    remove(TEMP_SOURCE_FILE);
    remove(RESPONSE_FILE);
    remove(OUTPUT_OBJ);
    remove(OUTPUT_EXE);
    remove("test_reset_source.i");
    remove("test_reset_source.s");
    remove("test_reset_source.o");
}

int run_gcc(const char *args) {
    printf("Running: gcc %s\n", args);
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "gcc %s", args);
        
        // Use system() in child to simplify
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
    printf("=== Testing GCC Driver Reset Logic ===\n\n");
    
    create_test_files();
    
    int overall_status = 0;
    
    // Invocation 1: Set print_help_list
    printf("\n--- Invocation 1: -print-help-list ---\n");
    int status1 = run_gcc("-print-help-list 2>&1 | head -5");
    printf("Exit status: %d\n", status1);
    
    // Invocation 2: Multiple flags with response file (will fail due to non-existent file)
    // This should set at_file_supplied, save_temps_flag, verbose_only_flag, etc.
    // and cause greatest_status != 1
    printf("\n--- Invocation 2: Response file with non-existent source (should fail) ---\n");
    int status2 = run_gcc("-v -save-temps=obj -o " OUTPUT_OBJ " @" RESPONSE_FILE " non_existent_file.c 2>&1 | tail -3");
    printf("Exit status: %d (non-zero expected)\n", status2);
    
    // Invocation 3: Set use_ld, sysroot, time report with valid source
    printf("\n--- Invocation 3: Valid compilation with various options ---\n");
    int status3 = run_gcc("-fuse-ld=bfd -ftime-report -o " OUTPUT_EXE " " TEMP_SOURCE_FILE " 2>&1 | tail -5");
    printf("Exit status: %d\n", status3);
    
    // Invocation 4: Try to set spec_machine (target-specific)
    printf("\n--- Invocation 4: Target/machine options ---\n");
    int status4 = run_gcc("-mtune=generic -march=x86-64 -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ " 2>&1 | tail -3");
    printf("Exit status: %d\n", status4);
    
    // Invocation 5: Version flag (sets print_version)
    printf("\n--- Invocation 5: --version ---\n");
    int status5 = run_gcc("--version 2>&1 | head -2");
    printf("Exit status: %d\n", status5);
    
    // Invocation 6: Save temps with dumpdir/dumpbase
    printf("\n--- Invocation 6: Save-temps with dump options ---\n");
    int status6 = run_gcc("-save-temps=cwd -dumpdir ./dump_ -dumpbase test_dump -c " TEMP_SOURCE_FILE " 2>&1 | tail -3");
    printf("Exit status: %d\n", status6);
    
    // Invocation 7: Sysroot options (affects target_system_root)
    printf("\n--- Invocation 7: Sysroot options ---\n");
    int status7 = run_gcc("--sysroot=/ -isysroot /usr/include -c " TEMP_SOURCE_FILE " 2>&1 | tail -3");
    printf("Exit status: %d\n", status7);
    
    // Invocation 8: Final simple compilation to ensure reset worked
    printf("\n--- Invocation 8: Simple compilation (post-reset check) ---\n");
    int status8 = run_gcc("-c " TEMP_SOURCE_FILE);
    printf("Exit status: %d\n", status8);
    
    printf("\n=== Test Summary ===\n");
    printf("Invocation 2 failed as expected: %s\n", status2 != 0 ? "YES" : "NO");
    printf("Final invocation succeeded: %s\n", status8 == 0 ? "YES" : "NO");
    
    cleanup_test_files();
    
    // Clean up dump files if created
    remove("dump_*");
    remove("test_dump*");
    
    return 0;
}
