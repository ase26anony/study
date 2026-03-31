/**
 * test_gcov_dump_coverage.c
 * 
 * A test program to exercise the uncovered command-line argument parsing
 * logic in gcov-dump.cc (lines 111-130).
 * 
 * This program:
 * 1. Builds an instrumented version of gcov-dump
 * 2. Creates a dummy program to generate GCOV data files
 * 3. Systematically tests all flag combinations
 * 4. Merges coverage data after each test
 * 5. Verifies coverage of the target lines
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_PATH 1024
#define MAX_CMD 4096

/**
 * Structure to hold test configuration
 */
typedef struct {
    char gcov_dump_src[MAX_PATH];      /* Path to gcov-dump.cc source */
    char gcov_dump_instr[MAX_PATH];    /* Path to instrumented gcov-dump binary */
    char dummy_src[MAX_PATH];          /* Path to dummy.c source */
    char dummy_prog[MAX_PATH];         /* Path to dummy program */
    char dummy_gcda[MAX_PATH];         /* Path to dummy.gcda */
    char libiberty_path[MAX_PATH];     /* Path to libiberty library */
    char include_path[MAX_PATH];       /* Path to include directory */
    char work_dir[MAX_PATH];           /* Working directory */
    int verbose;                       /* Verbose output flag */
} TestConfig;

/**
 * Initialize test configuration with default values
 */
void init_config(TestConfig *config) {
    config->verbose = 1;
    
    /* Get current working directory */
    if (getcwd(config->work_dir, MAX_PATH) == NULL) {
        strcpy(config->work_dir, ".");
    }
    
    /* Set default paths - adjust these based on your GCC source layout */
    snprintf(config->gcov_dump_src, MAX_PATH, "%s/gcov-dump.cc", config->work_dir);
    snprintf(config->gcov_dump_instr, MAX_PATH, "%s/gcov-dump-instrumented", config->work_dir);
    snprintf(config->dummy_src, MAX_PATH, "%s/dummy.c", config->work_dir);
    snprintf(config->dummy_prog, MAX_PATH, "%s/dummy_prog", config->work_dir);
    snprintf(config->dummy_gcda, MAX_PATH, "%s/dummy.gcda", config->work_dir);
    
    /* Try to find libiberty and include paths */
    snprintf(config->libiberty_path, MAX_PATH, "%s/../../libiberty/libiberty.a", config->work_dir);
    snprintf(config->include_path, MAX_PATH, "%s/../../include", config->work_dir);
}

/**
 * Execute a shell command and return its exit code
 */
int execute_command(const char *cmd, int verbose) {
    if (verbose) {
        printf("Executing: %s\n", cmd);
    }
    
    int result = system(cmd);
    
    if (verbose && result != 0) {
        printf("Command returned: %d\n", result);
    }
    
    return result;
}

/**
 * Create a dummy C program for generating GCOV data
 */
int create_dummy_program(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return 0;
    }
    
    fprintf(fp, "/* dummy.c - Simple program to generate GCOV data */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    printf(\"Dummy program executed.\\n\");\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Iteration %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 1;
}

/**
 * Build instrumented gcov-dump binary
 */
int build_instrumented_gcov_dump(TestConfig *config) {
    char cmd[MAX_CMD];
    
    printf("=== Building instrumented gcov-dump ===\n");
    
    /* Check if source exists */
    struct stat st;
    if (stat(config->gcov_dump_src, &st) != 0) {
        printf("Error: gcov-dump.cc not found at %s\n", config->gcov_dump_src);
        printf("Please ensure gcov-dump.cc is in the current directory.\n");
        return 0;
    }
    
    /* Build command for instrumented gcov-dump */
    snprintf(cmd, MAX_CMD,
             "g++ -O0 -fprofile-arcs -ftest-coverage "
             "-I%s -I%s -I%s/../../libiberty "
             "%s %s -o %s",
             config->include_path,
             config->work_dir,
             config->work_dir,
             config->gcov_dump_src,
             config->libiberty_path,
             config->gcov_dump_instr);
    
    if (execute_command(cmd, config->verbose) != 0) {
        printf("Failed to build instrumented gcov-dump\n");
        return 0;
    }
    
    printf("Instrumented gcov-dump built successfully: %s\n", config->gcov_dump_instr);
    return 1;
}

/**
 * Create and build dummy program for GCOV data generation
 */
int build_dummy_program(TestConfig *config) {
    char cmd[MAX_CMD];
    
    printf("\n=== Creating dummy program for GCOV data ===\n");
    
    /* Create dummy.c */
    if (!create_dummy_program(config->dummy_src)) {
        return 0;
    }
    printf("Created %s\n", config->dummy_src);
    
    /* Build dummy program with coverage */
    snprintf(cmd, MAX_CMD,
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
             config->dummy_src, config->dummy_prog);
    
    if (execute_command(cmd, config->verbose) != 0) {
        printf("Failed to build dummy program\n");
        return 0;
    }
    
    /* Execute dummy program to generate .gcda file */
    printf("\nExecuting dummy program to generate .gcda file...\n");
    if (execute_command(config->dummy_prog, config->verbose) != 0) {
        printf("Failed to execute dummy program\n");
        return 0;
    }
    
    /* Verify .gcda file was created */
    struct stat st;
    if (stat(config->dummy_gcda, &st) != 0) {
        printf("Warning: dummy.gcda not created. Trying alternative location...\n");
        /* Try to find it in current directory */
        snprintf(config->dummy_gcda, MAX_PATH, "./dummy.gcda");
    }
    
    if (stat(config->dummy_gcda, &st) == 0) {
        printf("GCOV data file created: %s\n", config->dummy_gcda);
    } else {
        printf("Warning: Could not find dummy.gcda file\n");
    }
    
    return 1;
}

/**
 * Run a single test with the instrumented gcov-dump
 */
int run_gcov_dump_test(TestConfig *config, const char *args, int expect_success) {
    char cmd[MAX_CMD];
    int result;
    
    /* Construct command */
    if (strstr(args, "-h") != NULL || strstr(args, "-v") != NULL) {
        /* Help and version don't need a data file */
        snprintf(cmd, MAX_CMD, "%s %s", config->gcov_dump_instr, args);
    } else if (strstr(args, "-x") != NULL) {
        /* Invalid flag test */
        snprintf(cmd, MAX_CMD, "%s %s %s 2>&1", 
                config->gcov_dump_instr, args, config->dummy_gcda);
    } else {
        /* Normal test with data file */
        snprintf(cmd, MAX_CMD, "%s %s %s", 
                config->gcov_dump_instr, args, config->dummy_gcda);
    }
    
    printf("\nTest: gcov-dump %s\n", args);
    result = execute_command(cmd, config->verbose);
    
    /* Check result */
    if (expect_success) {
        if (result == 0) {
            printf("  ✓ Success (expected)\n");
        } else {
            printf("  ✗ Failed (but expected success)\n");
            return 0;
        }
    } else {
        if (result != 0) {
            printf("  ✓ Failed as expected\n");
        } else {
            printf("  ✗ Succeeded (but expected failure)\n");
            return 0;
        }
    }
    
    /* Merge coverage data after each test */
    printf("  Merging coverage data...\n");
    snprintf(cmd, MAX_CMD, "gcov -i %s 2>/dev/null", config->gcov_dump_src);
    execute_command(cmd, 0);  /* Silent merge */
    
    return 1;
}

/**
 * Run all flag combination tests
 */
int run_all_tests(TestConfig *config) {
    int all_passed = 1;
    
    printf("\n=== Running flag combination tests ===\n");
    
    /* Test individual flags */
    all_passed &= run_gcov_dump_test(config, "-h", 1);      /* Help */
    all_passed &= run_gcov_dump_test(config, "-v", 1);      /* Version */
    all_passed &= run_gcov_dump_test(config, "-l", 1);      /* Dump contents */
    all_passed &= run_gcov_dump_test(config, "-p", 1);      /* Dump positions */
    all_passed &= run_gcov_dump_test(config, "-r", 1);      /* Dump raw */
    all_passed &= run_gcov_dump_test(config, "-s", 1);      /* Dump stable */
    
    /* Test combined flags (space-separated) */
    all_passed &= run_gcov_dump_test(config, "-l -p -r -s", 1);
    
    /* Test concatenated flags */
    all_passed &= run_gcov_dump_test(config, "-lprs", 1);
    
    /* Test invalid flag (should fail) */
    all_passed &= run_gcov_dump_test(config, "-x", 0);
    
    /* Test various combinations */
    all_passed &= run_gcov_dump_test(config, "-lp", 1);
    all_passed &= run_gcov_dump_test(config, "-rs", 1);
    all_passed &= run_gcov_dump_test(config, "-l -s", 1);
    
    return all_passed;
}

/**
 * Generate and display final coverage report
 */
void generate_coverage_report(TestConfig *config) {
    char cmd[MAX_CMD];
    
    printf("\n=== Generating coverage report ===\n");
    
    /* Generate human-readable coverage report */
    snprintf(cmd, MAX_CMD, "gcov -b %s", config->gcov_dump_src);
    printf("Running: %s\n", cmd);
    system(cmd);
    
    /* Display the .gcov file */
    char gcov_file[MAX_PATH];
    snprintf(gcov_file, MAX_PATH, "%s.gcov", config->gcov_dump_src);
    
    FILE *fp = fopen(gcov_file, "r");
    if (fp) {
        printf("\n=== Coverage summary for target lines (111-130) ===\n");
        
        char line[1024];
        int line_num = 0;
        int in_target_range = 0;
        int target_lines_covered = 0;
        int target_lines_total = 0;
        
        while (fgets(line, sizeof(line), fp)) {
            /* Parse gcov output line */
            if (strlen(line) > 10) {
                /* Check if this is a source line (starts with line count) */
                if (line[0] == '-' || line[0] == '#' || 
                    (line[0] >= '0' && line[0] <= '9')) {
                    
                    /* Extract line number */
                    char *colon = strchr(line, ':');
                    if (colon) {
                        line_num = atoi(colon + 1);
                        
                        if (line_num >= 111 && line_num <= 130) {
                            in_target_range = 1;
                            target_lines_total++;
                            
                            /* Check if line was executed */
                            if (line[0] != '-' && line[0] != '#') {
                                target_lines_covered++;
                                
                                /* Display executed lines */
                                printf("  Line %3d: EXECUTED - ", line_num);
                                /* Print the actual source (skip gcov prefix) */
                                char *source_start = strchr(colon + 1, ':');
                                if (source_start) {
                                    printf("%s", source_start + 1);
                                }
                            } else if (line[0] == '#') {
                                printf("  Line %3d: NOT EXECUTED\n", line_num);
                            }
                        } else if (in_target_range && line_num > 130) {
                            break;  /* Past target range */
                        }
                    }
                }
            }
        }
        
        fclose(fp);
        
        printf("\n=== Coverage Statistics ===\n");
        printf("Target lines (111-130): %d of %d executed\n", 
               target_lines_covered, target_lines_total);
        
        if (target_lines_covered == target_lines_total) {
            printf("✓ All target lines covered!\n");
        } else {
            printf("✗ Some target lines not covered\n");
        }
        
        /* Clean up gcov file */
        remove(gcov_file);
    } else {
        printf("Could not open coverage file: %s\n", gcov_file);
    }
}

/**
 * Clean up temporary files
 */
void cleanup(TestConfig *config) {
    printf("\n=== Cleaning up ===\n");
    
    /* Remove generated files */
    remove(config->dummy_src);
    remove(config->dummy_prog);
    remove("dummy.gcno");
    
    /* Remove gcov-dump coverage files */
    char cmd[MAX_CMD];
    snprintf(cmd, MAX_CMD, "rm -f %s.gcno %s.gcda *.gcov 2>/dev/null", 
             config->gcov_dump_src, config->gcov_dump_src);
    system(cmd);
    
    printf("Cleanup complete.\n");
}

/**
 * Main test program
 */
int main(int argc, char *argv[]) {
    TestConfig config;
    int build_success = 1;
    int test_success = 1;
    
    printf("========================================\n");
    printf("GCOV-DUMP Coverage Test Program\n");
    printf("Target: Lines 111-130 in gcov-dump.cc\n");
    printf("========================================\n");
    
    /* Initialize configuration */
    init_config(&config);
    
    /* Override paths from command line if provided */
    if (argc > 1) {
        strncpy(config.gcov_dump_src, argv[1], MAX_PATH);
    }
    if (argc > 2) {
        strncpy(config.libiberty_path, argv[2], MAX_PATH);
    }
    
    /* Step 1: Build instrumented gcov-dump */
    build_success = build_instrumented_gcov_dump(&config);
    if (!build_success) {
        printf("\nFailed to build instrumented gcov-dump.\n");
        printf("Please ensure:\n");
        printf("1. gcov-dump.cc is in the current directory\n");
        printf("2. GCC development libraries are installed\n");
        printf("3. libiberty.a is available at %s\n", config.libiberty_path);
        return 1;
    }
    
    /* Step 2: Create dummy program for GCOV data */
    build_success = build_dummy_program(&config);
    if (!build_success) {
        printf("\nFailed to create dummy program.\n");
        cleanup(&config);
        return 1;
    }
    
    /* Step 3: Run all flag combination tests */
    test_success = run_all_tests(&config);
    
    /* Step 4: Generate coverage report */
    generate_coverage_report(&config);
    
    /* Step 5: Cleanup */
    cleanup(&config);
    
    printf("\n========================================\n");
    if (test_success) {
        printf("All tests completed successfully!\n");
    } else {
        printf("Some tests failed.\n");
    }
    printf("========================================\n");
    
    return test_success ? 0 : 1;
}
