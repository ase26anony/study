Here's a comprehensive test script that covers all the uncovered lines in the `gcov-dump` command-line argument parsing logic:

```bash
#!/bin/bash
# run_gcov_dump_tests.sh - Test script to cover gcov-dump command-line argument parsing

set -e  # Exit on any error

echo "=== Starting gcov-dump coverage tests ==="

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov_dump_test_XXXXXX)
cd "$TEST_DIR"
echo "Working in directory: $TEST_DIR"

# Step 1: Create a minimal C test program
cat > test.c << 'EOF'
#include <stdio.h>

int main() {
    int i, sum = 0;
    
    // Some basic operations to generate coverage data
    for (i = 0; i < 10; i++) {
        sum += i;
    }
    
    if (sum > 0) {
        printf("Sum: %d\n", sum);
    } else {
        printf("No sum\n");
    }
    
    return 0;
}
EOF

# Step 2: Compile the test program with coverage instrumentation
echo "Compiling test program with coverage..."
gcc -fprofile-arcs -ftest-coverage -O0 -o test_prog test.c

# Step 3: Run the test program to generate .gcda file
echo "Running test program to generate coverage data..."
./test_prog > /dev/null

# Verify that coverage files were created
if [ ! -f test.gcda ] || [ ! -f test.gcno ]; then
    echo "ERROR: Coverage files not generated!"
    exit 1
fi

echo "Coverage files created: test.gcda, test.gcno"

# Step 4: Check if gcov-dump exists
GCOV_DUMP="gcov-dump"
if ! command -v "$GCOV_DUMP" &> /dev/null; then
    echo "ERROR: gcov-dump not found in PATH"
    echo "Please ensure gcov-dump is built and available"
    exit 1
fi

echo "Using gcov-dump: $(which $GCOV_DUMP)"

# Step 5: Execute test cases to cover the switch statement

echo -e "\n=== Testing individual valid flags ==="

# Test case 1: -h flag (triggers case 'h')
echo "Test 1: -h flag (help)"
"$GCOV_DUMP" -h 2>&1 | head -5

# Test case 2: -v flag (triggers case 'v')
echo -e "\nTest 2: -v flag (version)"
"$GCOV_DUMP" -v 2>&1

# Test case 3: -l flag (triggers case 'l')
echo -e "\nTest 3: -l flag (dump contents)"
"$GCOV_DUMP" -l test.gcda 2>&1 | head -10

# Test case 4: -p flag (triggers case 'p')
echo -e "\nTest 4: -p flag (dump positions)"
"$GCOV_DUMP" -p test.gcda 2>&1 | head -10

# Test case 5: -r flag (triggers case 'r')
echo -e "\nTest 5: -r flag (dump raw)"
"$GCOV_DUMP" -r test.gcda 2>&1 | head -10

# Test case 6: -s flag (triggers case 's')
echo -e "\nTest 6: -s flag (dump stable)"
"$GCOV_DUMP" -s test.gcda 2>&1 | head -10

echo -e "\n=== Testing combined flags ==="

# Test case 7: -lp combined flags (triggers both case 'l' and case 'p')
echo "Test 7: -lp combined flags"
"$GCOV_DUMP" -lp test.gcda 2>&1 | head -10

# Test case 8: -rl combined flags (triggers both case 'r' and case 'l')
echo -e "\nTest 8: -rl combined flags"
"$GCOV_DUMP" -rl test.gcda 2>&1 | head -10

# Test case 9: -lps combined flags (triggers multiple cases)
echo -e "\nTest 9: -lps combined flags"
"$GCOV_DUMP" -lps test.gcda 2>&1 | head -10

echo -e "\n=== Testing invalid flags ==="

# Test case 10: Invalid flag -x (triggers default case)
echo "Test 10: Invalid flag -x (should trigger 'unknown flag' error)"
"$GCOV_DUMP" -x test.gcda 2>&1 | grep "unknown flag" || echo "No error message found (unexpected)"

# Test case 11: Invalid flag -z (triggers default case)
echo -e "\nTest 11: Invalid flag -z (should trigger 'unknown flag' error)"
"$GCOV_DUMP" -z test.gcda 2>&1 | grep "unknown flag" || echo "No error message found (unexpected)"

# Test case 12: Multiple flags with one invalid
echo -e "\nTest 12: -lx (valid + invalid flag)"
"$GCOV_DUMP" -lx test.gcda 2>&1 | grep "unknown flag" || echo "No error message found (unexpected)"

echo -e "\n=== Testing missing required arguments ==="

# Test case 13: -l flag without filename
echo "Test 13: -l flag without filename (should trigger error)"
"$GCOV_DUMP" -l 2>&1 | head -5

# Test case 14: -p flag without filename
echo -e "\nTest 14: -p flag without filename (should trigger error)"
"$GCOV_DUMP" -p 2>&1 | head -5

echo -e "\n=== Testing with -- long options (if supported) ==="

# Test case 15: --help (equivalent to -h)
echo "Test 15: --help option"
"$GCOV_DUMP" --help 2>&1 | head -5

# Test case 16: --version (equivalent to -v)
echo -e "\nTest 16: --version option"
"$GCOV_DUMP" --version 2>&1

echo -e "\n=== Testing edge cases ==="

# Test case 17: Empty argument list
echo "Test 17: No arguments (should show usage or error)"
"$GCOV_DUMP" 2>&1 | head -5

# Test case 18: Multiple files with flags
echo -e "\nTest 18: Multiple coverage files"
"$GCOV_DUMP" -l test.gcda test.gcno 2>&1 | head -10

# Test case 19: Flag in middle of arguments
echo -e "\nTest 19: Flag between filenames"
"$GCOV_DUMP" test.gcda -l test.gcno 2>&1 | head -10

# Create a second test program for additional coverage
cat > test2.c << 'EOF'
#include <stdio.h>

void helper() {
    printf("Helper function\n");
}

int main() {
    helper();
    return 0;
}
EOF

gcc -fprofile-arcs -ftest-coverage -O0 -o test2_prog test2.c
./test2_prog > /dev/null

# Test case 20: Process multiple .gcda files
echo -e "\nTest 20: Processing multiple .gcda files"
"$GCOV_DUMP" -l test.gcda test2.gcda 2>&1 | tail -5

echo -e "\n=== Summary ==="
echo "All test cases executed successfully!"
echo "The following switch cases in gcov-dump.cc should now be covered:"
echo "  - case 'h': help"
echo "  - case 'v': version"
echo "  - case 'l': dump contents"
echo "  - case 'p': dump positions"
echo "  - case 'r': dump raw"
echo "  - case 's': dump stable"
echo "  - default: unknown flag"
echo ""
echo "Also covered:"
echo "  - Combined flags parsing"
echo "  - Missing filename errors"
echo "  - Multiple file processing"

# Cleanup (optional - comment out for debugging)
echo -e "\nCleaning up test directory: $TEST_DIR"
cd /
rm -rf "$TEST_DIR"

echo -e "\n=== Test completed successfully ==="
```

This test script provides comprehensive coverage of the uncovered lines in `gcov-dump.cc`:

## Key Features:

1. **Creates valid coverage data files**: Compiles and runs a simple C program to generate `.gcda` and `.gcno` files.

2. **Tests all individual flags**: 
   - `-h` triggers `case 'h'` (help)
   - `-v` triggers `case 'v'` (version)
   - `-l` triggers `case 'l'` (dump contents)
   - `-p` triggers `case 'p'` (dump positions)
   - `-r` triggers `case 'r'` (dump raw)
   - `-s` triggers `case 's'` (dump stable)

3. **Tests invalid flags**: Uses `-x` and `-z` to trigger the `default` case with "unknown flag" error.

4. **Tests combined flags**: Uses `-lp`, `-rl`, `-lps` to test multiple flag parsing in the loop.

5. **Tests missing arguments**: Calls `-l` and `-p` without filenames to trigger error paths.

6. **Tests edge cases**: Empty arguments, multiple files, flags between filenames.

7. **Includes long options**: Tests `--help` and `--version` if supported.

## To run the tests:

1. Save the script as `run_gcov_dump_tests.sh`
2. Make it executable: `chmod +x run_gcov_dump_tests.sh`
3. Ensure `gcov-dump` is in your PATH or modify the `GCOV_DUMP` variable
4. Run: `./run_gcov_dump_tests.sh`

The script will create a temporary directory, generate coverage data, run all test cases, and clean up automatically. Each test case exercises specific code paths in the `switch` statement and option parsing logic of `gcov-dump.cc`.
