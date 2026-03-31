#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define GCC_SRC_DIR "../../gcc"  /* Adjust based on your GCC source location */
#define GCOV_DUMP_SRC GCC_SRC_DIR "/gcc/gcov-dump.cc"

/* GCOV file format constants */
#define GCOV_DATA_MAGIC 0x67636461  /* "gcda" */
#define GCOV_NOTE_MAGIC 0x67636e6f  /* "gcno" */
#define GCOV_VERSION ((unsigned)0x3430362a) /* GCC 4.6.0 */

/* Create a minimal valid .gcda file */
void create_minimal_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create minimal gcda file");
        exit(1);
    }
    
    /* Write GCOV data magic */
    unsigned magic = GCOV_DATA_MAGIC;
    fwrite(&magic, sizeof(unsigned), 1, fp);
    
    /* Write version */
    unsigned version = GCOV_VERSION;
    fwrite(&version, sizeof(unsigned), 1, fp);
    
    /* Write a zero-length tag to indicate end of file */
    unsigned zero_tag = 0;
    fwrite(&zero_tag, sizeof(unsigned), 1, fp);
    
    fclose(fp);
}

/* Execute command and capture output */
int execute_capture(const char *cmd, char *output, size_t output_size, 
                    int capture_stderr, int *exit_status) {
    char full_cmd[1024];
    if (capture_stderr) {
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    } else {
        snprintf(full_cmd, sizeof(full_cmd), "%s", cmd);
    }
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        return -1;
    }
    
    output[0] = '\0';
    size_t total_read = 0;
    while (fgets(output + total_read, output_size - total_read, fp)) {
        total_read = strlen(output);
    }
    
    int status = pclose(fp);
    if (exit_status) {
        *exit_status = WEXITSTATUS(status);
    }
    
    return 0;
}

/* Build instrumented gcov-dump */
int build_instrumented_gcov_dump(const char *output_path) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             output_path, GCOV_DUMP_SRC);
    
    printf("Building instrumented gcov-dump: %s\n", cmd);
    int status = system(cmd);
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Successfully built instrumented gcov-dump at %s\n", output_path);
        return 1;
    } else {
        printf("Failed to build instrumented gcov-dump\n");
        return 0;
    }
}

/* Test a specific flag */
void test_flag(const char *gcov_dump_path, const char *flag, 
               const char *filename, int expect_success,
               const char *expected_output, int check_stderr) {
    char cmd[1024];
    char output[4096];
    int exit_status;
    
    printf("\n=== Testing flag '%s' ===\n", flag);
    
    if (filename) {
        snprintf(cmd, sizeof(cmd), "%s %s %s", gcov_dump_path, flag, filename);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, flag);
    }
    
    printf("Command: %s\n", cmd);
    
    if (execute_capture(cmd, output, sizeof(output), check_stderr, &exit_status) < 0) {
        printf("Failed to execute command\n");
        return;
    }
    
    printf("Exit status: %d\n", exit_status);
    
    if (expect_success) {
        if (exit_status != 0) {
            printf("FAIL: Expected success (exit 0), got %d\n", exit_status);
        } else {
            printf("PASS: Exit status as expected\n");
        }
    } else {
        if (exit_status == 0) {
            printf("FAIL: Expected failure, got success\n");
        } else {
            printf("PASS: Exit status indicates failure as expected\n");
        }
    }
    
    if (expected_output) {
        if (strstr(output, expected_output) != NULL) {
            printf("PASS: Found expected output '%s'\n", expected_output);
        } else {
            printf("FAIL: Did not find expected output '%s'\n", expected_output);
            printf("Actual output:\n%s\n", output);
        }
    }
    
    /* For debugging */
    if (strlen(output) > 0) {
        printf("Output (first 200 chars):\n%.200s\n", output);
    }
}

int main() {
    char tmp_gcda[] = "/tmp/minimal_gcda_XXXXXX";
    char gcov_dump_path[] = "/tmp/gcov_dump_instrumented_XXXXXX";
    
    /* Create temporary filenames */
    int fd = mkstemp(gcov_dump_path);
    if (fd < 0) {
        perror("Failed to create temporary gcov-dump path");
        return 1;
    }
    close(fd);
    
    strcpy(tmp_gcda, "/tmp/minimal_gcda_XXXXXX");
    fd = mkstemp(tmp_gcda);
    if (fd < 0) {
        perror("Failed to create temporary gcda path");
        return 1;
    }
    close(fd);
    
    printf("Temporary files:\n");
    printf("  gcov-dump: %s\n", gcov_dump_path);
    printf("  gcda file: %s\n", tmp_gcda);
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_instrumented_gcov_dump(gcov_dump_path)) {
        printf("Cannot proceed without instrumented gcov-dump\n");
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("\nCreating minimal .gcda file at %s\n", tmp_gcda);
    create_minimal_gcda(tmp_gcda);
    
    /* Step 3: Execute test sequence */
    
    /* Test -h flag (help) - no file needed */
    test_flag(gcov_dump_path, "-h", NULL, 1, "Usage:", 0);
    
    /* Test -v flag (version) - no file needed */
    test_flag(gcov_dump_path, "-v", NULL, 1, "gcov-dump", 0);
    
    /* Test -l flag (dump contents) with minimal file */
    test_flag(gcov_dump_path, "-l", tmp_gcda, 1, NULL, 0);
    
    /* Test -p flag (dump positions) with minimal file */
    test_flag(gcov_dump_path, "-p", tmp_gcda, 1, NULL, 0);
    
    /* Test -r flag (dump raw) with minimal file */
    test_flag(gcov_dump_path, "-r", tmp_gcda, 1, NULL, 0);
    
    /* Test -s flag (dump stable) with minimal file */
    test_flag(gcov_dump_path, "-s", tmp_gcda, 1, NULL, 0);
    
    /* Test flag combinations */
    test_flag(gcov_dump_path, "-l -p", tmp_gcda, 1, NULL, 0);
    test_flag(gcov_dump_path, "-p -l", tmp_gcda, 1, NULL, 0);
    test_flag(gcov_dump_path, "-r -s", tmp_gcda, 1, NULL, 0);
    test_flag(gcov_dump_path, "-s -r", tmp_gcda, 1, NULL, 0);
    
    /* Test invalid flag -X */
    test_flag(gcov_dump_path, "-X", tmp_gcda, 0, "unknown flag", 1);
    
    /* Test invalid flag with no file */
    test_flag(gcov_dump_path, "-Z", NULL, 0, "unknown flag", 1);
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(tmp_gcda);
    unlink(gcov_dump_path);
    
    /* Also clean up coverage files generated by instrumented binary */
    char coverage_files[1024];
    snprintf(coverage_files, sizeof(coverage_files), 
             "rm -f %s.gcno %s.gcda", gcov_dump_path, gcov_dump_path);
    system(coverage_files);
    
    printf("Temporary files removed\n");
    printf("\nTest sequence completed\n");
    
    return 0;
}
