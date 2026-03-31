/**
 * Test driver for gcov-dump to trigger uncovered command-line parsing code.
 * Compile with: gcc -O0 -fprofile-arcs -ftest-coverage gcov_dump_test.c -o gcov_dump_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define TEMP_SOURCE_TEMPLATE "/tmp/gcov_test_XXXXXX.c"
#define TEMP_BINARY_TEMPLATE "/tmp/gcov_test_XXXXXX"
#define BUFFER_SIZE 4096

/**
 * Creates a temporary C source file with coverage instrumentation.
 * Returns dynamically allocated filename or NULL on failure.
 */
char* create_temp_source(void) {
    char* template = strdup(TEMP_SOURCE_TEMPLATE);
    if (!template) return NULL;
    
    // Create the temporary file
    int fd = mkstemps(template, 2);  // 2 for ".c" extension
    if (fd < 0) {
        free(template);
        return NULL;
    }
    
    // Write a simple C program that will generate coverage data
    const char* source_code = 
        "#include <stdio.h>\n"
        "int main() {\n"
        "    int i, sum = 0;\n"
        "    for (i = 0; i < 10; i++) {\n"
        "        sum += i;\n"
        "    }\n"
        "    printf(\"Sum: %d\\n\", sum);\n"
        "    return 0;\n"
        "}\n";
    
    write(fd, source_code, strlen(source_code));
    close(fd);
    
    return template;
}

/**
 * Compiles the source file with coverage instrumentation.
 * Returns 0 on success, -1 on failure.
 */
int compile_with_coverage(const char* source_file, const char* binary_file) {
    char command[BUFFER_SIZE];
    snprintf(command, sizeof(command),
             "gcc -fprofile-arcs -ftest-coverage %s -o %s 2>&1",
             source_file, binary_file);
    
    printf("Compiling: %s\n", command);
    int result = system(command);
    
    if (result != 0) {
        fprintf(stderr, "Compilation failed with code %d\n", result);
        return -1;
    }
    
    return 0;
}

/**
 * Executes the compiled program to generate .gcda file.
 * Returns 0 on success, -1 on failure.
 */
int execute_for_coverage(const char* binary_file) {
    char command[BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s", binary_file);
    
    printf("Executing: %s\n", command);
    int result = system(command);
    
    if (result != 0) {
        fprintf(stderr, "Execution failed with code %d\n", result);
        return -1;
    }
    
    return 0;
}

/**
 * Runs gcov-dump with specified flags and checks for expected output.
 */
void run_gcov_dump(const char* flags, const char* gcda_file, int check_error) {
    char command[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    FILE* fp;
    int found_error = 0;
    
    if (gcda_file) {
        snprintf(command, sizeof(command), "gcov-dump %s %s 2>&1", flags, gcda_file);
    } else {
        snprintf(command, sizeof(command), "gcov-dump %s 2>&1", flags);
    }
    
    printf("\nRunning: %s\n", command);
    
    // Use popen to capture both stdout and stderr
    fp = popen(command, "r");
    if (!fp) {
        perror("popen failed");
        return;
    }
    
    // Read and optionally check output
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (check_error && strstr(buffer, "unknown flag")) {
            found_error = 1;
            printf("SUCCESS: Triggered default case with message: %s", buffer);
        }
    }
    
    pclose(fp);
    
    if (check_error && !found_error) {
        printf("WARNING: Expected 'unknown flag' message not found\n");
    }
}

/**
 * Checks if a file exists.
 */
int file_exists(const char* filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

/**
 * Creates the .gcda filename from binary name.
 */
char* create_gcda_filename(const char* binary_file) {
    char* gcda_file = malloc(strlen(binary_file) + 10);
    if (!gcda_file) return NULL;
    
    sprintf(gcda_file, "%s.gcda", binary_file);
    return gcda_file;
}

/**
 * Creates the .gcno filename from binary name.
 */
char* create_gcno_filename(const char* binary_file) {
    char* gcno_file = malloc(strlen(binary_file) + 10);
    if (!gcno_file) return NULL;
    
    sprintf(gcno_file, "%s.gcno", binary_file);
    return gcno_file;
}

/**
 * Cleans up temporary files.
 */
void cleanup_files(const char* source_file, const char* binary_file) {
    if (source_file && file_exists(source_file)) {
        unlink(source_file);
        printf("Cleaned up: %s\n", source_file);
    }
    
    if (binary_file && file_exists(binary_file)) {
        unlink(binary_file);
        printf("Cleaned up: %s\n", binary_file);
    }
    
    char* gcda_file = create_gcda_filename(binary_file);
    if (gcda_file && file_exists(gcda_file)) {
        unlink(gcda_file);
        printf("Cleaned up: %s\n", gcda_file);
    }
    free(gcda_file);
    
    char* gcno_file = create_gcno_filename(binary_file);
    if (gcno_file && file_exists(gcno_file)) {
        unlink(gcno_file);
        printf("Cleaned up: %s\n", gcno_file);
    }
    free(gcno_file);
}

int main(void) {
    char* source_file = NULL;
    char* binary_file = NULL;
    char* gcda_file = NULL;
    int ret = 0;
    
    printf("=== Starting gcov-dump test driver ===\n");
    
    // Step 1: Create temporary source file
    source_file = create_temp_source();
    if (!source_file) {
        fprintf(stderr, "Failed to create temporary source file\n");
        return 1;
    }
    printf("Created source file: %s\n", source_file);
    
    // Step 2: Create temporary binary filename
    binary_file = strdup(TEMP_BINARY_TEMPLATE);
    if (!binary_file) {
        fprintf(stderr, "Failed to allocate binary filename\n");
        ret = 1;
        goto cleanup;
    }
    
    // Create unique binary filename
    int fd = mkstemp(binary_file);
    if (fd < 0) {
        fprintf(stderr, "Failed to create unique binary filename\n");
        ret = 1;
        goto cleanup;
    }
    close(fd);
    unlink(binary_file);  // Remove the empty file, we'll create the binary properly
    
    printf("Binary will be: %s\n", binary_file);
    
    // Step 3: Compile with coverage
    if (compile_with_coverage(source_file, binary_file) != 0) {
        fprintf(stderr, "Compilation failed\n");
        ret = 1;
        goto cleanup;
    }
    
    // Step 4: Execute to generate coverage data
    if (execute_for_coverage(binary_file) != 0) {
        fprintf(stderr, "Execution failed\n");
        ret = 1;
        goto cleanup;
    }
    
    // Step 5: Verify .gcda file was created
    gcda_file = create_gcda_filename(binary_file);
    if (!gcda_file || !file_exists(gcda_file)) {
        fprintf(stderr, "Coverage data file not found: %s\n", 
                gcda_file ? gcda_file : "NULL");
        ret = 1;
        goto cleanup;
    }
    printf("Generated coverage file: %s\n", gcda_file);
    
    // Step 6: Run gcov-dump with various flags to trigger uncovered code
    
    // 6a: Trigger help flag (-h) -> case 'h': print_usage()
    printf("\n--- Testing -h flag (help) ---\n");
    run_gcov_dump("-h", NULL, 0);
    
    // 6b: Trigger version flag (-v) -> case 'v': print_version()
    printf("\n--- Testing -v flag (version) ---\n");
    run_gcov_dump("-v", NULL, 0);
    
    // 6c: Trigger dump contents flag (-l) -> case 'l': flag_dump_contents = 1
    printf("\n--- Testing -l flag (dump contents) ---\n");
    run_gcov_dump("-l", gcda_file, 0);
    
    // 6d: Trigger dump positions flag (-p) -> case 'p': flag_dump_positions = 1
    printf("\n--- Testing -p flag (dump positions) ---\n");
    run_gcov_dump("-p", gcda_file, 0);
    
    // 6e: Trigger dump raw flag (-r) -> case 'r': flag_dump_raw = 1
    printf("\n--- Testing -r flag (dump raw) ---\n");
    run_gcov_dump("-r", gcda_file, 0);
    
    // 6f: Trigger dump stable flag (-s) -> case 's': flag_dump_stable = 1
    printf("\n--- Testing -s flag (dump stable) ---\n");
    run_gcov_dump("-s", gcda_file, 0);
    
    // 6g: Test combined flags (-l -p)
    printf("\n--- Testing combined flags (-l -p) ---\n");
    run_gcov_dump("-l -p", gcda_file, 0);
    
    // 6h: Trigger default case with invalid flag (-X) -> default: fprintf(stderr, "unknown flag")
    printf("\n--- Testing invalid flag (-X) to trigger default case ---\n");
    run_gcov_dump("-X", gcda_file, 1);
    
    // 6i: Test another invalid flag (-z)
    printf("\n--- Testing another invalid flag (-z) ---\n");
    run_gcov_dump("-z", gcda_file, 1);
    
    printf("\n=== All tests completed ===\n");

cleanup:
    // Clean up temporary files
    if (source_file || binary_file) {
        printf("\nCleaning up temporary files...\n");
        cleanup_files(source_file, binary_file);
    }
    
    free(source_file);
    free(binary_file);
    free(gcda_file);
    
    return ret;
}
