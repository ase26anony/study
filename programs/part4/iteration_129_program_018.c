Here's a comprehensive shell script that covers all the specified requirements for testing the uncovered lines in `gcov-dump.cc`:

```bash
#!/bin/bash

# test_gcov_dump.sh - Comprehensive test for gcov-dump option parsing

set -e  # Exit on any command failure

echo "=== Starting gcov-dump option parsing test ==="

# Create a temporary directory for our test
TEST_DIR=$(mktemp -d /tmp/gcov_dump_test_XXXXXX)
echo "Created test directory: $TEST_DIR"
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    cd /
    rm -rf "$TEST_DIR"
    echo "Test directory removed."
}
trap cleanup EXIT

# 1. Create a minimal C test program
cat > test.c << 'EOF'
#include <stdio.h>

int main() {
    printf("Hello from test program for gcov-dump\n");
    return 0;
}
EOF

echo "Created test.c source file"

# 2. Compile with coverage instrumentation
echo "Compiling with coverage instrumentation..."
if ! gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test.exe 2>&1; then
    echo "ERROR: Failed to compile test program"
    exit 1
fi

# 3. Run the program to generate .gcda file
echo "Running test program to generate coverage data..."
if ! ./test.exe 2>&1; then
    echo "ERROR: Failed to run test program"
    exit 1
fi

# Verify .gcda file was created
if [ ! -f "test.gcda" ]; then
    echo "ERROR: test.gcda file not created"
    exit 1
fi
echo "Generated test.gcda file"

echo ""
echo "=== Testing gcov-dump with various options ==="
echo ""

# 4. Test each individual switch case

# Case 'h': Print usage
echo "1. Testing -h flag (print usage):"
gcov-dump -h 2>&1 | head -5
echo "---"

# Case 'v': Print version
echo "2. Testing -v flag (print version):"
gcov-dump -v 2>&1
echo "---"

# Case 'l': Dump contents
echo "3. Testing -l flag (dump contents):"
gcov-dump -l test.gcda 2>&1 | head -10
echo "---"

# Case 'p': Dump positions
echo "4. Testing -p flag (dump positions):"
gcov-dump -p test.gcda 2>&1 | head -10
echo "---"

# Case 'r': Dump raw
echo "5. Testing -r flag (dump raw):"
gcov-dump -r test.gcda 2>&1 | head -10
echo "---"

# Case 's': Dump stable
echo "6. Testing -s flag (dump stable):"
gcov-dump -s test.gcda 2>&1 | head -10
echo "---"

# 5. Test combination of multiple flags
echo "7. Testing combination of -l, -p, -s flags:"
gcov-dump -l -p -s test.gcda 2>&1 | head -10
echo "---"

# 6. Test invalid flag to trigger default case
echo "8. Testing invalid flag -x (should trigger default case):"
# We expect this to fail, so we don't use set -e for this command
if gcov-dump -x test.gcda 2>&1; then
    echo "WARNING: Invalid flag -x did not produce expected error"
else
    echo "Got expected error for invalid flag"
fi
echo "---"

# Test another invalid flag
echo "9. Testing invalid flag -Z (another default case trigger):"
if gcov-dump -Z test.gcda 2>&1; then
    echo "WARNING: Invalid flag -Z did not produce expected error"
else
    echo "Got expected error for invalid flag"
fi
echo "---"

# 7. Additional edge cases

# Test with no arguments (should show usage or error)
echo "10. Testing with no arguments:"
gcov-dump 2>&1 | head -5
echo "---"

# Test with flag but no filename
echo "11. Testing -l flag without filename:"
gcov-dump -l 2>&1 | head -5
echo "---"

# Test with multiple files
echo "12. Testing with multiple .gcda files:"
# Create another test file
cat > test2.c << 'EOF'
#include <stdio.h>
void helper() { printf("helper\n"); }
int main() { helper(); return 0; }
EOF
gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test2.exe
./test2.exe
gcov-dump -l test.gcda test2.gcda 2>&1 | head -5
echo "---"

# Test with non-existent file
echo "13. Testing with non-existent file:"
gcov-dump -l nonexistent.gcda 2>&1 | head -5
echo "---"

echo ""
echo "=== Test Summary ==="
echo "All gcov-dump option parsing tests completed successfully!"
echo "Covered all switch cases:"
echo "  -h (usage), -v (version), -l (contents), -p (positions)"
echo "  -r (raw), -s (stable), default case (invalid flags)"
echo "Also tested: flag combinations, edge cases"
echo ""
echo "SUCCESS: All required test cases executed"
```

This script provides comprehensive coverage of the uncovered lines in `gcov-dump.cc`:

1. **Covers all individual switch cases**: Each flag (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) is tested separately
2. **Triggers the default case**: Tests with invalid flags `-x` and `-Z` to trigger the "unknown flag" error
3. **Tests multiple flag combinations**: Uses `-l -p -s` together to ensure flag variables are set correctly
4. **Creates valid GCOV input**: Compiles and runs a test program to generate `.gcda` files
5. **Handles edge cases**: Tests no arguments, missing filenames, multiple files, and non-existent files
6. **Proper output handling**: Captures and displays output from each test case
7. **Clean execution**: Uses temporary directory and cleanup trap for isolation

The script will exit with a non-zero code only if compilation or execution of the test program fails, not when `gcov-dump` correctly reports errors for invalid flags or missing files.
