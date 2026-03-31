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
    unlink("test_reset_logic.i");
    unlink("test_reset_logic.s");
    unlink("test_reset_logic.o");
}

int main(int argc, char **argv) {
    // Register cleanup handler
    atexit(cleanup);
    
    // Create test files
    create_test_source();
    create_response_file();
    
    int overall_status = 0;
    
    // Invocation 1: Set print_help_list
    printf("\n=== Invocation 1: -print-help-list ===\n");
    int status1 = run_gcc("-print-help-list");
    printf("Exit status: %d\n", status1);
    
    // Invocation 2: Set multiple flags including response file (@file)
    // This will likely fail because dummy.c doesn't exist, setting greatest_status != 1
    printf("\n=== Invocation 2: Response file with non-existent source ===\n");
    int status2 = run_gcc("-v -save-temps=obj -o " OUTPUT_OBJ " @" RESPONSE_FILE " dummy.c");
    printf("Exit status: %d (expected non-zero)\n", status2);
    
    // Invocation 3: Set use_ld, sysroot, time report flags with valid source
    printf("\n=== Invocation 3: Valid compilation with various flags ===\n");
    int status3 = run_gcc("-fuse-ld=gold --sysroot=/ -ftime-report -o " OUTPUT_EXE " " TEMP_SOURCE);
    printf("Exit status: %d\n", status3);
    
    // Invocation 4: Try to set spec_machine (may be target-specific)
    printf("\n=== Invocation 4: Machine-specific options ===\n");
    int status4 = run_gcc("-mtune=generic -march=x86-64 -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    printf("Exit status: %d\n", status4);
    
    // Invocation 5: Set verbose_only_flag and version flag
    printf("\n=== Invocation 5: Version and verbose flags ===\n");
    int status5 = run_gcc("--version -v");
    printf("Exit status: %d\n", status5);
    
    // Invocation 6: Different save-temps variants to exercise dumpdir logic
    printf("\n=== Invocation 6: Various save-temps options ===\n");
    int status6 = run_gcc("-save-temps=cwd -c " TEMP_SOURCE);
    printf("Exit status: %d\n", status6);
    
    // Invocation 7: Another valid compilation to ensure reset after failures
    printf("\n=== Invocation 7: Clean compilation after all flags ===\n");
    int status7 = run_gcc("-c " TEMP_SOURCE " -o final.o");
    printf("Exit status: %d\n", status7);
    
    // Summary
    printf("\n=== Summary ===\n");
    printf("All invocations completed. The driver's reset logic should have been exercised.\n");
    printf("Check coverage data to verify lines 11228-11250 were executed.\n");
    
    return 0;
}
