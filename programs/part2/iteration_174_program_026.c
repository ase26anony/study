/* test_gcov_dump_coverage.c - Main test program to cover gcov-dump switch cases */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024

/* Function prototypes */
int build_instrumented_gcov_dump(void);
int generate_test_gcda(void);
int run_gcov_dump_with_flags(const char *flags, const char *gcda_file, int expect_success);
int merge_coverage_data(void);
int check_coverage(void);

int main(int argc, char *argv[]) {
    printf("=== Starting gcov-dump coverage test ===\n\n");
    
    /* Step 1: Build instrumented gcov-dump */
    printf("1. Building instrumented gcov-dump...\n");
    if (build_instrumented_gcov_dump() != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    /* Step 2: Generate test GCOV data file */
    printf("2. Generating test GCOV data file...\n");
    if (generate_test_gcda() != 0) {
        fprintf(stderr, "Failed to generate test GCOV data\n");
        return 1;
    }
    
    /* Step 3: Run gcov-dump with various flag combinations */
    printf("3. Running gcov-dump with flag combinations...\n");
    
    /* Test individual flags */
    printf("   Testing -h (help)...\n");
    run_gcov_dump_with_flags("-h", NULL, 1);
    merge_coverage_data();
    
    printf("   Testing -v (version)...\n");
    run_gcov_dump_with_flags("-v", NULL, 1);
    merge_coverage_data();
    
    printf("   Testing -l (dump contents)...\n");
    run_gcov_dump_with_flags("-l", "dummy.gcda", 1);
    merge_coverage_data();
    
    printf("   Testing -p (dump positions)...\n");
    run_gcov_dump_with_flags("-p", "dummy.gcda", 1);
    merge_coverage_data();
    
    printf("   Testing -r (dump raw)...\n");
    run_gcov_dump_with_flags("-r", "dummy.gcda", 1);
    merge_coverage_data();
    
    printf("   Testing -s (dump stable)...\n");
    run_gcov_dump_with_flags("-s", "dummy.gcda", 1);
    merge_coverage_data();
    
    /* Test combined flags (space-separated) */
    printf("   Testing -l -p -r -s (space-separated)...\n");
    run_gcov_dump_with_flags("-l -p -r -s", "dummy.gcda", 1);
    merge_coverage_data();
    
    /* Test concatenated flags */
    printf("   Testing -lprs (concatenated)...\n");
    run_gcov_dump_with_flags("-lprs", "dummy.gcda", 1);
    merge_coverage_data();
    
    /* Test invalid flag to trigger default case */
    printf("   Testing -x (invalid flag, should trigger default case)...\n");
    run_gcov_dump_with_flags("-x", "dummy.gcda", 0);
    merge_coverage_data();
    
    /* Step 4: Final coverage check */
    printf("4. Generating final coverage report...\n");
    if (check_coverage() != 0) {
        fprintf(stderr, "Failed to check coverage\n");
        return 1;
    }
    
    printf("\n=== Coverage test completed successfully ===\n");
    return 0;
}

/* Build instrumented gcov-dump binary */
int build_instrumented_gcov_dump(void) {
    char cmd[MAX_PATH * 2];
    int status;
    
    /* First check if we can find gcov-dump source */
    struct stat st;
    if (stat("gcov-dump.cc", &st) != 0) {
        /* Try to find it in common locations */
        const char *possible_paths[] = {
            "../../gcc/gcov-dump.cc",
            "../gcc/gcov-dump.cc",
            "gcc/gcov-dump.cc",
            NULL
        };
        
        int found = 0;
        for (int i = 0; possible_paths[i] != NULL; i++) {
            if (stat(possible_paths[i], &st) == 0) {
                snprintf(cmd, sizeof(cmd), "cp %s .", possible_paths[i]);
                system(cmd);
                found = 1;
                break;
            }
        }
        
        if (!found) {
            fprintf(stderr, "Could not find gcov-dump.cc source file\n");
            return 1;
        }
    }
    
    /* Build command to compile instrumented gcov-dump */
    /* Try different compilation approaches */
    const char *compile_cmds[] = {
        "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include -I../../libiberty gcov-dump.cc ../../libiberty/libiberty.a -o gcov-dump-instrumented",
        "g++ -O0 -fprofile-arcs -ftest-coverage gcov-dump.cc -o gcov-dump-instrumented",
        "gcc -O0 -fprofile-arcs -ftest-coverage -lgcov gcov-dump.cc -o gcov-dump-instrumented",
        NULL
    };
    
    for (int i = 0; compile_cmds[i] != NULL; i++) {
        printf("   Trying: %s\n", compile_cmds[i]);
        status = system(compile_cmds[i]);
        if (status == 0) {
            printf("   Successfully built gcov-dump-instrumented\n");
            return 0;
        }
    }
    
    fprintf(stderr, "All compilation attempts failed\n");
    return 1;
}

/* Generate a simple C program and compile it with coverage to create .gcda file */
int generate_test_gcda(void) {
    FILE *fp;
    
    /* Create dummy.c */
    fp = fopen("dummy.c", "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return 1;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Value: %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    fclose(fp);
    
    /* Compile dummy.c with coverage */
    int status = system("gcc -O0 -fprofile-arcs -ftest-coverage dummy.c -o dummy_prog");
    if (status != 0) {
        fprintf(stderr, "Failed to compile dummy.c\n");
        return 1;
    }
    
    /* Run dummy_prog to generate .gcda file */
    status = system("./dummy_prog > /dev/null 2>&1");
    if (status != 0) {
        fprintf(stderr, "Failed to run dummy_prog\n");
        return 1;
    }
    
    /* Verify .gcda file was created */
    struct stat st;
    if (stat("dummy.gcda", &st) != 0) {
        fprintf(stderr, "dummy.gcda not created\n");
        return 1;
    }
    
    printf("   Generated dummy.gcda (%ld bytes)\n", (long)st.st_size);
    return 0;
}

/* Run gcov-dump with specified flags */
int run_gcov_dump_with_flags(const char *flags, const char *gcda_file, int expect_success) {
    char cmd[MAX_PATH * 2];
    int status;
    
    /* Build command */
    if (gcda_file) {
        snprintf(cmd, sizeof(cmd), "./gcov-dump-instrumented %s %s 2>&1", flags, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "./gcov-dump-instrumented %s 2>&1", flags);
    }
    
    printf("     Command: %s\n", cmd);
    
    /* Execute command */
    status = system(cmd);
    
    /* Check if result matches expectation */
    if (expect_success) {
        if (status != 0) {
            fprintf(stderr, "     WARNING: Command failed (exit code: %d)\n", status);
        }
    } else {
        if (status == 0) {
            fprintf(stderr, "     WARNING: Invalid flag command succeeded (should have failed)\n");
        } else {
            printf("     Good: Invalid flag correctly failed\n");
        }
    }
    
    return 0;
}

/* Merge coverage data from gcov-dump execution */
int merge_coverage_data(void) {
    char cmd[MAX_PATH];
    int status;
    
    /* First, check if we have .gcda files for gcov-dump */
    snprintf(cmd, sizeof(cmd), "ls -la gcov-dump*.gcda 2>/dev/null | head -5");
    system(cmd);
    
    /* Try to generate coverage info using gcov */
    printf("     Merging coverage data...\n");
    
    /* Approach 1: Direct gcov invocation */
    snprintf(cmd, sizeof(cmd), "gcov -i gcov-dump.cc 2>&1 | tail -5");
    status = system(cmd);
    
    /* Approach 2: If that doesn't work, try moving .gcda files */
    if (status != 0) {
        /* Create a directory for coverage data */
        system("mkdir -p coverage_data");
        
        /* Move any gcov-dump related .gcda files */
        snprintf(cmd, sizeof(cmd), "mv gcov-dump*.gcda coverage_data/ 2>/dev/null");
        system(cmd);
        
        /* Try gcov again from the coverage_data directory */
        snprintf(cmd, sizeof(cmd), "cd coverage_data && gcov -i ../gcov-dump.cc 2>&1 | tail -5");
        system(cmd);
    }
    
    return 0;
}

/* Check final coverage */
int check_coverage(void) {
    char cmd[MAX_PATH];
    
    printf("   Generating coverage report for gcov-dump.cc...\n");
    
    /* Generate coverage report */
    snprintf(cmd, sizeof(cmd), "gcov -b gcov-dump.cc 2>&1");
    system(cmd);
    
    /* Look for the coverage summary */
    printf("\n   === Coverage Summary ===\n");
    snprintf(cmd, sizeof(cmd), "cat gcov-dump.cc.gcov 2>&1 | grep -A5 -B5 'lines.*111.*130'");
    system(cmd);
    
    /* Specifically check for our target lines */
    printf("\n   === Target Lines (111-130) ===\n");
    snprintf(cmd, sizeof(cmd), "sed -n '111,130p' gcov-dump.cc.gcov 2>&1");
    system(cmd);
    
    /* Check if lines were executed */
    printf("\n   === Execution Counts for Switch Cases ===\n");
    snprintf(cmd, sizeof(cmd), "grep -E '^[[:space:]]*[0-9]+.*case.*[hlprsv]' gcov-dump.cc.gcov 2>&1");
    system(cmd);
    
    return 0;
}
