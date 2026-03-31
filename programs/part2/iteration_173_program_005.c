#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main() {
    int status;
    pid_t pid;
    
    printf("=== Testing gcov-dump command-line parsing ===\n\n");
    
    // First, create a simple test program to generate GCOV data
    printf("1. Creating test program with GCOV instrumentation...\n");
    FILE *fp = fopen("test_coverage.c", "w");
    if (!fp) {
        perror("Failed to create test_coverage.c");
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
    
    // Compile with coverage instrumentation
    printf("2. Compiling test program...\n");
    status = system("gcc -fprofile-arcs -ftest-coverage test_coverage.c -o test_coverage");
    if (status != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return 1;
    }
    
    // Run the program to generate .gcda file
    printf("3. Running test program to generate coverage data...\n");
    status = system("./test_coverage > /dev/null");
    if (status != 0) {
        fprintf(stderr, "Failed to run test program\n");
        return 1;
    }
    
    // Now test gcov-dump with various flags
    printf("\n4. Testing gcov-dump with various flags:\n");
    
    // Array of test cases: {description, arguments, expected_exit_code}
    struct {
        const char *desc;
        const char *args;
        int expected_exit;
    } tests[] = {
        {"Help flag (-h)", "-h", 0},
        {"Version flag (-v)", "-v", 0},
        {"Dump contents (-l)", "-l test_coverage.gcda", 0},
        {"Dump positions (-p)", "-p test_coverage.gcda", 0},
        {"Dump raw (-r)", "-r test_coverage.gcda", 0},
        {"Dump stable (-s)", "-s test_coverage.gcda", 0},
        {"Combined flags separately (-l -p -r -s)", "-l -p -r -s test_coverage.gcda", 0},
        {"Combined flags concatenated (-lprs)", "-lprs test_coverage.gcda", 0},
        {"Invalid flag (-x) - should fail", "-x test_coverage.gcda", 1},
        {"Multiple files with flags", "-l test_coverage.gcda test_coverage.gcno", 0},
        {"Flag with no argument", "-l", 1},  // Should fail without file
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    
    for (int i = 0; i < num_tests; i++) {
        printf("\nTest %d: %s\n", i + 1, tests[i].desc);
        printf("Command: gcov-dump %s\n", tests[i].args);
        
        // Build the command
        char command[1024];
        snprintf(command, sizeof(command), "gcov-dump %s 2>&1", tests[i].args);
        
        // Execute and capture output
        FILE *cmd = popen(command, "r");
        if (!cmd) {
            fprintf(stderr, "Failed to execute command\n");
            continue;
        }
        
        char buffer[1024];
        int has_output = 0;
        while (fgets(buffer, sizeof(buffer), cmd)) {
            if (!has_output) {
                printf("Output:\n");
                has_output = 1;
            }
            printf("  %s", buffer);
        }
        
        // Get exit status
        status = pclose(cmd);
        int exit_code = WEXITSTATUS(status);
        
        // Check result
        if (exit_code == tests[i].expected_exit) {
            printf("✓ PASSED (exit code: %d)\n", exit_code);
            passed++;
        } else {
            printf("✗ FAILED (expected %d, got %d)\n", tests[i].expected_exit, exit_code);
        }
    }
    
    // Cleanup
    printf("\n5. Cleaning up...\n");
    remove("test_coverage.c");
    remove("test_coverage");
    remove("test_coverage.gcda");
    remove("test_coverage.gcno");
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d tests\n", passed, num_tests);
    
    return (passed == num_tests) ? 0 : 1;
}
