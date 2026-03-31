/**
 * Test program to exercise GCC driver cleanup functionality.
 * This program invokes the GCC driver with various flags that set
 * internal state variables, ensuring the cleanup block in gcc.cc
 * (lines 11228-11250) is executed and covered.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define SIMPLE_C_FILE "simple_test.c"
#define TEMP_DIR "test_coverage_temp"

/**
 * Creates a minimal valid C source file for compilation tests.
 */
static void create_simple_c_file(void) {
    FILE *f = fopen(SIMPLE_C_FILE, "w");
    if (!f) {
        perror("Failed to create test C file");
        exit(1);
    }
    
    fprintf(f, "/* Simple test file for GCC driver coverage */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main(void) {\n");
    fprintf(f, "    printf(\"Hello from coverage test\\n\");\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
    printf("Created test file: %s\n", SIMPLE_C_FILE);
}

/**
 * Creates a C source file with a deliberate syntax error
 * that will be caught by the compiler proper (not the driver).
 */
static void create_error_c_file(void) {
    const char *filename = "error_test.c";
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create error test C file");
        exit(1);
    }
    
    fprintf(f, "/* Test file with syntax error for late failure */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main(void) {\n");
    fprintf(f, "    printf(\"This will fail\\n\")\n");  // Missing semicolon
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
    printf("Created error test file: %s\n", filename);
}

/**
 * Executes a GCC command and returns the exit status.
 */
static int run_gcc_command(const char *description, const char *command) {
    printf("\n=== %s ===\n", description);
    printf("Command: %s\n", command);
    
    int status = system(command);
    
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    } else {
        printf("Command terminated abnormally\n");
    }
    
    return status;
}

/**
 * Creates a temporary directory for test artifacts.
 */
static void create_temp_dir(void) {
    if (mkdir(TEMP_DIR, 0755) != 0) {
        // Directory might already exist
        if (access(TEMP_DIR, F_OK) != 0) {
            perror("Failed to create temp directory");
            exit(1);
        }
    }
    printf("Created temp directory: %s\n", TEMP_DIR);
}

/**
 * Cleans up generated files and directories.
 */
static void cleanup_test_files(void) {
    // Remove generated source files
    remove(SIMPLE_C_FILE);
    remove("error_test.c");
    
    // Remove compilation artifacts
    remove("test_output.o");
    remove("fail_output.o");
    remove("test_output");
    remove("fail_output");
    
    // Remove save-temps files
    remove("simple_test.i");
    remove("simple_test.s");
    remove("error_test.i");
    remove("error_test.s");
    
    // Remove dumpdir contents
    system("rm -rf ./test_artifacts");
    system("rm -rf ./fail_artifacts");
    system("rm -rf " TEMP_DIR);
    
    // Remove any other potential artifacts
    remove("coverage_test.*");
    remove("fail_test.*");
    remove("mydump.*");
    
    printf("\nCleaned up test files\n");
}

int main(void) {
    printf("=== GCC Driver Cleanup Coverage Test ===\n");
    
    // Create test directory
    create_temp_dir();
    
    // Create test source files
    create_simple_c_file();
    create_error_c_file();
    
    // Track overall test success
    int test_success = 1;
    
    /**
     * INVOCATION A: Successful compilation with state-setting flags
     * This sets: save_temps_flag, dumpdir, dumpbase, outbase
     */
    const char *invocation_a = 
        "gcc -save-temps "
        "-dumpdir ./test_artifacts "
        "-dumpbase coverage_test "
        "-o test_output.o "
        "-c " SIMPLE_C_FILE;
    
    run_gcc_command("Invocation A: Successful compilation with state flags", 
                   invocation_a);
    
    /**
     * INVOCATION B: Compilation that fails after driver initialization
     * Uses invalid architecture flag to trigger backend failure
     */
    const char *invocation_b = 
        "gcc -save-temps=obj "
        "-dumpdir ./fail_artifacts "
        "-dumpbase fail_test "
        "-o fail_output.o "
        "-march=invalid-architecture "
        SIMPLE_C_FILE;
    
    run_gcc_command("Invocation B: Failure with invalid architecture", 
                   invocation_b);
    
    /**
     * INVOCATION C: Use -specs flag to influence spec_machine
     * This may set spec_machine and print_version
     */
    const char *invocation_c = 
        "gcc -specs=nosuch.spec "
        "-V " SIMPLE_C_FILE " 2>&1 | head -5";
    
    run_gcc_command("Invocation C: Testing specs and version flags", 
                   invocation_c);
    
    /**
     * INVOCATION D: Test with -Werror to generate error exit status
     * Uses a file with actual warning to test error status propagation
     */
    const char *invocation_d = 
        "gcc -Werror -Wall "
        "-dumpdir " TEMP_DIR " "
        "-dumpbase mydump "
        "-o " TEMP_DIR "/werror_output.o "
        "-c " SIMPLE_C_FILE;
    
    run_gcc_command("Invocation D: Testing -Werror flag", 
                   invocation_d);
    
    /**
     * INVOCATION E: Test with explicit output base flags
     * Exercises outbase and dumpbase_ext handling
     */
    const char *invocation_e = 
        "gcc -save-temps=cwd "
        "-fdump-rtl-all "
        "-fdump-tree-all "
        "-o output_prog "
        SIMPLE_C_FILE " 2>&1 | tail -3";
    
    run_gcc_command("Invocation E: Testing dump and output base flags", 
                   invocation_e);
    
    /**
     * INVOCATION F: Test compilation that will fail at linker stage
     * Requests non-existent library to trigger linker error
     */
    const char *invocation_f = 
        "gcc -save-temps "
        "-dumpdir " TEMP_DIR "/linker_fail "
        "-o linker_fail_prog "
        SIMPLE_C_FILE " -lnonexistentlibraryxyz 2>&1";
    
    run_gcc_command("Invocation F: Linker failure test", 
                   invocation_f);
    
    /**
     * INVOCATION G: Test with syntax error (caught by compiler proper)
     * This ensures driver cleanup runs even with compilation errors
     */
    const char *invocation_g = 
        "gcc -save-temps "
        "-dumpdir ./syntax_err "
        "-dumpbase syntax_dump "
        "-o syntax_output "
        "error_test.c";
    
    run_gcc_command("Invocation G: Syntax error test", 
                   invocation_g);
    
    /**
     * INVOCATION H: Multiple flag combinations in one invocation
     * Tests interaction of multiple state-setting flags
     */
    const char *invocation_h = 
        "gcc -v -save-temps=obj "
        "-dumpdir ./combined "
        "-dumpbase combined_test "
        "-fdump-ipa-all "
        "-fdump-noaddr "
        "-o combined_output "
        SIMPLE_C_FILE " 2>&1 | grep -E '(cc1|as|ld)'";
    
    run_gcc_command("Invocation H: Combined flags test", 
                   invocation_h);
    
    // Clean up generated files
    cleanup_test_files();
    
    printf("\n=== Test Complete ===\n");
    printf("All GCC invocations attempted. Check coverage data for gcc.cc cleanup block.\n");
    
    return 0;
}
