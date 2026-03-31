#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define TEMP_GCOV_DUMP "/tmp/gcov-dump-instrumented"

/* Minimal valid .gcda file structure:
 * - Magic number: 0x67636461 ('gcda')
 * - Version: 0x3430372a ('407*' for GCC 4.7+)
 * - Stamp: 0x00000000
 * - Zero-length record: 0x00000000
 */
static unsigned char minimal_gcda[] = {
    0x61, 0x63, 0x64, 0x67,  /* 'gcda' in little-endian */
    0x2a, 0x37, 0x30, 0x34,  /* '407*' in little-endian */
    0x00, 0x00, 0x00, 0x00,  /* stamp */
    0x00, 0x00, 0x00, 0x00   /* zero-length record */
};

/* Execute command and return exit status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Execute command and capture output to buffer */
int execute_and_capture(const char *cmd, char *buffer, size_t buffer_size, int capture_stderr) {
    char full_cmd[1024];
    if (capture_stderr) {
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    } else {
        snprintf(full_cmd, sizeof(full_cmd), "%s", cmd);
    }
    
    printf("Executing: %s\n", full_cmd);
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        return -1;
    }
    
    size_t bytes_read = fread(buffer, 1, buffer_size - 1, fp);
    buffer[bytes_read] = '\0';
    
    int status = pclose(fp);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Search for substring in string */
int contains_string(const char *str, const char *substr) {
    return strstr(str, substr) != NULL;
}

/* Create minimal .gcda file */
int create_minimal_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create .gcda file");
        return 0;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    return written == sizeof(minimal_gcda);
}

/* Find gcov-dump.cc in common locations */
char* find_gcov_dump_cc() {
    static char path[1024];
    const char *locations[] = {
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "../../../gcc/gcov-dump.cc",
        "gcc/gcov-dump.cc",
        "/usr/src/gcc/gcc/gcov-dump.cc",
        NULL
    };
    
    for (int i = 0; locations[i]; i++) {
        if (access(locations[i], R_OK) == 0) {
            realpath(locations[i], path);
            return path;
        }
    }
    
    /* Try to find via gcc installation */
    FILE *fp = popen("gcc -print-file-name=include 2>/dev/null", "r");
    if (fp) {
        char gcc_path[1024];
        if (fgets(gcc_path, sizeof(gcc_path), fp)) {
            /* Remove trailing newline */
            gcc_path[strcspn(gcc_path, "\n")] = 0;
            
            /* Go up directories to find gcov-dump.cc */
            char *slash = strrchr(gcc_path, '/');
            if (slash) {
                *slash = '\0';  /* Remove include */
                slash = strrchr(gcc_path, '/');
                if (slash) {
                    *slash = '\0';  /* Remove gcc version */
                    snprintf(path, sizeof(path), "%s/../gcc/gcov-dump.cc", gcc_path);
                    if (access(path, R_OK) == 0) {
                        pclose(fp);
                        return path;
                    }
                }
            }
        }
        pclose(fp);
    }
    
    return NULL;
}

/* Build instrumented gcov-dump */
int build_gcov_dump() {
    char *gcov_dump_cc = find_gcov_dump_cc();
    if (!gcov_dump_cc) {
        fprintf(stderr, "ERROR: Could not find gcov-dump.cc\n");
        fprintf(stderr, "Please specify path to gcov-dump.cc: ");
        fgets(gcov_dump_cc, 1024, stdin);
        gcov_dump_cc[strcspn(gcov_dump_cc, "\n")] = 0;
    }
    
    printf("Found gcov-dump.cc at: %s\n", gcov_dump_cc);
    
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             TEMP_GCOV_DUMP, gcov_dump_cc);
    
    printf("Building instrumented gcov-dump...\n");
    int status = execute_command(cmd);
    
    if (status != 0) {
        fprintf(stderr, "Failed to build gcov-dump. Trying alternative compilation...\n");
        
        /* Try without -lgcov for older GCC versions */
        snprintf(cmd, sizeof(cmd),
                 "g++ -O0 -fprofile-arcs -ftest-coverage -o %s %s",
                 TEMP_GCOV_DUMP, gcov_dump_cc);
        status = execute_command(cmd);
    }
    
    if (status != 0) {
        fprintf(stderr, "ERROR: Failed to compile gcov-dump\n");
        return 0;
    }
    
    /* Verify the binary exists and is executable */
    if (access(TEMP_GCOV_DUMP, X_OK) != 0) {
        fprintf(stderr, "ERROR: Instrumented binary not created\n");
        return 0;
    }
    
    printf("Successfully built instrumented gcov-dump\n");
    return 1;
}

int main() {
    printf("=== Starting gcov-dump coverage test ===\n");
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_gcov_dump()) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("\nCreating minimal .gcda file...\n");
    if (!create_minimal_gcda(TEMP_GCDA_FILE)) {
        fprintf(stderr, "Failed to create minimal .gcda file\n");
        unlink(TEMP_GCOV_DUMP);
        return 1;
    }
    printf("Created %s\n", TEMP_GCDA_FILE);
    
    /* Step 3: Execute test sequence */
    printf("\n=== Testing flag parsing ===\n");
    
    char buffer[4096];
    int passed = 0;
    int total = 0;
    
    /* Test -h flag (help) */
    printf("\n1. Testing -h flag...\n");
    total++;
    int status = execute_and_capture(TEMP_GCOV_DUMP " -h", buffer, sizeof(buffer), 0);
    if (status == 0) {
        printf("✓ -h flag passed (exit code 0)\n");
        passed++;
    } else {
        printf("✗ -h flag failed (exit code %d)\n", status);
    }
    
    /* Test -v flag (version) */
    printf("\n2. Testing -v flag...\n");
    total++;
    status = execute_and_capture(TEMP_GCOV_DUMP " -v", buffer, sizeof(buffer), 0);
    if (status == 0 && contains_string(buffer, "gcov-dump") && 
        (contains_string(buffer, "version") || contains_string(buffer, "GCC"))) {
        printf("✓ -v flag passed. Output contains version info\n");
        passed++;
    } else {
        printf("✗ -v flag failed. Output:\n%s\n", buffer);
    }
    
    /* Test -l flag (dump contents) */
    printf("\n3. Testing -l flag...\n");
    total++;
    status = execute_and_capture(TEMP_GCOV_DUMP " -l " TEMP_GCDA_FILE, 
                                 buffer, sizeof(buffer), 0);
    if (status == 0) {
        printf("✓ -l flag passed\n");
        passed++;
    } else {
        printf("✗ -l flag failed (exit code %d)\n", status);
    }
    
    /* Test -p flag (dump positions) */
    printf("\n4. Testing -p flag...\n");
    total++;
    status = execute_and_capture(TEMP_GCOV_DUMP " -p " TEMP_GCDA_FILE, 
                                 buffer, sizeof(buffer), 0);
    if (status == 0) {
        printf("✓ -p flag passed\n");
        passed++;
    } else {
        printf("✗ -p flag failed (exit code %d)\n", status);
    }
    
    /* Test -r flag (dump raw) */
    printf("\n5. Testing -r flag...\n");
    total++;
    status = execute_and_capture(TEMP_GCOV_DUMP " -r " TEMP_GCDA_FILE, 
                                 buffer, sizeof(buffer), 0);
    if (status == 0) {
        printf("✓ -r flag passed\n");
        passed++;
    } else {
        printf("✗ -r flag failed (exit code %d)\n", status);
    }
    
    /* Test -s flag (dump stable) */
    printf("\n6. Testing -s flag...\n");
    total++;
    status = execute_and_capture(TEMP_GCOV_DUMP " -s " TEMP_GCDA_FILE, 
                                 buffer, sizeof(buffer), 0);
    if (status == 0) {
        printf("✓ -s flag passed\n");
        passed++;
    } else {
        printf("✗ -s flag failed (exit code %d)\n", status);
    }
    
    /* Test flag combinations */
    printf("\n7. Testing flag combination -l -p...\n");
    total++;
    status = execute_and_capture(TEMP_GCOV_DUMP " -l -p " TEMP_GCDA_FILE, 
                                 buffer, sizeof(buffer), 0);
    if (status == 0) {
        printf("✓ -l -p combination passed\n");
        passed++;
    } else {
        printf("✗ -l -p combination failed\n");
    }
    
    printf("\n8. Testing flag combination -r -s...\n");
    total++;
    status = execute_and_capture(TEMP_GCOV_DUMP " -r -s " TEMP_GCDA_FILE, 
                                 buffer, sizeof(buffer), 0);
    if (status == 0) {
        printf("✓ -r -s combination passed\n");
        passed++;
    } else {
        printf("✗ -r -s combination failed\n");
    }
    
    /* Test different flag ordering */
    printf("\n9. Testing flag ordering -p -l...\n");
    total++;
    status = execute_and_capture(TEMP_GCOV_DUMP " -p -l " TEMP_GCDA_FILE, 
                                 buffer, sizeof(buffer), 0);
    if (status == 0) {
        printf("✓ -p -l ordering passed\n");
        passed++;
    } else {
        printf("✗ -p -l ordering failed\n");
    }
    
    /* Test invalid flag -X */
    printf("\n10. Testing invalid flag -X...\n");
    total++;
    status = execute_and_capture(TEMP_GCOV_DUMP " -X " TEMP_GCDA_FILE, 
                                 buffer, sizeof(buffer), 1);  /* Capture stderr */
    if (status != 0 && contains_string(buffer, "unknown flag")) {
        printf("✓ Invalid flag test passed. Got expected error:\n");
        printf("  %s", buffer);
        passed++;
    } else {
        printf("✗ Invalid flag test failed. Output:\n%s\n", buffer);
    }
    
    /* Test invalid flag without file argument */
    printf("\n11. Testing invalid flag -Y (no file)...\n");
    total++;
    status = execute_and_capture(TEMP_GCOV_DUMP " -Y", buffer, sizeof(buffer), 1);
    if (status != 0 && contains_string(buffer, "unknown flag")) {
        printf("✓ Invalid flag without file passed. Got expected error:\n");
        printf("  %s", buffer);
        passed++;
    } else {
        printf("✗ Invalid flag without file failed. Output:\n%s\n", buffer);
    }
    
    /* Step 4: Cleanup and report */
    printf("\n=== Cleanup ===\n");
    unlink(TEMP_GCDA_FILE);
    unlink(TEMP_GCOV_DUMP);
    
    /* Also clean up coverage files generated by instrumented gcov-dump */
    char coverage_files[][50] = {
        TEMP_GCOV_DUMP ".gcda",
        TEMP_GCOV_DUMP ".gcno",
        "gcov-dump-instrumented.gcda",
        "gcov-dump-instrumented.gcno"
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (access(coverage_files[i], F_OK) == 0) {
            unlink(coverage_files[i]);
        }
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d tests\n", passed, total);
    
    if (passed == total) {
        printf("✓ All tests passed!\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n");
        return 1;
    }
}
