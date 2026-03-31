/* test_gcov_tool_overlap.c - Test harness for gcov-tool overlap command parsing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Simple C program templates for generating coverage data */
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
"int process(int n) {\n"
"    int sum = 0;\n"
"    for (int i = 0; i < n; i++) {\n"
"        for (int j = 0; j < i; j++) {\n"
"            sum += i * j;\n"
"        }\n"
"    }\n"
"    return sum;\n"
"}\n"
"int main(int argc, char **argv) {\n"
"    int iterations = 5;\n"
"    if (argc > 1) {\n"
"        iterations = atoi(argv[1]);\n"
"    }\n"
"    int result = process(iterations);\n"
"    printf(\"Result: %d\\n\", result);\n"
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
"    for (int i = 0; i < 2; i++) {\n"
"        printf(\"Helper2 iteration %d\\n\", i);\n"
"    }\n"
"}\n";

const char *test_header = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"void helper2();\n"
"#endif\n";

const char *test_program_d = 
"#include <stdio.h>\n"
"int unused_function() {\n"
"    return 42;\n"
"}\n"
"int main() {\n"
"    /* This path doesn't call any instrumented functions */\n"
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

/* Write a string to a file */
int write_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return -1;
    }
    fputs(content, f);
    fclose(f);
    return 0;
}

/* Compile a test program with coverage flags */
int compile_with_coverage(const char *source_file, const char *output_name) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage -o %s %s",
             output_name, source_file);
    return execute_command(cmd);
}

/* Run a program to generate .gcda files */
int run_program(const char *program, const char *args) {
    char cmd[512];
    if (args && args[0]) {
        snprintf(cmd, sizeof(cmd), "./%s %s", program, args);
    } else {
        snprintf(cmd, sizeof(cmd), "./%s", program);
    }
    return execute_command(cmd);
}

/* Test gcov-tool overlap with various flag combinations */
void test_overlap_flags(const char *gcda_file, const char *gcno_file) {
    char cmd[1024];
    
    /* Test case 'v' - verbose flag */
    printf("\n=== Testing -v flag (verbose) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -v %s %s 2>&1 | head -5",
             gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 'f' - function level overlap */
    printf("\n=== Testing -f flag (function level) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -f %s %s 2>&1 | head -5",
             gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 'F' - use fullname */
    printf("\n=== Testing -F flag (fullname) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -F %s %s 2>&1 | head -5",
             gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 'o' - object level */
    printf("\n=== Testing -o flag (object level) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -o %s %s 2>&1 | head -5",
             gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 'h' - hot only */
    printf("\n=== Testing -h flag (hot only) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -h %s %s 2>&1 | head -5",
             gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 't' - hot threshold with argument */
    printf("\n=== Testing -t flag (hot threshold) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.5 %s %s 2>&1 | head -5",
             gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test with multiple flags combined */
    printf("\n=== Testing combined flags (-v -f -o) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -v -f -o %s %s 2>&1 | head -5",
             gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test with different threshold values */
    printf("\n=== Testing various threshold values ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.75 %s %s 2>&1 | head -5",
             gcda_file, gcno_file);
    execute_command(cmd);
    
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.25 %s %s 2>&1 | head -5",
             gcda_file, gcno_file);
    execute_command(cmd);
    
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 1.0 %s %s 2>&1 | head -5",
             gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test default case (invalid option) to trigger overlap_usage() */
    printf("\n=== Testing invalid option (trigger default case) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -z %s %s 2>&1 | head -10",
             gcda_file, gcno_file);
    execute_command(cmd);
}

/* Clean up generated files */
void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "rm -f %s", files[i]);
            system(cmd);
        }
    }
}

int main(int argc, char **argv) {
    printf("=== Starting gcov-tool overlap command parsing test ===\n");
    
    /* Check if gcov-tool exists */
    if (system("which gcov-tool > /dev/null 2>&1") != 0) {
        fprintf(stderr, "Error: gcov-tool not found in PATH\n");
        fprintf(stderr, "Try: ./gcov-tool or build/gcov-tool\n");
        
        /* Try local build directory */
        if (access("./gcov-tool", X_OK) == 0) {
            printf("Using ./gcov-tool from current directory\n");
            if (system("ln -sf ./gcov-tool /tmp/gcov-tool-test 2>/dev/null") == 0) {
                system("export PATH=/tmp:$PATH");
            }
        } else if (access("../gcov-tool", X_OK) == 0) {
            printf("Using ../gcov-tool from parent directory\n");
            if (system("ln -sf ../gcov-tool /tmp/gcov-tool-test 2>/dev/null") == 0) {
                system("export PATH=/tmp:$PATH");
            }
        } else {
            fprintf(stderr, "Please build gcov-tool first or add to PATH\n");
            return 1;
        }
    }
    
    /* Create test directory */
    system("rm -rf /tmp/gcov_test && mkdir -p /tmp/gcov_test");
    chdir("/tmp/gcov_test");
    
    /* Scenario A: Simple function with basic branches */
    printf("\n--- Scenario A: Simple function ---\n");
    write_file("test_a.c", test_program_a);
    compile_with_coverage("test_a.c", "test_a");
    run_program("test_a", "");
    
    /* Test all flag combinations with scenario A */
    test_overlap_flags("test_a.gcda", "test_a.gcno");
    
    /* Scenario B: Loop-heavy program with multiple runs */
    printf("\n--- Scenario B: Loop-heavy program ---\n");
    write_file("test_b.c", test_program_b);
    compile_with_coverage("test_b.c", "test_b");
    
    /* Run multiple times with different inputs */
    run_program("test_b", "3");
    run_program("test_b", "5");
    run_program("test_b", "2");
    
    /* Test flags with scenario B */
    test_overlap_flags("test_b.gcda", "test_b.gcno");
    
    /* Scenario C: Multiple source files */
    printf("\n--- Scenario C: Multiple source files ---\n");
    write_file("test_c.h", test_header);
    write_file("test_c1.c", test_program_c1);
    write_file("test_c2.c", test_program_c2);
    
    /* Compile both files together */
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_c1.c test_c2.c -o test_c");
    run_program("test_c", "");
    
    /* Test with multiple .gcda files */
    printf("\n=== Testing with multiple coverage files ===\n");
    execute_command("gcov-tool overlap -v test_c1.gcda test_c1.gcno 2>&1 | head -5");
    execute_command("gcov-tool overlap -f -o test_c2.gcda test_c2.gcno 2>&1 | head -5");
    
    /* Test overlap between two files */
    execute_command("gcov-tool overlap -v test_c1.gcda test_c2.gcda 2>&1 | head -5");
    
    /* Scenario D: Empty/zero counts */
    printf("\n--- Scenario D: Zero coverage ---\n");
    write_file("test_d.c", test_program_d);
    compile_with_coverage("test_d.c", "test_d");
    run_program("test_d", "");
    
    /* Test flags with zero coverage (tests hot threshold logic) */
    printf("\n=== Testing with zero coverage data ===\n");
    execute_command("gcov-tool overlap -h test_d.gcda test_d.gcno 2>&1 | head -5");
    execute_command("gcov-tool overlap -t 0.5 test_d.gcda test_d.gcno 2>&1 | head -5");
    
    /* Additional comprehensive tests */
    printf("\n=== Additional comprehensive tests ===\n");
    
    /* Test with all flags except invalid ones */
    execute_command("gcov-tool overlap -v -f -F -o -h -t 0.3 test_a.gcda test_a.gcno 2>&1 | head -8");
    
    /* Test with just threshold (no other flags) */
    execute_command("gcov-tool overlap -t 0.9 test_b.gcda test_b.gcno 2>&1 | head -5");
    
    /* Test invalid threshold (should still parse, may fail later) */
    execute_command("gcov-tool overlap -t abc test_a.gcda test_a.gcno 2>&1 | head -5");
    
    /* Test another invalid option to ensure default case */
    execute_command("gcov-tool overlap -x -y test_a.gcda test_a.gcno 2>&1 | head -5");
    
    /* Test missing required arguments */
    execute_command("gcov-tool overlap -v 2>&1 | head -5");
    
    /* Cleanup */
    printf("\n=== Cleaning up test files ===\n");
    chdir("..");
    system("rm -rf /tmp/gcov_test");
    
    printf("\n=== Test completed ===\n");
    printf("The following gcov-tool overlap flags were tested:\n");
    printf("  -v (verbose)           - Triggers case 'v' and gcov_set_verbose()\n");
    printf("  -f (function level)    - Triggers case 'f', sets overlap_func_level\n");
    printf("  -F (fullname)          - Triggers case 'F', sets overlap_use_fullname\n");
    printf("  -o (object level)      - Triggers case 'o', sets overlap_obj_level\n");
    printf("  -h (hot only)          - Triggers case 'h', sets overlap_hot_only\n");
    printf("  -t (threshold)         - Triggers case 't', calls atof(optarg)\n");
    printf("  -z (invalid)           - Triggers default case, calls overlap_usage()\n");
    
    return 0;
}
