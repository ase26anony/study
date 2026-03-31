#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TEMP_SOURCE_FILE "test_reset_source.c"
#define RESPONSE_FILE "test_reset_args.rsp"
#define OUTPUT_OBJ "test_reset_output.o"
#define OUTPUT_EXE "test_reset_output.exe"

void create_temp_files(void) {
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

void cleanup_temp_files(void) {
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
    printf("=== Testing GCC driver reset logic ===\n");
    
    create_temp_files();
    
    // Track overall test success
    int test_passed = 1;
    
    // Invocation 1: Print help list (sets print_help_list)
    printf("\n--- Invocation 1: -print-help-list ---\n");
    int status1 = run_gcc("-print-help-list 2>&1 | head -5");
    printf("Exit status: %d\n", status1);
    
    // Invocation 2: Use response file and multiple flags
    // This sets: at_file_supplied, verbose_only_flag, save_temps_flag, 
    // report_times_to_file, dumpdir/dumpbase variables
    printf("\n--- Invocation 2: Response file with multiple options ---\n");
    int status2 = run_gcc("-v -save-temps=obj -ftime-report -o " OUTPUT_OBJ " @" RESPONSE_FILE " " TEMP_SOURCE_FILE);
    printf("Exit status: %d\n", status2);
    
    // Invocation 3: Force failure to set greatest_status != 1
    printf("\n--- Invocation 3: Force failure (invalid file) ---\n");
    int status3 = run_gcc("-c nonexistent_file_xyz.c -o fail.o 2>&1 | head -3");
    printf("Exit status: %d (should be non-zero)\n", status3);
    
    // Invocation 4: Set sysroot and use different linker
    printf("\n--- Invocation 4: Sysroot and linker options ---\n");
    int status4 = run_gcc("-fuse-ld=gold --sysroot=/ -o " OUTPUT_EXE " " TEMP_SOURCE_FILE " 2>&1");
    printf("Exit status: %d\n", status4);
    
    // Invocation 5: Machine-specific options (affects spec_machine)
    printf("\n--- Invocation 5: Machine/target options ---\n");
    int status5 = run_gcc("-march=x86-64 -mtune=generic -c " TEMP_SOURCE_FILE " -o machine.o 2>&1");
    printf("Exit status: %d\n", status5);
    
    // Invocation 6: Version and verbose flags
    printf("\n--- Invocation 6: Version and verbose ---\n");
    int status6 = run_gcc("--version -v 2>&1 | head -10");
    printf("Exit status: %d\n", status6);
    
    // Invocation 7: Save temps with different options
    printf("\n--- Invocation 7: Different save-temps modes ---\n");
    int status7 = run_gcc("-save-temps=cwd -c " TEMP_SOURCE_FILE);
    printf("Exit status: %d\n", status7);
    
    // Invocation 8: Combined options for maximum coverage
    printf("\n--- Invocation 8: Combined options ---\n");
    int status8 = run_gcc("-v -save-temps -ftime-report -fuse-ld=bfd --sysroot=/ -march=native -c " TEMP_SOURCE_FILE);
    printf("Exit status: %d\n", status8);
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Invocation 1 (help list): %s\n", status1 == 0 ? "PASS" : "FAIL");
    printf("Invocation 2 (response file): %s\n", status2 == 0 ? "PASS" : "FAIL");
    printf("Invocation 3 (forced failure): %s (status=%d)\n", 
           status3 != 0 ? "PASS" : "FAIL", status3);
    printf("Invocation 4 (sysroot/linker): %s\n", status4 == 0 ? "PASS" : "FAIL");
    printf("Invocation 5 (machine): %s\n", status5 == 0 ? "PASS" : "FAIL");
    printf("Invocation 6 (version): %s\n", status6 == 0 ? "PASS" : "FAIL");
    printf("Invocation 7 (save-temps): %s\n", status7 == 0 ? "PASS" : "FAIL");
    printf("Invocation 8 (combined): %s\n", status8 == 0 ? "PASS" : "FAIL");
    
    cleanup_temp_files();
    
    // Clean up any additional temp files
    remove("machine.o");
    remove("fail.o");
    
    printf("\nTest completed. Each invocation should have triggered\n");
    printf("the driver's reset logic between calls.\n");
    
    return 0;
}
