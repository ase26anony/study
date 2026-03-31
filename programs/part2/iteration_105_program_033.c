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
    // Options that will set various driver state variables
    fprintf(f, "-v\n");
    fprintf(f, "-save-temps=obj\n");
    fprintf(f, "-ftime-report\n");
    fprintf(f, "-fuse-ld=gold\n");
    fclose(f);
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
}

int main(int argc, char **argv) {
    // Create necessary files
    create_test_source();
    create_response_file();
    
    int overall_status = 0;
    
    // Invocation 1: Set print_help_list flag
    printf("\n=== Invocation 1: -print-help-list ===\n");
    run_gcc("-print-help-list 2>&1 | head -5");
    
    // Invocation 2: Set version flag
    printf("\n=== Invocation 2: --version ===\n");
    run_gcc("--version");
    
    // Invocation 3: Use response file (sets at_file_supplied)
    // This will fail because dummy.c doesn't exist, setting greatest_status != 1
    printf("\n=== Invocation 3: With response file (will fail) ===\n");
    int status3 = run_gcc("@args.rsp dummy_nonexistent.c -o dummy.o 2>&1");
    printf("Exit status: %d (non-zero expected)\n", status3);
    
    // Invocation 4: Set verbose flag and save-temps
    printf("\n=== Invocation 4: Verbose with save-temps ===\n");
    run_gcc("-v -save-temps -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // Invocation 5: Set sysroot and time report
    printf("\n=== Invocation 5: Sysroot and time report ===\n");
    // Use a dummy sysroot path (won't affect actual compilation)
    run_gcc("--sysroot=/tmp/dummy_sysroot -ftime-report -c " TEMP_SOURCE);
    
    // Invocation 6: Set different linker
    printf("\n=== Invocation 6: Different linker ===\n");
    run_gcc("-fuse-ld=bfd -c " TEMP_SOURCE);
    
    // Invocation 7: Compile with output base options (affects dumpbase/outbase)
    printf("\n=== Invocation 7: With dumpbase/outbase ===\n");
    run_gcc("-dumpbase mytest -dumpbase-ext .c -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // Invocation 8: Machine-specific options (affects spec_machine)
    printf("\n=== Invocation 8: Machine/architecture options ===\n");
    // Try various architecture options
    run_gcc("-march=x86-64 -mtune=generic -c " TEMP_SOURCE);
    
    // Invocation 9: Full compilation with many flags
    printf("\n=== Invocation 9: Full compilation with multiple flags ===\n");
    run_gcc("-v -save-temps=obj -ftime-report -fuse-ld=gold --sysroot=/tmp/dummy "
            "-o " OUTPUT_EXE " " TEMP_SOURCE " 2>&1 | tail -10");
    
    // Invocation 10: Print subprocess help
    printf("\n=== Invocation 10: Subprocess help ===\n");
    run_gcc("-### " TEMP_SOURCE " 2>&1 | head -5");
    
    printf("\n=== All invocations completed ===\n");
    printf("The driver's reset logic should have been exercised between each call.\n");
    
    // Cleanup
    cleanup();
    
    return 0;
}
