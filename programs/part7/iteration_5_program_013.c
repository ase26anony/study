#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define TEMP_GCOV_DUMP "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file format:
   - Magic number: 0x67636461 ('gcda')
   - Version: 0x3430322a ('402*' for gcov 4.2 format)
   - Zero-length record: 0x00000000
*/
static const unsigned char minimal_gcda[] = {
    0x67, 0x63, 0x64, 0x61,  /* 'gcda' magic */
    0x34, 0x30, 0x32, 0x2a,  /* '402*' version */
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

/* Capture stderr from command and check for expected string */
int check_stderr_contains(const char *cmd, const char *expected) {
    char buffer[1024];
    char full_cmd[2048];
    FILE *fp;
    
    /* Redirect stderr to stdout for capture */
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    printf("Executing (capturing stderr): %s\n", cmd);
    fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen failed");
        return 0;
    }
    
    int found = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, expected) != NULL) {
            found = 1;
            break;
        }
    }
    
    pclose(fp);
    return found;
}

/* Create minimal valid .gcda file */
int create_minimal_gcda(void) {
    FILE *fp = fopen(TEMP_GCDA_FILE, "wb");
    if (!fp) {
        perror("Failed to create minimal .gcda file");
        return 0;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    return (written == sizeof(minimal_gcda));
}

/* Build instrumented gcov-dump */
int build_gcov_dump(void) {
    const char *gcov_dump_src = NULL;
    
    /* Try to find gcov-dump.cc in common locations */
    const char *possible_paths[] = {
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "../../../gcc/gcov-dump.cc",
        "/usr/src/gcc/gcc/gcov-dump.cc",
        NULL
    };
    
    for (int i = 0; possible_paths[i] != NULL; i++) {
        struct stat st;
        if (stat(possible_paths[i], &st) == 0 && S_ISREG(st.st_mode)) {
            gcov_dump_src = possible_paths[i];
            break;
        }
    }
    
    if (!gcov_dump_src) {
        fprintf(stderr, "Could not find gcov-dump.cc source file\n");
        return 0;
    }
    
    printf("Found gcov-dump source at: %s\n", gcov_dump_src);
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             TEMP_GCOV_DUMP, gcov_dump_src);
    
    printf("Building instrumented gcov-dump...\n");
    return (execute_command(cmd) == 0);
}

/* Test individual flag */
void test_flag(const char *flag, const char *file_arg, int expect_success) {
    char cmd[1024];
    
    if (file_arg) {
        snprintf(cmd, sizeof(cmd), "%s %s %s", TEMP_GCOV_DUMP, flag, file_arg);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s", TEMP_GCOV_DUMP, flag);
    }
    
    int status = execute_command(cmd);
    
    if (expect_success) {
        if (status == 0) {
            printf("✓ Flag %s passed as expected\n", flag);
        } else {
            printf("✗ Flag %s failed unexpectedly (status=%d)\n", flag, status);
        }
    } else {
        if (status != 0) {
            printf("✓ Flag %s failed as expected (status=%d)\n", flag, status);
        } else {
            printf("✗ Flag %s succeeded unexpectedly\n", flag);
        }
    }
}

/* Test flag combinations */
void test_flag_combination(const char *flags, const char *file_arg) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s %s %s", TEMP_GCOV_DUMP, flags, file_arg);
    
    int status = execute_command(cmd);
    
    if (status == 0) {
        printf("✓ Flag combination %s passed\n", flags);
    } else {
        printf("✗ Flag combination %s failed (status=%d)\n", flags, status);
    }
}

int main(void) {
    printf("=== Starting gcov-dump coverage tests ===\n\n");
    
    /* Step 1: Build instrumented gcov-dump */
    printf("1. Building instrumented gcov-dump...\n");
    if (!build_gcov_dump()) {
        fprintf(stderr, "Failed to build gcov-dump. Exiting.\n");
        return 1;
    }
    printf("✓ Built instrumented gcov-dump at %s\n\n", TEMP_GCOV_DUMP);
    
    /* Step 2: Create minimal coverage file */
    printf("2. Creating minimal .gcda file...\n");
    if (!create_minimal_gcda()) {
        fprintf(stderr, "Failed to create minimal .gcda file. Exiting.\n");
        unlink(TEMP_GCOV_DUMP);
        return 1;
    }
    printf("✓ Created minimal .gcda file at %s\n\n", TEMP_GCDA_FILE);
    
    /* Step 3: Execute test sequence */
    printf("3. Executing test sequence...\n\n");
    
    /* Test -h flag (no file argument needed) */
    printf("Testing -h flag (help)...\n");
    test_flag("-h", NULL, 1);
    
    /* Test -v flag (no file argument needed) */
    printf("\nTesting -v flag (version)...\n");
    test_flag("-v", NULL, 1);
    
    /* Test flags that require a coverage file */
    printf("\nTesting -l flag (dump contents)...\n");
    test_flag("-l", TEMP_GCDA_FILE, 1);
    
    printf("\nTesting -p flag (dump positions)...\n");
    test_flag("-p", TEMP_GCDA_FILE, 1);
    
    printf("\nTesting -r flag (dump raw)...\n");
    test_flag("-r", TEMP_GCDA_FILE, 1);
    
    printf("\nTesting -s flag (dump stable)...\n");
    test_flag("-s", TEMP_GCDA_FILE, 1);
    
    /* Test flag combinations */
    printf("\nTesting flag combination -l -p...\n");
    test_flag_combination("-l -p", TEMP_GCDA_FILE);
    
    printf("\nTesting flag combination -p -l (different order)...\n");
    test_flag_combination("-p -l", TEMP_GCDA_FILE);
    
    printf("\nTesting flag combination -r -s...\n");
    test_flag_combination("-r -s", TEMP_GCDA_FILE);
    
    /* Test invalid flag */
    printf("\nTesting invalid flag -X...\n");
    char invalid_cmd[1024];
    snprintf(invalid_cmd, sizeof(invalid_cmd), "%s -X", TEMP_GCOV_DUMP);
    
    if (check_stderr_contains(invalid_cmd, "unknown flag `X'")) {
        printf("✓ Invalid flag test passed - found expected error message\n");
    } else {
        printf("✗ Invalid flag test failed - expected error message not found\n");
    }
    
    /* Test invalid flag with file argument */
    printf("\nTesting invalid flag -X with file argument...\n");
    snprintf(invalid_cmd, sizeof(invalid_cmd), "%s -X %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    
    if (check_stderr_contains(invalid_cmd, "unknown flag `X'")) {
        printf("✓ Invalid flag with file test passed - found expected error message\n");
    } else {
        printf("✗ Invalid flag with file test failed - expected error message not found\n");
    }
    
    /* Step 4: Cleanup */
    printf("\n4. Cleaning up temporary files...\n");
    unlink(TEMP_GCDA_FILE);
    unlink(TEMP_GCOV_DUMP);
    
    /* Also remove coverage data files generated by instrumented gcov-dump */
    char coverage_files[1024];
    snprintf(coverage_files, sizeof(coverage_files), "rm -f %s.gcno %s.gcda", 
             TEMP_GCOV_DUMP, TEMP_GCOV_DUMP);
    system(coverage_files);
    
    printf("✓ Cleanup completed\n");
    printf("\n=== Test sequence completed ===\n");
    
    return 0;
}
