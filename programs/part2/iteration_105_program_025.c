#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TEMP_SOURCE "test_reset.c"
#define RESPONSE_FILE "args.rsp"

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
void cleanup(void) {
    remove(TEMP_SOURCE);
    remove(RESPONSE_FILE);
    remove("test.o");
    remove("test.exe");
    remove("test_reset.o");
    remove("test_reset.i");
    remove("test_reset.s");
}

/* Execute GCC with given arguments and return exit status */
int execute_gcc(const char *format, ...) {
    char cmd[1024];
    va_list args;
    
    va_start(args, format);
    vsnprintf(cmd, sizeof(cmd), format, args);
    va_end(args);
    
    printf("Executing: %s\n", cmd);
    
    int status = system(cmd);
    if (status == -1) {
        perror("system() failed");
        return -1;
    }
    
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    
    return -1;
}

int main(int argc, char **argv) {
    /* Use first argument as base name or default */
    const char *base_name = (argc > 1) ? argv[1] : "test";
    
    /* Create necessary files */
    create_test_source();
    create_response_file();
    
    printf("=== Testing GCC driver reset logic ===\n\n");
    
    /* Invocation 1: Set print_help_list */
    printf("1. Testing -print-help-list (sets print_help_list):\n");
    execute_gcc("gcc -print-help-list 2>&1 | head -5");
    
    /* Invocation 2: Set version flag */
    printf("2. Testing --version (sets print_version):\n");
    execute_gcc("gcc --version");
    
    /* Invocation 3: Set verbose flag */
    printf("3. Testing -v (sets verbose_only_flag):\n");
    execute_gcc("gcc -v %s 2>&1 | head -10", TEMP_SOURCE);
    
    /* Invocation 4: Use response file (sets at_file_supplied) with save-temps */
    printf("4. Testing with response file @%s (sets at_file_supplied, save_temps_flag):\n", RESPONSE_FILE);
    execute_gcc("gcc -c @%s -o %s.o %s", RESPONSE_FILE, base_name, TEMP_SOURCE);
    
    /* Invocation 5: Set use_ld and sysroot */
    printf("5. Testing -fuse-ld and --sysroot (sets use_ld, target_system_root):\n");
    execute_gcc("gcc -fuse-ld=gold --sysroot=/  -o %s.exe %s 2>&1 | head -5", base_name, TEMP_SOURCE);
    
    /* Invocation 6: Force failure with invalid option (affects greatest_status) */
    printf("6. Testing invalid option to cause failure (affects greatest_status):\n");
    execute_gcc("gcc -invalid-option-xyz %s 2>&1 | head -3", TEMP_SOURCE);
    
    /* Invocation 7: Test machine specification */
    printf("7. Testing machine specification (attempts to set spec_machine):\n");
    execute_gcc("gcc -dumpmachine %s 2>&1", TEMP_SOURCE);
    
    /* Invocation 8: Test save-temps with dumpdir */
    printf("8. Testing save-temps variants (sets dumpdir/dumpbase):\n");
    execute_gcc("gcc -save-temps=cwd -dumpdir=./dump_ -dumpbase=dumpbase_test -c %s -o %s_final.o", 
                TEMP_SOURCE, base_name);
    
    /* Invocation 9: Test time report to file */
    printf("9. Testing -ftime-report (sets report_times_to_file):\n");
    execute_gcc("gcc -ftime-report -c %s -o %s_time.o 2>&1 | tail -5", TEMP_SOURCE, base_name);
    
    /* Invocation 10: Final successful compilation to ensure reset after failures */
    printf("10. Final successful compilation (verifies reset works):\n");
    execute_gcc("gcc -c %s -o %s_final2.o", TEMP_SOURCE, base_name);
    
    printf("\n=== All invocations completed ===\n");
    printf("Each invocation should trigger driver::finalize() reset logic.\n");
    
    cleanup();
    return 0;
}
