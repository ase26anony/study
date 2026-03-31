#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define TEMP_SOURCE "test_coverage.c"
#define TEMP_BINARY "test_coverage"
#define GCOV_DATA "test_coverage.gcda"

/* Create a simple C program that will generate coverage data */
void create_test_source(void) {
    FILE *fp = fopen(TEMP_SOURCE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        exit(1);
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i, sum = 0;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        sum += i;\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    printf(\"Sum: %%d\\n\", sum);\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
}

/* Compile the test program with coverage instrumentation */
void compile_with_coverage(void) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s 2>/dev/null",
             TEMP_SOURCE, TEMP_BINARY);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile test program with coverage\n");
        exit(1);
    }
}

/* Execute the test program to generate .gcda file */
void generate_gcda(void) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "./%s > /dev/null", TEMP_BINARY);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to execute test program\n");
        exit(1);
    }
    
    /* Verify .gcda file was created */
    struct stat st;
    if (stat(GCOV_DATA, &st) != 0) {
        fprintf(stderr, "Failed to generate .gcda file\n");
        exit(1);
    }
}

/* Execute gcov-dump with a specific flag and capture output if needed */
void run_gcov_dump(const char *flags, int capture_stderr) {
    char cmd[512];
    char buffer[1024];
    FILE *fp;
    
    if (capture_stderr) {
        /* For invalid flag, capture stderr to verify error message */
        snprintf(cmd, sizeof(cmd), "gcov-dump %s 2>&1", flags);
        fp = popen(cmd, "r");
        if (!fp) {
            perror("popen failed");
            return;
        }
        
        int found_error = 0;
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            if (strstr(buffer, "unknown flag")) {
                printf("SUCCESS: Triggered default case with invalid flag\n");
                found_error = 1;
            }
        }
        
        if (!found_error) {
            printf("WARNING: Did not see 'unknown flag' message\n");
        }
        
        pclose(fp);
    } else {
        /* For valid flags, just execute */
        snprintf(cmd, sizeof(cmd), "gcov-dump %s > /dev/null 2>&1", flags);
        system(cmd);
    }
}

/* Clean up temporary files */
void cleanup(void) {
    char *files_to_remove[] = {
        TEMP_SOURCE,
        TEMP_BINARY,
        GCOV_DATA,
        "test_coverage.gcno",
        NULL
    };
    
    for (int i = 0; files_to_remove[i] != NULL; i++) {
        unlink(files_to_remove[i]);
    }
}

int main(void) {
    printf("=== Generating coverage data file ===\n");
    
    /* Step 1: Create test source file */
    create_test_source();
    
    /* Step 2: Compile with coverage */
    compile_with_coverage();
    
    /* Step 3: Execute to generate .gcda */
    generate_gcda();
    
    printf("=== Testing gcov-dump flags ===\n");
    
    /* Test help flag (-h) - triggers print_usage() */
    printf("Testing -h flag (help)...\n");
    run_gcov_dump("-h", 0);
    
    /* Test version flag (-v) - triggers print_version() */
    printf("Testing -v flag (version)...\n");
    run_gcov_dump("-v", 0);
    
    /* Test dump flags with valid .gcda file */
    printf("Testing -l flag (dump contents)...\n");
    run_gcov_dump("-l " GCOV_DATA, 0);
    
    printf("Testing -p flag (dump positions)...\n");
    run_gcov_dump("-p " GCOV_DATA, 0);
    
    printf("Testing -r flag (dump raw)...\n");
    run_gcov_dump("-r " GCOV_DATA, 0);
    
    printf("Testing -s flag (dump stable)...\n");
    run_gcov_dump("-s " GCOV_DATA, 0);
    
    /* Test combination of flags */
    printf("Testing -l -p flags (combined)...\n");
    run_gcov_dump("-l -p " GCOV_DATA, 0);
    
    /* Test invalid flag to trigger default case */
    printf("Testing invalid flag (-X)...\n");
    run_gcov_dump("-X " GCOV_DATA, 1);
    
    /* Test invalid flag without file argument */
    printf("Testing invalid flag alone (-Z)...\n");
    run_gcov_dump("-Z", 1);
    
    /* Clean up */
    cleanup();
    
    printf("=== All tests completed ===\n");
    return 0;
}
