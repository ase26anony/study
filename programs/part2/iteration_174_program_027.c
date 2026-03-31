/* test_gcov_dump_coverage.c - Test wrapper to cover gcov-dump switch cases */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024

/* Function to check if file exists */
int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/* Function to execute command and check return code */
int execute_command(const char *cmd, int expect_success) {
    printf("Executing: %s\n", cmd);
    int ret = system(cmd);
    int exit_code = WEXITSTATUS(ret);
    
    if (expect_success) {
        if (exit_code != 0) {
            fprintf(stderr, "Command failed with exit code %d: %s\n", exit_code, cmd);
            return 0;
        }
    } else {
        if (exit_code == 0) {
            fprintf(stderr, "Command expected to fail but succeeded: %s\n", cmd);
            return 0;
        }
    }
    return 1;
}

/* Function to merge coverage data */
void merge_coverage(const char *gcov_dump_path, const char *source_file) {
    char cmd[MAX_PATH * 3];
    
    /* First, find and copy any .gcda files from the gcov-dump directory */
    printf("Merging coverage data...\n");
    
    /* Use gcov tool to generate coverage info */
    snprintf(cmd, sizeof(cmd), "cd %s && gcov -i %s 2>/dev/null || true", 
             gcov_dump_path, source_file);
    system(cmd);
}

int main(int argc, char *argv[]) {
    char cwd[MAX_PATH];
    char gcov_dump_path[MAX_PATH];
    char dummy_gcda_path[MAX_PATH];
    char cmd[MAX_PATH * 4];
    int all_tests_passed = 1;
    
    /* Get current directory */
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return 1;
    }
    
    printf("Current directory: %s\n", cwd);
    
    /* Step 1: Build or locate instrumented gcov-dump */
    printf("\n=== Step 1: Building/Locating instrumented gcov-dump ===\n");
    
    /* Try to find gcov-dump in current directory first */
    const char *gcov_dump_exe = "./gcov-dump-instrumented";
    
    if (!file_exists(gcov_dump_exe)) {
        /* Try to build it from gcov-dump.cc */
        printf("Building instrumented gcov-dump...\n");
        
        /* Check if gcov-dump.cc exists */
        if (!file_exists("gcov-dump.cc")) {
            fprintf(stderr, "Error: gcov-dump.cc not found in current directory\n");
            fprintf(stderr, "Please copy gcov-dump.cc to %s or provide path\n", cwd);
            return 1;
        }
        
        /* Simple compilation command - adjust as needed for your environment */
        const char *build_cmd = "g++ -O0 -fprofile-arcs -ftest-coverage "
                                "-I. -I../../include -I../../libiberty "
                                "gcov-dump.cc ../../libiberty/libiberty.a "
                                "-o gcov-dump-instrumented 2>&1";
        
        if (!execute_command(build_cmd, 1)) {
            fprintf(stderr, "Failed to build gcov-dump. Trying alternative...\n");
            
            /* Try simpler build without extra includes */
            const char *alt_build_cmd = "g++ -O0 -fprofile-arcs -ftest-coverage "
                                        "gcov-dump.cc -o gcov-dump-instrumented 2>&1";
            
            if (!execute_command(alt_build_cmd, 1)) {
                fprintf(stderr, "Cannot build gcov-dump. Exiting.\n");
                return 1;
            }
        }
    }
    
    if (!file_exists(gcov_dump_exe)) {
        fprintf(stderr, "Error: gcov-dump-instrumented not found after build attempt\n");
        return 1;
    }
    
    printf("Found instrumented gcov-dump at: %s\n", gcov_dump_exe);
    
    /* Get absolute path for gcov-dump */
    if (realpath(gcov_dump_exe, gcov_dump_path) == NULL) {
        strncpy(gcov_dump_path, gcov_dump_exe, sizeof(gcov_dump_path));
    }
    
    /* Step 2: Generate test GCOV data file */
    printf("\n=== Step 2: Generating test GCOV data file ===\n");
    
    /* Create dummy.c program */
    FILE *dummy_c = fopen("dummy.c", "w");
    if (!dummy_c) {
        perror("Failed to create dummy.c");
        return 1;
    }
    
    fprintf(dummy_c, "/* dummy.c - Simple program to generate GCOV data */\n");
    fprintf(dummy_c, "#include <stdio.h>\n\n");
    fprintf(dummy_c, "int main() {\n");
    fprintf(dummy_c, "    int i;\n");
    fprintf(dummy_c, "    for (i = 0; i < 10; i++) {\n");
    fprintf(dummy_c, "        printf(\"Iteration %%d\\n\", i);\n");
    fprintf(dummy_c, "    }\n");
    fprintf(dummy_c, "    return 0;\n");
    fprintf(dummy_c, "}\n");
    fclose(dummy_c);
    
    /* Compile dummy.c with coverage */
    printf("Compiling dummy.c with coverage instrumentation...\n");
    if (!execute_command("gcc -O0 -fprofile-arcs -ftest-coverage dummy.c -o dummy_prog", 1)) {
        fprintf(stderr, "Failed to compile dummy.c\n");
        return 1;
    }
    
    /* Run dummy_prog to generate .gcda file */
    printf("Running dummy_prog to generate coverage data...\n");
    if (!execute_command("./dummy_prog", 1)) {
        fprintf(stderr, "Failed to run dummy_prog\n");
        return 1;
    }
    
    /* Check if dummy.gcda was created */
    if (!file_exists("dummy.gcda")) {
        /* Try to find it with full path */
        fprintf(stderr, "dummy.gcda not found in current directory\n");
        fprintf(stderr, "Looking for .gcda files...\n");
        system("find . -name \"*.gcda\" -type f 2>/dev/null");
        return 1;
    }
    
    /* Get absolute path for dummy.gcda */
    if (realpath("dummy.gcda", dummy_gcda_path) == NULL) {
        strncpy(dummy_gcda_path, "dummy.gcda", sizeof(dummy_gcda_path));
    }
    
    printf("Test GCOV data file: %s\n", dummy_gcda_path);
    
    /* Step 3: Execute comprehensive flag coverage tests */
    printf("\n=== Step 3: Executing flag coverage tests ===\n");
    
    /* Clean any existing coverage data for gcov-dump */
    printf("Cleaning existing gcov-dump coverage data...\n");
    snprintf(cmd, sizeof(cmd), "rm -f %s.gcda %s.gcno 2>/dev/null", gcov_dump_path, gcov_dump_path);
    system(cmd);
    
    /* Test 1: Help flag (-h) */
    printf("\n--- Test 1: Help flag (-h) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -h", gcov_dump_path);
    if (!execute_command(cmd, 1)) {
        all_tests_passed = 0;
    }
    merge_coverage(cwd, "gcov-dump.cc");
    
    /* Test 2: Version flag (-v) */
    printf("\n--- Test 2: Version flag (-v) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -v", gcov_dump_path);
    if (!execute_command(cmd, 1)) {
        all_tests_passed = 0;
    }
    merge_coverage(cwd, "gcov-dump.cc");
    
    /* Test 3: Contents flag (-l) with GCOV file */
    printf("\n--- Test 3: Contents flag (-l) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", gcov_dump_path, dummy_gcda_path);
    if (!execute_command(cmd, 1)) {
        all_tests_passed = 0;
    }
    merge_coverage(cwd, "gcov-dump.cc");
    
    /* Test 4: Positions flag (-p) with GCOV file */
    printf("\n--- Test 4: Positions flag (-p) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -p %s", gcov_dump_path, dummy_gcda_path);
    if (!execute_command(cmd, 1)) {
        all_tests_passed = 0;
    }
    merge_coverage(cwd, "gcov-dump.cc");
    
    /* Test 5: Raw flag (-r) with GCOV file */
    printf("\n--- Test 5: Raw flag (-r) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -r %s", gcov_dump_path, dummy_gcda_path);
    if (!execute_command(cmd, 1)) {
        all_tests_passed = 0;
    }
    merge_coverage(cwd, "gcov-dump.cc");
    
    /* Test 6: Stable flag (-s) with GCOV file */
    printf("\n--- Test 6: Stable flag (-s) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -s %s", gcov_dump_path, dummy_gcda_path);
    if (!execute_command(cmd, 1)) {
        all_tests_passed = 0;
    }
    merge_coverage(cwd, "gcov-dump.cc");
    
    /* Test 7: Combined flags (space-separated) */
    printf("\n--- Test 7: Combined flags (space-separated) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -l -p -r -s %s", gcov_dump_path, dummy_gcda_path);
    if (!execute_command(cmd, 1)) {
        all_tests_passed = 0;
    }
    merge_coverage(cwd, "gcov-dump.cc");
    
    /* Test 8: Concatenated flags */
    printf("\n--- Test 8: Concatenated flags (-lprs) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -lprs %s", gcov_dump_path, dummy_gcda_path);
    if (!execute_command(cmd, 1)) {
        all_tests_passed = 0;
    }
    merge_coverage(cwd, "gcov-dump.cc");
    
    /* Test 9: Invalid flag (to trigger default case) */
    printf("\n--- Test 9: Invalid flag (-x) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -x %s 2>&1", gcov_dump_path, dummy_gcda_path);
    
    /* For invalid flag, we expect failure */
    printf("Executing: %s\n", cmd);
    FILE *fp = popen(cmd, "r");
    if (fp) {
        char output[1024];
        int found_error = 0;
        while (fgets(output, sizeof(output), fp)) {
            if (strstr(output, "unknown flag")) {
                printf("Found expected error message: %s", output);
                found_error = 1;
            }
        }
        pclose(fp);
        
        if (!found_error) {
            fprintf(stderr, "Expected 'unknown flag' error not found\n");
            all_tests_passed = 0;
        }
    }
    merge_coverage(cwd, "gcov-dump.cc");
    
    /* Step 4: Generate final coverage report */
    printf("\n=== Step 4: Generating final coverage report ===\n");
    
    /* Generate coverage report for gcov-dump.cc */
    printf("Generating coverage report for gcov-dump.cc...\n");
    
    /* First, make sure we're in the right directory */
    snprintf(cmd, sizeof(cmd), "cd %s && gcov -b gcov-dump.cc 2>&1 | grep -A 20 'Lines executed:'", cwd);
    printf("Running: %s\n", cmd);
    system(cmd);
    
    /* Specifically check for our target lines */
    printf("\nChecking coverage for target lines (111-130)...\n");
    snprintf(cmd, sizeof(cmd), "cd %s && gcov -b gcov-dump.cc 2>&1 | "
             "grep -n '^' | grep -A 5 -B 5 '111\\|112\\|113\\|114\\|115\\|116\\|117\\|118\\|119\\|120\\|"
             "121\\|122\\|123\\|124\\|125\\|126\\|127\\|128\\|129\\|130'", cwd);
    system(cmd);
    
    /* Alternative: Use gcov with line-by-line output */
    printf("\n=== Detailed line coverage for gcov-dump.cc ===\n");
    snprintf(cmd, sizeof(cmd), "cd %s && gcov -l gcov-dump.cc 2>&1 | "
             "grep -E '^\\[|^ *[0-9]+:|^ *[0-9]+\\.' | head -50", cwd);
    system(cmd);
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    printf("Keeping generated files for inspection.\n");
    printf("Files generated:\n");
    printf("  - dummy.c\n");
    printf("  - dummy_prog\n");
    printf("  - dummy.gcda\n");
    printf("  - dummy.gcno\n");
    printf("  - gcov-dump-instrumented\n");
    printf("  - gcov-dump.gcda (coverage data)\n");
    printf("  - gcov-dump.gcno\n");
    printf("  - gcov-dump.cc.gcov (coverage report)\n");
    
    if (all_tests_passed) {
        printf("\n✅ All tests passed! Target lines should now be covered.\n");
        printf("Check gcov-dump.cc.gcov for coverage details.\n");
    } else {
        printf("\n⚠️  Some tests failed. Check coverage report manually.\n");
    }
    
    return all_tests_passed ? 0 : 1;
}
