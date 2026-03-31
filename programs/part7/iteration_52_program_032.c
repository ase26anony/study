I'll create a comprehensive test script to cover the uncovered lines in `gcov-dump.cc`. This script will generate GCOV data files and test all the command-line options.

```bash
#!/bin/bash
# Test script for gcov-dump to cover the uncovered lines in gcov-dump.cc
# Specifically targeting lines 111-130 in the switch statement

set -e

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test-XXXXXX)
echo "Test directory: $TEST_DIR"
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    cd /
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

# Create a simple C source file to generate coverage data
cat > test1.c << 'EOF'
#include <stdio.h>

int main() {
    int x = 5;
    int y = 10;
    
    if (x < y) {
        printf("x is less than y\n");
    } else {
        printf("x is not less than y\n");
    }
    
    for (int i = 0; i < 3; i++) {
        printf("Loop iteration %d\n", i);
    }
    
    return 0;
}
EOF

# Create another test file for multiple file testing
cat > test2.c << 'EOF'
#include <stdio.h>

void helper_function() {
    printf("Helper function called\n");
}

int main() {
    printf("Test program 2\n");
    helper_function();
    return 0;
}
EOF

# Compile test programs with coverage instrumentation
echo "Compiling test programs with coverage instrumentation..."
gcc -fprofile-arcs -ftest-coverage -O0 -g test1.c -o test1_prog
gcc -fprofile-arcs -ftest-coverage -O0 -g test2.c -o test2_prog

# Run the programs to generate .gcda files
echo "Running test programs to generate coverage data..."
./test1_prog
./test2_prog

# Verify that .gcno and .gcda files were created
echo "Checking generated files..."
ls -la *.gcno *.gcda

# Test 1: Help flag (-h)
echo -e "\n=== Test 1: Help flag (-h) ==="
gcov-dump -h 2>&1 | head -5

# Test 2: Help flag (--help)
echo -e "\n=== Test 2: Help flag (--help) ==="
gcov-dump --help 2>&1 | head -5

# Test 3: Version flag (-v)
echo -e "\n=== Test 3: Version flag (-v) ==="
gcov-dump -v

# Test 4: Version flag (--version)
echo -e "\n=== Test 4: Version flag (--version) ==="
gcov-dump --version

# Test 5: Long dump flag (-l) - covers line 111-130 case 'l'
echo -e "\n=== Test 5: Long dump flag (-l) ==="
gcov-dump -l test1.gcda 2>&1 | head -20

# Test 6: Positions dump flag (-p) - covers line 111-130 case 'p'
echo -e "\n=== Test 6: Positions dump flag (-p) ==="
gcov-dump -p test1.gcda 2>&1 | head -20

# Test 7: Raw dump flag (-r) - covers line 111-130 case 'r'
echo -e "\n=== Test 7: Raw dump flag (-r) ==="
gcov-dump -r test1.gcda 2>&1 | head -20

# Test 8: Stable dump flag (-s) - covers line 111-130 case 's'
echo -e "\n=== Test 8: Stable dump flag (-s) ==="
gcov-dump -s test1.gcda 2>&1 | head -20

# Test 9: Multiple flags combined - covers multiple cases
echo -e "\n=== Test 9: Multiple flags combined (-l -p -r -s) ==="
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -30

# Test 10: Invalid flag (-x) - covers default case
echo -e "\n=== Test 10: Invalid flag (-x) - should trigger default case ==="
gcov-dump -x test1.gcda 2>&1 || true

# Test 11: No flags (default behavior)
echo -e "\n=== Test 11: No flags (default behavior) ==="
gcov-dump test1.gcda 2>&1 | head -20

# Test 12: Process .gcno file instead of .gcda
echo -e "\n=== Test 12: Process .gcno file ==="
gcov-dump -l test1.gcno 2>&1 | head -20

# Test 13: Multiple input files
echo -e "\n=== Test 13: Multiple input files ==="
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -30

# Test 14: Different flag order
echo -e "\n=== Test 14: Different flag order ==="
gcov-dump -s -r -p -l test1.gcda 2>&1 | head -30

# Test 15: Test with both .gcda and .gcno files
echo -e "\n=== Test 15: Both .gcda and .gcno files ==="
gcov-dump -l test1.gcda test1.gcno 2>&1 | head -30

# Test 16: Test with non-existent file
echo -e "\n=== Test 16: Non-existent file ==="
gcov-dump -l nonexistent.gcda 2>&1 || true

# Test 17: Empty flags (just program name)
echo -e "\n=== Test 17: No arguments at all ==="
gcov-dump 2>&1 || true

# Test 18: Flag with no file argument
echo -e "\n=== Test 18: Flag with no file argument ==="
gcov-dump -l 2>&1 || true

# Test 19: Mix valid and invalid flags
echo -e "\n=== Test 19: Mix valid and invalid flags ==="
gcov-dump -l -x -p test1.gcda 2>&1 || true

# Test 20: Test all flags on .gcno file
echo -e "\n=== Test 20: All flags on .gcno file ==="
gcov-dump -l -p -r -s test1.gcno 2>&1 | head -30

echo -e "\n=== All tests completed ==="
echo "Summary of tests executed:"
echo "1. -h flag (help)"
echo "2. --help flag"
echo "3. -v flag (version)"
echo "4. --version flag"
echo "5. -l flag (long dump)"
echo "6. -p flag (positions dump)"
echo "7. -r flag (raw dump)"
echo "8. -s flag (stable dump)"
echo "9. Multiple flags combined"
echo "10. Invalid flag (-x) to trigger default case"
echo "11. No flags (default behavior)"
echo "12. Process .gcno file"
echo "13. Multiple input files"
echo "14. Different flag order"
echo "15. Both .gcda and .gcno files"
echo "16. Non-existent file"
echo "17. No arguments"
echo "18. Flag with no file"
echo "19. Mix valid and invalid flags"
echo "20. All flags on .gcno file"

echo -e "\nAll tests have been executed. The uncovered lines in gcov-dump.cc should now be covered."
```

This test script comprehensively covers all the cases in the switch statement:

1. **`case 'h':`** - Covered by tests 1 and 2 (`-h` and `--help`)
2. **`case 'v':`** - Covered by tests 3 and 4 (`-v` and `--version`)
3. **`case 'l':`** - Covered by tests 5, 9, 13, 14, 15, 18, 19, 20
4. **`case 'p':`** - Covered by tests 6, 9, 14, 19, 20
5. **`case 'r':`** - Covered by tests 7, 9, 14, 20
6. **`case 's':`** - Covered by tests 8, 9, 14, 20
7. **`default:`** - Covered by tests 10 and 19 (invalid flag `-x`)

The script also tests various combinations and edge cases to ensure robust coverage. Each test invocation of `gcov-dump` with the specific flags will execute the corresponding code paths in the switch statement, ensuring that the uncovered lines are exercised.
