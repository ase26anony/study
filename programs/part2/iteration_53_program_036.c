/* test_gcov_tool_overlap.c
 * Test harness to trigger uncovered lines in gcov-tool.cc (lines 534-554)
 * Compile and run: gcc -o test_gcov_tool test_gcov_tool_overlap.c && ./test_gcov_tool
 */

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
"#include \"test_header.h\"\n"
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
"#include \"test_header.h\"\n"
"void helper2() {\n"
"    printf(\"Helper2 called\\n\");\n"
"}\n";

const char *test_header = 
"#ifndef TEST_HEADER_H\n"
"#define TEST_HEADER_H\n"
"void helper2();\n"
"#endif\n";

const char *test_program_d = 
"#include <stdio.h>\n"
"int main() {\n"
"    /* This program runs but doesn't execute instrumented paths */\n"
"    /* Or we can choose not to run it at all */\n"
"    return 0;\n"
"}\n";

/* Function to write a string to a file */
int write_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fputs(content, f);
    fclose(f);
    return 1;
}

/* Function to compile a C program with coverage flags */
int compile_with_coverage(const char *source_file, const char *output_name, 
                          const char *additional_flags) {
    char command[512];
    snprintf(command, sizeof(command),
             "gcc -O0 -fprofile-arcs -ftest-coverage %s %s -o %s",
             additional_flags ? additional_flags : "",
             source_file, output_name);
    
    printf("Compiling: %s\n", command);
    int status = system(command);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

/* Function to run a program and generate .gcda files */
int run_program(const char *program, const char *args) {
    char command[256];
    if (args && args[0]) {
        snprintf(command, sizeof(command), "./%s %s", program, args);
    } else {
        snprintf(command, sizeof(command), "./%s", program);
    }
    
    printf("Running: %s\n", command);
    int status = system(command);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

/* Function to execute gcov-tool with specific arguments */
int run_gcov_tool_overlap(const char *description, const char *gcda_files, 
                          const char *gcno_files, const char *flags) {
    char command[1024];
    
    /* Try to find gcov-tool in common locations */
    const char *gcov_tool_path = NULL;
    const char *possible_paths[] = {
        "./gcov-tool",
        "gcov-tool",
        "../gcc/build/gcc/gcov-tool",
        "/usr/bin/gcov-tool",
        NULL
    };
    
    for (int i = 0; possible_paths[i]; i++) {
        if (access(possible_paths[i], X_OK) == 0) {
            gcov_tool_path = possible_paths[i];
            break;
        }
    }
    
    if (!gcov_tool_path) {
        fprintf(stderr, "ERROR: gcov-tool not found in PATH or common locations\n");
        fprintf(stderr, "Please build gcov-tool with: gcc -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
        return 0;
    }
    
    snprintf(command, sizeof(command), "%s overlap %s %s %s 2>&1",
             gcov_tool_path, flags ? flags : "", gcda_files, gcno_files);
    
    printf("\n=== Testing: %s ===\n", description);
    printf("Command: %s\n", command);
    
    int status = system(command);
    int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    
    /* For invalid options (default case), we expect non-zero exit */
    if (strstr(flags, "-z")) {
        printf("Note: Invalid option -z should trigger default case and usage\n");
    }
    
    return 1; /* Return success if command was executed */
}

/* Clean up generated files */
void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i] && access(files[i], F_OK) == 0) {
            remove(files[i]);
        }
    }
}

int main() {
    printf("=== Test Harness for gcov-tool overlap command-line parsing ===\n");
    printf("Target: Lines 534-554 in gcov-tool.cc\n\n");
    
    /* Arrays to track generated files for cleanup */
    const char *generated_files[50];
    int file_count = 0;
    
    /* Generate test programs */
    if (!write_file("test_a.c", test_program_a)) return 1;
    generated_files[file_count++] = "test_a.c";
    
    if (!write_file("test_b.c", test_program_b)) return 1;
    generated_files[file_count++] = "test_b.c";
    
    if (!write_file("test_c1.c", test_program_c1)) return 1;
    generated_files[file_count++] = "test_c1.c";
    
    if (!write_file("test_c2.c", test_program_c2)) return 1;
    generated_files[file_count++] = "test_c2.c";
    
    if (!write_file("test_header.h", test_header)) return 1;
    generated_files[file_count++] = "test_header.h";
    
    if (!write_file("test_d.c", test_program_d)) return 1;
    generated_files[file_count++] = "test_d.c";
    
    /* Compile test programs with coverage */
    if (!compile_with_coverage("test_a.c", "test_a", "")) {
        fprintf(stderr, "Failed to compile test_a.c\n");
        goto cleanup;
    }
    generated_files[file_count++] = "test_a";
    generated_files[file_count++] = "test_a.gcno";
    
    if (!compile_with_coverage("test_b.c", "test_b", "")) {
        fprintf(stderr, "Failed to compile test_b.c\n");
        goto cleanup;
    }
    generated_files[file_count++] = "test_b";
    generated_files[file_count++] = "test_b.gcno";
    
    /* Compile multi-file program */
    if (!compile_with_coverage("test_c1.c", "test_c", "test_c2.c")) {
        fprintf(stderr, "Failed to compile test_c program\n");
        goto cleanup;
    }
    generated_files[file_count++] = "test_c";
    generated_files[file_count++] = "test_c1.gcno";
    generated_files[file_count++] = "test_c2.gcno";
    
    if (!compile_with_coverage("test_d.c", "test_d", "")) {
        fprintf(stderr, "Failed to compile test_d.c\n");
        goto cleanup;
    }
    generated_files[file_count++] = "test_d";
    generated_files[file_count++] = "test_d.gcno";
    
    /* Run programs to generate .gcda files */
    printf("\n=== Generating coverage data ===\n");
    
    if (!run_program("test_a", "")) {
        fprintf(stderr, "Failed to run test_a\n");
    }
    generated_files[file_count++] = "test_a.gcda";
    
    /* Run test_b multiple times with different arguments */
    if (!run_program("test_b", "2")) {
        fprintf(stderr, "Failed to run test_b with arg 2\n");
    }
    generated_files[file_count++] = "test_b.gcda";
    
    /* Run test_b again to accumulate counts */
    if (!run_program("test_b", "4")) {
        fprintf(stderr, "Failed to run test_b with arg 4\n");
    }
    
    if (!run_program("test_c", "")) {
        fprintf(stderr, "Failed to run test_c\n");
    }
    generated_files[file_count++] = "test_c1.gcda";
    generated_files[file_count++] = "test_c2.gcda";
    
    /* Don't run test_d to get zero counts, or run it once */
    if (!run_program("test_d", "")) {
        fprintf(stderr, "Failed to run test_d\n");
    }
    generated_files[file_count++] = "test_d.gcda";
    
    /* Now test gcov-tool overlap with various flag combinations */
    printf("\n=== Testing gcov-tool overlap with different flags ===\n");
    
    /* Test 1: -v flag (verbose) - triggers case 'v' */
    run_gcov_tool_overlap("Verbose mode (-v)", 
                         "test_a.gcda", "test_a.gcno", "-v");
    
    /* Test 2: -f flag (function level) - triggers case 'f' */
    run_gcov_tool_overlap("Function level overlap (-f)",
                         "test_a.gcda test_b.gcda", 
                         "test_a.gcno test_b.gcno", "-f");
    
    /* Test 3: -F flag (fullname) - triggers case 'F' */
    run_gcov_tool_overlap("Fullname mode (-F)",
                         "test_b.gcda", "test_b.gcno", "-F");
    
    /* Test 4: -o flag (object level) - triggers case 'o' */
    run_gcov_tool_overlap("Object level overlap (-o)",
                         "test_a.gcda test_b.gcda",
                         "test_a.gcno test_b.gcno", "-o");
    
    /* Test 5: -h flag (hot only) - triggers case 'h' */
    run_gcov_tool_overlap("Hot only mode (-h)",
                         "test_b.gcda", "test_b.gcno", "-h");
    
    /* Test 6: -t flag with threshold - triggers case 't' */
    run_gcov_tool_overlap("Hot threshold 0.5 (-t 0.5)",
                         "test_b.gcda", "test_b.gcno", "-t 0.5");
    
    /* Test 7: -t flag with different threshold */
    run_gcov_tool_overlap("Hot threshold 0.75 (-t 0.75)",
                         "test_b.gcda", "test_b.gcno", "-t 0.75");
    
    /* Test 8: Combination of flags */
    run_gcov_tool_overlap("Combination -v -f -o",
                         "test_a.gcda test_b.gcda",
                         "test_a.gcno test_b.gcno", "-v -f -o");
    
    /* Test 9: Another combination */
    run_gcov_tool_overlap("Combination -F -h -t 0.3",
                         "test_b.gcda", "test_b.gcno", "-F -h -t 0.3");
    
    /* Test 10: Multi-file comparison */
    run_gcov_tool_overlap("Multi-file with -v -f",
                         "test_c1.gcda test_c2.gcda",
                         "test_c1.gcno test_c2.gcno", "-v -f");
    
    /* Test 11: Zero-count file */
    run_gcov_tool_overlap("Zero counts with -t 0.1",
                         "test_d.gcda", "test_d.gcno", "-t 0.1");
    
    /* Test 12: Invalid option - triggers default case and overlap_usage() */
    run_gcov_tool_overlap("Invalid option (-z) to trigger default case",
                         "test_a.gcda", "test_a.gcno", "-z");
    
    /* Test 13: Another invalid option */
    run_gcov_tool_overlap("Another invalid option (-X)",
                         "test_a.gcda", "test_a.gcno", "-X");
    
    /* Test 14: Valid flag after invalid (should still parse) */
    run_gcov_tool_overlap("Mixed valid/invalid (-z -v)",
                         "test_a.gcda", "test_a.gcno", "-z -v");
    
    printf("\n=== All tests completed ===\n");
    printf("The following switch cases in gcov-tool.cc should have been triggered:\n");
    printf("  case 'v': verbose = true; gcov_set_verbose();\n");
    printf("  case 'f': overlap_func_level = 1;\n");
    printf("  case 'F': overlap_use_fullname = 1;\n");
    printf("  case 'o': overlap_obj_level = 1;\n");
    printf("  case 'h': overlap_hot_only = 1;\n");
    printf("  case 't': overlap_hot_threshold = atof(optarg);\n");
    printf("  default: overlap_usage();\n");
    
cleanup:
    /* Clean up generated files */
    printf("\n=== Cleaning up generated files ===\n");
    cleanup_files(generated_files, file_count);
    
    /* Also clean up any .gcov files that might have been created */
    const char *extra_files[] = {
        "*.gcov", "*.gcda", "*.gcno", "test_*", "a.out"
    };
    
    for (int i = 0; i < sizeof(extra_files)/sizeof(extra_files[0]); i++) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "rm -f %s 2>/dev/null", extra_files[i]);
        system(cmd);
    }
    
    printf("Test harness completed.\n");
    printf("To verify coverage, run gcov on gcov-tool.cc after executing this test.\n");
    
    return 0;
}
