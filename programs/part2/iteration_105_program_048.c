#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TEMP_SOURCE "test_reset_source.c"
#define RESPONSE_FILE "test_args.rsp"
#define OUTPUT_OBJ "test_reset_output.o"
#define OUTPUT_EXE "test_reset_output.exe"

void create_test_files(void) {
    // Create a minimal valid C source file
    FILE *src = fopen(TEMP_SOURCE, "w");
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
    // Clean up temporary files
    remove(TEMP_SOURCE);
    remove(RESPONSE_FILE);
    remove(OUTPUT_OBJ);
    remove(OUTPUT_EXE);
    remove("test_reset_output.i");
    remove("test_reset_output.s");
    remove("test_reset_output.o");
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
    
    create_test_files();
    
    int overall_status = 0;
    
    // Invocation 1: Set print_help_list
    printf("\n--- Invocation 1: -print-help-list ---\n");
    int status1 = run_gcc("-print-help-list 2>&1 | head -5");
    printf("Exit status: %d\n", status1);
    
    // Invocation 2: Set multiple flags with response file, will fail (dummy.c doesn't exist)
    // This should set at_file_supplied, save_temps_flag, verbose_only_flag,
    // and cause failure (affecting greatest_status)
    printf("\n--- Invocation 2: Response file with non-existent source ---\n");
    int status2 = run_gcc("-v -save-temps=obj -o " OUTPUT_OBJ " @" RESPONSE_FILE " dummy.c 2>&1 | tail -3");
    printf("Exit status: %d (expected non-zero)\n", status2);
    
    // Invocation 3: Set use_ld, sysroot, time report with valid source
    // Use a dummy sysroot path (won't affect actual compilation)
    printf("\n--- Invocation 3: Valid compilation with various options ---\n");
    int status3 = run_gcc("-fuse-ld=gold --sysroot=/tmp/dummy_sysroot -ftime-report -o " OUTPUT_EXE " " TEMP_SOURCE " 2>&1 | tail -5");
    printf("Exit status: %d\n", status3);
    
    // Invocation 4: Try to set spec_machine (architecture-specific)
    // Use -march and -mtune to potentially affect spec_machine
    printf("\n--- Invocation 4: Architecture/machine options ---\n");
    int status4 = run_gcc("-march=x86-64 -mtune=generic -c " TEMP_SOURCE " -o " OUTPUT_OBJ " 2>&1 | tail -3");
    printf("Exit status: %d\n", status4);
    
    // Invocation 5: Test save-temps variants and dumpdir/dumpbase
    printf("\n--- Invocation 5: Save-temps variants ---\n");
    int status5 = run_gcc("-save-temps=cwd -dumpdir=./dump_test -dumpbase=testdump -c " TEMP_SOURCE " -o " OUTPUT_OBJ " 2>&1 | tail -3");
    printf("Exit status: %d\n", status5);
    
    // Invocation 6: Version and verbose flags
    printf("\n--- Invocation 6: Version and verbose ---\n");
    int status6 = run_gcc("--version -v 2>&1 | head -10");
    printf("Exit status: %d\n", status6);
    
    // Invocation 7: Another valid compilation to ensure reset after previous operations
    printf("\n--- Invocation 7: Clean compilation (testing reset) ---\n");
    int status7 = run_gcc("-c " TEMP_SOURCE " -o final.o 2>&1 | tail -2");
    printf("Exit status: %d\n", status7);
    
    printf("\n=== Summary of invocations ===\n");
    printf("1. Help list: %d\n", status1);
    printf("2. Failed with response file: %d\n", status2);
    printf("3. With sysroot/time report: %d\n", status3);
    printf("4. Architecture options: %d\n", status4);
    printf("5. Save-temps variants: %d\n", status5);
    printf("6. Version/verbose: %d\n", status6);
    printf("7. Clean compilation: %d\n", status7);
    
    cleanup_test_files();
    
    // Clean up additional files that might have been created
    remove("final.o");
    remove("test_reset_source.i");
    remove("test_reset_source.s");
    remove("testdump.i");
    remove("testdump.s");
    
    printf("\nTest completed. Check coverage of gcc.cc reset lines.\n");
    return 0;
}
