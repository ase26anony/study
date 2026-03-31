#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define INSTRUMENTED_BINARY "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file structure:
   - Magic number: 0x67636461 ('gcda')
   - Version: 0x3430392a ('409*' for gcc 4.9 format)
   - Zero-length record: 0x00000000
*/
static const unsigned char minimal_gcda[] = {
    0x67, 0x63, 0x64, 0x61,  /* 'gcda' magic */
    0x34, 0x30, 0x39, 0x2a,  /* '409*' version */
    0x00, 0x00, 0x00, 0x00   /* zero-length record */
};

/* Execute command and capture output */
static char *execute_capture(const char *cmd, int capture_stderr) {
    char *result = NULL;
    size_t result_size = 0;
    size_t result_len = 0;
    FILE *fp;
    char buffer[1024];
    
    if (capture_stderr) {
        fp = popen(cmd, "r");
    } else {
        char cmd_with_stderr[1024];
        snprintf(cmd_with_stderr, sizeof(cmd_with_stderr), "%s 2>&1", cmd);
        fp = popen(cmd_with_stderr, "r");
    }
    
    if (!fp) {
        return NULL;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t len = strlen(buffer);
        if (result_len + len + 1 > result_size) {
            result_size = (result_size == 0) ? 1024 : result_size * 2;
            result = realloc(result, result_size);
            if (!result) {
                pclose(fp);
                return NULL;
            }
        }
        memcpy(result + result_len, buffer, len);
        result_len += len;
        result[result_len] = '\0';
    }
    
    pclose(fp);
    return result;
}

/* Execute command and return exit status */
static int execute_check(const char *cmd) {
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Create minimal valid .gcda file */
static int create_minimal_gcda(void) {
    FILE *fp = fopen(TEMP_GCDA_FILE, "wb");
    if (!fp) {
        perror("Failed to create temporary .gcda file");
        return 0;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    return (written == sizeof(minimal_gcda));
}

/* Build instrumented gcov-dump */
static int build_instrumented_gcov_dump(void) {
    const char *gcov_dump_src = NULL;
    
    /* Try to find gcov-dump.cc in common locations */
    const char *possible_paths[] = {
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "../../../gcc/gcov-dump.cc",
        "/usr/src/gcc/gcc/gcov-dump.cc",
        NULL
    };
    
    for (int i = 0; possible_paths[i]; i++) {
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
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, gcov_dump_src);
    
    printf("Building instrumented gcov-dump: %s\n", cmd);
    
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 0;
    }
    
    /* Verify the binary was created */
    struct stat st;
    if (stat(INSTRUMENTED_BINARY, &st) != 0 || !S_ISREG(st.st_mode)) {
        fprintf(stderr, "Instrumented binary not created\n");
        return 0;
    }
    
    return 1;
}

/* Test -h flag (help) */
static void test_help_flag(void) {
    printf("Testing -h flag...\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    
    int exit_code = execute_check(cmd);
    if (exit_code == 0) {
        printf("  ✓ -h flag exited successfully\n");
    } else {
        printf("  ✗ -h flag failed with exit code %d\n", exit_code);
    }
}

/* Test -v flag (version) */
static void test_version_flag(void) {
    printf("Testing -v flag...\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    
    char *output = execute_capture(cmd, 0);
    if (output) {
        if (strstr(output, "gcov-dump") || strstr(output, "GCC")) {
            printf("  ✓ -v flag printed version information\n");
        } else {
            printf("  ✗ -v flag output doesn't contain expected version info\n");
        }
        free(output);
    } else {
        printf("  ✗ Failed to capture -v flag output\n");
    }
}

/* Test invalid flag */
static void test_invalid_flag(void) {
    printf("Testing invalid flag -X...\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -X", INSTRUMENTED_BINARY);
    
    char *output = execute_capture(cmd, 1);
    if (output) {
        if (strstr(output, "unknown flag `X'")) {
            printf("  ✓ Invalid flag correctly detected\n");
        } else {
            printf("  ✗ Expected 'unknown flag' message not found\n");
            printf("    Output: %s\n", output);
        }
        free(output);
    } else {
        printf("  ✗ Failed to capture invalid flag output\n");
    }
}

/* Test flag with coverage file */
static void test_flag_with_file(const char *flag, const char *description) {
    printf("Testing %s flag (%s)...\n", flag, description);
    
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flag, TEMP_GCDA_FILE);
    
    int exit_code = execute_check(cmd);
    if (exit_code == 0) {
        printf("  ✓ %s flag executed successfully\n", flag);
    } else {
        printf("  ✗ %s flag failed with exit code %d\n", flag, exit_code);
    }
}

/* Test flag combination */
static void test_flag_combination(const char *flags, const char *description) {
    printf("Testing flag combination %s (%s)...\n", flags, description);
    
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flags, TEMP_GCDA_FILE);
    
    int exit_code = execute_check(cmd);
    if (exit_code == 0) {
        printf("  ✓ Combination %s executed successfully\n", flags);
    } else {
        printf("  ✗ Combination %s failed with exit code %d\n", flags, exit_code);
    }
}

/* Test flag ordering variations */
static void test_flag_ordering(void) {
    printf("Testing flag ordering variations...\n");
    
    /* Test -l -p */
    char cmd1[512];
    snprintf(cmd1, sizeof(cmd1), "%s -l -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    int exit1 = execute_check(cmd1);
    
    /* Test -p -l */
    char cmd2[512];
    snprintf(cmd2, sizeof(cmd2), "%s -p -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    int exit2 = execute_check(cmd2);
    
    if (exit1 == 0 && exit2 == 0) {
        printf("  ✓ Both flag orderings executed successfully\n");
    } else {
        printf("  ✗ Flag ordering issues: -l -p=%d, -p -l=%d\n", exit1, exit2);
    }
}

/* Cleanup temporary files */
static void cleanup(void) {
    unlink(TEMP_GCDA_FILE);
    unlink(INSTRUMENTED_BINARY);
    unlink("gcov-dump-instrumented.gcno");
    unlink("gcov-dump-instrumented.gcda");
}

int main(void) {
    printf("=== Starting gcov-dump coverage test ===\n\n");
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_instrumented_gcov_dump()) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    if (!create_minimal_gcda()) {
        fprintf(stderr, "Failed to create minimal .gcda file\n");
        cleanup();
        return 1;
    }
    
    /* Step 3: Execute test sequence */
    
    /* Test flags without file argument */
    test_help_flag();
    printf("\n");
    
    test_version_flag();
    printf("\n");
    
    test_invalid_flag();
    printf("\n");
    
    /* Test flags requiring coverage file */
    test_flag_with_file("-l", "dump contents");
    printf("\n");
    
    test_flag_with_file("-p", "dump positions");
    printf("\n");
    
    test_flag_with_file("-r", "dump raw");
    printf("\n");
    
    test_flag_with_file("-s", "dump stable");
    printf("\n");
    
    /* Test flag combinations */
    test_flag_combination("-l -p", "dump contents and positions");
    printf("\n");
    
    test_flag_combination("-r -s", "dump raw and stable");
    printf("\n");
    
    /* Test flag ordering */
    test_flag_ordering();
    printf("\n");
    
    /* Step 4: Cleanup */
    printf("=== Cleaning up temporary files ===\n");
    cleanup();
    
    printf("\n=== Test sequence completed ===\n");
    
    return 0;
}
