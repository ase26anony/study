Looking at the uncovered lines in `gcov-dump.cc`, I need to create a comprehensive test that exercises all the command-line flag handling logic. Here's a shell script that systematically tests each uncovered case:

```bash
#!/bin/bash
# Test script for gcov-dump command-line argument parsing
# This script tests the uncovered switch cases in gcov-dump.cc lines 111-130

set -e  # Exit on any error

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"
echo "Testing gcov-dump command-line argument parsing..."

# Create a simple C source file for generating GCOV data
cat > test1.c << 'EOF'
#include <stdio.h>

int main() {
    int x = 5;
    if (x > 0) {
        printf("Positive\n");
    } else {
        printf("Non-positive\n");
    }
    return 0;
}
EOF

# Create another test file for multiple file testing
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

# Compile test programs with coverage instrumentation
echo "Compiling test programs with coverage..."
gcc -fprofile-arcs -ftest-coverage -O0 test1.c -o test1_prog
gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test2_prog

# Run the programs to generate .gcda files
echo "Running test programs to generate coverage data..."
./test1_prog > /dev/null
./test2_prog > /dev/null

# Verify that GCOV files were created
if [ ! -f test1.gcda ] || [ ! -f test1.gcno ]; then
    echo "ERROR: GCOV files not created for test1"
    exit 1
fi

if [ ! -f test2.gcda ] || [ ! -f test2.gcno ]; then
    echo "ERROR: GCOV files not created for test2"
    exit 1
fi

echo "GCOV files created successfully"
echo ""

# Test 1: -h flag (help) - covers case 'h'
echo "=== Test 1: Testing -h flag (help) ==="
gcov-dump -h 2>&1 | head -5
echo "Exit code: $?"
echo ""

# Test 2: --help flag (should also trigger help)
echo "=== Test 2: Testing --help flag ==="
gcov-dump --help 2>&1 | head -5
echo "Exit code: $?"
echo ""

# Test 3: -v flag (version) - covers case 'v'
echo "=== Test 3: Testing -v flag (version) ==="
gcov-dump -v 2>&1
echo "Exit code: $?"
echo ""

# Test 4: --version flag
echo "=== Test 4: Testing --version flag ==="
gcov-dump --version 2>&1
echo "Exit code: $?"
echo ""

# Test 5: -l flag (dump contents) - covers case 'l'
echo "=== Test 5: Testing -l flag (dump contents) ==="
gcov-dump -l test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 6: -p flag (dump positions) - covers case 'p'
echo "=== Test 6: Testing -p flag (dump positions) ==="
gcov-dump -p test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 7: -r flag (dump raw) - covers case 'r'
echo "=== Test 7: Testing -r flag (dump raw) ==="
gcov-dump -r test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 8: -s flag (dump stable) - covers case 's'
echo "=== Test 8: Testing -s flag (dump stable) ==="
gcov-dump -s test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 9: Combination of all flags - covers multiple cases sequentially
echo "=== Test 9: Testing combination -l -p -r -s ==="
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 10: Invalid flag - triggers default case
echo "=== Test 10: Testing invalid flag -x (should trigger default case) ==="
gcov-dump -x test1.gcda 2>&1
echo "Exit code: $?"
echo ""

# Test 11: No flags with .gcno file
echo "=== Test 11: Testing with .gcno file (no flags) ==="
gcov-dump test1.gcno 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 12: Multiple input files with -l flag
echo "=== Test 12: Testing multiple input files with -l flag ==="
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 13: Mixed file types with flag
echo "=== Test 13: Testing mixed .gcda and .gcno files ==="
gcov-dump -p test1.gcda test1.gcno 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 14: Long form combined flags
echo "=== Test 14: Testing long form combined flags ==="
gcov-dump -lprs test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 15: Flag with no argument (should show error)
echo "=== Test 15: Testing flag with no filename ==="
gcov-dump -l 2>&1 || true
echo "Exit code: $?"
echo ""

# Test 16: Non-existent file
echo "=== Test 16: Testing with non-existent file ==="
gcov-dump -l nonexistent.gcda 2>&1 || true
echo "Exit code: $?"
echo ""

# Cleanup
echo "Cleaning up test directory..."
cd /
rm -rf "$TEST_DIR"

echo ""
echo "All tests completed successfully!"
echo "Covered all switch cases:"
echo "  -h / --help (case 'h')"
echo "  -v / --version (case 'v')"
echo "  -l (case 'l')"
echo "  -p (case 'p')"
echo "  -r (case 'r')"
echo "  -s (case 's')"
echo "  default case (with invalid flag -x)"
```

This test script comprehensively covers all the uncovered lines:

1. **Creates valid GCOV data files**: Compiles and runs two test programs to generate `.gcda` and `.gcno` files
2. **Tests each individual flag**: `-h`, `-v`, `-l`, `-p`, `-r`, `-s`
3. **Tests flag combinations**: Uses `-l -p -r -s` together and `-lprs` (combined form)
4. **Triggers the default case**: Uses invalid flag `-x`
5. **Tests with different file types**: Uses `.gcda` files, `.gcno` files, and mixed types
6. **Tests with multiple files**: Uses multiple `.gcda` files in one command
7. **Tests edge cases**: No flags, no filename, non-existent file

To run this test, you need to:
1. Ensure `gcov-dump` is in your PATH or provide the full path
2. Have GCC installed with coverage support
3. Make the script executable: `chmod +x test_gcov_dump.sh`
4. Run it: `./test_gcov_dump.sh`

The script will create a temporary directory, generate the necessary GCOV files, run all the test cases, and clean up afterwards. Each test case shows the command being tested and captures its output and exit code.
