#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TEMP_SOURCE_FILE "test_reset_source.c"
#define RESPONSE_FILE "test_reset_args.rsp"
#define OUTPUT_OBJ "test_reset_output.o"
#define OUTPUT_EXE "test_reset_output.exe"

/* Create a minimal valid C source file */
void create_test_source(void) {
    FILE *f = fopen(TEMP_SOURCE_FILE, "w");
    if (!f) {
        perror("Failed to create source file");
        exit(1);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

/* Create a response file with various options */
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

/* Clean up temporary files */
void cleanup_files(void) {
    unlink(TEMP_SOURCE_FILE);
    unlink(RESPONSE_FILE);
    unlink(OUTPUT_OBJ);
    unlink(OUTPUT_EXE);
    unlink("test_reset_source.i");
    unlink("test_reset_source.s");
    unlink("test_reset_source.o");
}

/* Execute GCC driver with given arguments and return exit status */
int execute_gcc(const char *args) {
    printf("Executing: gcc %s\n", args);
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "gcc %s", args);
        
        /* Use system() in child to simplify error handling */
        int ret = system(cmd);
        exit(WEXITSTATUS(ret));
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    } else {
        perror("fork failed");
        return -1;
    }
}

int main(int argc, char **argv) {
    printf("=== GCC Driver Reset Logic Test ===\n");
    
    /* Create necessary files */
    create_test_source();
    create_response_file();
    
    int overall_status = 0;
    
    /* Invocation 1: Set print_help_list flag */
    printf("\n--- Invocation 1: -print-help-list ---\n");
    int status1 = execute_gcc("-print-help-list 2>&1 | head -5");
    printf("Exit status: %d\n", status1);
    
    /* Invocation 2: Use response file with multiple flags, 
       includes non-existent file to potentially fail */
    printf("\n--- Invocation 2: Response file with verbose/save-temps ---\n");
    int status2 = execute_gcc("-v -save-temps=obj -o " OUTPUT_OBJ " @" RESPONSE_FILE " " TEMP_SOURCE_FILE);
    printf("Exit status: %d\n", status2);
    
    /* Invocation 3: Set use_ld, sysroot, and time report flags */
    printf("\n--- Invocation 3: Linker, sysroot, and timing flags ---\n");
    int status3 = execute_gcc("-fuse-ld=gold -ftime-report -o " OUTPUT_EXE " " TEMP_SOURCE_FILE);
    printf("Exit status: %d\n", status3);
    
    /* Invocation 4: Attempt to set spec_machine (may be target-specific) */
    printf("\n--- Invocation 4: Machine-specific options ---\n");
    int status4 = execute_gcc("-mtune=generic -march=x86-64 -c " TEMP_SOURCE_FILE);
    printf("Exit status: %d\n", status4);
    
    /* Invocation 5: Version flag to set print_version */
    printf("\n--- Invocation 5: Version information ---\n");
    int status5 = execute_gcc("--version");
    printf("Exit status: %d\n", status5);
    
    /* Invocation 6: Verbose only flag */
    printf("\n--- Invocation 6: Verbose flag ---\n");
    int status6 = execute_gcc("-v");
    printf("Exit status: %d\n", status6);
    
    /* Invocation 7: Different save-temps option */
    printf("\n--- Invocation 7: Alternative save-temps ---\n");
    int status7 = execute_gcc("-save-temps=cwd -c " TEMP_SOURCE_FILE);
    printf("Exit status: %d\n", status7);
    
    /* Invocation 8: With sysroot (using dummy path) */
    printf("\n--- Invocation 8: Sysroot option ---\n");
    int status8 = execute_gcc("--sysroot=/tmp/dummy_sysroot -c " TEMP_SOURCE_FILE);
    printf("Exit status: %d\n", status8);
    
    /* Invocation 9: Invalid option to cause failure (sets greatest_status != 1) */
    printf("\n--- Invocation 9: Invalid option to trigger failure ---\n");
    int status9 = execute_gcc("-invalid-option-that-does-not-exist 2>/dev/null");
    printf("Exit status: %d (should be non-zero)\n", status9);
    
    /* Invocation 10: Successful compilation after failure */
    printf("\n--- Invocation 10: Successful compilation after failure ---\n");
    int status10 = execute_gcc("-c " TEMP_SOURCE_FILE " -o final.o");
    printf("Exit status: %d\n", status10);
    
    /* Clean up */
    cleanup_files();
    
    printf("\n=== Test completed ===\n");
    printf("Summary of exit statuses: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
           status1, status2, status3, status4, status5,
           status6, status7, status8, status9, status10);
    
    return 0;
}
