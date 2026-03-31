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
    // Remove temporary files
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
    
    // Invocation 1: Set print_help_list
    printf("\n=== Invocation 1: -print-help-list ===\n");
    run_gcc("-print-help-list 2>/dev/null");
    
    // Invocation 2: Set print_version
    printf("\n=== Invocation 2: --version ===\n");
    run_gcc("--version");
    
    // Invocation 3: Set verbose_only_flag and use response file (@file syntax)
    // This also sets at_file_supplied = 1
    printf("\n=== Invocation 3: With response file ===\n");
    run_gcc("-v -save-temps=obj -o " OUTPUT_OBJ " @" RESPONSE_FILE " " TEMP_SOURCE);
    
    // Invocation 4: Set use_ld, target_system_root (via --sysroot), 
    // and report_times_to_file
    printf("\n=== Invocation 4: Multiple state-setting options ===\n");
    // Note: --sysroot with dummy path to trigger the variable setting
    // even if the path doesn't exist
    run_gcc("-fuse-ld=gold --sysroot=/tmp/dummy_sysroot -ftime-report "
            "-o " OUTPUT_EXE " " TEMP_SOURCE);
    
    // Invocation 5: Attempt to set spec_machine via -mtune
    printf("\n=== Invocation 5: Set machine-specific options ===\n");
    run_gcc("-mtune=generic -march=x86-64 -c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // Invocation 6: Force failure to set greatest_status != 1
    printf("\n=== Invocation 6: Force compilation failure ===\n");
    int fail_status = run_gcc("-c nonexistent_file.c -o fail.o 2>/dev/null");
    printf("Failure status: %d (should be non-zero)\n", fail_status);
    
    // Invocation 7: Successful compilation after failure
    printf("\n=== Invocation 7: Successful compilation after failure ===\n");
    run_gcc("-c " TEMP_SOURCE " -o " OUTPUT_OBJ);
    
    // Invocation 8: Test save_temps variants
    printf("\n=== Invocation 8: Test save_temps variants ===\n");
    run_gcc("-save-temps=cwd -c " TEMP_SOURCE);
    
    // Invocation 9: Test with dumpdir/dumpbase options
    printf("\n=== Invocation 9: Test dumpdir/dumpbase ===\n");
    run_gcc("-dumpdir ./dump_test -dumpbase test.dump -c " TEMP_SOURCE);
    
    // Invocation 10: Final clean compilation to ensure reset works
    printf("\n=== Invocation 10: Final clean compilation ===\n");
    run_gcc("-c " TEMP_SOURCE);
    
    // Cleanup
    cleanup();
    
    printf("\n=== Test completed ===\n");
    printf("The GCC driver's reset logic should have been exercised multiple times.\n");
    printf("Check coverage for lines 11228-11250 in gcc.cc\n");
    
    return overall_status;
}
