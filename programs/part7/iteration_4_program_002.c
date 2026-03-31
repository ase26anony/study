/**
 * test_gcov_dump_switches.c
 * 
 * Test program to exercise the command-line switch cases in gcov-dump.cc
 * Specifically targets lines 111-130 covering flags: h, v, l, p, r, s, and invalid flags
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 1024
#define TEMP_FILENAME "test_gcov_dump_coverage"

/**
 * Creates a minimal C source file with coverage instrumentation
 * Returns 0 on success, -1 on failure
 */
int create_test_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i, sum = 0;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        sum += i;\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    printf(\"Sum: %%d\\n\", sum);\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/**
 * Compiles the test program with coverage instrumentation
 * Returns 0 on success, -1 on failure
 */
int compile_with_coverage(const char *source_file, const char *output_file) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s 2>&1",
             source_file, output_file);
    
    printf("Compiling: %s\n", cmd);
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Compilation failed\n");
        return -1;
    }
    
    return 0;
}

/**
 * Executes the test program to generate .gcda file
 * Returns 0 on success, -1 on failure
 */
int run_test_program(const char *program) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "./%s", program);
    
    printf("Running test program: %s\n", cmd);
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Test program execution failed\n");
        return -1;
    }
    
    // Verify .gcda file was created
    char gcda_file[MAX_CMD_LEN];
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", program);
    
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "Failed to create .gcda file: %s\n", gcda_file);
        return -1;
    }
    
    printf("Generated coverage file: %s (size: %ld bytes)\n", 
           gcda_file, (long)st.st_size);
    return 0;
}

/**
 * Executes gcov-dump with given arguments and captures output
 * Returns 0 on success, -1 on failure
 */
int run_gcov_dump(const char *args, int capture_stderr) {
    char cmd[MAX_CMD_LEN];
    char buffer[1024];
    int found_error = 0;
    
    if (capture_stderr) {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s 2>&1", args);
    } else {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s", args);
    }
    
    printf("\nExecuting: %s\n", cmd);
    
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return -1;
    }
    
    // Read and optionally analyze output
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (capture_stderr && strstr(buffer, "unknown flag")) {
            printf("SUCCESS: Triggered default case with message: %s", buffer);
            found_error = 1;
        }
    }
    
    int status = pclose(fp);
    
    if (capture_stderr && !found_error) {
        printf("WARNING: Expected 'unknown flag' message not found\n");
    }
    
    return WEXITSTATUS(status);
}

/**
 * Main test driver
 */
int main(int argc, char *argv[]) {
    int ret = 0;
    char source_file[MAX_CMD_LEN];
    char binary_file[MAX_CMD_LEN];
    char gcda_file[MAX_CMD_LEN];
    
    // Setup filenames
    snprintf(source_file, sizeof(source_file), "%s.c", TEMP_FILENAME);
    snprintf(binary_file, sizeof(binary_file), "%s", TEMP_FILENAME);
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", TEMP_FILENAME);
    
    printf("=== Testing gcov-dump command-line switches ===\n");
    
    // Step 1: Create test source file
    printf("\n1. Creating test source file: %s\n", source_file);
    if (create_test_source(source_file) != 0) {
        fprintf(stderr, "Failed to create test source\n");
        return EXIT_FAILURE;
    }
    
    // Step 2: Compile with coverage
    printf("\n2. Compiling with coverage instrumentation\n");
    if (compile_with_coverage(source_file, binary_file) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        unlink(source_file);
        return EXIT_FAILURE;
    }
    
    // Step 3: Run test program to generate .gcda
    printf("\n3. Running test program to generate coverage data\n");
    if (run_test_program(binary_file) != 0) {
        fprintf(stderr, "Failed to generate coverage data\n");
        // Clean up and exit
        unlink(source_file);
        unlink(binary_file);
        char gcno_file[MAX_CMD_LEN];
        snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", TEMP_FILENAME);
        unlink(gcno_file);
        return EXIT_FAILURE;
    }
    
    // Step 4: Test gcov-dump with various flags
    printf("\n4. Testing gcov-dump command-line switches\n");
    
    // Test -h flag (help) - triggers print_usage()
    printf("\n--- Testing -h flag (help) ---\n");
    if (run_gcov_dump("-h", 0) != 0) {
        printf("WARNING: gcov-dump -h returned non-zero\n");
    }
    
    // Test -v flag (version) - triggers print_version()
    printf("\n--- Testing -v flag (version) ---\n");
    if (run_gcov_dump("-v", 0) != 0) {
        printf("WARNING: gcov-dump -v returned non-zero\n");
    }
    
    // Test -l flag (dump contents) with .gcda file
    printf("\n--- Testing -l flag (dump contents) ---\n");
    char cmd_with_gcda[MAX_CMD_LEN];
    snprintf(cmd_with_gcda, sizeof(cmd_with_gcda), "-l %s", gcda_file);
    if (run_gcov_dump(cmd_with_gcda, 0) != 0) {
        printf("WARNING: gcov-dump -l returned non-zero\n");
    }
    
    // Test -p flag (dump positions) with .gcda file
    printf("\n--- Testing -p flag (dump positions) ---\n");
    snprintf(cmd_with_gcda, sizeof(cmd_with_gcda), "-p %s", gcda_file);
    if (run_gcov_dump(cmd_with_gcda, 0) != 0) {
        printf("WARNING: gcov-dump -p returned non-zero\n");
    }
    
    // Test -r flag (dump raw) with .gcda file
    printf("\n--- Testing -r flag (dump raw) ---\n");
    snprintf(cmd_with_gcda, sizeof(cmd_with_gcda), "-r %s", gcda_file);
    if (run_gcov_dump(cmd_with_gcda, 0) != 0) {
        printf("WARNING: gcov-dump -r returned non-zero\n");
    }
    
    // Test -s flag (dump stable) with .gcda file
    printf("\n--- Testing -s flag (dump stable) ---\n");
    snprintf(cmd_with_gcda, sizeof(cmd_with_gcda), "-s %s", gcda_file);
    if (run_gcov_dump(cmd_with_gcda, 0) != 0) {
        printf("WARNING: gcov-dump -s returned non-zero\n");
    }
    
    // Test combined flags
    printf("\n--- Testing combined flags (-l -p) ---\n");
    snprintf(cmd_with_gcda, sizeof(cmd_with_gcda), "-l -p %s", gcda_file);
    if (run_gcov_dump(cmd_with_gcda, 0) != 0) {
        printf("WARNING: gcov-dump -l -p returned non-zero\n");
    }
    
    // Test invalid flag - triggers default case and fprintf
    printf("\n--- Testing invalid flag (-X) ---\n");
    snprintf(cmd_with_gcda, sizeof(cmd_with_gcda), "-X %s", gcda_file);
    if (run_gcov_dump(cmd_with_gcda, 1) != 0) {
        printf("Note: Invalid flag correctly caused error exit\n");
    }
    
    // Test another invalid flag
    printf("\n--- Testing another invalid flag (-z) ---\n");
    snprintf(cmd_with_gcda, sizeof(cmd_with_gcda), "-z %s", gcda_file);
    if (run_gcov_dump(cmd_with_gcda, 1) != 0) {
        printf("Note: Invalid flag correctly caused error exit\n");
    }
    
    // Step 5: Cleanup
    printf("\n5. Cleaning up temporary files\n");
    unlink(source_file);
    unlink(binary_file);
    
    char gcno_file[MAX_CMD_LEN];
    snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", TEMP_FILENAME);
    unlink(gcno_file);
    unlink(gcda_file);
    
    printf("\n=== Test completed ===\n");
    printf("All gcov-dump switch cases should have been exercised:\n");
    printf("  -h (help/usage)\n");
    printf("  -v (version)\n");
    printf("  -l (dump contents)\n");
    printf("  -p (dump positions)\n");
    printf("  -r (dump raw)\n");
    printf("  -s (dump stable)\n");
    printf("  default case (invalid flags)\n");
    
    return EXIT_SUCCESS;
}
