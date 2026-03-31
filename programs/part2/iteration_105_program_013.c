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
        
        // Use system to properly handle shell parsing
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
    printf("=== Testing GCC driver reset logic ===\n");
    
    // Create test files
    create_test_source();
    create_response_file();
    
    int overall_status = 0;
    
    // Invocation 1: Set print_help_list flag
    printf("\n--- Invocation 1: -print-help-list ---\n");
    int status1 = run_gcc("-print-help-list 2>&1 | head -5");
    printf("Exit status: %d\n", status1);
    
    // Invocation 2: Use response file with verbose and save-temps
    // This will likely fail because dummy.c doesn't exist, setting greatest_status != 1
    printf("\n--- Invocation 2: Response file with non-existent source ---\n");
    int status2 = run_gcc("-v -save-temps=obj -o test.o @" RESPONSE_FILE " dummy.c 2>&1");
    printf("Exit status: %d (expected non-zero)\n", status2);
    
    // Invocation 3: Multiple flags including sysroot and time report
    printf("\n--- Invocation 3: Multiple flags with valid source ---\n");
    int status3 = run_gcc("-fuse-ld=gold -ftime-report -o test.exe " TEMP_SOURCE " 2>&1");
    printf("Exit status: %d\n", status3);
    
    // Invocation 4: Try to set spec_machine (may be target-specific)
    printf("\n--- Invocation 4: Machine-specific options ---\n");
    int status4 = run_gcc("-march=native -mtune=generic -c " TEMP_SOURCE " 2>&1");
    printf("Exit status: %d\n", status4);
    
    // Invocation 5: Version and verbose flags
    printf("\n--- Invocation 5: Version and verbose ---\n");
    int status5 = run_gcc("--version -v 2>&1 | head -10");
    printf("Exit status: %d\n", status5);
    
    // Invocation 6: Different save-temps modes
    printf("\n--- Invocation 6: Various save-temps options ---\n");
    int status6 = run_gcc("-save-temps=cwd -c " TEMP_SOURCE " 2>&1");
    printf("Exit status: %d\n", status6);
    
    // Invocation 7: Sysroot options (using dummy paths)
    printf("\n--- Invocation 7: Sysroot options ---\n");
    int status7 = run_gcc("--sysroot=/dummy/sysroot -isysroot /dummy/isysroot -c " TEMP_SOURCE " 2>&1");
    printf("Exit status: %d\n", status7);
    
    // Invocation 8: Print subprocess help
    printf("\n--- Invocation 8: Subprocess help ---\n");
    int status8 = run_gcc("-print-prog-name=cc1 2>&1");
    printf("Exit status: %d\n", status8);
    
    // Invocation 9: Final successful compilation to ensure reset after failures
    printf("\n--- Invocation 9: Clean compilation ---\n");
    int status9 = run_gcc("-c " TEMP_SOURCE " -o final.o 2>&1");
    printf("Exit status: %d\n", status9);
    
    printf("\n=== Summary of invocations ===\n");
    printf("Invocation 2 (with failure): %d\n", status2);
    printf("Most successful compilations should return 0\n");
    
    // Clean up
    cleanup_files();
    
    return 0;
}
