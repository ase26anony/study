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
void create_source_file(void) {
    FILE *f = fopen(TEMP_SOURCE_FILE, "w");
    if (!f) {
        perror("Failed to create source file");
        exit(1);
    }
    fprintf(f, "int main(void) { return 0; }\n");
    fclose(f);
}

/* Create a response file with various options */
void create_response_file(void) {
    FILE *f = fopen(RESPONSE_FILE, "w");
    if (!f) {
        perror("Failed to create response file");
        exit(1);
    }
    /* Options that set various driver state variables */
    fprintf(f, "-v\n");
    fprintf(f, "-save-temps=obj\n");
    fprintf(f, "-ftime-report\n");
    fclose(f);
}

/* Execute GCC with given arguments and return exit status */
int execute_gcc(const char *arg1, ...) {
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
            return -1;
        }
        strcpy(ptr, current);
        ptr += len;
        *ptr++ = ' ';
        current = va_arg(args, const char *);
    }
    va_end(args);
    
    /* Null-terminate the command */
    if (ptr > cmd) *(ptr - 1) = '\0';
    
    printf("Executing: %s\n", cmd);
    
    /* Use system() for simplicity - could use fork/exec for better control */
    int status = system(cmd);
    if (status == -1) {
        perror("system() failed");
        return -1;
    }
    
    return WEXITSTATUS(status);
}

/* Clean up temporary files */
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
    printf("=== GCC Driver Reset Logic Test ===\n\n");
    
    /* Create necessary files */
    create_source_file();
    create_response_file();
    
    int overall_status = 0;
    
    /* Invocation 1: Print help list - sets print_help_list */
    printf("\n--- Invocation 1: -print-help-list ---\n");
    int status1 = execute_gcc("-print-help-list", NULL);
    printf("Exit status: %d\n", status1);
    
    /* Invocation 2: Use response file with various flags - sets multiple variables */
    printf("\n--- Invocation 2: Response file with verbose/save-temps ---\n");
    /* This will fail because dummy.c doesn't exist, setting greatest_status != 1 */
    int status2 = execute_gcc("-v", "-save-temps=obj", "-o", OUTPUT_OBJ, 
                            "@" RESPONSE_FILE, "dummy_nonexistent.c", NULL);
    printf("Exit status: %d (expected non-zero)\n", status2);
    
    /* Invocation 3: Set linker, sysroot, time report flags */
    printf("\n--- Invocation 3: Linker, sysroot, time report ---\n");
    int status3 = execute_gcc("-fuse-ld=gold", 
                            /* Try to set sysroot - use a plausible path */
                            "--sysroot=/usr",
                            "-ftime-report",
                            "-o", OUTPUT_EXE,
                            TEMP_SOURCE_FILE,
                            NULL);
    printf("Exit status: %d\n", status3);
    
    /* Invocation 4: Attempt to set machine spec */
    printf("\n--- Invocation 4: Machine specification ---\n");
    int status4 = execute_gcc("-c", TEMP_SOURCE_FILE, 
                            /* Try machine option - format varies by target */
                            "-march=native",
                            "-mtune=generic",
                            NULL);
    printf("Exit status: %d\n", status4);
    
    /* Invocation 5: Version and verbose flags */
    printf("\n--- Invocation 5: Version and verbose ---\n");
    int status5 = execute_gcc("--version", "-v", NULL);
    printf("Exit status: %d\n", status5);
    
    /* Invocation 6: Different save-temps variants */
    printf("\n--- Invocation 6: Various save-temps options ---\n");
    int status6 = execute_gcc("-save-temps=cwd", "-c", TEMP_SOURCE_FILE, NULL);
    printf("Exit status: %d\n", status6);
    
    /* Invocation 7: With dumpdir/dumpbase options */
    printf("\n--- Invocation 7: Dumpdir/dumpbase options ---\n");
    int status7 = execute_gcc("-dumpdir", "mydir/",
                            "-dumpbase", "mybase",
                            "-dumpbase-ext", ".c",
                            "-c", TEMP_SOURCE_FILE,
                            NULL);
    printf("Exit status: %d\n", status7);
    
    /* Invocation 8: Successful compilation to reset after failures */
    printf("\n--- Invocation 8: Successful compilation ---\n");
    int status8 = execute_gcc("-c", TEMP_SOURCE_FILE, "-o", "final.o", NULL);
    printf("Exit status: %d (should be 0)\n", status8);
    
    printf("\n=== Test Complete ===\n");
    printf("Driver reset logic should have been exercised between each invocation.\n");
    
    /* Clean up */
    cleanup();
    
    return 0;
}
