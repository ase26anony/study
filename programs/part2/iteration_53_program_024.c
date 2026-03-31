/* test_gcov_tool_overlap.c - Test harness for gcov-tool overlap command parsing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_PATH 1024

/* Simple test programs to generate varied coverage data */
const char *test_program_a = 
"#include <stdio.h>\n"
"void func1(int x) {\n"
"    if (x > 0) {\n"
"        printf(\"Positive\\n\");\n"
"    } else {\n"
"        printf(\"Non-positive\\n\");\n"
"    }\n"
"}\n"
"void func2() {\n"
"    for (int i = 0; i < 3; i++) {\n"
"        printf(\"Loop %d\\n\", i);\n"
"    }\n"
"}\n"
"int main() {\n"
"    func1(5);\n"
"    func1(-2);\n"
"    func2();\n"
"    return 0;\n"
"}\n";

const char *test_program_b = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"void nested_loops(int n) {\n"
"    int total = 0;\n"
"    for (int i = 0; i < n; i++) {\n"
"        for (int j = 0; j < n; j++) {\n"
"            total += i * j;\n"
"        }\n"
"    }\n"
"    printf(\"Total: %d\\n\", total);\n"
"}\n"
"int main(int argc, char **argv) {\n"
"    int iterations = 5;\n"
"    if (argc > 1) iterations = atoi(argv[1]);\n"
"    \n"
"    for (int run = 0; run < iterations; run++) {\n"
"        nested_loops(run + 2);\n"
"    }\n"
"    return 0;\n"
"}\n";

const char *test_program_c1 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper1() {\n"
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
"void helper2() {\n"
"    printf(\"Helper2 called\\n\");\n"
"}\n";

const char *test_header_c = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"void helper2();\n"
"#endif\n";

const char *test_program_d = 
"#include <stdio.h>\n"
"int main() {\n"
"    /* This program runs but doesn't execute instrumented paths */\n"
"    /* Useful for generating zero-count coverage data */\n"
"    return 0;\n"
"}\n";

/* Function prototypes */
int compile_and_run(const char *source, const char *output, 
                    const char *extra_args, int run_times);
int run_gcov_tool(const char *args, const char *gcda_file, 
                  const char *gcno_file);
void cleanup_files(const char *base_name);
int file_exists(const char *path);

int main(int argc, char **argv) {
    printf("=== Starting gcov-tool overlap command parsing test ===\\n\\n");
    
    /* Check if gcov-tool exists */
    if (system("which gcov-tool > /dev/null 2>&1") != 0) {
        /* Try in current directory */
        if (!file_exists("./gcov-tool")) {
            fprintf(stderr, "Error: gcov-tool not found in PATH or current directory\\n");
            fprintf(stderr, "Please build gcov-tool with: gcc -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\\n");
            return 1;
        }
    }
    
    /* Create test directory */
    mkdir("gcov_test_dir", 0755);
    chdir("gcov_test_dir");
    
    /* Test Scenario A: Simple function with branches */
    printf("--- Test Scenario A: Simple function with branches ---\\n");
    if (!compile_and_run(test_program_a, "test_a", NULL, 1)) {
        fprintf(stderr, "Failed to compile/run test A\\n");
        return 1;
    }
    
    /* Test Scenario B: Loop-heavy program with multiple runs */
    printf("\\n--- Test Scenario B: Loop-heavy program ---\\n");
    if (!compile_and_run(test_program_b, "test_b", NULL, 3)) {
        fprintf(stderr, "Failed to compile/run test B\\n");
        return 1;
    }
    
    /* Test Scenario C: Multiple source files */
    printf("\\n--- Test Scenario C: Multiple source files ---\\n");
    /* Write header file */
    FILE *header = fopen("test_c.h", "w");
    if (!header) {
        perror("Failed to create header file");
        return 1;
    }
    fputs(test_header_c, header);
    fclose(header);
    
    /* Write source files */
    FILE *src1 = fopen("test_c1.c", "w");
    FILE *src2 = fopen("test_c2.c", "w");
    if (!src1 || !src2) {
        perror("Failed to create source files");
        return 1;
    }
    fputs(test_program_c1, src1);
    fputs(test_program_c2, src2);
    fclose(src1);
    fclose(src2);
    
    /* Compile multi-file program */
    char compile_cmd[MAX_PATH];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -O0 -fprofile-arcs -ftest-coverage test_c1.c test_c2.c -o test_c");
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile test C\\n");
        return 1;
    }
    
    /* Run to generate coverage data */
    if (system("./test_c > /dev/null") != 0) {
        fprintf(stderr, "Failed to run test C\\n");
        return 1;
    }
    
    /* Test Scenario D: Zero-count coverage */
    printf("\\n--- Test Scenario D: Zero-count coverage ---\\n");
    if (!compile_and_run(test_program_d, "test_d", NULL, 1)) {
        fprintf(stderr, "Failed to compile/run test D\\n");
        return 1;
    }
    
    /* ============================================================
       NOW TEST GCOV-TOOL OVERLAP WITH VARIOUS FLAGS
       Targeting the uncovered switch cases in gcov-tool.cc
       ============================================================ */
    
    printf("\\n=== Testing gcov-tool overlap with various flags ===\\n");
    
    /* Test 1: -v flag (verbose) - triggers case 'v' */
    printf("\\n1. Testing -v flag (verbose)...\\n");
    if (!run_gcov_tool("-v", "test_a.gcda", "test_a.gcno")) {
        fprintf(stderr, "Failed to run gcov-tool with -v\\n");
    }
    
    /* Test 2: -f flag (function level) - triggers case 'f' */
    printf("\\n2. Testing -f flag (function level)...\\n");
    if (!run_gcov_tool("-f", "test_b.gcda", "test_b.gcno")) {
        fprintf(stderr, "Failed to run gcov-tool with -f\\n");
    }
    
    /* Test 3: -F flag (fullname) - triggers case 'F' */
    printf("\\n3. Testing -F flag (fullname)...\\n");
    if (!run_gcov_tool("-F", "test_a.gcda", "test_a.gcno")) {
        fprintf(stderr, "Failed to run gcov-tool with -F\\n");
    }
    
    /* Test 4: -o flag (object level) - triggers case 'o' */
    printf("\\n4. Testing -o flag (object level)...\\n");
    if (!run_gcov_tool("-o", "test_b.gcda", "test_b.gcno")) {
        fprintf(stderr, "Failed to run gcov-tool with -o\\n");
    }
    
    /* Test 5: -h flag (hot only) - triggers case 'h' */
    printf("\\n5. Testing -h flag (hot only)...\\n");
    if (!run_gcov_tool("-h", "test_a.gcda", "test_a.gcno")) {
        fprintf(stderr, "Failed to run gcov-tool with -h\\n");
    }
    
    /* Test 6: -t flag with threshold - triggers case 't' */
    printf("\\n6. Testing -t flag with threshold 0.5...\\n");
    if (!run_gcov_tool("-t 0.5", "test_b.gcda", "test_b.gcno")) {
        fprintf(stderr, "Failed to run gcov-tool with -t 0.5\\n");
    }
    
    /* Test 7: -t flag with different threshold */
    printf("\\n7. Testing -t flag with threshold 0.75...\\n");
    if (!run_gcov_tool("-t 0.75", "test_a.gcda", "test_a.gcno")) {
        fprintf(stderr, "Failed to run gcov-tool with -t 0.75\\n");
    }
    
    /* Test 8: -t flag with threshold 0.0 */
    printf("\\n8. Testing -t flag with threshold 0.0...\\n");
    if (!run_gcov_tool("-t 0.0", "test_d.gcda", "test_d.gcno")) {
        fprintf(stderr, "Failed to run gcov-tool with -t 0.0\\n");
    }
    
    /* Test 9: Combination of flags */
    printf("\\n9. Testing combination -v -f -o...\\n");
    if (!run_gcov_tool("-v -f -o", "test_a.gcda", "test_a.gcno")) {
        fprintf(stderr, "Failed to run gcov-tool with -v -f -o\\n");
    }
    
    /* Test 10: Another combination */
    printf("\\n10. Testing combination -F -h -t 0.3...\\n");
    if (!run_gcov_tool("-F -h -t 0.3", "test_b.gcda", "test_b.gcno")) {
        fprintf(stderr, "Failed to run gcov-tool with -F -h -t 0.3\\n");
    }
    
    /* Test 11: With multiple input files */
    printf("\\n11. Testing with multiple .gcda files...\\n");
    char multi_cmd[MAX_PATH];
    snprintf(multi_cmd, sizeof(multi_cmd),
             "gcov-tool overlap -v test_a.gcda test_b.gcda test_a.gcno test_b.gcno 2>&1");
    system(multi_cmd);
    
    /* Test 12: With multi-file program coverage data */
    printf("\\n12. Testing with multi-source program...\\n");
    if (!run_gcov_tool("-v -f", "test_c1.gcda", "test_c1.gcno")) {
        fprintf(stderr, "Failed to run gcov-tool with multi-source data\\n");
    }
    
    /* Test 13: DEFAULT CASE - invalid option to trigger overlap_usage() */
    printf("\\n13. Testing invalid option -z (should trigger default case and usage)...\\n");
    char invalid_cmd[MAX_PATH];
    snprintf(invalid_cmd, sizeof(invalid_cmd),
             "gcov-tool overlap -z test_a.gcda test_a.gcno 2>&1");
    system(invalid_cmd);
    
    /* Test 14: Another invalid option combination */
    printf("\\n14. Testing invalid option -x (another default case trigger)...\\n");
    snprintf(invalid_cmd, sizeof(invalid_cmd),
             "gcov-tool overlap -x -y test_b.gcda test_b.gcno 2>&1");
    system(invalid_cmd);
    
    /* Test 15: Missing required argument for -t */
    printf("\\n15. Testing -t without argument (may trigger error or default)...\\n");
    snprintf(invalid_cmd, sizeof(invalid_cmd),
             "gcov-tool overlap -t test_a.gcda test_a.gcno 2>&1");
    system(invalid_cmd);
    
    /* Cleanup */
    printf("\\n=== Cleaning up test files ===\\n");
    chdir("..");
    system("rm -rf gcov_test_dir");
    
    printf("\\n=== Test completed ===\\n");
    printf("The gcov-tool overlap command should have been invoked with:\\n");
    printf("  - -v flag (case 'v')\\n");
    printf("  - -f flag (case 'f')\\n");
    printf("  - -F flag (case 'F')\\n");
    printf("  - -o flag (case 'o')\\n");
    printf("  - -h flag (case 'h')\\n");
    printf("  - -t flag with various thresholds (case 't')\\n");
    printf("  - Invalid options to trigger default case\\n");
    printf("\\nAll target lines in gcov-tool.cc (534-554) should now be covered.\\n");
    
    return 0;
}

/* Compile and run a test program to generate coverage data */
int compile_and_run(const char *source, const char *output, 
                    const char *extra_args, int run_times) {
    char filename[MAX_PATH];
    char compile_cmd[MAX_PATH];
    char run_cmd[MAX_PATH];
    
    /* Create source file */
    snprintf(filename, sizeof(filename), "%s.c", output);
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create source file");
        return 0;
    }
    fputs(source, fp);
    fclose(fp);
    
    /* Compile with coverage flags */
    if (extra_args) {
        snprintf(compile_cmd, sizeof(compile_cmd),
                 "gcc -O0 -fprofile-arcs -ftest-coverage %s.c -o %s %s",
                 output, output, extra_args);
    } else {
        snprintf(compile_cmd, sizeof(compile_cmd),
                 "gcc -O0 -fprofile-arcs -ftest-coverage %s.c -o %s",
                 output, output);
    }
    
    printf("Compiling %s...\\n", output);
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Compilation failed for %s\\n", output);
        return 0;
    }
    
    /* Run the program multiple times to generate coverage data */
    printf("Running %s %d time(s)...\\n", output, run_times);
    for (int i = 0; i < run_times; i++) {
        if (strcmp(output, "test_b") == 0) {
            /* For test_b, run with different arguments */
            snprintf(run_cmd, sizeof(run_cmd), "./%s %d > /dev/null", output, i + 2);
        } else {
            snprintf(run_cmd, sizeof(run_cmd), "./%s > /dev/null", output);
        }
        
        if (system(run_cmd) != 0) {
            fprintf(stderr, "Execution failed for %s\\n", output);
            return 0;
        }
    }
    
    /* Verify coverage files were created */
    snprintf(filename, sizeof(filename), "%s.gcda", output);
    if (!file_exists(filename)) {
        fprintf(stderr, "Coverage data file not created: %s\\n", filename);
        return 0;
    }
    
    snprintf(filename, sizeof(filename), "%s.gcno", output);
    if (!file_exists(filename)) {
        fprintf(stderr, "Coverage note file not created: %s\\n", filename);
        return 0;
    }
    
    return 1;
}

/* Run gcov-tool overlap with specified arguments */
int run_gcov_tool(const char *args, const char *gcda_file, 
                  const char *gcno_file) {
    char cmd[MAX_PATH * 2];
    
    /* Construct the command */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s %s %s 2>&1", 
             args, gcda_file, gcno_file);
    
    /* Execute and capture exit status */
    int status = system(cmd);
    
    /* Check if command was executed (not if gcov-tool succeeded) */
    if (status == -1) {
        perror("system() failed");
        return 0;
    }
    
    return 1;
}

/* Check if a file exists */
int file_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0);
}
