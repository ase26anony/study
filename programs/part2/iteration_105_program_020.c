#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TEMP_SOURCE "test_reset.c"
#define RESPONSE_FILE "args.rsp"
#define OUTPUT_OBJ "test_reset.o"
#define OUTPUT_EXE "test_reset.exe"

void create_temp_files(void) {
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
        fprintf(rsp, "-save-temps\n");
        fprintf(rsp, "-Wall\n");
        fclose(rsp);
    }
}

void cleanup_temp_files(void) {
    remove(TEMP_SOURCE);
    remove(RESPONSE_FILE);
    remove(OUTPUT_OBJ);
    remove(OUTPUT_EXE);
    // Clean up temporary files created by -save-temps
    system("rm -f test_reset.i test_reset.s test_reset.o");
}

int run_gcc(const char *args) {
    printf("Running: gcc %s\n", args);
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "gcc %s", args);
        
        // Use system() in child to simplify, or could execvp()
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
    
    // Series of GCC invocations to set and reset various driver state variables
    
    // 1. Set print_help_list
    printf("\n--- Invocation 1: Setting print_help_list ---\n");
    run_gcc("-print-help-list 2>&1 | head -5");
    
    // 2. Set print_version
    printf("\n--- Invocation 2: Setting print_version ---\n");
    run_gcc("--version");
    
    // 3. Set verbose_only_flag and use response file (at_file_supplied)
    printf("\n--- Invocation 3: Using response file with verbose and save-temps ---\n");
    run_gcc("-v -save-temps=obj -o " OUTPUT_OBJ " @" RESPONSE_FILE " " TEMP_SOURCE);
    
    // 4. Set multiple flags including use_ld, sysroot, time report
    printf("\n--- Invocation 4: Setting linker, sysroot, time report ---\n");
    // Note: --sysroot path may not exist, but that's OK for testing
    run_gcc("-fuse-ld=gold --sysroot=/tmp/nonexistent -ftime-report "
            "-o " OUTPUT_EXE " " TEMP_SOURCE " 2>&1 | tail -10");
    
    // 5. Attempt to set spec_machine (machine= option may not exist on all targets)
    printf("\n--- Invocation 5: Attempting to set machine spec ---\n");
    run_gcc("-march=x86-64 -mtune=generic -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 6. Force a failure to set greatest_status != 1
    printf("\n--- Invocation 6: Forcing compilation failure ---\n");
    int status = run_gcc("-c nonexistent_file.c -o fail.o 2>&1 | head -3");
    printf("Failure exit status: %d\n", status);
    
    // 7. Successful compilation after failure
    printf("\n--- Invocation 7: Successful compilation after failure ---\n");
    run_gcc("-c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 8. Test with dumpdir/dumpbase options
    printf("\n--- Invocation 8: Testing dumpdir/dumpbase options ---\n");
    run_gcc("-save-temps=cwd -dumpdir=./mydump -dumpbase=mydump "
            "-fdump-ipa-all -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // 9. Test verbose only flag
    printf("\n--- Invocation 9: Testing verbose-only flag ---\n");
    run_gcc("-v -### " TEMP_SOURCE " 2>&1 | head -10");
    
    // 10. Test with isysroot for target_sysroot_hdrs_suffix
    printf("\n--- Invocation 10: Testing sysroot suffix options ---\n");
    run_gcc("-isysroot /usr -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    printf("\n=== All invocations completed ===\n");
    
    cleanup_temp_files();
    
    return 0;
}
