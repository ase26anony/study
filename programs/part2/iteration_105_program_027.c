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
    char cmd[4096] = "gcc ";
    char *ptr = cmd + 4;  /* Start after "gcc " */
    
    /* Build command string from variable arguments */
    va_list args;
    va_start(args, arg1);
    
    const char *current = arg1;
    while (current) {
        size_t len = strlen(current);
        if (ptr + len + 2 > cmd + sizeof(cmd)) {
            fprintf(stderr, "Command too long\n");
            va_end(args);
            return -1;
        }
        strcpy(ptr, current);
        ptr += len;
        *ptr++ = ' ';
        current = va_arg(args, const char *);
    }
    va_end(args);
    
    if (ptr > cmd) {
        *(ptr - 1) = '\0';  /* Remove trailing space */
    }
    
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

int main(int argc, char **argv) {
    int overall_status = 0;
    
    /* Create necessary files */
    create_test_source();
    create_response_file();
    
    printf("=== Starting GCC driver reset logic test ===\n\n");
    
    /* Invocation 1: Set print_help_list flag */
    printf("1. Testing -print-help-list (sets print_help_list)\n");
    int status1 = run_gcc("-print-help-list", NULL);
    printf("Exit status: %d\n\n", status1);
    
    /* Invocation 2: Use response file and multiple flags
       This will likely fail (dummy.c doesn't exist), setting greatest_status != 1 */
    printf("2. Testing with response file and multiple flags (should fail)\n");
    int status2 = run_gcc("-v", "-save-temps=obj", "-o", OUTPUT1, 
                         "@" RESPONSE_FILE, "dummy.c", NULL);
    printf("Exit status: %d\n\n", status2);
    
    /* Invocation 3: Set multiple flags including sysroot and time report
       Use a valid source file this time */
    printf("3. Testing with sysroot, time report, and valid source\n");
    int status3 = run_gcc("-fuse-ld=gold", 
                         "--sysroot=/usr",  /* Use existing sysroot path */
                         "-ftime-report",
                         "-o", OUTPUT2,
                         TEMP_SOURCE,
                         NULL);
    printf("Exit status: %d\n\n", status3);
    
    /* Invocation 4: Attempt to set spec_machine via -march/-mtune */
    printf("4. Testing machine-specific options\n");
    int status4 = run_gcc("-march=x86-64", "-mtune=generic", 
                         "-c", TEMP_SOURCE, "-o", OUTPUT1, NULL);
    printf("Exit status: %d\n\n", status4);
    
    /* Invocation 5: Test save-temps with dumpdir */
    printf("5. Testing save-temps with explicit dumpdir\n");
    int status5 = run_gcc("-save-temps", "-fdumpdir=./dumps/",
                         "-c", TEMP_SOURCE, "-o", OUTPUT1, NULL);
    printf("Exit status: %d\n\n", status5);
    
    /* Invocation 6: Test verbose only flag */
    printf("6. Testing verbose flag\n");
    int status6 = run_gcc("-v", "-c", TEMP_SOURCE, "-o", OUTPUT1, NULL);
    printf("Exit status: %d\n\n", status6);
    
    /* Invocation 7: Test version flag */
    printf("7. Testing version flag\n");
    int status7 = run_gcc("--version", NULL);
    printf("Exit status: %d\n\n", status7);
    
    /* Invocation 8: Test isysroot for target_system_root_changed */
    printf("8. Testing isysroot option\n");
    int status8 = run_gcc("-isysroot", "/usr/include",
                         "-c", TEMP_SOURCE, "-o", OUTPUT1, NULL);
    printf("Exit status: %d\n\n", status8);
    
    /* Invocation 9: Test with -B option (affects target_system_root) */
    printf("9. Testing -B option\n");
    int status9 = run_gcc("-B/usr/bin",
                         "-c", TEMP_SOURCE, "-o", OUTPUT1, NULL);
    printf("Exit status: %d\n\n", status9);
    
    /* Invocation 10: Final successful compilation to ensure reset after failures */
    printf("10. Final successful compilation\n");
    int status10 = run_gcc("-c", TEMP_SOURCE, "-o", OUTPUT1, NULL);
    printf("Exit status: %d\n\n", status10);
    
    printf("=== Test complete ===\n");
    printf("Summary of exit statuses: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
           status1, status2, status3, status4, status5,
           status6, status7, status8, status9, status10);
    
    /* Clean up */
    cleanup();
    
    return overall_status;
}
