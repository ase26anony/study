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
    // Clean up any .i, .s, .o files that might have been created
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
    
    int overall_status = 0;
    
    // Invocation 1: Set print_help_list flag
    printf("\n--- Invocation 1: -print-help-list ---\n");
    int status1 = run_gcc("-print-help-list 2>&1 | head -5");
    printf("Exit status: %d\n", status1);
    
    // Invocation 2: Use response file with multiple flags
    // This will likely fail because dummy.c doesn't exist, setting greatest_status != 1
    printf("\n--- Invocation 2: Response file with non-existent source ---\n");
    int status2 = run_gcc("-v -save-temps=obj -o " OUTPUT_OBJ " @" RESPONSE_FILE " dummy.c 2>&1 | tail -3");
    printf("Exit status: %d\n", status2);
    
    // Invocation 3: Set multiple flags with valid source
    printf("\n--- Invocation 3: Multiple flags with valid source ---\n");
    int status3 = run_gcc("-fuse-ld=gold -ftime-report -o " OUTPUT_EXE " " TEMP_SOURCE_FILE " 2>&1 | tail -5");
    printf("Exit status: %d\n", status3);
    
    // Invocation 4: Try to set spec_machine (may be target-specific)
    printf("\n--- Invocation 4: Attempt to set machine spec ---\n");
    int status4 = run_gcc("-c " TEMP_SOURCE_FILE " -march=x86-64 -mtune=generic 2>&1");
    printf("Exit status: %d\n", status4);
    
    // Invocation 5: Test sysroot options (use dummy path)
    printf("\n--- Invocation 5: Sysroot options ---\n");
    int status5 = run_gcc("--sysroot=/dummy/sysroot -c " TEMP_SOURCE_FILE " 2>&1");
    printf("Exit status: %d\n", status5);
    
    // Invocation 6: Test verbose only flag
    printf("\n--- Invocation 6: Verbose flag ---\n");
    int status6 = run_gcc("-v -E " TEMP_SOURCE_FILE " 2>&1 | head -10");
    printf("Exit status: %d\n", status6);
    
    // Invocation 7: Test version flag
    printf("\n--- Invocation 7: Version flag ---\n");
    int status7 = run_gcc("--version 2>&1 | head -2");
    printf("Exit status: %d\n", status7);
    
    // Invocation 8: Test save-temps with different options
    printf("\n--- Invocation 8: Various save-temps options ---\n");
    int status8 = run_gcc("-save-temps=cwd -c " TEMP_SOURCE_FILE " 2>&1");
    printf("Exit status: %d\n", status8);
    
    // Invocation 9: Test dumpbase options
    printf("\n--- Invocation 9: Dumpbase options ---\n");
    int status9 = run_gcc("-dumpbase test_dump -c " TEMP_SOURCE_FILE " 2>&1");
    printf("Exit status: %d\n", status9);
    
    // Invocation 10: Final simple compilation to ensure reset worked
    printf("\n--- Invocation 10: Simple compilation (post-reset check) ---\n");
    int status10 = run_gcc("-c " TEMP_SOURCE_FILE " -o final.o 2>&1");
    printf("Exit status: %d\n", status10);
    
    printf("\n=== Test completed ===\n");
    printf("Summary of exit statuses: %d %d %d %d %d %d %d %d %d %d\n",
           status1, status2, status3, status4, status5,
           status6, status7, status8, status9, status10);
    
    cleanup_temp_files();
    
    return 0;
}
