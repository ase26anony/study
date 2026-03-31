/**
 * test_gcov_dump_switches.c
 * 
 * Test driver to exercise specific command-line switch cases in gcov-dump
 * Targets lines 111-130 in gcov-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define TEMP_SOURCE "test_coverage_XXXXXX.c"
#define TEMP_BINARY "test_coverage_XXXXXX"

/**
 * Creates a minimal C source file with coverage instrumentation
 * Returns dynamically allocated filename or NULL on failure
 */
char* create_coverage_source(void) {
    char template[] = "test_coverage_XXXXXX.c";
    int fd = mkstemps(template, 2);  // .c suffix is 2 chars
    if (fd < 0) {
        perror("mkstemps failed");
        return NULL;
    }
    
    char* filename = strdup(template);
    if (!filename) {
        close(fd);
        return NULL;
    }
    
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
    
    if (write(fd, source_code, strlen(source_code)) < 0) {
        perror("write failed");
        free(filename);
        close(fd);
        return NULL;
    }
    
    close(fd);
    return filename;
}

/**
 * Compiles the source file with coverage instrumentation
 * Returns dynamically allocated binary name or NULL on failure
 */
char* compile_with_coverage(const char* source_file) {
    char* binary_name = strdup("test_coverage_XXXXXX");
    if (!binary_name) return NULL;
    
    int fd = mkstemp(binary_name);
    if (fd < 0) {
        perror("mkstemp failed");
        free(binary_name);
        return NULL;
    }
    close(fd);
    
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s 2>/dev/null",
             source_file, binary_name);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Compilation failed for %s\n", source_file);
        free(binary_name);
        return NULL;
    }
    
    return binary_name;
}

/**
 * Executes the binary to generate .gcda file
 */
int generate_gcda(const char* binary_name) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "./%s >/dev/null 2>&1", binary_name);
    return system(cmd);
}

/**
 * Runs gcov-dump with specified flags and checks for expected output
 */
void test_gcov_dump(const char* gcda_file, const char* flags, 
                    const char* expected_stderr_substring) {
    char cmd[MAX_CMD_LEN];
    
    if (gcda_file) {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", flags, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s 2>&1", flags);
    }
    
    printf("Testing: %s\n", cmd);
    
    FILE* fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return;
    }
    
    char buffer[1024];
    int found_expected = 0;
    
    while (fgets(buffer, sizeof(buffer), fp)) {
        // Check for expected error message in stderr
        if (expected_stderr_substring && 
            strstr(buffer, expected_stderr_substring)) {
            found_expected = 1;
        }
    }
    
    int status = pclose(fp);
    
    if (expected_stderr_substring) {
        if (found_expected) {
            printf("  ✓ Successfully triggered expected error: '%s'\n", 
                   expected_stderr_substring);
        } else {
            printf("  ✗ Did not find expected error: '%s'\n", 
                   expected_stderr_substring);
        }
    } else {
        printf("  ✓ Command executed (status: %d)\n", WEXITSTATUS(status));
    }
    printf("\n");
}

/**
 * Cleans up temporary files
 */
void cleanup_files(const char* source, const char* binary, 
                   const char* gcda_file) {
    if (source) {
        unlink(source);
        char* gcno_file = malloc(strlen(source) + 6);
        if (gcno_file) {
            snprintf(gcno_file, strlen(source) + 6, "%s.gcno", source);
            unlink(gcno_file);
            free(gcno_file);
        }
        free((void*)source);
    }
    
    if (binary) {
        unlink(binary);
        free((void*)binary);
    }
    
    if (gcda_file) {
        unlink(gcda_file);
    }
}

int main(void) {
    printf("=== Testing gcov-dump command-line switches ===\n\n");
    
    // Step 1: Create coverage data file
    printf("1. Creating coverage data file...\n");
    char* source_file = create_coverage_source();
    if (!source_file) {
        fprintf(stderr, "Failed to create source file\n");
        return 1;
    }
    
    char* binary_name = compile_with_coverage(source_file);
    if (!binary_name) {
        fprintf(stderr, "Failed to compile with coverage\n");
        free(source_file);
        return 1;
    }
    
    if (generate_gcda(binary_name) != 0) {
        fprintf(stderr, "Failed to generate .gcda file\n");
        cleanup_files(source_file, binary_name, NULL);
        return 1;
    }
    
    // Construct .gcda filename (binary_name + ".gcda")
    char gcda_file[256];
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", binary_name);
    
    // Verify .gcda file exists
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, ".gcda file not created: %s\n", gcda_file);
        cleanup_files(source_file, binary_name, gcda_file);
        return 1;
    }
    
    printf("Generated: %s, %s, %s\n\n", source_file, binary_name, gcda_file);
    
    // Step 2: Test each switch case from gcov-dump.cc lines 111-130
    
    // Case 'h': print_usage()
    printf("2. Testing -h flag (print_usage)...\n");
    test_gcov_dump(NULL, "-h", NULL);
    
    // Case 'v': print_version()
    printf("3. Testing -v flag (print_version)...\n");
    test_gcov_dump(NULL, "-v", NULL);
    
    // Case 'l': flag_dump_contents = 1
    printf("4. Testing -l flag (dump contents)...\n");
    test_gcov_dump(gcda_file, "-l", NULL);
    
    // Case 'p': flag_dump_positions = 1
    printf("5. Testing -p flag (dump positions)...\n");
    test_gcov_dump(gcda_file, "-p", NULL);
    
    // Case 'r': flag_dump_raw = 1
    printf("6. Testing -r flag (dump raw)...\n");
    test_gcov_dump(gcda_file, "-r", NULL);
    
    // Case 's': flag_dump_stable = 1
    printf("7. Testing -s flag (dump stable)...\n");
    test_gcov_dump(gcda_file, "-s", NULL);
    
    // Test combination of flags
    printf("8. Testing combined flags -l -p...\n");
    test_gcov_dump(gcda_file, "-l -p", NULL);
    
    // Default case: invalid flag
    printf("9. Testing invalid flag -X (trigger default case)...\n");
    test_gcov_dump(gcda_file, "-X", "unknown flag");
    
    // Test another invalid flag
    printf("10. Testing invalid flag -z (trigger default case)...\n");
    test_gcov_dump(gcda_file, "-z", "unknown flag");
    
    // Step 3: Cleanup
    printf("11. Cleaning up temporary files...\n");
    cleanup_files(source_file, binary_name, gcda_file);
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
