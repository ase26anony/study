#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

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
    /* Options that set various flags in the driver */
    fprintf(f, "-v\n");
    fprintf(f, "-save-temps=obj\n");
    fprintf(f, "-Wall\n");
    fprintf(f, "-Wextra\n");
    fclose(f);
}

/* Execute GCC with given arguments and return exit status */
int run_gcc(const char *gcc_path, const char *args) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s %s", gcc_path, args);
    
    printf("Executing: %s\n", cmd);
    fflush(stdout);
    
    int status = system(cmd);
    if (status == -1) {
        perror("system() failed");
        return -1;
    }
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Clean up temporary files */
void cleanup(void) {
    remove(TEMP_SOURCE_FILE);
    remove(RESPONSE_FILE);
    remove(OUTPUT_OBJ);
    remove(OUTPUT_EXE);
    remove("test_reset_output.i");
    remove("test_reset_output.s");
    remove("test_reset_output.cmd");
}

int main(int argc, char **argv) {
    /* Use provided GCC path or default to "gcc" */
    const char *gcc_path = (argc > 1) ? argv[1] : "gcc";
    
    printf("Using GCC at: %s\n", gcc_path);
    
    /* Create necessary files */
    create_test_source();
    create_response_file();
    
    /* Register cleanup handler */
    atexit(cleanup);
    
    int overall_status = 0;
    
    /* Invocation 1: Print help list - sets print_help_list */
    printf("\n=== Invocation 1: -print-help-list ===\n");
    run_gcc(gcc_path, "-print-help-list 2>&1 | head -5");
    
    /* Invocation 2: Version - sets print_version */
    printf("\n=== Invocation 2: --version ===\n");
    run_gcc(gcc_path, "--version");
    
    /* Invocation 3: Verbose only - sets verbose_only_flag */
    printf("\n=== Invocation 3: -v (verbose) ===\n");
    run_gcc(gcc_path, "-v");
    
    /* Invocation 4: Use response file - sets at_file_supplied */
    printf("\n=== Invocation 4: With response file ===\n");
    char cmd4[512];
    snprintf(cmd4, sizeof(cmd4), "-c @%s -o %s %s", 
             RESPONSE_FILE, OUTPUT_OBJ, TEMP_SOURCE_FILE);
    run_gcc(gcc_path, cmd4);
    
    /* Invocation 5: Save temps with various flags - sets save_temps_flag, dumpdir, etc. */
    printf("\n=== Invocation 5: -save-temps variants ===\n");
    run_gcc(gcc_path, "-save-temps=cwd -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    run_gcc(gcc_path, "-save-temps=obj -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    
    /* Invocation 6: Set sysroot (even if dummy path) - affects target_system_root */
    printf("\n=== Invocation 6: --sysroot (dummy path) ===\n");
    run_gcc(gcc_path, "--sysroot=/tmp/dummy_sysroot -c " TEMP_SOURCE_FILE);
    
    /* Invocation 7: Use specific linker - sets use_ld */
    printf("\n=== Invocation 7: -fuse-ld ===\n");
    run_gcc(gcc_path, "-fuse-ld=bfd -o " OUTPUT_EXE " " TEMP_SOURCE_FILE " 2>&1 | grep -i 'ld'");
    
    /* Invocation 8: Time report - sets report_times_to_file */
    printf("\n=== Invocation 8: -ftime-report ===\n");
    run_gcc(gcc_path, "-ftime-report -c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    
    /* Invocation 9: Machine specification - attempts to set spec_machine */
    printf("\n=== Invocation 9: Machine/arch variants ===\n");
    run_gcc(gcc_path, "-march=x86-64 -mtune=generic -c " TEMP_SOURCE_FILE);
    
    /* Invocation 10: Force failure - should affect greatest_status */
    printf("\n=== Invocation 10: Force failure ===\n");
    int fail_status = run_gcc(gcc_path, "-c nonexistent_file.c -o fail.o 2>&1");
    printf("Failure invocation returned: %d\n", fail_status);
    
    /* Invocation 11: Successful compilation after failure - tests reset */
    printf("\n=== Invocation 11: Success after failure ===\n");
    run_gcc(gcc_path, "-c " TEMP_SOURCE_FILE " -o " OUTPUT_OBJ);
    
    /* Invocation 12: Multiple options combined */
    printf("\n=== Invocation 12: Combined options ===\n");
    run_gcc(gcc_path, "-v -save-temps --sysroot=/ -fuse-ld=gold -ftime-report -o " 
             OUTPUT_EXE " " TEMP_SOURCE_FILE " 2>&1 | tail -3");
    
    printf("\n=== All invocations completed ===\n");
    printf("Check coverage data to verify reset logic was exercised.\n");
    
    return overall_status;
}
