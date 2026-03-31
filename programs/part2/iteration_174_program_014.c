/**
 * test_gcov_dump_coverage.c
 * 
 * A test program to achieve coverage of the switch-case argument parsing
 * in gcov-dump.cc (lines 111-130).
 * 
 * This program:
 * 1. Builds an instrumented version of gcov-dump
 * 2. Creates a dummy program to generate GCOV data files
 * 3. Systematically tests all flag combinations
 * 4. Merges coverage data after each test
 * 5. Verifies coverage was achieved
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024
#define MAX_CMD 4096

/* Global paths */
char gcov_dump_instrumented[MAX_PATH] = "./gcov-dump-instrumented";
char dummy_prog[MAX_PATH] = "./dummy_prog";
char dummy_gcda[MAX_PATH] = "./dummy.gcda";
char dummy_c[MAX_PATH] = "./dummy.c";
char gcov_dump_cc[MAX_PATH] = "gcov-dump.cc";
char current_dir[MAX_PATH];

/* Function prototypes */
int build_instrumented_gcov_dump(void);
int create_dummy_program(void);
int run_dummy_program(void);
int run_gcov_dump_test(const char *args, int expect_success);
int merge_coverage(void);
int check_coverage(void);
void cleanup(void);

int main(int argc, char *argv[]) {
    int result = 0;
    
    printf("=== Starting gcov-dump switch-case coverage test ===\n\n");
    
    /* Get current directory for relative paths */
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        perror("getcwd");
        return 1;
    }
    
    /* Step 1: Build instrumented gcov-dump */
    printf("1. Building instrumented gcov-dump...\n");
    if (build_instrumented_gcov_dump() != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    printf("   ✓ Built instrumented gcov-dump\n\n");
    
    /* Step 2: Create and run dummy program for GCOV data */
    printf("2. Creating dummy program for GCOV data generation...\n");
    if (create_dummy_program() != 0) {
        fprintf(stderr, "Failed to create dummy program\n");
        cleanup();
        return 1;
    }
    printf("   ✓ Created dummy.c\n");
    
    printf("3. Running dummy program to generate .gcda file...\n");
    if (run_dummy_program() != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        cleanup();
        return 1;
    }
    printf("   ✓ Generated dummy.gcda\n\n");
    
    /* Step 3: Test all flag combinations */
    printf("4. Testing gcov-dump flag combinations...\n");
    
    /* Test help flag */
    printf("   Testing -h (help)...\n");
    if (run_gcov_dump_test("-h", 1) != 0) {
        fprintf(stderr, "Help test failed\n");
        result = 1;
    }
    merge_coverage();
    
    /* Test version flag */
    printf("   Testing -v (version)...\n");
    if (run_gcov_dump_test("-v", 1) != 0) {
        fprintf(stderr, "Version test failed\n");
        result = 1;
    }
    merge_coverage();
    
    /* Test individual flags with GCOV data file */
    printf("   Testing -l (dump contents)...\n");
    if (run_gcov_dump_test("-l dummy.gcda", 1) != 0) {
        fprintf(stderr, "-l test failed\n");
        result = 1;
    }
    merge_coverage();
    
    printf("   Testing -p (dump positions)...\n");
    if (run_gcov_dump_test("-p dummy.gcda", 1) != 0) {
        fprintf(stderr, "-p test failed\n");
        result = 1;
    }
    merge_coverage();
    
    printf("   Testing -r (dump raw)...\n");
    if (run_gcov_dump_test("-r dummy.gcda", 1) != 0) {
        fprintf(stderr, "-r test failed\n");
        result = 1;
    }
    merge_coverage();
    
    printf("   Testing -s (dump stable)...\n");
    if (run_gcov_dump_test("-s dummy.gcda", 1) != 0) {
        fprintf(stderr, "-s test failed\n");
        result = 1;
    }
    merge_coverage();
    
    /* Test combined flags (space-separated) */
    printf("   Testing -l -p -r -s (space-separated)...\n");
    if (run_gcov_dump_test("-l -p -r -s dummy.gcda", 1) != 0) {
        fprintf(stderr, "Combined flags test failed\n");
        result = 1;
    }
    merge_coverage();
    
    /* Test concatenated flags */
    printf("   Testing -lprs (concatenated)...\n");
    if (run_gcov_dump_test("-lprs dummy.gcda", 1) != 0) {
        fprintf(stderr, "Concatenated flags test failed\n");
        result = 1;
    }
    merge_coverage();
    
    /* Test invalid flag (should trigger default case) */
    printf("   Testing -x (invalid flag, should trigger default case)...\n");
    if (run_gcov_dump_test("-x dummy.gcda", 0) != 0) {
        fprintf(stderr, "Invalid flag test failed\n");
        result = 1;
    }
    merge_coverage();
    
    /* Step 4: Check coverage */
    printf("\n5. Checking coverage results...\n");
    if (check_coverage() != 0) {
        fprintf(stderr, "Coverage check failed\n");
        result = 1;
    }
    
    /* Cleanup */
    printf("\n6. Cleaning up...\n");
    cleanup();
    
    if (result == 0) {
        printf("\n=== All tests completed successfully! ===\n");
        printf("The switch-case in lines 111-130 of gcov-dump.cc should now be covered.\n");
    } else {
        printf("\n=== Some tests failed ===\n");
    }
    
    return result;
}

/**
 * Build an instrumented version of gcov-dump
 */
int build_instrumented_gcov_dump(void) {
    char cmd[MAX_CMD];
    int status;
    
    /* Check if gcov-dump.cc exists */
    struct stat st;
    if (stat(gcov_dump_cc, &st) != 0) {
        /* Try to find it in common locations */
        const char *possible_paths[] = {
            "../gcov-dump.cc",
            "../../gcc/gcov-dump.cc",
            "/usr/src/gcc/gcc/gcov-dump.cc",
            NULL
        };
        
        for (int i = 0; possible_paths[i] != NULL; i++) {
            if (stat(possible_paths[i], &st) == 0) {
                strcpy(gcov_dump_cc, possible_paths[i]);
                break;
            }
        }
        
        if (stat(gcov_dump_cc, &st) != 0) {
            fprintf(stderr, "Could not find gcov-dump.cc\n");
            return 1;
        }
    }
    
    /* Build command - adjust based on your environment */
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-I. -I../../include -I../../libiberty "
        "%s ../../libiberty/libiberty.a "
        "-o %s",
        gcov_dump_cc, gcov_dump_instrumented);
    
    printf("   Building with: %s\n", cmd);
    status = system(cmd);
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return 0;
    }
    
    /* Try alternative build if first fails */
    fprintf(stderr, "   First build attempt failed, trying simpler build...\n");
    
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "%s -o %s",
        gcov_dump_cc, gcov_dump_instrumented);
    
    printf("   Building with: %s\n", cmd);
    status = system(cmd);
    
    return (WIFEXITED(status) && WEXITSTATUS(status) == 0) ? 0 : 1;
}

/**
 * Create a dummy C program to generate GCOV data
 */
int create_dummy_program(void) {
    FILE *fp = fopen(dummy_c, "w");
    if (!fp) {
        perror("fopen dummy.c");
        return 1;
    }
    
    fprintf(fp, "/* dummy.c - Simple program to generate GCOV data */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i, sum = 0;\n");
    fprintf(fp, "    \n");
    fprintf(fp, "    /* Generate some branches for coverage */\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        if (i %% 2 == 0) {\n");
    fprintf(fp, "            sum += i;\n");
    fprintf(fp, "        } else {\n");
    fprintf(fp, "            sum -= i;\n");
    fprintf(fp, "        }\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    \n");
    fprintf(fp, "    printf(\"Result: %%d\\n\", sum);\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/**
 * Compile and run the dummy program
 */
int run_dummy_program(void) {
    char cmd[MAX_CMD];
    int status;
    
    /* Compile dummy program with coverage */
    snprintf(cmd, sizeof(cmd),
        "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
        dummy_c, dummy_prog);
    
    status = system(cmd);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return 1;
    }
    
    /* Run the dummy program to generate .gcda */
    status = system(dummy_prog);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return 1;
    }
    
    /* Verify .gcda was created */
    struct stat st;
    if (stat("dummy.gcda", &st) != 0) {
        fprintf(stderr, "dummy.gcda not created\n");
        return 1;
    }
    
    return 0;
}

/**
 * Run gcov-dump with given arguments
 * expect_success: 1 if command should succeed, 0 if it should fail
 */
int run_gcov_dump_test(const char *args, int expect_success) {
    char cmd[MAX_CMD];
    int status;
    
    snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_instrumented, args);
    
    printf("     Command: %s\n", cmd);
    status = system(cmd);
    
    if (expect_success) {
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("     ✓ Success (as expected)\n");
            return 0;
        } else {
            fprintf(stderr, "     ✗ Failed but expected success\n");
            return 1;
        }
    } else {
        /* For invalid flag, we expect failure */
        if (!WIFEXITED(status) || WEXITSTATUS(status) == 0) {
            fprintf(stderr, "     ✗ Succeeded but expected failure\n");
            return 1;
        } else {
            printf("     ✓ Failed as expected (invalid flag)\n");
            return 0;
        }
    }
}

/**
 * Merge coverage data from gcov-dump execution
 */
int merge_coverage(void) {
    char cmd[MAX_CMD];
    int status;
    
    /* First, ensure we have the .gcda file for gcov-dump */
    /* The instrumented binary creates it in current directory */
    
    /* Use gcov to generate coverage info */
    snprintf(cmd, sizeof(cmd), "gcov -i %s 2>&1", gcov_dump_cc);
    status = system(cmd);
    
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        /* Try without -i flag */
        snprintf(cmd, sizeof(cmd), "gcov %s 2>&1", gcov_dump_cc);
        status = system(cmd);
    }
    
    return (WIFEXITED(status) && WEXITSTATUS(status) == 0) ? 0 : 1;
}

/**
 * Check if target lines are covered
 */
int check_coverage(void) {
    char cmd[MAX_CMD];
    FILE *fp;
    char line[1024];
    int found_coverage = 0;
    
    /* Generate coverage report */
    snprintf(cmd, sizeof(cmd), "gcov -b %s 2>&1", gcov_dump_cc);
    fp = popen(cmd, "r");
    if (!fp) {
        perror("popen");
        return 1;
    }
    
    printf("   Coverage report for %s:\n", gcov_dump_cc);
    printf("   -----------------------------\n");
    
    while (fgets(line, sizeof(line), fp)) {
        /* Look for lines around 111-130 */
        if (strstr(line, "111:") || strstr(line, "112:") || 
            strstr(line, "120:") || strstr(line, "130:")) {
            printf("   %s", line);
            
            /* Check if line was executed */
            if (strstr(line, "#####") == NULL && 
                strstr(line, " -:") == NULL) {
                found_coverage = 1;
            }
        }
        
        /* Also print summary */
        if (strstr(line, "Lines executed:") || 
            strstr(line, "Branches executed:")) {
            printf("   %s", line);
        }
    }
    
    pclose(fp);
    
    if (found_coverage) {
        printf("   ✓ Target lines (111-130) appear to be covered\n");
        return 0;
    } else {
        printf("   ✗ Target lines may not be fully covered\n");
        return 1;
    }
}

/**
 * Clean up temporary files
 */
void cleanup(void) {
    char *files_to_remove[] = {
        dummy_prog,
        dummy_c,
        "dummy.gcda",
        "dummy.gcno",
        "dummy.c.gcov",
        "gcov-dump.gcda",
        "gcov-dump.gcno",
        NULL
    };
    
    for (int i = 0; files_to_remove[i] != NULL; i++) {
        if (access(files_to_remove[i], F_OK) == 0) {
            remove(files_to_remove[i]);
        }
    }
    
    /* Also clean up gcov-dump-instrumented if desired */
    /* remove(gcov_dump_instrumented); */
}
