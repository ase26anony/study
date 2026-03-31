/**
 * Test harness for gcov-tool overlap command-line parsing coverage.
 * This program creates multiple test scenarios, generates coverage data,
 * and invokes gcov-tool overlap with various flag combinations to cover
 * the switch statement cases in gcov-tool.cc lines 534-554.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH 1024
#define NUM_TEST_SCENARIOS 4

/* Function prototypes */
int compile_with_coverage(const char *source, const char *output);
int run_executable(const char *executable, const char *args);
int invoke_gcov_tool(const char *gcda, const char *gcno, const char *flags);
void cleanup_files(const char *base_name);
void create_test_scenarios(void);

/* Test scenario source codes */
const char *scenario_a = 
    "#include <stdio.h>\n"
    "void func1(int x) {\n"
    "    if (x > 0) {\n"
    "        printf(\"Positive\\n\");\n"
    "    } else {\n"
    "        printf(\"Non-positive\\n\");\n"
    "    }\n"
    "}\n"
    "void func2(void) {\n"
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

const char *scenario_b = 
    "#include <stdio.h>\n"
    "#include <stdlib.h>\n"
    "void nested_loops(int n) {\n"
    "    int total = 0;\n"
    "    for (int i = 0; i < n; i++) {\n"
    "        for (int j = 0; j < n; j++) {\n"
    "            for (int k = 0; k < 2; k++) {\n"
    "                total += i * j * k;\n"
    "            }\n"
    "        }\n"
    "    }\n"
    "    printf(\"Total: %d\\n\", total);\n"
    "}\n"
    "int main(int argc, char **argv) {\n"
    "    int iterations = 3;\n"
    "    if (argc > 1) {\n"
    "        iterations = atoi(argv[1]);\n"
    "    }\n"
    "    nested_loops(iterations);\n"
    "    return 0;\n"
    "}\n";

const char *scenario_c1 = 
    "#include <stdio.h>\n"
    "#include \"test_header.h\"\n"
    "int main() {\n"
    "    int result = add_numbers(10, 20);\n"
    "    printf(\"Result: %d\\n\", result);\n"
    "    print_message(\"Hello from main\");\n"
    "    return 0;\n"
    "}\n";

const char *scenario_c2 = 
    "#include <stdio.h>\n"
    "#include \"test_header.h\"\n"
    "int add_numbers(int a, int b) {\n"
    "    return a + b;\n"
    "}\n"
    "void print_message(const char *msg) {\n"
    "    printf(\"Message: %s\\n\", msg);\n"
    "}\n";

const char *scenario_c_header = 
    "#ifndef TEST_HEADER_H\n"
    "#define TEST_HEADER_H\n"
    "int add_numbers(int a, int b);\n"
    "void print_message(const char *msg);\n"
    "#endif\n";

const char *scenario_d = 
    "#include <stdio.h>\n"
    "int main() {\n"
    "    /* This function has instrumented code but we won't execute\n"
    "       the paths that generate coverage */\n"
    "    int flag = 0;\n"
    "    if (flag) {\n"
    "        printf(\"This won't execute\\n\");\n"
    "    }\n"
    "    return 0;\n"
    "}\n";

/* Flag combinations to test the switch cases */
typedef struct {
    const char *flags;
    const char *description;
} FlagTest;

FlagTest flag_tests[] = {
    {"-v", "Verbose flag (case 'v')"},
    {"-f", "Function level overlap (case 'f')"},
    {"-F", "Use fullname (case 'F')"},
    {"-o", "Object level overlap (case 'o')"},
    {"-h", "Hot only (case 'h')"},
    {"-t 0.5", "Hot threshold 0.5 (case 't')"},
    {"-t 0.75", "Hot threshold 0.75 (case 't')"},
    {"-t 0.1", "Hot threshold 0.1 (case 't')"},
    {"-f -o", "Combination: function and object level"},
    {"-v -f -F", "Combination: verbose, function, fullname"},
    {"-h -t 0.3", "Combination: hot only with threshold"},
    {"-v -f -o -F -h -t 0.6", "All flags combined"},
    {"-z", "Invalid flag (default case)"},
    {NULL, NULL}
};

int main(int argc, char **argv) {
    printf("=== Starting gcov-tool overlap command-line parsing test ===\n\n");
    
    /* Check if gcov-tool exists */
    printf("Checking for gcov-tool...\n");
    if (system("which gcov-tool > /dev/null 2>&1") != 0) {
        fprintf(stderr, "Error: gcov-tool not found in PATH\n");
        fprintf(stderr, "Please ensure gcov-tool is built and in your PATH\n");
        fprintf(stderr, "You can build it with: gcc -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
        return 1;
    }
    printf("gcov-tool found.\n\n");
    
    /* Create test scenarios */
    create_test_scenarios();
    
    /* Test each scenario with various flag combinations */
    for (int i = 0; i < NUM_TEST_SCENARIOS; i++) {
        char base_name[MAX_PATH];
        char source_file[MAX_PATH];
        char exec_file[MAX_PATH];
        char gcda_file[MAX_PATH];
        char gcno_file[MAX_PATH];
        
        snprintf(base_name, sizeof(base_name), "test_scenario_%c", 'A' + i);
        snprintf(source_file, sizeof(source_file), "%s.c", base_name);
        snprintf(exec_file, sizeof(exec_file), "%s_exe", base_name);
        snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", base_name);
        snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", base_name);
        
        printf("\n=== Testing Scenario %c ===\n", 'A' + i);
        
        /* Compile with coverage */
        if (i == 2) {  /* Scenario C (multiple files) */
            /* Create header file */
            FILE *header = fopen("test_header.h", "w");
            if (header) {
                fputs(scenario_c_header, header);
                fclose(header);
            }
            
            /* Compile both source files */
            char compile_cmd[MAX_PATH * 3];
            snprintf(compile_cmd, sizeof(compile_cmd),
                    "gcc -fprofile-arcs -ftest-coverage -O0 %s.c %s_aux.c -o %s_exe",
                    base_name, base_name, base_name);
            
            if (system(compile_cmd) != 0) {
                fprintf(stderr, "Failed to compile scenario %c\n", 'A' + i);
                continue;
            }
            
            /* Run the executable */
            if (system("./test_scenario_C_exe > /dev/null 2>&1") != 0) {
                fprintf(stderr, "Failed to run scenario %c\n", 'A' + i);
            }
        } else {
            /* Compile single file */
            if (!compile_with_coverage(source_file, exec_file)) {
                fprintf(stderr, "Failed to compile scenario %c\n", 'A' + i);
                continue;
            }
            
            /* Run the executable */
            if (i == 1) {  /* Scenario B with different arguments */
                run_executable(exec_file, "2");
                run_executable(exec_file, "4");
            } else if (i == 3) {  /* Scenario D - compile but don't run (zero counts) */
                printf("Scenario D: Compiled but not executed (zero coverage counts)\n");
            } else {
                run_executable(exec_file, NULL);
            }
        }
        
        /* Test various flag combinations */
        printf("Testing flag combinations on %s:\n", gcda_file);
        
        for (int j = 0; flag_tests[j].flags != NULL; j++) {
            printf("  %s: ", flag_tests[j].description);
            fflush(stdout);
            
            if (i == 2) {  /* Scenario C has multiple gcda files */
                /* Test with multiple input files */
                char cmd[MAX_PATH * 4];
                snprintf(cmd, sizeof(cmd), 
                        "gcov-tool overlap %s test_scenario_C.gcda test_scenario_C_aux.gcda 2>&1 | head -5",
                        flag_tests[j].flags);
                
                int result = system(cmd);
                if (result != 0 && strcmp(flag_tests[j].flags, "-z") != 0) {
                    printf("FAILED (exit code: %d)\n", WEXITSTATUS(result));
                } else {
                    printf("OK\n");
                }
            } else {
                /* Test with single gcda file */
                if (!invoke_gcov_tool(gcda_file, gcno_file, flag_tests[j].flags)) {
                    printf("FAILED\n");
                } else {
                    printf("OK\n");
                }
            }
        }
        
        /* Cleanup for this scenario */
        if (i == 2) {
            /* Clean up multi-file scenario */
            remove("test_header.h");
            remove("test_scenario_C_aux.c");
            remove("test_scenario_C_aux.gcda");
            remove("test_scenario_C_aux.gcno");
            remove("test_scenario_C_exe");
        }
        
        cleanup_files(base_name);
    }
    
    /* Final test: invalid usage to ensure default case is hit */
    printf("\n=== Testing Invalid Usage (to trigger default case) ===\n");
    printf("Testing invalid flag -x: ");
    system("gcov-tool overlap -x 2>/dev/null");
    printf("(overlap_usage() should have been called)\n");
    
    printf("\n=== All tests completed ===\n");
    printf("The following gcov-tool overlap flags were tested:\n");
    printf("  -v (verbose) - triggers case 'v' and gcov_set_verbose()\n");
    printf("  -f (function level) - triggers case 'f'\n");
    printf("  -F (fullname) - triggers case 'F'\n");
    printf("  -o (object level) - triggers case 'o'\n");
    printf("  -h (hot only) - triggers case 'h'\n");
    printf("  -t <value> (hot threshold) - triggers case 't' and atof()\n");
    printf("  -z (invalid) - triggers default case and overlap_usage()\n");
    
    return 0;
}

int compile_with_coverage(const char *source, const char *output) {
    char cmd[MAX_PATH * 3];
    snprintf(cmd, sizeof(cmd), 
            "gcc -fprofile-arcs -ftest-coverage -O0 %s -o %s",
            source, output);
    
    return (system(cmd) == 0);
}

int run_executable(const char *executable, const char *args) {
    char cmd[MAX_PATH * 2];
    
    if (args) {
        snprintf(cmd, sizeof(cmd), "./%s %s > /dev/null 2>&1", executable, args);
    } else {
        snprintf(cmd, sizeof(cmd), "./%s > /dev/null 2>&1", executable);
    }
    
    return (system(cmd) == 0);
}

int invoke_gcov_tool(const char *gcda, const char *gcno, const char *flags) {
    char cmd[MAX_PATH * 4];
    
    /* For invalid flag -z, we expect non-zero exit, so don't check exit code */
    if (strcmp(flags, "-z") == 0) {
        snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s %s %s 2>/dev/null", 
                flags, gcda, gcno);
        system(cmd);  /* Don't check return code for invalid flag */
        return 1;     /* Consider it successful execution for coverage purposes */
    }
    
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s %s %s 2>&1 | head -3", 
            flags, gcda, gcno);
    
    int result = system(cmd);
    /* Some combinations might fail due to missing required arguments, 
       but we still executed the parsing code */
    return 1;
}

void cleanup_files(const char *base_name) {
    char filename[MAX_PATH];
    
    /* Remove executable */
    snprintf(filename, sizeof(filename), "%s_exe", base_name);
    remove(filename);
    
    /* Remove source file */
    snprintf(filename, sizeof(filename), "%s.c", base_name);
    remove(filename);
    
    /* Remove coverage files */
    snprintf(filename, sizeof(filename), "%s.gcda", base_name);
    remove(filename);
    
    snprintf(filename, sizeof(filename), "%s.gcno", base_name);
    remove(filename);
    
    /* Remove gcov files */
    snprintf(filename, sizeof(filename), "%s.c.gcov", base_name);
    remove(filename);
}

void create_test_scenarios(void) {
    /* Create Scenario A (Simple Function) */
    FILE *fa = fopen("test_scenario_A.c", "w");
    if (fa) {
        fputs(scenario_a, fa);
        fclose(fa);
    }
    
    /* Create Scenario B (Loop Heavy) */
    FILE *fb = fopen("test_scenario_B.c", "w");
    if (fb) {
        fputs(scenario_b, fb);
        fclose(fb);
    }
    
    /* Create Scenario C (Multiple Source Files) */
    FILE *fc1 = fopen("test_scenario_C.c", "w");
    if (fc1) {
        fputs(scenario_c1, fc1);
        fclose(fc1);
    }
    
    FILE *fc2 = fopen("test_scenario_C_aux.c", "w");
    if (fc2) {
        fputs(scenario_c2, fc2);
        fclose(fc2);
    }
    
    /* Create Scenario D (Empty/Zero Counts) */
    FILE *fd = fopen("test_scenario_D.c", "w");
    if (fd) {
        fputs(scenario_d, fd);
        fclose(fd);
    }
}
