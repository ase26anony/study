#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TEMP_SOURCE "test_reset.c"
#define RESPONSE_FILE "args.rsp"
#define OUTPUT1 "test_reset.o"
#define OUTPUT2 "test_reset.exe"

/* Create a minimal valid C source file */
void create_test_source(void) {
    FILE *f = fopen(TEMP_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(1);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

/* Create a response file with compiler arguments */
void create_response_file(void) {
    FILE *f = fopen(RESPONSE_FILE, "w");
    if (!f) {
        perror("Failed to create response file");
        exit(1);
    }
    fprintf(f, "-v\n");
    fprintf(f, "-save-temps=obj\n");
    fprintf(f, "-o %s\n", OUTPUT1);
    fclose(f);
}

/* Clean up temporary files */
void cleanup(void) {
    unlink(TEMP_SOURCE);
    unlink(RESPONSE_FILE);
    unlink(OUTPUT1);
    unlink(OUTPUT2);
    unlink("test_reset.i");
    unlink("test_reset.s");
}

/* Execute GCC driver with given arguments and return exit status */
int run_gcc(const char *arg1, ...) {
    char cmd[1024] = "gcc ";
    strcat(cmd, arg1);
    
    /* Handle variable arguments */
    va_list args;
    va_start(args, arg1);
    const char *arg;
    while ((arg = va_arg(args, const char *)) != NULL) {
        strcat(cmd, " ");
        strcat(cmd, arg);
    }
    va_end(args);
    
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    return WEXITSTATUS(status);
}

int main(int argc, char **argv) {
    printf("=== Testing GCC driver reset logic ===\n");
    
    /* Create necessary files */
    create_test_source();
    create_response_file();
    
    /* Register cleanup handler */
    atexit(cleanup);
    
    /* Series of GCC invocations to exercise reset logic */
    
    /* 1. Set print_help_list flag */
    printf("\n--- Invocation 1: -print-help-list ---\n");
    run_gcc("-print-help-list", NULL);
    
    /* 2. Set print_version flag */
    printf("\n--- Invocation 2: --version ---\n");
    run_gcc("--version", NULL);
    
    /* 3. Set verbose_only_flag and use response file (@file syntax) */
    /* Also uses non-existent file to potentially cause failure */
    printf("\n--- Invocation 3: Verbose with response file ---\n");
    run_gcc("-v", "@" RESPONSE_FILE, "non_existent.c", NULL);
    
    /* 4. Set save_temps_flag and related dumpdir/dumpbase variables */
    printf("\n--- Invocation 4: Save temps with different options ---\n");
    run_gcc("-save-temps", "-dumpdir", "mydir/", "-dumpbase", "mybase", 
            "-o", OUTPUT1, "-c", TEMP_SOURCE, NULL);
    
    /* 5. Set use_ld, sysroot, and time report flags */
    printf("\n--- Invocation 5: Linker and sysroot options ---\n");
    /* Note: -fuse-ld=gold may not be available on all systems */
    run_gcc("-fuse-ld=bfd", "--sysroot=/", "-ftime-report", 
            "-o", OUTPUT2, TEMP_SOURCE, NULL);
    
    /* 6. Attempt to set spec_machine (machine-specific) */
    printf("\n--- Invocation 6: Machine-specific options ---\n");
    /* Try various architecture/tuning options */
    run_gcc("-march=x86-64", "-mtune=generic", "-c", TEMP_SOURCE, "-o", OUTPUT1, NULL);
    
    /* 7. Combination of many flags to maximize coverage */
    printf("\n--- Invocation 7: Combined flags ---\n");
    run_gcc("-v", "-save-temps=cwd", "-ftime-report", 
            "--sysroot=/", "-fuse-ld=bfd",
            "-dumpdir", "dump/", "-dumpbase", "combined",
            "-o", OUTPUT2, TEMP_SOURCE, NULL);
    
    /* 8. Test with isysroot for target_sysroot_hdrs_suffix */
    printf("\n--- Invocation 8: Header sysroot ---\n");
    run_gcc("-isysroot", "/usr/include", "-c", TEMP_SOURCE, "-o", OUTPUT1, NULL);
    
    /* 9. Force failure to set greatest_status != 1 */
    printf("\n--- Invocation 9: Force failure ---\n");
    run_gcc("-invalid-option", TEMP_SOURCE, NULL);
    
    /* 10. Successful compilation after failure to ensure reset */
    printf("\n--- Invocation 10: Success after failure ---\n");
    run_gcc("-c", TEMP_SOURCE, "-o", OUTPUT1, NULL);
    
    /* 11. Test print_subprocess_help */
    printf("\n--- Invocation 11: Subprocess help ---\n");
    run_gcc("-###", TEMP_SOURCE, NULL);
    
    /* 12. Test with wrapper option if available */
    printf("\n--- Invocation 12: With wrapper ---\n");
    /* This would require a wrapper script; using echo as example */
    run_gcc("-wrapper", "echo", "-c", TEMP_SOURCE, "-o", OUTPUT1, NULL);
    
    printf("\n=== Test complete ===\n");
    printf("Check coverage data to verify reset logic was exercised.\n");
    
    return 0;
}
