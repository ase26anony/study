#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define TEMP_SOURCE_FILE "test_coverage.c"
#define TEMP_BINARY "test_coverage"
#define GCOV_DATA_FILE "test_coverage.gcda"

/* Create a simple C source file with coverage instrumentation */
void create_test_source(void) {
    FILE *fp = fopen(TEMP_SOURCE_FILE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        exit(EXIT_FAILURE);
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    int i, sum = 0;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        sum += i;\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    printf(\"Sum: %%d\\n\", sum);\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
}

/* Compile the test program with coverage flags */
void compile_with_coverage(void) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             TEMP_SOURCE_FILE, TEMP_BINARY);
    
    printf("Compiling: %s\n", cmd);
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed\n");
        exit(EXIT_FAILURE);
    }
}

/* Execute the test program to generate .gcda file */
void generate_gcda(void) {
    printf("Executing test program to generate .gcda file\n");
    int ret = system("./" TEMP_BINARY);
    if (ret != 0) {
        fprintf(stderr, "Execution failed\n");
        exit(EXIT_FAILURE);
    }
    
    /* Verify .gcda file was created */
    struct stat st;
    if (stat(GCOV_DATA_FILE, &st) != 0) {
        fprintf(stderr, "Failed to generate .gcda file\n");
        exit(EXIT_FAILURE);
    }
    printf("Generated %s (size: %ld bytes)\n", GCOV_DATA_FILE, st.st_size);
}

/* Execute gcov-dump with a specific flag and capture output if needed */
void run_gcov_dump(const char *flag, int capture_stderr) {
    char cmd[512];
    
    if (strcmp(flag, "-h") == 0 || strcmp(flag, "-v") == 0) {
        /* -h and -v don't need a file argument */
        snprintf(cmd, sizeof(cmd), "gcov-dump %s", flag);
    } else if (strcmp(flag, "-X") == 0) {
        /* Invalid flag - needs a file to trigger the error */
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", flag, GCOV_DATA_FILE);
    } else {
        /* Regular flags need the .gcda file */
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s", flag, GCOV_DATA_FILE);
    }
    
    printf("\nRunning: %s\n", cmd);
    
    if (capture_stderr) {
        /* Use popen to capture stderr for invalid flag check */
        FILE *fp = popen(cmd, "r");
        if (fp) {
            char buffer[256];
            int found_error = 0;
            
            while (fgets(buffer, sizeof(buffer), fp) != NULL) {
                printf("Output: %s", buffer);
                if (strstr(buffer, "unknown flag")) {
                    found_error = 1;
                }
            }
            
            pclose(fp);
            
            if (found_error) {
                printf("SUCCESS: Triggered 'unknown flag' error message\n");
            }
        }
    } else {
        /* Just execute normally */
        int ret = system(cmd);
        if (ret != 0 && strcmp(flag, "-X") != 0) {
            /* Don't fail for invalid flag - that's expected */
            fprintf(stderr, "Command failed with return code: %d\n", ret);
        }
    }
}

/* Execute gcov-dump with multiple flags */
void run_gcov_dump_multiple(const char *flags) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "gcov-dump %s %s", flags, GCOV_DATA_FILE);
    
    printf("\nRunning: %s\n", cmd);
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Command failed with return code: %d\n", ret);
    }
}

/* Clean up temporary files */
void cleanup(void) {
    remove(TEMP_SOURCE_FILE);
    remove(TEMP_BINARY);
    remove(GCOV_DATA_FILE);
    remove("test_coverage.gcno");  /* Also remove the .gcno file */
    printf("\nCleaned up temporary files\n");
}

int main(void) {
    printf("=== Starting gcov-dump test driver ===\n");
    
    /* Step 1: Create test source file */
    printf("\n1. Creating test source file...\n");
    create_test_source();
    
    /* Step 2: Compile with coverage */
    printf("\n2. Compiling with coverage flags...\n");
    compile_with_coverage();
    
    /* Step 3: Generate .gcda file */
    printf("\n3. Generating coverage data...\n");
    generate_gcda();
    
    /* Step 4: Test various gcov-dump flags */
    printf("\n4. Testing gcov-dump flags...\n");
    
    /* Test -h flag (help) - triggers print_usage() */
    run_gcov_dump("-h", 0);
    
    /* Test -v flag (version) - triggers print_version() */
    run_gcov_dump("-v", 0);
    
    /* Test -l flag (dump contents) - sets flag_dump_contents = 1 */
    run_gcov_dump("-l", 0);
    
    /* Test -p flag (dump positions) - sets flag_dump_positions = 1 */
    run_gcov_dump("-p", 0);
    
    /* Test -r flag (dump raw) - sets flag_dump_raw = 1 */
    run_gcov_dump("-r", 0);
    
    /* Test -s flag (dump stable) - sets flag_dump_stable = 1 */
    run_gcov_dump("-s", 0);
    
    /* Test combined flags */
    run_gcov_dump_multiple("-l -p");
    run_gcov_dump_multiple("-r -s");
    run_gcov_dump_multiple("-l -p -r -s");
    
    /* Test invalid flag - triggers default case and fprintf */
    printf("\n5. Testing invalid flag (should trigger 'unknown flag' error)...\n");
    run_gcov_dump("-X", 1);  /* Capture stderr to verify error message */
    
    /* Test another invalid flag */
    run_gcov_dump("-z", 1);
    
    /* Cleanup */
    printf("\n6. Cleaning up...\n");
    cleanup();
    
    printf("\n=== Test completed ===\n");
    return 0;
}
