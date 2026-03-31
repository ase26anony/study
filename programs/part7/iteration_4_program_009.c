#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_XXXXXX.c"
#define TEMP_BINARY_FILE "test_coverage_XXXXXX"
#define MAX_CMD_LEN 1024

/* Create a minimal C source file with coverage instrumentation */
const char *test_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    int i, sum = 0;\n"
"    for (i = 0; i < 10; i++) {\n"
"        sum += i;\n"
"    }\n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Function to create a temporary filename */
void create_temp_filename(char *template, const char *suffix) {
    int fd;
    char *temp;
    
    /* Create a temporary file to get a unique name */
    fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        exit(EXIT_FAILURE);
    }
    close(fd);
    unlink(template);  /* Remove the file, we just want the name */
    
    /* Append suffix if provided */
    if (suffix) {
        strcat(template, suffix);
    }
}

/* Execute a system command and check return status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        printf("Command failed with status: %d\n", WEXITSTATUS(status));
    }
    return status;
}

/* Execute command and capture stderr to check for specific output */
int execute_and_check_stderr(const char *cmd, const char *expected_error) {
    char cmd_with_stderr[MAX_CMD_LEN];
    char buffer[256];
    FILE *fp;
    int found = 0;
    
    /* Redirect stderr to stdout and capture it */
    snprintf(cmd_with_stderr, sizeof(cmd_with_stderr), "%s 2>&1", cmd);
    
    printf("Executing (checking stderr): %s\n", cmd);
    
    fp = popen(cmd_with_stderr, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, expected_error) != NULL) {
            found = 1;
            printf("Found expected error message: %s", buffer);
        }
    }
    
    pclose(fp);
    return found;
}

/* Clean up temporary files */
void cleanup_files(const char *source_file, const char *binary_file, 
                   const char *gcda_file, const char *gcno_file) {
    if (source_file) unlink(source_file);
    if (binary_file) unlink(binary_file);
    if (gcda_file) unlink(gcda_file);
    if (gcno_file) unlink(gcno_file);
}

int main() {
    char source_file[256] = TEMP_SOURCE_FILE;
    char binary_file[256] = TEMP_BINARY_FILE;
    char gcda_file[256];
    char gcno_file[256];
    char cmd[MAX_CMD_LEN];
    int ret;
    
    printf("=== Starting gcov-dump test driver ===\n");
    
    /* Step 1: Create unique filenames */
    create_temp_filename(source_file, ".c");
    create_temp_filename(binary_file, "");
    
    /* Derive gcda and gcno filenames from binary filename */
    strcpy(gcda_file, binary_file);
    strcat(gcda_file, ".gcda");
    
    strcpy(gcno_file, binary_file);
    strcat(gcno_file, ".gcno");
    
    printf("Source file: %s\n", source_file);
    printf("Binary file: %s\n", binary_file);
    printf("Expected gcda file: %s\n", gcda_file);
    printf("Expected gcno file: %s\n", gcno_file);
    
    /* Step 2: Create test source file */
    FILE *fp = fopen(source_file, "w");
    if (!fp) {
        perror("Failed to create source file");
        return EXIT_FAILURE;
    }
    fprintf(fp, "%s", test_source);
    fclose(fp);
    
    /* Step 3: Compile with coverage flags */
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_file);
    ret = execute_command(cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_files(source_file, binary_file, NULL, NULL);
        return EXIT_FAILURE;
    }
    
    /* Step 4: Execute the program to generate gcda file */
    ret = execute_command(binary_file);
    if (ret != 0) {
        fprintf(stderr, "Execution of test program failed\n");
        cleanup_files(source_file, binary_file, gcda_file, gcno_file);
        return EXIT_FAILURE;
    }
    
    /* Step 5: Verify gcda file was created */
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "gcda file not created: %s\n", gcda_file);
        cleanup_files(source_file, binary_file, gcda_file, gcno_file);
        return EXIT_FAILURE;
    }
    printf("Successfully generated gcda file: %s\n", gcda_file);
    
    printf("\n=== Testing gcov-dump with various flags ===\n");
    
    /* Test 1: Help flag (-h) - triggers print_usage() */
    printf("\n1. Testing -h flag (help):\n");
    execute_command("gcov-dump -h");
    
    /* Test 2: Version flag (-v) - triggers print_version() */
    printf("\n2. Testing -v flag (version):\n");
    execute_command("gcov-dump -v");
    
    /* Test 3: Dump contents flag (-l) - sets flag_dump_contents = 1 */
    printf("\n3. Testing -l flag (dump contents):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    execute_command(cmd);
    
    /* Test 4: Dump positions flag (-p) - sets flag_dump_positions = 1 */
    printf("\n4. Testing -p flag (dump positions):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s", gcda_file);
    execute_command(cmd);
    
    /* Test 5: Dump raw flag (-r) - sets flag_dump_raw = 1 */
    printf("\n5. Testing -r flag (dump raw):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s", gcda_file);
    execute_command(cmd);
    
    /* Test 6: Dump stable flag (-s) - sets flag_dump_stable = 1 */
    printf("\n7. Testing -s flag (dump stable):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s", gcda_file);
    execute_command(cmd);
    
    /* Test 7: Combined flags (-l -p) */
    printf("\n8. Testing combined flags (-l -p):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s", gcda_file);
    execute_command(cmd);
    
    /* Test 8: Invalid flag (-X) - triggers default case and fprintf */
    printf("\n9. Testing invalid flag (-X) to trigger default case:\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s", gcda_file);
    int found_error = execute_and_check_stderr(cmd, "unknown flag");
    
    if (found_error) {
        printf("SUCCESS: Invalid flag triggered the expected error message\n");
    } else {
        printf("WARNING: Invalid flag did not produce expected error message\n");
    }
    
    /* Test 9: Test with gcno file as well */
    printf("\n10. Testing with gcno file (-l flag):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcno_file);
    execute_command(cmd);
    
    /* Cleanup */
    printf("\n=== Cleaning up temporary files ===\n");
    cleanup_files(source_file, binary_file, gcda_file, gcno_file);
    
    printf("\n=== Test completed ===\n");
    printf("All gcov-dump command-line parsing paths should have been exercised.\n");
    
    return EXIT_SUCCESS;
}
