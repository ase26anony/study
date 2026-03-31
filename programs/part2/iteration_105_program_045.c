#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TEMP_SOURCE_FILE "test_reset_source.c"
#define RESPONSE_FILE "test_args.rsp"
#define OUTPUT_OBJ "test_reset.o"
#define OUTPUT_EXE "test_reset.exe"

void create_test_source(void) {
    FILE *f = fopen(TEMP_SOURCE_FILE, "w");
    if (!f) {
        perror("Failed to create test source file");
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
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        perror("execl failed");
        exit(255);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    } else {
        perror("fork failed");
        return -1;
    }
}

void cleanup(void) {
    remove(TEMP_SOURCE_FILE);
    remove(RESPONSE_FILE);
    remove(OUTPUT_OBJ);
    remove(OUTPUT_EXE);
    remove("test_reset_source.i");
    remove("test_reset_source.s");
    remove("test_reset_source.o");
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
    
    // Invocation 3: Set verbose_only_flag and use response file (at_file_supplied)
    printf("\n=== Invocation 3: Verbose with response file ===\n");
    run_gcc("-v @test_args.rsp -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    
    // Invocation 4: Set save_temps_flag and related variables
    printf("\n=== Invocation 4: Save temps with dumpdir ===\n");
    run_gcc("-save-temps=obj -dumpdir=./dump_test/ -dumpbase=testdump "
            "-c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    
    // Invocation 5: Set use_ld and sysroot variables
    printf("\n=== Invocation 5: Linker and sysroot options ===\n");
    // Try different linkers if available
    run_gcc("-fuse-ld=bfd --sysroot=/tmp/dummy_sysroot "
            TEMP_SOURCE_FILE " -o " OUTPUT_EXE " 2>/dev/null");
    
    // Invocation 6: Set report_times_to_file
    printf("\n=== Invocation 6: Time reporting ===\n");
    run_gcc("-ftime-report -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    
    // Invocation 7: Attempt to set spec_machine (architecture-specific)
    printf("\n=== Invocation 7: Machine/architecture options ===\n");
    run_gcc("-march=x86-64 -mtune=generic -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    
    // Invocation 8: Cause failure to set greatest_status != 1
    printf("\n=== Invocation 8: Force failure (non-existent file) ===\n");
    int status = run_gcc("-c non_existent_file.c -o fail.o 2>/dev/null");
    printf("Failure exit status: %d\n", status);
    
    // Invocation 9: Successful compilation after failure
    printf("\n=== Invocation 9: Success after failure ===\n");
    run_gcc("-c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    
    // Invocation 10: Complex combination
    printf("\n=== Invocation 10: Complex option combination ===\n");
    run_gcc("-v -save-temps -ftime-report -fuse-ld=gold "
            "-c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    
    // Cleanup
    cleanup();
    
    printf("\n=== Test completed ===\n");
    printf("The GCC driver's reset logic should have been exercised multiple times.\n");
    printf("Check coverage for lines 11228-11250 in gcc.cc\n");
    
    return overall_status;
}
