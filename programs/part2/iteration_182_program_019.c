/**
 * Test program to cover GCC driver cleanup lines in gcc.cc
 * This program invokes the GCC driver multiple times with different
 * flags to ensure the cleanup block (lines 11228-11250) is executed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define SIMPLE_C_CONTENT "int main(void) { return 0; }\n"
#define ERROR_C_CONTENT "int main(void) { return undefined_var; }\n"

/**
 * Create a temporary directory for test artifacts
 */
static char* create_temp_dir(void) {
    char template[] = "/tmp/gcc_test_XXXXXX";
    char *dir = mkdtemp(template);
    if (!dir) {
        perror("Failed to create temp directory");
        exit(1);
    }
    return strdup(dir);
}

/**
 * Write a C source file to disk
 */
static void write_c_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to write C file");
        exit(1);
    }
    fputs(content, f);
    fclose(f);
}

/**
 * Execute a GCC command and return its exit status
 */
static int run_gcc(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status == -1) {
        perror("system() failed");
        return -1;
    }
    return WEXITSTATUS(status);
}

/**
 * Clean up generated files
 */
static void cleanup_files(const char *temp_dir) {
    char cmd[1024];
    
    // Remove common GCC output files
    const char *files[] = {
        "simple.c", "error.c",
        "test_output.o", "test_output.i", "test_output.s",
        "fail_output.o", "fail_output.i", "fail_output.s",
        "coverage_test.*", "fail_test.*",
        "output", "a.out",
        NULL
    };
    
    for (int i = 0; files[i]; i++) {
        snprintf(cmd, sizeof(cmd), "rm -f %s", files[i]);
        system(cmd);
    }
    
    // Remove temp directory if it exists
    if (temp_dir && access(temp_dir, F_OK) == 0) {
        snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
        system(cmd);
    }
}

int main(void) {
    char *temp_dir = NULL;
    char cmd[2048];
    int overall_status = 0;
    
    printf("=== GCC Driver Cleanup Coverage Test ===\n");
    
    // Create temp directory for test artifacts
    temp_dir = create_temp_dir();
    printf("Created temp directory: %s\n", temp_dir);
    
    // Create test source files
    write_c_file("simple.c", SIMPLE_C_CONTENT);
    write_c_file("error.c", ERROR_C_CONTENT);
    
    printf("\n--- Test 1: Successful compilation with state variables set ---\n");
    // This sets: save_temps_flag, dumpdir, dumpbase, outbase
    snprintf(cmd, sizeof(cmd),
             "gcc -save-temps -dumpdir %s -dumpbase coverage_test "
             "-o test_output.o -c simple.c",
             temp_dir);
    int status1 = run_gcc(cmd);
    printf("Exit status: %d\n", status1);
    
    printf("\n--- Test 2: Compilation with syntax error (late failure) ---\n");
    // This should still trigger cleanup after parser/backend failure
    snprintf(cmd, sizeof(cmd),
             "gcc -save-temps -dumpdir %s/fail -dumpbase fail_test "
             "-o fail_output.o -c error.c",
             temp_dir);
    int status2 = run_gcc(cmd);
    printf("Exit status: %d\n", status2);
    
    printf("\n--- Test 3: Invalid architecture flag (backend rejection) ---\n");
    // Driver accepts -march but backend rejects invalid value
    snprintf(cmd, sizeof(cmd),
             "gcc -save-temps -dumpdir %s -dumpbase arch_test "
             "-o arch_output.o -march=invalid-arch -c simple.c",
             temp_dir);
    int status3 = run_gcc(cmd);
    printf("Exit status: %d\n", status3);
    
    printf("\n--- Test 4: Linker error (undefined library) ---\n");
    // Driver processes flags, linker fails
    snprintf(cmd, sizeof(cmd),
             "gcc -save-temps -dumpdir %s -dumpbase link_test "
             "-o output -lnonexistentlibrary simple.c",
             temp_dir);
    int status4 = run_gcc(cmd);
    printf("Exit status: %d\n", status4);
    
    printf("\n--- Test 5: Version printing and specs ---\n");
    // This influences print_version and spec_machine
    snprintf(cmd, sizeof(cmd),
             "gcc -specs=nosuch.spec -V simple.c 2>&1 | head -5");
    int status5 = run_gcc(cmd);
    printf("Exit status: %d\n", status5);
    
    printf("\n--- Test 6: Warning as error ---\n");
    // Use -Werror to create a different failure mode
    snprintf(cmd, sizeof(cmd),
             "gcc -save-temps -dumpdir %s -dumpbase werror_test "
             "-o werror_output.o -Werror -Wunused-variable -c simple.c",
             temp_dir);
    int status6 = run_gcc(cmd);
    printf("Exit status: %d\n", status6);
    
    printf("\n--- Test 7: Multiple output base variations ---\n");
    // Test different -o and dumpbase combinations
    snprintf(cmd, sizeof(cmd),
             "gcc -save-temps -dumpbase myprog -dumpbase-ext .ext "
             "-o myoutput.o -c simple.c");
    int status7 = run_gcc(cmd);
    printf("Exit status: %d\n", status7);
    
    printf("\n--- Test 8: Verbose mode ---\n");
    // Sets verbose_only_flag
    snprintf(cmd, sizeof(cmd),
             "gcc -v -c simple.c 2>&1 | tail -3");
    int status8 = run_gcc(cmd);
    printf("Exit status: %d\n", status8);
    
    printf("\n--- Test 9: Help and version flags ---\n");
    // These set print_help_list, print_version
    snprintf(cmd, sizeof(cmd), "gcc --help | head -5");
    int status9 = run_gcc(cmd);
    printf("Exit status: %d\n", status9);
    
    snprintf(cmd, sizeof(cmd), "gcc --version");
    int status10 = run_gcc(cmd);
    printf("Exit status: %d\n", status10);
    
    printf("\n=== Summary ===\n");
    printf("All GCC invocations attempted. Driver cleanup should have been triggered for each.\n");
    printf("Temporary directory: %s\n", temp_dir);
    
    // Clean up
    printf("\nCleaning up test files...\n");
    cleanup_files(temp_dir);
    
    free(temp_dir);
    
    printf("\nTest completed successfully.\n");
    return 0;
}
