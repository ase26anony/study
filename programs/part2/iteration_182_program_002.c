/**
 * Test program to cover GCC driver cleanup block (lines 11228-11250 in gcc.cc)
 * This program invokes the GCC driver with various flags to ensure the
 * cleanup routine executes and resets the target variables.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define SIMPLE_C_FILE "simple_test.c"
#define SIMPLE_C_CONTENT "int main(void) { return 0; }"

/**
 * Create a simple C source file for compilation tests
 */
static int create_simple_c_file(void) {
    FILE *fp = fopen(SIMPLE_C_FILE, "w");
    if (!fp) {
        perror("Failed to create simple.c");
        return 0;
    }
    fprintf(fp, "%s\n", SIMPLE_C_CONTENT);
    fclose(fp);
    return 1;
}

/**
 * Remove a file if it exists
 */
static void remove_if_exists(const char *filename) {
    if (access(filename, F_OK) == 0) {
        unlink(filename);
    }
}

/**
 * Remove a directory and its contents recursively
 */
static void remove_directory_recursive(const char *path) {
    char command[256];
    snprintf(command, sizeof(command), "rm -rf %s", path);
    system(command);
}

/**
 * Execute a GCC command and capture its return status
 */
static int execute_gcc_command(const char *command) {
    printf("Executing: %s\n", command);
    int status = system(command);
    printf("Return status: %d\n\n", status);
    return status;
}

int main(void) {
    int overall_success = 1;
    
    // Create test directories
    mkdir("test_artifacts", 0755);
    mkdir("fail_artifacts", 0755);
    
    // Create the simple C source file
    if (!create_simple_c_file()) {
        fprintf(stderr, "Failed to create test source file\n");
        return 1;
    }
    
    printf("=== GCC Driver Cleanup Coverage Test ===\n\n");
    
    // Invocation A: Sets state with -save-temps, -dumpdir, -dumpbase, -o
    // This should succeed (compile only, no link)
    printf("Invocation A: Setting state with successful compilation\n");
    execute_gcc_command("gcc -save-temps -dumpdir ./test_artifacts "
                       "-dumpbase coverage_test -o test_output.o "
                       "-c " SIMPLE_C_FILE);
    
    // Invocation B: Sets different state but fails late with invalid architecture
    // This triggers cleanup after backend failure
    printf("Invocation B: Setting state with late failure (invalid arch)\n");
    execute_gcc_command("gcc -save-temps=obj -dumpdir ./fail_artifacts "
                       "-dumpbase fail_test -o fail_output.o "
                       "-march=invalid-architecture " SIMPLE_C_FILE);
    
    // Invocation C: Uses -specs and -V to influence spec_machine and print_version
    printf("Invocation C: Using -specs and -V flags\n");
    execute_gcc_command("gcc -specs=nosuch.spec -V " SIMPLE_C_FILE " 2>&1 | head -5");
    
    // Invocation D: Test with -Werror turning warnings into errors
    // Create a file with a warning
    FILE *fp = fopen("warning.c", "w");
    if (fp) {
        fprintf(fp, "int main(void) { int x; return 0; } /* unused variable warning */\n");
        fclose(fp);
        
        printf("Invocation D: Testing -Werror cleanup path\n");
        execute_gcc_command("gcc -Werror -save-temps -dumpdir ./werror_artifacts "
                           "-dumpbase werror_test -o werror_output.o warning.c");
        remove_if_exists("warning.c");
    }
    
    // Invocation E: Test linker error path (undefined library)
    printf("Invocation E: Testing linker error cleanup path\n");
    execute_gcc_command("gcc -save-temps -dumpdir ./linker_artifacts "
                       "-dumpbase linker_test -o linker_output "
                       SIMPLE_C_FILE " -lnosuchlibrary999");
    
    // Invocation F: Test multiple dumpbase extensions
    printf("Invocation F: Testing with dumpbase extensions\n");
    execute_gcc_command("gcc -save-temps -dumpdir ./ext_artifacts "
                       "-dumpbase ext_test -dumpbase-ext .extra "
                       "-o ext_output.o -c " SIMPLE_C_FILE);
    
    // Invocation G: Test verbose flag combinations
    printf("Invocation G: Testing verbose flags\n");
    execute_gcc_command("gcc -v -save-temps=obj -dumpdir ./verbose_artifacts "
                       "-dumpbase verbose_test -o verbose_output.o "
                       "-c " SIMPLE_C_FILE);
    
    // Invocation H: Test with different optimization levels and machine-specific flags
    printf("Invocation H: Testing with optimization flags\n");
    execute_gcc_command("gcc -O2 -mtune=generic -save-temps "
                       "-dumpdir ./opt_artifacts -dumpbase opt_test "
                       "-o opt_output.o -c " SIMPLE_C_FILE);
    
    // Invocation I: Test with --help and --version (should set print_help_list/print_version)
    printf("Invocation I: Testing help and version flags\n");
    execute_gcc_command("gcc --help 2>&1 | head -3");
    execute_gcc_command("gcc --version 2>&1 | head -3");
    
    // Invocation J: Test with time reporting
    printf("Invocation J: Testing time reporting\n");
    execute_gcc_command("gcc -ftime-report -save-temps -dumpdir ./time_artifacts "
                       "-dumpbase time_test -o time_output.o -c " SIMPLE_C_FILE);
    
    // Cleanup generated files
    printf("\n=== Cleaning up test artifacts ===\n");
    
    // Remove source files
    remove_if_exists(SIMPLE_C_FILE);
    
    // Remove object and output files
    remove_if_exists("test_output.o");
    remove_if_exists("fail_output.o");
    remove_if_exists("werror_output.o");
    remove_if_exists("linker_output");
    remove_if_exists("ext_output.o");
    remove_if_exists("verbose_output.o");
    remove_if_exists("opt_output.o");
    remove_if_exists("time_output.o");
    
    // Remove temporary files created by -save-temps
    remove_if_exists("simple_test.i");
    remove_if_exists("simple_test.s");
    remove_if_exists("warning.i");
    remove_if_exists("warning.s");
    
    // Remove artifact directories
    remove_directory_recursive("test_artifacts");
    remove_directory_recursive("fail_artifacts");
    remove_directory_recursive("werror_artifacts");
    remove_directory_recursive("linker_artifacts");
    remove_directory_recursive("ext_artifacts");
    remove_directory_recursive("verbose_artifacts");
    remove_directory_recursive("opt_artifacts");
    remove_directory_recursive("time_artifacts");
    
    printf("\n=== Test completed ===\n");
    printf("All GCC invocations attempted. Check coverage data for gcc.cc lines 11228-11250.\n");
    
    return overall_success ? 0 : 1;
}
