#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096

/* Simple helper program to generate GCOV data */
const char *helper_source = 
    "#include <stdio.h>\n"
    "int main() {\n"
    "    printf(\"Generating GCOV data...\\n\");\n"
    "    return 0;\n"
    "}\n";

/* Execute a command and capture its output */
int execute_and_capture(const char *cmd, char *output, size_t output_size) {
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        return -1;
    }
    
    size_t total_read = 0;
    while (fgets(output + total_read, output_size - total_read, fp) != NULL) {
        total_read = strlen(output);
        if (total_read >= output_size - 1) {
            break;
        }
    }
    
    int status = pclose(fp);
    output[total_read] = '\0';
    return WEXITSTATUS(status);
}

/* Check if output contains the expected error message */
int contains_unknown_flag(const char *output) {
    return strstr(output, "unknown flag") != NULL;
}

int main() {
    char cmd[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN];
    int failures = 0;
    
    /* Create temporary directory for test files */
    char tmpdir[] = "/tmp/gcov_test_XXXXXX";
    if (!mkdtemp(tmpdir)) {
        perror("Failed to create temp directory");
        return 1;
    }
    
    /* Write helper source file */
    char helper_c_path[256];
    snprintf(helper_c_path, sizeof(helper_c_path), "%s/helper.c", tmpdir);
    
    FILE *fp = fopen(helper_c_path, "w");
    if (!fp) {
        perror("Failed to create helper.c");
        return 1;
    }
    fputs(helper_source, fp);
    fclose(fp);
    
    /* Compile helper with coverage instrumentation */
    char helper_exe_path[256];
    snprintf(helper_exe_path, sizeof(helper_exe_path), "%s/helper", tmpdir);
    
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -o %s %s 2>&1",
             helper_exe_path, helper_c_path);
    
    printf("Compiling helper program...\n");
    if (execute_and_capture(cmd, output, sizeof(output)) != 0) {
        printf("Compilation failed:\n%s\n", output);
        failures++;
        goto cleanup;
    }
    
    /* Run helper to generate .gcda file */
    printf("Running helper to generate GCOV data...\n");
    snprintf(cmd, sizeof(cmd), "%s", helper_exe_path);
    if (execute_and_capture(cmd, output, sizeof(output)) != 0) {
        printf("Helper execution failed:\n%s\n", output);
        failures++;
        goto cleanup;
    }
    
    /* Path to the generated .gcda file */
    char gcda_path[256];
    snprintf(gcda_path, sizeof(gcda_path), "%s/helper.gcda", tmpdir);
    
    /* Test 1: Valid command to ensure gcov-dump works */
    printf("\n=== Test 1: Valid flag (-l) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s 2>&1", gcda_path);
    int status = execute_and_capture(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status);
    if (status == 0) {
        printf("Valid flag test PASSED\n");
    } else {
        printf("Valid flag test FAILED\n");
        failures++;
    }
    
    /* Test 2-7: Invalid single-character flags to trigger default case */
    char invalid_flags[] = "a?x9Z!";
    for (int i = 0; i < strlen(invalid_flags); i++) {
        printf("\n=== Test %d: Invalid flag (-%c) ===\n", i+2, invalid_flags[i]);
        snprintf(cmd, sizeof(cmd), "gcov-dump -%c %s 2>&1", 
                 invalid_flags[i], gcda_path);
        
        status = execute_and_capture(cmd, output, sizeof(output));
        printf("Exit status: %d\n", status);
        printf("Output:\n%s\n", output);
        
        if (contains_unknown_flag(output) && status != 0) {
            printf("Invalid flag test PASSED (correctly rejected -%c)\n", 
                   invalid_flags[i]);
        } else {
            printf("Invalid flag test FAILED for -%c\n", invalid_flags[i]);
            failures++;
        }
    }
    
    /* Test 8: Multiple invalid flags in one call */
    printf("\n=== Test 8: Multiple invalid flags (-ab) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -ab %s 2>&1", gcda_path);
    status = execute_and_capture(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status);
    printf("Output:\n%s\n", output);
    
    if (contains_unknown_flag(output) && status != 0) {
        printf("Multiple invalid flags test PASSED\n");
    } else {
        printf("Multiple invalid flags test FAILED\n");
        failures++;
    }
    
    /* Test 9: Valid flag followed by invalid flag */
    printf("\n=== Test 9: Valid + invalid flag (-la) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -la %s 2>&1", gcda_path);
    status = execute_and_capture(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status);
    printf("Output:\n%s\n", output);
    
    if (contains_unknown_flag(output) && status != 0) {
        printf("Mixed flags test PASSED\n");
    } else {
        printf("Mixed flags test FAILED\n");
        failures++;
    }

cleanup:
    /* Cleanup temporary files */
    printf("\n=== Cleaning up ===\n");
    snprintf(cmd, sizeof(cmd), "rm -rf %s", tmpdir);
    system(cmd);
    
    printf("\n=== Summary ===\n");
    printf("Total failures: %d\n", failures);
    
    return failures > 0 ? 1 : 0;
}
