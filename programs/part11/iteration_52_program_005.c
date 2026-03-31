/* test_driver_init.c - Test GCC driver initialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Function prototypes for the temporary source files */
int foo(void);
int bar(void);

/* Create temporary source files with different characteristics */
static int create_temp_files(void) {
    FILE *f;
    
    /* Valid source file 1 */
    f = fopen("temp1.c", "w");
    if (!f) return 0;
    fprintf(f, "int foo(void) { return 0; }\n");
    fclose(f);
    
    /* Valid source file 2 */
    f = fopen("temp2.c", "w");
    if (!f) return 0;
    fprintf(f, "int bar(void) { return 1; }\n");
    fclose(f);
    
    /* Source file with syntax error */
    f = fopen("syntax_error.c", "w");
    if (!f) return 0;
    fprintf(f, "int baz(void) { return /* missing semicolon and value */ }\n");
    fclose(f);
    
    /* Another valid source file for additional tests */
    f = fopen("temp3.c", "w");
    if (!f) return 0;
    fprintf(f, "int main(void) { return foo() + bar(); }\n");
    fclose(f);
    
    return 1;
}

/* Clean up temporary files */
static void cleanup_temp_files(void) {
    remove("temp1.c");
    remove("temp2.c");
    remove("temp3.c");
    remove("syntax_error.c");
    remove("temp1.o");
    remove("temp2.o");
    remove("temp3.o");
    remove("temp1.i");
    remove("temp1.s");
    remove("base.i");
    remove("base.s");
    remove("base.o");
    
    /* Clean up dump directory */
    system("rm -rf ./dump1 ./dump2 2>/dev/null");
}

/* Execute a system command and return success status */
static int run_command(const char *cmd) {
    int status = system(cmd);
    return (status == 0 || WEXITSTATUS(status) == 0);
}

int main(void) {
    int checksum = 0;
    int result;
    
    printf("Testing GCC driver initialization logic...\n");
    
    /* Create temporary source files */
    if (!create_temp_files()) {
        fprintf(stderr, "Failed to create temporary files\n");
        return 1;
    }
    
    /* Create dump directories */
    mkdir("./dump1", 0755);
    mkdir("./dump2", 0755);
    
    /* Sequence 1: Help flag then compilation
       This should set print_help_list, then reset it */
    printf("\n1. Testing help flag reset...\n");
    result = run_command("gcc --help=common > /dev/null 2>&1");
    checksum = (checksum << 1) | (result ? 1 : 0);
    
    result = run_command("gcc -c temp1.c -o temp1.o");
    checksum = (checksum << 1) | (result ? 1 : 0);
    printf("   Help then compile: %s\n", result ? "PASS" : "FAIL");
    
    /* Sequence 2: Version flag then compilation
       This should set print_version, then reset it */
    printf("\n2. Testing version flag reset...\n");
    result = run_command("gcc --version > /dev/null 2>&1");
    checksum = (checksum << 1) | (result ? 1 : 0);
    
    result = run_command("gcc -c temp2.c -o temp2.o");
    checksum = (checksum << 1) | (result ? 1 : 0);
    printf("   Version then compile: %s\n", result ? "PASS" : "FAIL");
    
    /* Sequence 3: Save-temps with dumpdir then plain compilation
       This exercises save_temps_flag, dumpdir, dumpbase reset logic */
    printf("\n3. Testing save-temps and dumpdir reset...\n");
    result = run_command("gcc -save-temps -dumpdir ./dump1 -dumpbase base -c temp1.c 2>/dev/null");
    checksum = (checksum << 1) | (result ? 1 : 0);
    
    /* Check if dump files were created */
    result = access("./dump1/base.i", F_OK) == 0;
    checksum = (checksum << 1) | (result ? 1 : 0);
    printf("   Save-temps with dumpdir: %s\n", result ? "PASS" : "FAIL");
    
    /* Now compile without save-temps - should reset the flags */
    result = run_command("gcc -c temp2.c -o temp2.o");
    checksum = (checksum << 1) | (result ? 1 : 0);
    printf("   Plain compile after save-temps: %s\n", result ? "PASS" : "FAIL");
    
    /* Sequence 4: Verbose flag then normal compilation
       This exercises verbose_only_flag reset */
    printf("\n4. Testing verbose flag reset...\n");
    result = run_command("gcc -v -c temp1.c 2>&1 | grep -q 'COLLECT_GCC_OPTIONS'");
    checksum = (checksum << 1) | (result ? 1 : 0);
    
    result = run_command("gcc -c temp2.c -o temp2.o");
    checksum = (checksum << 1) | (result ? 1 : 0);
    printf("   Verbose then normal: %s\n", result ? "PASS" : "FAIL");
    
    /* Sequence 5: Linker selection then default
       This exercises use_ld reset */
    printf("\n5. Testing linker selection reset...\n");
    result = run_command("gcc -fuse-ld=bfd -c temp1.c -o temp1.o 2>/dev/null");
    /* Some systems might not have bfd linker, so we accept either success or
       a specific error about missing linker */
    if (!result) {
        result = run_command("gcc -fuse-ld=bfd -c temp1.c 2>&1 | grep -q 'cannot find'");
    }
    checksum = (checksum << 1) | (result ? 1 : 0);
    
    result = run_command("gcc -c temp2.c -o temp2.o");
    checksum = (checksum << 1) | (result ? 1 : 0);
    printf("   Specific linker then default: %s\n", result ? "PASS" : "FAIL");
    
    /* Sequence 6: Error then success compilation
       This exercises greatest_status reset */
    printf("\n6. Testing error recovery status reset...\n");
    result = !run_command("gcc -c syntax_error.c 2>/dev/null");
    checksum = (checksum << 1) | (result ? 1 : 0);
    
    result = run_command("gcc -c temp1.c -o temp1.o");
    checksum = (checksum << 1) | (result ? 1 : 0);
    printf("   Error then success: %s\n", result ? "PASS" : "FAIL");
    
    /* Sequence 7: Multiple jobs in single invocation
       This should trigger initialization between jobs */
    printf("\n7. Testing multiple input files (multiple jobs)...\n");
    result = run_command("gcc -c temp1.c temp2.c temp3.c");
    checksum = (checksum << 1) | (result ? 1 : 0);
    printf("   Multiple files compile: %s\n", result ? "PASS" : "FAIL");
    
    /* Sequence 8: Combined flags reset test */
    printf("\n8. Testing combined flags reset...\n");
    result = run_command("gcc -v --help=optimizers -save-temps -dumpdir ./dump2 -c temp1.c 2>&1 | head -20 > /dev/null");
    checksum = (checksum << 1) | (result ? 1 : 0);
    
    result = run_command("gcc -c temp2.c -o temp2.o");
    checksum = (checksum << 1) | (result ? 1 : 0);
    printf("   Combined flags then plain: %s\n", result ? "PASS" : "FAIL");
    
    /* Sequence 9: Using -x to specify language (creates separate jobs) */
    printf("\n9. Testing -x language specification jobs...\n");
    result = run_command("echo 'int x = 42;' | gcc -x c - -c -o temp1.o 2>/dev/null");
    checksum = (checksum << 1) | (result ? 1 : 0);
    
    result = run_command("echo 'int y = 43;' | gcc -x c - -c -o temp2.o 2>/dev/null");
    checksum = (checksum << 1) | (result ? 1 : 0);
    printf("   Multiple -x compile jobs: %s\n", result ? "PASS" : "FAIL");
    
    /* Clean up */
    cleanup_temp_files();
    
    printf("\nAll tests completed. Checksum: 0x%08X\n", checksum);
    
    /* Reference the functions to avoid unused function warnings */
    (void)foo;
    (void)bar;
    
    return 0;
}

/* Dummy implementations to satisfy linker if needed */
int foo(void) { return 0; }
int bar(void) { return 1; }
