/* test_gcov_dump_coverage.c - Test program to cover gcov-dump.cc switch-case lines */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024

/* Function prototypes */
int build_instrumented_gcov_dump(void);
int generate_test_gcov_data(void);
int run_gcov_dump_tests(void);
int merge_coverage_data(void);
int check_coverage(void);
void cleanup(void);

/* Global paths */
char gcov_dump_path[MAX_PATH] = "./gcov-dump-instrumented";
char dummy_gcda_path[MAX_PATH] = "./dummy.gcda";
char dummy_source_path[MAX_PATH] = "./dummy.c";
char dummy_prog_path[MAX_PATH] = "./dummy_prog";

int main(int argc, char *argv[]) {
    printf("=== Starting gcov-dump coverage test ===\n");
    
    /* Step 1: Build instrumented gcov-dump */
    printf("\n1. Building instrumented gcov-dump...\n");
    if (build_instrumented_gcov_dump() != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    /* Step 2: Generate test GCOV data */
    printf("\n2. Generating test GCOV data...\n");
    if (generate_test_gcov_data() != 0) {
        fprintf(stderr, "Failed to generate test GCOV data\n");
        return 1;
    }
    
    /* Step 3: Run comprehensive flag tests */
    printf("\n3. Running gcov-dump flag tests...\n");
    if (run_gcov_dump_tests() != 0) {
        fprintf(stderr, "Failed during gcov-dump tests\n");
        return 1;
    }
    
    /* Step 4: Merge coverage data */
    printf("\n4. Merging coverage data...\n");
    if (merge_coverage_data() != 0) {
        fprintf(stderr, "Failed to merge coverage data\n");
        return 1;
    }
    
    /* Step 5: Check coverage results */
    printf("\n5. Checking coverage results...\n");
    if (check_coverage() != 0) {
        fprintf(stderr, "Coverage check failed\n");
        return 1;
    }
    
    printf("\n=== Coverage test completed successfully ===\n");
    
    /* Optional: Clean up temporary files */
    /* cleanup(); */
    
    return 0;
}

/* Build instrumented gcov-dump binary */
int build_instrumented_gcov_dump(void) {
    struct stat st;
    
    /* Check if gcov-dump source exists */
    if (stat("gcov-dump.cc", &st) != 0) {
        fprintf(stderr, "gcov-dump.cc not found in current directory\n");
        fprintf(stderr, "Looking for it in common locations...\n");
        
        /* Try to find gcov-dump source in GCC build tree */
        const char *possible_paths[] = {
            "../gcov-dump.cc",
            "../../gcc/gcov-dump.cc",
            "/usr/src/gcc/gcc/gcov-dump.cc",
            NULL
        };
        
        for (int i = 0; possible_paths[i] != NULL; i++) {
            if (stat(possible_paths[i], &st) == 0) {
                char cmd[2048];
                snprintf(cmd, sizeof(cmd), "cp %s .", possible_paths[i]);
                system(cmd);
                break;
            }
        }
        
        if (stat("gcov-dump.cc", &st) != 0) {
            fprintf(stderr, "Could not find gcov-dump.cc source file\n");
            return 1;
        }
    }
    
    /* Build command for instrumented gcov-dump */
    char build_cmd[4096];
    snprintf(build_cmd, sizeof(build_cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-I. -I../../include -I../../libiberty "
        "gcov-dump.cc ../../libiberty/libiberty.a "
        "-o %s 2>&1", gcov_dump_path);
    
    printf("Executing: %s\n", build_cmd);
    int result = system(build_cmd);
    
    if (result != 0 || stat(gcov_dump_path, &st) != 0) {
        /* Try alternative build without libiberty path */
        snprintf(build_cmd, sizeof(build_cmd),
            "g++ -O0 -fprofile-arcs -ftest-coverage "
            "gcov-dump.cc -lgcov -o %s 2>&1", gcov_dump_path);
        
        printf("Trying alternative build: %s\n", build_cmd);
        result = system(build_cmd);
        
        if (result != 0 || stat(gcov_dump_path, &st) != 0) {
            fprintf(stderr, "Failed to build instrumented gcov-dump\n");
            return 1;
        }
    }
    
    printf("Successfully built instrumented gcov-dump: %s\n", gcov_dump_path);
    return 0;
}

/* Generate dummy C program and its GCOV data */
int generate_test_gcov_data(void) {
    /* Create dummy.c source file */
    FILE *fp = fopen(dummy_source_path, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return 1;
    }
    
    fprintf(fp, "/* dummy.c - Simple program to generate GCOV data */\n");
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
    
    /* Compile dummy program with coverage */
    char compile_cmd[1024];
    snprintf(compile_cmd, sizeof(compile_cmd),
        "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
        dummy_source_path, dummy_prog_path);
    
    printf("Compiling dummy program: %s\n", compile_cmd);
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return 1;
    }
    
    /* Run dummy program to generate .gcda file */
    printf("Running dummy program to generate .gcda...\n");
    if (system(dummy_prog_path) != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return 1;
    }
    
    /* Check if .gcda was created */
    struct stat st;
    if (stat(dummy_gcda_path, &st) != 0) {
        /* Try alternative name */
        snprintf(dummy_gcda_path, sizeof(dummy_gcda_path), "./dummy.gcda");
        if (stat(dummy_gcda_path, &st) != 0) {
            fprintf(stderr, "Failed to generate .gcda file\n");
            return 1;
        }
    }
    
    printf("Generated GCOV data file: %s\n", dummy_gcda_path);
    return 0;
}

/* Run comprehensive gcov-dump tests with various flag combinations */
int run_gcov_dump_tests(void) {
    char cmd[2048];
    int exit_status;
    
    printf("\nTesting individual flags:\n");
    printf("=========================\n");
    
    /* Test help flag */
    printf("\n1. Testing -h (help flag):\n");
    snprintf(cmd, sizeof(cmd), "%s -h 2>&1", gcov_dump_path);
    exit_status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(exit_status));
    
    /* Test version flag */
    printf("\n2. Testing -v (version flag):\n");
    snprintf(cmd, sizeof(cmd), "%s -v 2>&1", gcov_dump_path);
    exit_status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(exit_status));
    
    /* Test individual dump flags with GCOV data file */
    const char *flags[] = {"-l", "-p", "-r", "-s"};
    const char *flag_names[] = {"contents", "positions", "raw", "stable"};
    
    for (int i = 0; i < 4; i++) {
        printf("\n3.%d. Testing -%c (%s dump flag):\n", 
               i + 1, flags[i][1], flag_names[i]);
        snprintf(cmd, sizeof(cmd), "%s %s %s 2>&1", 
                gcov_dump_path, flags[i], dummy_gcda_path);
        exit_status = system(cmd);
        printf("Exit status: %d\n", WEXITSTATUS(exit_status));
        
        /* Merge coverage after each test */
        system("gcov -i gcov-dump.cc 2>&1");
    }
    
    /* Test combined flags (space-separated) */
    printf("\n4. Testing combined flags (space-separated):\n");
    snprintf(cmd, sizeof(cmd), "%s -l -p -r -s %s 2>&1", 
            gcov_dump_path, dummy_gcda_path);
    exit_status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(exit_status));
    system("gcov -i gcov-dump.cc 2>&1");
    
    /* Test concatenated flags */
    printf("\n5. Testing concatenated flags:\n");
    snprintf(cmd, sizeof(cmd), "%s -lprs %s 2>&1", 
            gcov_dump_path, dummy_gcda_path);
    exit_status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(exit_status));
    system("gcov -i gcov-dump.cc 2>&1");
    
    /* Test invalid flag (to trigger default case) */
    printf("\n6. Testing invalid flag (should trigger default case):\n");
    snprintf(cmd, sizeof(cmd), "%s -x %s 2>&1", 
            gcov_dump_path, dummy_gcda_path);
    exit_status = system(cmd);
    printf("Exit status: %d (expected non-zero)\n", WEXITSTATUS(exit_status));
    system("gcov -i gcov-dump.cc 2>&1");
    
    /* Test multiple invalid flags */
    printf("\n7. Testing multiple invalid flags:\n");
    snprintf(cmd, sizeof(cmd), "%s -xyz %s 2>&1", 
            gcov_dump_path, dummy_gcda_path);
    exit_status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(exit_status));
    system("gcov -i gcov-dump.cc 2>&1");
    
    /* Test flag with no argument (should show usage) */
    printf("\n8. Testing with no GCOV file argument:\n");
    snprintf(cmd, sizeof(cmd), "%s -l 2>&1", gcov_dump_path);
    exit_status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(exit_status));
    system("gcov -i gcov-dump.cc 2>&1");
    
    return 0;
}

/* Merge coverage data from all invocations */
int merge_coverage_data(void) {
    printf("Merging coverage data for gcov-dump.cc...\n");
    
    /* First, ensure we have the .gcno file */
    if (system("test -f gcov-dump.gcno || test -f gcov-dump.cc.gcno") != 0) {
        printf("No .gcno file found, coverage may not be available\n");
    }
    
    /* Merge using gcov intermediate format */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "gcov -i gcov-dump.cc 2>&1");
    int result = system(cmd);
    
    if (result != 0) {
        /* Try alternative approach */
        printf("Trying alternative coverage merge...\n");
        system("ls -la *.gcda 2>&1");
        system("gcov -b gcov-dump.cc 2>&1");
    }
    
    return 0;
}

/* Check if target lines are covered */
int check_coverage(void) {
    printf("Generating coverage report...\n");
    
    /* Generate human-readable coverage report */
    system("gcov -b gcov-dump.cc 2>&1 | tail -50");
    
    /* Check for gcov-dump.cc.gcov file */
    FILE *gcov_file = fopen("gcov-dump.cc.gcov", "r");
    if (!gcov_file) {
        /* Try alternative name */
        gcov_file = fopen("gcov-dump.gcov", "r");
        if (!gcov_file) {
            fprintf(stderr, "Could not find coverage output file\n");
            return 1;
        }
    }
    
    printf("\nChecking coverage for target lines (111-130)...\n");
    
    char line[1024];
    int target_lines_covered = 0;
    int target_lines_total = 0;
    
    while (fgets(line, sizeof(line), gcov_file)) {
        int line_num, count;
        char coverage_indicator[10];
        
        if (sscanf(line, "%s %d", coverage_indicator, &line_num) == 2) {
            if (line_num >= 111 && line_num <= 130) {
                target_lines_total++;
                
                /* Check if line was executed */
                if (strstr(coverage_indicator, "#####") == NULL && 
                    strstr(coverage_indicator, "-:") == NULL) {
                    target_lines_covered++;
                    printf("Line %d: COVERED (%s)\n", line_num, coverage_indicator);
                } else {
                    printf("Line %d: NOT COVERED\n", line_num);
                }
            }
        }
    }
    
    fclose(gcov_file);
    
    printf("\nCoverage Summary for lines 111-130:\n");
    printf("Lines covered: %d/%d (%.1f%%)\n", 
           target_lines_covered, target_lines_total,
           target_lines_total > 0 ? 
           (100.0 * target_lines_covered / target_lines_total) : 0.0);
    
    if (target_lines_covered > 0) {
        printf("\nSUCCESS: Target switch-case lines were executed!\n");
        return 0;
    } else {
        printf("\nFAILURE: Target lines were not covered\n");
        return 1;
    }
}

/* Clean up temporary files */
void cleanup(void) {
    printf("\nCleaning up temporary files...\n");
    system("rm -f dummy.c dummy_prog dummy.gcda dummy.gcno 2>/dev/null");
    system("rm -f gcov-dump.gcda gcov-dump.gcno gcov-dump.cc.gcov 2>/dev/null");
    system("rm -f gcov-dump-instrumented 2>/dev/null");
}
