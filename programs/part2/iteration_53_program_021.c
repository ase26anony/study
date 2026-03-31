/**
 * gcov_tool_overlap_test.c
 * 
 * Test harness to trigger uncovered lines in gcov-tool.cc (lines 534-554)
 * Specifically tests the 'overlap' subcommand argument parsing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH 1024
#define MAX_CMD 4096

/* Simple test programs to generate coverage data */
const char *test_program_a = 
"#include <stdio.h>\n"
"void func1(int x) {\n"
"    if (x > 0) printf(\"Positive\\n\");\n"
"    else printf(\"Non-positive\\n\");\n"
"}\n"
"int main() {\n"
"    func1(1);\n"
"    func1(-1);\n"
"    return 0;\n"
"}\n";

const char *test_program_b = 
"#include <stdio.h>\n"
"int main(int argc, char **argv) {\n"
"    int i, j, iterations = 2;\n"
"    if (argc > 1) iterations = atoi(argv[1]);\n"
"    for (i = 0; i < iterations; i++) {\n"
"        for (j = 0; j < 3; j++) {\n"
"            printf(\"Loop %d-%d\\n\", i, j);\n"
"        }\n"
"    }\n"
"    return 0;\n"
"}\n";

const char *test_program_c1 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper1(void) {\n"
"    printf(\"Helper1 called\\n\");\n"
"}\n"
"int main() {\n"
"    helper1();\n"
"    helper2();\n"
"    return 0;\n"
"}\n";

const char *test_program_c2 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper2(void) {\n"
"    printf(\"Helper2 called\\n\");\n"
"}\n";

const char *test_header_c = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"void helper2(void);\n"
"#endif\n";

const char *test_program_d = 
"#include <stdio.h>\n"
"int main() {\n"
"    /* This program runs but doesn't hit instrumented code */\n"
"    /* (if we don't call any functions with coverage) */\n"
"    return 0;\n"
"}\n";

/* Function prototypes */
int compile_with_coverage(const char *source, const char *output, 
                         const char **extra_files, int extra_count);
int run_program(const char *program, const char *args);
int run_gcov_tool_overlap(const char *gcda_file, const char *gcno_file, 
                         const char *flags);
void cleanup_files(const char **files, int count);

int main(int argc, char **argv) {
    char cmd[MAX_CMD];
    char gcda_file[MAX_PATH];
    char gcno_file[MAX_PATH];
    int status;
    int i;
    
    printf("=== Starting gcov-tool overlap argument parsing test ===\n");
    
    /* Check if gcov-tool exists */
    status = system("which gcov-tool > /dev/null 2>&1");
    if (status != 0) {
        /* Try in current directory */
        if (access("./gcov-tool", X_OK) != 0) {
            fprintf(stderr, "Error: gcov-tool not found in PATH or current directory\n");
            fprintf(stderr, "Please build gcov-tool with coverage first:\n");
            fprintf(stderr, "  gcc -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
            return 1;
        }
    }
    
    /* Test Scenario A: Simple function with branches */
    printf("\n--- Scenario A: Simple function with branches ---\n");
    if (compile_with_coverage(test_program_a, "test_a", NULL, 0) != 0) {
        fprintf(stderr, "Failed to compile test A\n");
        return 1;
    }
    
    run_program("./test_a", NULL);
    
    /* Get the gcda and gcno files */
    snprintf(gcda_file, MAX_PATH, "test_a.gcda");
    snprintf(gcno_file, MAX_PATH, "test_a.gcno");
    
    /* Test each flag from the uncovered block */
    printf("\nTesting -v flag (verbose)...\n");
    run_gcov_tool_overlap(gcda_file, gcno_file, "-v");
    
    printf("\nTesting -f flag (function level)...\n");
    run_gcov_tool_overlap(gcda_file, gcno_file, "-f");
    
    printf("\nTesting -F flag (fullname)...\n");
    run_gcov_tool_overlap(gcda_file, gcno_file, "-F");
    
    printf("\nTesting -o flag (object level)...\n");
    run_gcov_tool_overlap(gcda_file, gcno_file, "-o");
    
    printf("\nTesting -h flag (hot only)...\n");
    run_gcov_tool_overlap(gcda_file, gcno_file, "-h");
    
    printf("\nTesting -t flag with threshold 0.5...\n");
    run_gcov_tool_overlap(gcda_file, gcno_file, "-t 0.5");
    
    printf("\nTesting -t flag with threshold 0.75...\n");
    run_gcov_tool_overlap(gcda_file, gcno_file, "-t 0.75");
    
    printf("\nTesting -t flag with threshold 1.0...\n");
    run_gcov_tool_overlap(gcda_file, gcno_file, "-t 1.0");
    
    /* Test combination of flags */
    printf("\nTesting combination -v -f -o...\n");
    run_gcov_tool_overlap(gcda_file, gcno_file, "-v -f -o");
    
    printf("\nTesting combination -F -h -t 0.8...\n");
    run_gcov_tool_overlap(gcda_file, gcno_file, "-F -h -t 0.8");
    
    /* Test Scenario B: Loop heavy program */
    printf("\n--- Scenario B: Loop heavy program ---\n");
    if (compile_with_coverage(test_program_b, "test_b", NULL, 0) != 0) {
        fprintf(stderr, "Failed to compile test B\n");
        return 1;
    }
    
    /* Run multiple times with different arguments */
    run_program("./test_b", "1");
    run_program("./test_b", "3");
    run_program("./test_b", "5");
    
    snprintf(gcda_file, MAX_PATH, "test_b.gcda");
    snprintf(gcno_file, MAX_PATH, "test_b.gcno");
    
    printf("\nTesting -v -t 0.3 with loop program...\n");
    run_gcov_tool_overlap(gcda_file, gcno_file, "-v -t 0.3");
    
    printf("\nTesting -f -F -o with loop program...\n");
    run_gcov_tool_overlap(gcda_file, gcno_file, "-f -F -o");
    
    /* Test Scenario C: Multiple source files */
    printf("\n--- Scenario C: Multiple source files ---\n");
    /* Write header and source files */
    FILE *fp = fopen("test_c.h", "w");
    if (fp) {
        fputs(test_header_c, fp);
        fclose(fp);
    }
    
    fp = fopen("test_c1.c", "w");
    if (fp) {
        fputs(test_program_c1, fp);
        fclose(fp);
    }
    
    fp = fopen("test_c2.c", "w");
    if (fp) {
        fputs(test_program_c2, fp);
        fclose(fp);
    }
    
    /* Compile with multiple source files */
    const char *c_files[] = {"test_c1.c", "test_c2.c"};
    if (compile_with_coverage(NULL, "test_c", c_files, 2) != 0) {
        fprintf(stderr, "Failed to compile test C\n");
        return 1;
    }
    
    run_program("./test_c", NULL);
    
    printf("\nTesting with multiple gcda files...\n");
    snprintf(cmd, MAX_CMD, "gcov-tool overlap -v test_c1.gcda test_c2.gcda test_c1.gcno test_c2.gcno 2>&1");
    system(cmd);
    
    printf("\nTesting -f -o with multiple files...\n");
    snprintf(cmd, MAX_CMD, "gcov-tool overlap -f -o test_c1.gcda test_c2.gcda test_c1.gcno test_c2.gcno 2>&1");
    system(cmd);
    
    /* Test Scenario D: Empty/zero counts */
    printf("\n--- Scenario D: Empty/zero counts ---\n");
    if (compile_with_coverage(test_program_d, "test_d", NULL, 0) != 0) {
        fprintf(stderr, "Failed to compile test D\n");
        return 1;
    }
    
    /* Run but don't generate meaningful counts */
    run_program("./test_d", NULL);
    
    snprintf(gcda_file, MAX_PATH, "test_d.gcda");
    snprintf(gcno_file, MAX_PATH, "test_d.gcno");
    
    printf("\nTesting -h -t 0.1 with zero counts...\n");
    run_gcov_tool_overlap(gcda_file, gcno_file, "-h -t 0.1");
    
    /* Test invalid option to trigger default case and overlap_usage() */
    printf("\n--- Testing invalid option (to trigger default case) ---\n");
    printf("This should show usage information:\n");
    snprintf(cmd, MAX_CMD, "gcov-tool overlap -z 2>&1 | head -20");
    system(cmd);
    
    /* Also test with valid files but invalid option */
    snprintf(cmd, MAX_CMD, "gcov-tool overlap -v -z test_a.gcda test_a.gcno 2>&1 | head -20");
    system(cmd);
    
    /* Test with no arguments to trigger usage */
    printf("\n--- Testing no arguments ---\n");
    system("gcov-tool overlap 2>&1 | head -20");
    
    /* Cleanup */
    printf("\n=== Cleaning up generated files ===\n");
    const char *cleanup_files[] = {
        "test_a", "test_a.gcda", "test_a.gcno", "test_a.c",
        "test_b", "test_b.gcda", "test_b.gcno", "test_b.c",
        "test_c", "test_c.h", "test_c1.c", "test_c2.c",
        "test_c1.gcda", "test_c1.gcno", "test_c2.gcda", "test_c2.gcno",
        "test_d", "test_d.gcda", "test_d.gcno", "test_d.c"
    };
    
    for (i = 0; i < sizeof(cleanup_files)/sizeof(cleanup_files[0]); i++) {
        if (access(cleanup_files[i], F_OK) == 0) {
            remove(cleanup_files[i]);
        }
    }
    
    printf("\n=== Test completed ===\n");
    printf("The gcov-tool overlap command has been invoked with all flags:\n");
    printf("  -v (verbose)          - Triggered case 'v'\n");
    printf("  -f (func level)       - Triggered case 'f'\n");
    printf("  -F (fullname)         - Triggered case 'F'\n");
    printf("  -o (object level)     - Triggered case 'o'\n");
    printf("  -h (hot only)         - Triggered case 'h'\n");
    printf("  -t (threshold)        - Triggered case 't' with various values\n");
    printf("  -z (invalid)          - Triggered default case and overlap_usage()\n");
    
    return 0;
}

int compile_with_coverage(const char *source, const char *output, 
                         const char **extra_files, int extra_count) {
    char cmd[MAX_CMD];
    FILE *fp;
    
    /* If source is provided directly as string, write to file */
    if (source != NULL) {
        char source_file[MAX_PATH];
        snprintf(source_file, MAX_PATH, "%s.c", output);
        
        fp = fopen(source_file, "w");
        if (!fp) {
            perror("Failed to create source file");
            return -1;
        }
        fputs(source, fp);
        fclose(fp);
        
        /* Compile single file */
        snprintf(cmd, MAX_CMD, "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s 2>&1", 
                source_file, output);
    } else if (extra_files != NULL && extra_count > 0) {
        /* Compile multiple files */
        snprintf(cmd, MAX_CMD, "gcc -O0 -fprofile-arcs -ftest-coverage ");
        int i;
        for (i = 0; i < extra_count; i++) {
            strncat(cmd, extra_files[i], MAX_CMD - strlen(cmd) - 1);
            strncat(cmd, " ", MAX_CMD - strlen(cmd) - 1);
        }
        strncat(cmd, "-o ", MAX_CMD - strlen(cmd) - 1);
        strncat(cmd, output, MAX_CMD - strlen(cmd) - 1);
        strncat(cmd, " 2>&1", MAX_CMD - strlen(cmd) - 1);
    } else {
        fprintf(stderr, "No source files specified\n");
        return -1;
    }
    
    printf("Compiling: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Compilation failed\n");
        return -1;
    }
    
    return 0;
}

int run_program(const char *program, const char *args) {
    char cmd[MAX_CMD];
    
    if (args) {
        snprintf(cmd, MAX_CMD, "%s %s > /dev/null 2>&1", program, args);
    } else {
        snprintf(cmd, MAX_CMD, "%s > /dev/null 2>&1", program);
    }
    
    printf("Running: %s\n", cmd);
    return system(cmd);
}

int run_gcov_tool_overlap(const char *gcda_file, const char *gcno_file, 
                         const char *flags) {
    char cmd[MAX_CMD];
    
    snprintf(cmd, MAX_CMD, "gcov-tool overlap %s %s %s 2>&1", 
            flags, gcda_file, gcno_file);
    
    /* Execute and capture first few lines of output */
    printf("Executing: %s\n", cmd);
    
    /* Use popen to read output */
    FILE *fp = popen(cmd, "r");
    if (fp) {
        char buffer[256];
        int lines = 0;
        while (fgets(buffer, sizeof(buffer), fp) != NULL && lines < 3) {
            printf("  Output: %s", buffer);
            lines++;
        }
        pclose(fp);
    }
    
    return 0;
}
