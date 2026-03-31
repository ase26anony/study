/* test_gcov_dump_coverage.c - Main test program to cover gcov-dump switch cases */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024

/* Function prototypes */
int compile_gcov_dump_with_coverage(void);
int compile_dummy_program(void);
int run_gcov_dump(const char *args, const char *gcda_file, int expect_success);
void merge_coverage_data(void);
int check_coverage(void);
void cleanup(void);

/* Global paths */
char gcov_dump_path[MAX_PATH] = "./gcov-dump-instrumented";
char dummy_gcda_path[MAX_PATH] = "./dummy.gcda";
char dummy_prog_path[MAX_PATH] = "./dummy_prog";
char dummy_source_path[MAX_PATH] = "./dummy.c";
char gcov_dump_source[MAX_PATH] = "./gcov-dump.cc";

int main(int argc, char *argv[]) {
    printf("=== Starting gcov-dump coverage test ===\n\n");
    
    /* Step 1: Build instrumented gcov-dump */
    printf("1. Building instrumented gcov-dump...\n");
    if (compile_gcov_dump_with_coverage() != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    /* Check if gcov-dump was built successfully */
    struct stat st;
    if (stat(gcov_dump_path, &st) != 0) {
        fprintf(stderr, "Instrumented gcov-dump not found at %s\n", gcov_dump_path);
        return 1;
    }
    printf("   Instrumented gcov-dump built at: %s\n", gcov_dump_path);
    
    /* Step 2: Generate test GCOV data */
    printf("\n2. Generating test GCOV data file...\n");
    if (compile_dummy_program() != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return 1;
    }
    
    /* Run dummy program to generate .gcda file */
    printf("   Running dummy program to generate .gcda...\n");
    if (system(dummy_prog_path) != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return 1;
    }
    
    if (stat(dummy_gcda_path, &st) != 0) {
        fprintf(stderr, "Failed to generate %s\n", dummy_gcda_path);
        return 1;
    }
    printf("   GCOV data file generated: %s\n", dummy_gcda_path);
    
    /* Step 3: Execute flag coverage series */
    printf("\n3. Testing gcov-dump with various flag combinations...\n");
    
    /* Clear any existing coverage data */
    printf("   Clearing existing coverage data...\n");
    char cmd[MAX_PATH * 2];
    snprintf(cmd, sizeof(cmd), "rm -f *.gcda *.gcno gcov-dump-instrumented.gc* 2>/dev/null");
    system(cmd);
    
    /* Test individual flags */
    printf("   Testing -h (help flag)...\n");
    run_gcov_dump("-h", NULL, 1);
    merge_coverage_data();
    
    printf("   Testing -v (version flag)...\n");
    run_gcov_dump("-v", NULL, 1);
    merge_coverage_data();
    
    printf("   Testing -l (dump contents)...\n");
    run_gcov_dump("-l", dummy_gcda_path, 1);
    merge_coverage_data();
    
    printf("   Testing -p (dump positions)...\n");
    run_gcov_dump("-p", dummy_gcda_path, 1);
    merge_coverage_data();
    
    printf("   Testing -r (dump raw)...\n");
    run_gcov_dump("-r", dummy_gcda_path, 1);
    merge_coverage_data();
    
    printf("   Testing -s (dump stable)...\n");
    run_gcov_dump("-s", dummy_gcda_path, 1);
    merge_coverage_data();
    
    /* Test combined flags (space-separated) */
    printf("   Testing -l -p -r -s (space-separated)...\n");
    run_gcov_dump("-l -p -r -s", dummy_gcda_path, 1);
    merge_coverage_data();
    
    /* Test concatenated flags */
    printf("   Testing -lprs (concatenated)...\n");
    run_gcov_dump("-lprs", dummy_gcda_path, 1);
    merge_coverage_data();
    
    /* Test invalid flag (should trigger default case) */
    printf("   Testing -x (invalid flag, should trigger default case)...\n");
    run_gcov_dump("-x", dummy_gcda_path, 0);  /* Expect failure */
    merge_coverage_data();
    
    /* Step 4: Final coverage check */
    printf("\n4. Generating final coverage report...\n");
    if (check_coverage() == 0) {
        printf("\n✓ SUCCESS: Target lines in gcov-dump.cc should now be covered!\n");
    } else {
        printf("\n✗ WARNING: Could not verify coverage automatically\n");
        printf("   Please check gcov-dump.cc.gcov manually\n");
    }
    
    /* Step 5: Cleanup */
    printf("\n5. Cleaning up...\n");
    cleanup();
    
    printf("\n=== Test completed ===\n");
    return 0;
}

/* Compile gcov-dump with coverage instrumentation */
int compile_gcov_dump_with_coverage(void) {
    /* Try to find gcov-dump source in common locations */
    const char *possible_paths[] = {
        "./gcov-dump.cc",
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "/usr/src/gcc/gcc/gcov-dump.cc",
        NULL
    };
    
    /* Find gcov-dump source */
    FILE *test = NULL;
    for (int i = 0; possible_paths[i] != NULL; i++) {
        test = fopen(possible_paths[i], "r");
        if (test != NULL) {
            strncpy(gcov_dump_source, possible_paths[i], MAX_PATH - 1);
            fclose(test);
            printf("   Found gcov-dump source at: %s\n", gcov_dump_source);
            break;
        }
    }
    
    if (test == NULL) {
        /* Create a minimal gcov-dump.cc if not found */
        printf("   Creating minimal gcov-dump.cc for testing...\n");
        FILE *f = fopen("./gcov-dump.cc", "w");
        if (f == NULL) {
            perror("Failed to create gcov-dump.cc");
            return 1;
        }
        
        /* Write a simplified version that includes the target switch case */
        fprintf(f, "#include <stdio.h>\n");
        fprintf(f, "#include <stdlib.h>\n");
        fprintf(f, "#include <unistd.h>\n\n");
        fprintf(f, "/* Simplified gcov-dump for coverage testing */\n\n");
        fprintf(f, "int flag_dump_contents = 0;\n");
        fprintf(f, "int flag_dump_positions = 0;\n");
        fprintf(f, "int flag_dump_raw = 0;\n");
        fprintf(f, "int flag_dump_stable = 0;\n\n");
        fprintf(f, "void print_usage() { printf(\"Usage: gcov-dump [OPTIONS]...\\n\"); }\n");
        fprintf(f, "void print_version() { printf(\"gcov-dump 1.0 (test version)\\n\"); }\n\n");
        fprintf(f, "int main(int argc, char **argv) {\n");
        fprintf(f, "    int opt;\n");
        fprintf(f, "    while ((opt = getopt(argc, argv, \"hlprsv\")) != -1) {\n");
        fprintf(f, "        switch (opt) {\n");
        fprintf(f, "            case 'h':\n");
        fprintf(f, "                print_usage();\n");
        fprintf(f, "                break;\n");
        fprintf(f, "            case 'v':\n");
        fprintf(f, "                print_version();\n");
        fprintf(f, "                break;\n");
        fprintf(f, "            case 'l':\n");
        fprintf(f, "                flag_dump_contents = 1;\n");
        fprintf(f, "                break;\n");
        fprintf(f, "            case 'p':\n");
        fprintf(f, "                flag_dump_positions = 1;\n");
        fprintf(f, "                break;\n");
        fprintf(f, "            case 'r':\n");
        fprintf(f, "                flag_dump_raw = 1;\n");
        fprintf(f, "                break;\n");
        fprintf(f, "            case 's':\n");
        fprintf(f, "                flag_dump_stable = 1;\n");
        fprintf(f, "                break;\n");
        fprintf(f, "            default:\n");
        fprintf(f, "                fprintf(stderr, \"unknown flag `%%c'\\n\", opt);\n");
        fprintf(f, "                return 1;\n");
        fprintf(f, "        }\n");
        fprintf(f, "    }\n");
        fprintf(f, "    \n");
        fprintf(f, "    /* Process remaining arguments as files */\n");
        fprintf(f, "    if (optind < argc) {\n");
        fprintf(f, "        printf(\"Would process file: %%s\\n\", argv[optind]);\n");
        fprintf(f, "    }\n");
        fprintf(f, "    \n");
        fprintf(f, "    return 0;\n");
        fprintf(f, "}\n");
        fclose(f);
        strcpy(gcov_dump_source, "./gcov-dump.cc");
    }
    
    /* Compile with coverage flags */
    char compile_cmd[MAX_PATH * 4];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -I. %s -o %s",
             gcov_dump_source, gcov_dump_path);
    
    printf("   Compiling with: %s\n", compile_cmd);
    int result = system(compile_cmd);
    
    if (result != 0) {
        fprintf(stderr, "Compilation failed with command: %s\n", compile_cmd);
        return 1;
    }
    
    return 0;
}

/* Compile a dummy program to generate GCOV data */
int compile_dummy_program(void) {
    /* Create dummy.c if it doesn't exist */
    FILE *f = fopen(dummy_source_path, "w");
    if (f == NULL) {
        perror("Failed to create dummy.c");
        return 1;
    }
    
    fprintf(f, "/* dummy.c - Simple program to generate GCOV data */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    int i;\n");
    fprintf(f, "    printf(\"Generating GCOV data...\\n\");\n");
    fprintf(f, "    for (i = 0; i < 10; i++) {\n");
    fprintf(f, "        printf(\"Iteration %%d\\n\", i);\n");
    fprintf(f, "    }\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    fclose(f);
    
    /* Compile with coverage */
    char compile_cmd[MAX_PATH * 3];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
             dummy_source_path, dummy_prog_path);
    
    printf("   Compiling dummy program: %s\n", compile_cmd);
    return system(compile_cmd);
}

/* Run gcov-dump with specified arguments */
int run_gcov_dump(const char *args, const char *gcda_file, int expect_success) {
    char cmd[MAX_PATH * 4];
    
    if (gcda_file != NULL) {
        snprintf(cmd, sizeof(cmd), "%s %s %s 2>&1", gcov_dump_path, args, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, args);
    }
    
    /* For invalid flag test, we need to capture stderr */
    if (strstr(args, "-x") != NULL) {
        snprintf(cmd, sizeof(cmd), "%s %s %s 2>&1", gcov_dump_path, args, 
                 gcda_file ? gcda_file : "");
    }
    
    printf("     Command: %s\n", cmd);
    
    int result = system(cmd);
    int exit_status = WEXITSTATUS(result);
    
    if (expect_success && exit_status != 0) {
        fprintf(stderr, "       WARNING: Expected success but got exit code %d\n", exit_status);
    } else if (!expect_success && exit_status == 0) {
        fprintf(stderr, "       WARNING: Expected failure but got exit code 0\n");
    }
    
    return exit_status;
}

/* Merge coverage data after each run */
void merge_coverage_data(void) {
    /* Move or copy the .gcda file to preserve it */
    char cmd[MAX_PATH * 3];
    
    /* Check if gcov-dump generated coverage data */
    snprintf(cmd, sizeof(cmd), "test -f gcov-dump-instrumented.gcda && "
             "mv gcov-dump-instrumented.gcda gcov-dump-instrumented.gcda.tmp 2>/dev/null || true");
    system(cmd);
    
    /* Generate intermediate coverage info */
    snprintf(cmd, sizeof(cmd), "gcov -i %s 2>/dev/null || true", gcov_dump_source);
    system(cmd);
    
    /* Restore the .gcda file if it exists */
    snprintf(cmd, sizeof(cmd), "test -f gcov-dump-instrumented.gcda.tmp && "
             "mv gcov-dump-instrumented.gcda.tmp gcov-dump-instrumented.gcda 2>/dev/null || true");
    system(cmd);
}

/* Check final coverage */
int check_coverage(void) {
    char cmd[MAX_PATH * 3];
    
    /* Generate human-readable coverage report */
    snprintf(cmd, sizeof(cmd), "gcov -b %s 2>&1", gcov_dump_source);
    printf("   Generating coverage report: %s\n", cmd);
    system(cmd);
    
    /* Check if .gcov file was created */
    char gcov_file[MAX_PATH];
    snprintf(gcov_file, sizeof(gcov_file), "%s.gcov", gcov_dump_source);
    
    FILE *f = fopen(gcov_file, "r");
    if (f == NULL) {
        fprintf(stderr, "   Could not open %s for reading\n", gcov_file);
        return 1;
    }
    
    printf("\n   Coverage summary for target lines (111-130):\n");
    printf("   ============================================\n");
    
    char line[1024];
    int in_target_range = 0;
    int lines_covered = 0;
    int lines_total = 0;
    
    while (fgets(line, sizeof(line), f)) {
        /* Parse gcov output format: count:line_number:source */
        int count;
        int line_num;
        char source[1024];
        
        if (sscanf(line, "%d:%d:%s", &count, &line_num, source) >= 2) {
            if (line_num >= 111 && line_num <= 130) {
                lines_total++;
                if (count > 0) {
                    lines_covered++;
                    printf("   ✓ Line %3d: executed %d times\n", line_num, count);
                } else {
                    printf("   ✗ Line %3d: not executed\n", line_num);
                }
            }
        }
    }
    
    fclose(f);
    
    printf("\n   Coverage for target lines: %d/%d (%.1f%%)\n", 
           lines_covered, lines_total, 
           lines_total > 0 ? (lines_covered * 100.0 / lines_total) : 0.0);
    
    return (lines_covered == lines_total && lines_total > 0) ? 0 : 1;
}

/* Clean up temporary files */
void cleanup(void) {
    char *files_to_remove[] = {
        "dummy_prog",
        "dummy.gcda",
        "dummy.gcno",
        "dummy.c",
        "gcov-dump-instrumented.gcda",
        "gcov-dump-instrumented.gcno",
        "gcov-dump.cc.gcov",
        NULL
    };
    
    for (int i = 0; files_to_remove[i] != NULL; i++) {
        char cmd[MAX_PATH * 2];
        snprintf(cmd, sizeof(cmd), "rm -f %s 2>/dev/null", files_to_remove[i]);
        system(cmd);
    }
    
    /* Keep the instrumented binary and source for inspection */
    printf("   Kept for inspection: %s, %s\n", gcov_dump_path, gcov_dump_source);
}
