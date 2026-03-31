/**
 * test_gcov_tool_overlap.c
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

/* Simple test programs to generate coverage data */
const char *test_program_a = 
"#include <stdio.h>\n"
"void func1(int x) {\n"
"    if (x > 0) printf(\"Positive\\n\");\n"
"    else printf(\"Non-positive\\n\");\n"
"}\n"
"int main() {\n"
"    func1(5);\n"
"    func1(-3);\n"
"    return 0;\n"
"}\n";

const char *test_program_b = 
"#include <stdio.h>\n"
"int main(int argc, char **argv) {\n"
"    int i, j, iterations = 3;\n"
"    if (argc > 1) iterations = atoi(argv[1]);\n"
"    for (i = 0; i < iterations; i++) {\n"
"        for (j = 0; j < i+1; j++) {\n"
"            printf(\"Loop: i=%d, j=%d\\n\", i, j);\n"
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
"int main() {\n"
"    /* This program runs but doesn't hit instrumented code paths */\n"
"    return 0;\n"
"}\n";

/* Function prototypes */
int compile_and_run(const char *source, const char *output_name, 
                    const char *extra_args);
int run_gcov_tool(const char *gcda_file, const char *gcno_file, 
                  const char *flags);
void cleanup_files(void);

/* Global list of files to clean up */
char *files_to_clean[MAX_PATH];
int file_count = 0;

void add_file_to_cleanup(const char *filename) {
    if (file_count < MAX_PATH) {
        files_to_clean[file_count] = strdup(filename);
        file_count++;
    }
}

int main(int argc, char **argv) {
    printf("=== Test harness for gcov-tool overlap argument parsing ===\n\n");
    
    /* Check if gcov-tool exists */
    if (system("which gcov-tool > /dev/null 2>&1") != 0) {
        fprintf(stderr, "Error: gcov-tool not found in PATH\n");
        fprintf(stderr, "Please ensure gcov-tool is built and in your PATH\n");
        return 1;
    }
    
    /* Create test directory */
    system("mkdir -p test_coverage_data");
    chdir("test_coverage_data");
    
    /* Generate coverage data from multiple scenarios */
    printf("1. Generating coverage data from different scenarios...\n");
    
    /* Scenario A: Simple function with branches */
    printf("  Scenario A: Simple function with branches\n");
    if (!compile_and_run(test_program_a, "test_a", NULL)) {
        fprintf(stderr, "Failed to compile/run test A\n");
        return 1;
    }
    
    /* Scenario B: Loop-heavy program */
    printf("  Scenario B: Loop-heavy program\n");
    if (!compile_and_run(test_program_b, "test_b", "5")) {
        fprintf(stderr, "Failed to compile/run test B\n");
        return 1;
    }
    
    /* Scenario C: Multiple source files */
    printf("  Scenario C: Multiple source files\n");
    FILE *fp = fopen("test_c.h", "w");
    if (fp) {
        fputs(test_header_c, fp);
        fclose(fp);
        add_file_to_cleanup("test_c.h");
    }
    
    fp = fopen("test_c1.c", "w");
    if (fp) {
        fputs(test_program_c1, fp);
        fclose(fp);
        add_file_to_cleanup("test_c1.c");
    }
    
    fp = fopen("test_c2.c", "w");
    if (fp) {
        fputs(test_program_c2, fp);
        fclose(fp);
        add_file_to_cleanup("test_c2.c");
    }
    
    /* Compile multi-file program */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage test_c1.c test_c2.c -o test_c");
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile test C\n");
        return 1;
    }
    add_file_to_cleanup("test_c");
    add_file_to_cleanup("test_c.gcno");
    
    /* Run it */
    if (system("./test_c > /dev/null 2>&1") != 0) {
        fprintf(stderr, "Failed to run test C\n");
        return 1;
    }
    add_file_to_cleanup("test_c1.gcda");
    add_file_to_cleanup("test_c2.gcda");
    
    /* Scenario D: Program with zero counts */
    printf("  Scenario D: Program with zero counts\n");
    if (!compile_and_run(test_program_d, "test_d", NULL)) {
        fprintf(stderr, "Failed to compile/run test D\n");
        return 1;
    }
    
    printf("\n2. Testing gcov-tool overlap with various flags...\n\n");
    
    /* Test case 1: -v flag (verbose) - triggers case 'v' */
    printf("Test 1: Testing -v flag (verbose)\n");
    run_gcov_tool("test_a.gcda", "test_a.gcno", "-v");
    
    /* Test case 2: -f flag (function level) - triggers case 'f' */
    printf("\nTest 2: Testing -f flag (function level)\n");
    run_gcov_tool("test_a.gcda", "test_a.gcno", "-f");
    
    /* Test case 3: -F flag (fullname) - triggers case 'F' */
    printf("\nTest 3: Testing -F flag (fullname)\n");
    run_gcov_tool("test_a.gcda", "test_a.gcno", "-F");
    
    /* Test case 4: -o flag (object level) - triggers case 'o' */
    printf("\nTest 4: Testing -o flag (object level)\n");
    run_gcov_tool("test_a.gcda", "test_a.gcno", "-o");
    
    /* Test case 5: -h flag (hot only) - triggers case 'h' */
    printf("\nTest 5: Testing -h flag (hot only)\n");
    run_gcov_tool("test_b.gcda", "test_b.gcno", "-h");
    
    /* Test case 6: -t flag with threshold - triggers case 't' */
    printf("\nTest 6: Testing -t flag with threshold 0.5\n");
    run_gcov_tool("test_b.gcda", "test_b.gcno", "-t 0.5");
    
    /* Test case 7: -t flag with different threshold */
    printf("\nTest 7: Testing -t flag with threshold 0.75\n");
    run_gcov_tool("test_b.gcda", "test_b.gcno", "-t 0.75");
    
    /* Test case 8: Combination of flags */
    printf("\nTest 8: Testing combination -v -f -o\n");
    run_gcov_tool("test_a.gcda", "test_a.gcno", "-v -f -o");
    
    /* Test case 9: Another combination */
    printf("\nTest 9: Testing combination -F -h -t 0.3\n");
    run_gcov_tool("test_b.gcda", "test_b.gcno", "-F -h -t 0.3");
    
    /* Test case 10: With multiple input files */
    printf("\nTest 10: Testing with multiple .gcda files\n");
    char multi_cmd[2048];
    snprintf(multi_cmd, sizeof(multi_cmd),
             "gcov-tool overlap -v test_c1.gcda test_c2.gcda test_c.gcno 2>&1 | head -20");
    system(multi_cmd);
    
    /* Test case 11: Invalid option to trigger default case */
    printf("\nTest 11: Testing invalid option -z (triggers default case)\n");
    system("gcov-tool overlap -z 2>&1 | head -5");
    
    /* Test case 12: Another invalid option */
    printf("\nTest 12: Testing invalid option --invalid (triggers default case)\n");
    system("gcov-tool overlap --invalid 2>&1 | head -5");
    
    /* Test case 13: Valid flags with zero-count data */
    printf("\nTest 13: Testing with zero-count coverage data\n");
    run_gcov_tool("test_d.gcda", "test_d.gcno", "-v -h -t 0.1");
    
    /* Test case 14: All flags together */
    printf("\nTest 14: Testing all valid flags together\n");
    run_gcov_tool("test_b.gcda", "test_b.gcno", "-v -f -F -o -h -t 0.25");
    
    printf("\n3. Cleaning up...\n");
    cleanup_files();
    chdir("..");
    system("rmdir test_coverage_data 2>/dev/null");
    
    printf("\n=== Test completed ===\n");
    printf("The gcov-tool overlap command has been invoked with various flag combinations.\n");
    printf("Each flag combination should have triggered the corresponding case in the switch statement.\n");
    
    return 0;
}

int compile_and_run(const char *source, const char *output_name, 
                    const char *extra_args) {
    char source_file[256];
    char exe_file[256];
    char gcda_file[256];
    char gcno_file[256];
    char cmd[1024];
    
    /* Create source file */
    snprintf(source_file, sizeof(source_file), "%s.c", output_name);
    FILE *fp = fopen(source_file, "w");
    if (!fp) {
        perror("Failed to create source file");
        return 0;
    }
    fputs(source, fp);
    fclose(fp);
    add_file_to_cleanup(source_file);
    
    /* Compile with coverage flags */
    snprintf(exe_file, sizeof(exe_file), "%s", output_name);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, exe_file);
    if (system(cmd) != 0) {
        fprintf(stderr, "Compilation failed: %s\n", cmd);
        return 0;
    }
    add_file_to_cleanup(exe_file);
    
    /* Record .gcno file */
    snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", source_file);
    add_file_to_cleanup(gcno_file);
    
    /* Run the program */
    snprintf(cmd, sizeof(cmd), "./%s", exe_file);
    if (extra_args) {
        strcat(cmd, " ");
        strcat(cmd, extra_args);
    }
    strcat(cmd, " > /dev/null 2>&1");
    if (system(cmd) != 0) {
        fprintf(stderr, "Execution failed: %s\n", cmd);
        return 0;
    }
    
    /* Record .gcda file */
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", source_file);
    add_file_to_cleanup(gcda_file);
    
    return 1;
}

int run_gcov_tool(const char *gcda_file, const char *gcno_file, 
                  const char *flags) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap %s %s %s 2>&1 | head -10",
             flags, gcda_file, gcno_file);
    return system(cmd);
}

void cleanup_files(void) {
    int i;
    for (i = 0; i < file_count; i++) {
        if (files_to_clean[i]) {
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "rm -f %s", files_to_clean[i]);
            system(cmd);
            free(files_to_clean[i]);
        }
    }
    
    /* Also clean up any other generated files */
    system("rm -f *.gcda *.gcno *.c *.h test_* 2>/dev/null");
}
