/* test_gcov_tool_overlap.c - Test harness for gcov-tool overlap command-line parsing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Simple test programs to generate coverage data */
const char *test_program_a = 
"#include <stdio.h>\n"
"void func1(int x) {\n"
"    if (x > 0) printf(\"Positive\\n\");\n"
"    else printf(\"Non-positive\\n\");\n"
"}\n"
"void func2(int y) {\n"
"    for (int i = 0; i < y; i++) {\n"
"        printf(\"Iteration %d\\n\", i);\n"
"    }\n"
"}\n"
"int main() {\n"
"    func1(5);\n"
"    func1(-3);\n"
"    func2(3);\n"
"    return 0;\n"
"}\n";

const char *test_program_b = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"int process_value(int val) {\n"
"    int result = 0;\n"
"    for (int i = 0; i < val; i++) {\n"
"        for (int j = 0; j < i; j++) {\n"
"            result += i * j;\n"
"        }\n"
"    }\n"
"    return result;\n"
"}\n"
"int main(int argc, char **argv) {\n"
"    int iterations = 5;\n"
"    if (argc > 1) iterations = atoi(argv[1]);\n"
"    \n"
"    int total = 0;\n"
"    for (int k = 0; k < iterations; k++) {\n"
"        total += process_value(k + 2);\n"
"    }\n"
"    printf(\"Total: %d\\n\", total);\n"
"    return 0;\n"
"}\n";

const char *test_program_c1 = 
"#include <stdio.h>\n"
"#include \"test_header.h\"\n"
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
"#include \"test_header.h\"\n"
"void helper2(void) {\n"
"    printf(\"Helper2 called\\n\");\n"
"}\n";

const char *test_header = 
"#ifndef TEST_HEADER_H\n"
"#define TEST_HEADER_H\n"
"void helper1(void);\n"
"void helper2(void);\n"
"#endif\n";

const char *test_program_d = 
"#include <stdio.h>\n"
"int never_called(void) {\n"
"    return 42;\n"
"}\n"
"int main() {\n"
"    /* This main function doesn't call any instrumented functions */\n"
"    printf(\"No coverage generated\\n\");\n"
"    return 0;\n"
"}\n";

/* Execute a shell command and check return status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed with status %d: %s\n", status, cmd);
    }
    return status;
}

/* Compile a test program with coverage flags */
int compile_with_coverage(const char *source, const char *output, const char **extra_args, int extra_count) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "gcc -O0 -fprofile-arcs -ftest-coverage -o %s %s", output, source);
    
    for (int i = 0; i < extra_count; i++) {
        strncat(cmd, " ", sizeof(cmd) - strlen(cmd) - 1);
        strncat(cmd, extra_args[i], sizeof(cmd) - strlen(cmd) - 1);
    }
    
    return execute_command(cmd);
}

/* Write a string to a file */
int write_to_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return -1;
    }
    fputs(content, f);
    fclose(f);
    return 0;
}

/* Run gcov-tool overlap with various flag combinations */
void test_gcov_tool_flags(const char *gcda_file, const char *gcno_file) {
    char cmd[1024];
    
    /* Test case 'v' - verbose flag */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -v %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 'f' - function level overlap */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -f %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 'F' - use fullname */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -F %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 'o' - object level overlap */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -o %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 'h' - hot only */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -h %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 't' - hot threshold with argument */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.5 %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 't' with different threshold values */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.75 %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.25 %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case with multiple flags combined */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -v -f -o %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -F -h -t 0.6 %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test default case - invalid option to trigger overlap_usage() */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -z 2>/dev/null");
    execute_command(cmd);
    
    /* Another invalid option combination */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -X 2>/dev/null");
    execute_command(cmd);
}

/* Clean up generated files */
void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "rm -f %s", files[i]);
        system(cmd);
    }
}

int main(int argc, char **argv) {
    printf("=== Starting gcov-tool overlap command-line parsing tests ===\n\n");
    
    /* Check if gcov-tool exists */
    if (system("which gcov-tool > /dev/null 2>&1") != 0) {
        fprintf(stderr, "Error: gcov-tool not found in PATH\n");
        fprintf(stderr, "Try: ./gcov-tool or build/gcc/gcov-tool\n");
        return 1;
    }
    
    /* Test Scenario A: Simple function with branches */
    printf("\n--- Scenario A: Simple function with branches ---\n");
    write_to_file("test_a.c", test_program_a);
    compile_with_coverage("test_a.c", "test_a", NULL, 0);
    execute_command("./test_a");
    
    test_gcov_tool_flags("test_a.gcda", "test_a.gcno");
    
    /* Test Scenario B: Loop-heavy program with different runs */
    printf("\n--- Scenario B: Loop-heavy program ---\n");
    write_to_file("test_b.c", test_program_b);
    compile_with_coverage("test_b.c", "test_b", NULL, 0);
    
    /* Run multiple times with different arguments */
    execute_command("./test_b 3");
    execute_command("./test_b 5");
    execute_command("./test_b 2");
    
    test_gcov_tool_flags("test_b.gcda", "test_b.gcno");
    
    /* Test Scenario C: Multiple source files */
    printf("\n--- Scenario C: Multiple source files ---\n");
    write_to_file("test_header.h", test_header);
    write_to_file("test_c1.c", test_program_c1);
    write_to_file("test_c2.c", test_program_c2);
    
    const char *extra_args[] = {"test_c2.c"};
    compile_with_coverage("test_c1.c", "test_c", extra_args, 1);
    execute_command("./test_c");
    
    /* Test with both gcda files */
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -v test_c1.gcda test_c1.gcno test_c2.gcda test_c2.gcno");
    execute_command(cmd);
    
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -f -F test_c1.gcda test_c1.gcno");
    execute_command(cmd);
    
    /* Test Scenario D: Empty/zero counts */
    printf("\n--- Scenario D: Zero coverage data ---\n");
    write_to_file("test_d.c", test_program_d);
    compile_with_coverage("test_d.c", "test_d", NULL, 0);
    execute_command("./test_d");
    
    /* Test hot threshold with zero counts */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.1 test_d.gcda test_d.gcno");
    execute_command(cmd);
    
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -h -t 0.5 test_d.gcda test_d.gcno");
    execute_command(cmd);
    
    /* Additional edge case: Test with non-existent file to see error handling */
    printf("\n--- Edge case: Non-existent file ---\n");
    execute_command("gcov-tool overlap -v nonexistent.gcda nonexistent.gcno 2>/dev/null");
    
    /* Test with just help flag (should not trigger our target lines but good to check) */
    execute_command("gcov-tool overlap --help 2>&1 | head -5");
    
    /* Cleanup */
    printf("\n=== Cleaning up generated files ===\n");
    const char *files_to_clean[] = {
        "test_a.c", "test_a", "test_a.gcda", "test_a.gcno",
        "test_b.c", "test_b", "test_b.gcda", "test_b.gcno",
        "test_c1.c", "test_c2.c", "test_header.h", "test_c",
        "test_c1.gcda", "test_c1.gcno", "test_c2.gcda", "test_c2.gcno",
        "test_d.c", "test_d", "test_d.gcda", "test_d.gcno",
        "*.gcov"
    };
    
    for (int i = 0; i < sizeof(files_to_clean)/sizeof(files_to_clean[0]); i++) {
        char cleanup_cmd[256];
        snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -f %s", files_to_clean[i]);
        system(cleanup_cmd);
    }
    
    printf("\n=== Test completed ===\n");
    printf("All gcov-tool overlap flag combinations have been exercised.\n");
    printf("Check gcov-tool's own .gcda file to verify coverage of lines 534-554.\n");
    
    return 0;
}
