#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define TEMP_DIR "/tmp/gcov_test_XXXXXX"
#define HELPER_SRC "helper.c"
#define HELPER_BIN "helper"
#define GCOV_DATA "helper.gcda"

/* Simple C program that will generate GCOV data */
const char *helper_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Generating GCOV data...\\n\");\n"
"    return 0;\n"
"}\n";

/* Execute a command and capture its stderr output */
int execute_and_check(const char *cmd, const char *expected_error) {
    char buffer[1024];
    int found_error = 0;
    FILE *fp;
    
    printf("Executing: %s\n", cmd);
    
    /* Use popen with stderr redirected to stdout */
    char full_cmd[1024];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    fp = popen(full_cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    /* Read output and check for expected error message */
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("  Output: %s", buffer);
        if (expected_error && strstr(buffer, expected_error)) {
            found_error = 1;
        }
    }
    
    int status = pclose(fp);
    int exit_code = WEXITSTATUS(status);
    
    printf("  Exit code: %d\n", exit_code);
    
    if (expected_error) {
        /* For invalid flags, we expect error message AND non-zero exit */
        return found_error && (exit_code != 0);
    } else {
        /* For valid flags, we expect success (exit code 0) */
        return exit_code == 0;
    }
}

int main(int argc, char *argv[]) {
    char temp_dir[256];
    char helper_src_path[512];
    char helper_bin_path[512];
    char gcda_path[512];
    char cmd[1024];
    int success = 1;
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMP_DIR);
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temp directory");
        return 1;
    }
    
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Create paths */
    snprintf(helper_src_path, sizeof(helper_src_path), "%s/%s", temp_dir, HELPER_SRC);
    snprintf(helper_bin_path, sizeof(helper_bin_path), "%s/%s", temp_dir, HELPER_BIN);
    snprintf(gcda_path, sizeof(gcda_path), "%s/%s", temp_dir, GCOV_DATA);
    
    /* Step 1: Write helper source file */
    FILE *src = fopen(helper_src_path, "w");
    if (!src) {
        perror("Failed to create helper source");
        return 1;
    }
    fputs(helper_source, src);
    fclose(src);
    
    /* Step 2: Compile helper with coverage flags */
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -o %s %s 2>&1",
             helper_bin_path, helper_src_path);
    
    printf("Compiling helper program...\n");
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        /* Clean up and exit */
        snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
        system(cmd);
        return 1;
    }
    
    /* Step 3: Run helper to generate .gcda file */
    printf("Running helper to generate GCOV data...\n");
    if (chdir(temp_dir) != 0) {
        perror("chdir failed");
        success = 0;
        goto cleanup;
    }
    
    if (system("./" HELPER_BIN " 2>&1") != 0) {
        fprintf(stderr, "Failed to run helper program\n");
        success = 0;
        goto cleanup;
    }
    
    /* Verify .gcda file was created */
    struct stat st;
    if (stat(GCOV_DATA, &st) != 0 || !S_ISREG(st.st_mode)) {
        fprintf(stderr, "GCOV data file not created\n");
        success = 0;
        goto cleanup;
    }
    
    printf("GCOV data file created successfully\n");
    
    /* Step 4: Test gcov-dump with various flags */
    
    /* First, test with a valid flag to ensure basic functionality */
    printf("\n=== Testing valid flag ===\n");
    if (!execute_and_check("gcov-dump -l " GCOV_DATA, NULL)) {
        fprintf(stderr, "Valid flag test failed\n");
        success = 0;
    }
    
    /* Test with multiple invalid single-character flags */
    printf("\n=== Testing invalid flags ===\n");
    
    /* Test alphabetic characters not in {h,v,l,p,r,s} */
    const char *invalid_flags[] = {"-a", "-b", "-c", "-x", "-y", "-z", "-1", "-?", "-@"};
    int num_flags = sizeof(invalid_flags) / sizeof(invalid_flags[0]);
    
    for (int i = 0; i < num_flags; i++) {
        printf("\nTest %d/%d:\n", i + 1, num_flags);
        snprintf(cmd, sizeof(cmd), "gcov-dump %s " GCOV_DATA, invalid_flags[i]);
        
        if (!execute_and_check(cmd, "unknown flag")) {
            fprintf(stderr, "Invalid flag test failed for %s\n", invalid_flags[i]);
            success = 0;
        }
    }
    
    /* Test combination: valid flag followed by invalid flag */
    printf("\n=== Testing flag combination ===\n");
    if (!execute_and_check("gcov-dump -l -x " GCOV_DATA, "unknown flag")) {
        fprintf(stderr, "Flag combination test failed\n");
        success = 0;
    }
    
    /* Test invalid flag without data file (should also trigger error) */
    printf("\n=== Testing invalid flag without data file ===\n");
    if (!execute_and_check("gcov-dump -z", "unknown flag")) {
        fprintf(stderr, "Invalid flag without file test failed\n");
        success = 0;
    }

cleanup:
    /* Step 5: Cleanup */
    chdir("..");  /* Leave temp directory */
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    system(cmd);
    
    printf("\n=== Summary ===\n");
    if (success) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Some tests failed\n");
        return 1;
    }
}
