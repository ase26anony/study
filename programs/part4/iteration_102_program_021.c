Here's a comprehensive test script that covers all the uncovered lines in the `gcov-dump.cc` switch statement:

```bash
#!/bin/bash
# run_gcov_dump_tests.sh - Test script to cover gcov-dump command-line parsing

set -e  # Exit on any error

echo "=== Starting gcov-dump command-line parsing tests ==="

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
        printf("Sum is %d\n", sum);
    } else {
        printf("Sum is zero\n");
    }
    
    return 0;
}
EOF

# Step 2: Compile the test program with coverage instrumentation
echo "Compiling test program with coverage instrumentation..."
gcc -fprofile-arcs -ftest-coverage -O0 -o test_prog test.c

# Step 3: Run the test program to generate .gcda file
echo "Running test program to generate coverage data..."
./test_prog > /dev/null

# Verify that coverage files were created
if [[ ! -f test.gcda ]]; then
    echo "ERROR: test.gcda file not created!"
    exit 1
fi

if [[ ! -f test.gcno ]]; then
    echo "ERROR: test.gcno file not created!"
    exit 1
fi

echo "Coverage files created successfully: test.gcda, test.gcno"

# Step 4: Find gcov-dump executable
# Try to find gcov-dump in common locations
GCOV_DUMP=""
for path in gcov-dump /usr/bin/gcov-dump /usr/local/bin/gcov-dump; do
    if command -v "$path" > /dev/null 2>&1; then
        GCOV_DUMP="$path"
        break
    fi
done

if [[ -z "$GCOV_DUMP" ]]; then
    # Try to build gcov-dump from current directory (if we're in GCC source tree)
    if [[ -f "gcov-dump.cc" ]]; then
        echo "Building gcov-dump from source..."
        g++ -o gcov-dump gcov-dump.cc
        GCOV_DUMP="./gcov-dump"
    else
        echo "ERROR: gcov-dump not found in PATH and not in current directory"
        echo "Please install gcov-dump or build it from GCC sources"
        exit 1
    fi
fi

echo "Using gcov-dump: $GCOV_DUMP"

# Step 5: Execute test cases to cover all switch cases

echo -e "\n=== Test Case 1: -h flag (help) ==="
echo "Testing: $GCOV_DUMP -h"
$GCOV_DUMP -h 2>&1 | head -5  # Show first 5 lines of help

echo -e "\n=== Test Case 2: -v flag (version) ==="
echo "Testing: $GCOV_DUMP -v"
$GCOV_DUMP -v 2>&1

echo -e "\n=== Test Case 3: -l flag (dump contents) ==="
echo "Testing: $GCOV_DUMP -l test.gcda"
$GCOV_DUMP -l test.gcda 2>&1 | head -10  # Show first 10 lines

echo -e "\n=== Test Case 4: -p flag (dump positions) ==="
echo "Testing: $GCOV_DUMP -p test.gcda"
$GCOV_DUMP -p test.gcda 2>&1 | head -10

echo -e "\n=== Test Case 5: -r flag (dump raw) ==="
echo "Testing: $GCOV_DUMP -r test.gcda"
$GCOV_DUMP -r test.gcda 2>&1 | head -10

echo -e "\n=== Test Case 6: -s flag (dump stable) ==="
echo "Testing: $GCOV_DUMP -s test.gcda"
$GCOV_DUMP -s test.gcda 2>&1 | head -10

echo -e "\n=== Test Case 7: Combined flags -lp ==="
echo "Testing: $GCOV_DUMP -lp test.gcda"
$GCOV_DUMP -lp test.gcda 2>&1 | head -10

echo -e "\n=== Test Case 8: Combined flags -rl ==="
echo "Testing: $GCOV_DUMP -rl test.gcda"
$GCOV_DUMP -rl test.gcda 2>&1 | head -10

echo -e "\n=== Test Case 9: Combined flags -spr ==="
echo "Testing: $GCOV_DUMP -spr test.gcda"
$GCOV_DUMP -spr test.gcda 2>&1 | head -10

echo -e "\n=== Test Case 10: Invalid flag -x (trigger default case) ==="
echo "Testing: $GCOV_DUMP -x test.gcda"
$GCOV_DUMP -x test.gcda 2>&1 || true  # Continue even if command fails

echo -e "\n=== Test Case 11: Another invalid flag -z ==="
echo "Testing: $GCOV_DUMP -z test.gcda"
$GCOV_DUMP -z test.gcda 2>&1 || true

echo -e "\n=== Test Case 12: Multiple invalid flags -xyz ==="
echo "Testing: $GCOV_DUMP -xyz test.gcda"
$GCOV_DUMP -xyz test.gcda 2>&1 || true

echo -e "\n=== Test Case 13: Missing filename with -l flag ==="
echo "Testing: $GCOV_DUMP -l"
$GCOV_DUMP -l 2>&1 || true

echo -e "\n=== Test Case 14: Missing filename with -p flag ==="
echo "Testing: $GCOV_DUMP -p"
$GCOV_DUMP -p 2>&1 || true

echo -e "\n=== Test Case 15: Test with .gcno file instead of .gcda ==="
echo "Testing: $GCOV_DUMP -l test.gcno"
$GCOV_DUMP -l test.gcno 2>&1 | head -10

echo -e "\n=== Test Case 16: Test with both file types ==="
echo "Testing: $GCOV_DUMP -l test.gcda test.gcno"
$GCOV_DUMP -l test.gcda test.gcno 2>&1 | head -10

echo -e "\n=== Test Case 17: Test with no flags (should show usage or error) ==="
echo "Testing: $GCOV_DUMP"
$GCOV_DUMP 2>&1 || true

echo -e "\n=== Test Case 18: Test with --help (long option if supported) ==="
echo "Testing: $GCOV_DUMP --help"
$GCOV_DUMP --help 2>&1 | head -5 || true

echo -e "\n=== Test Case 19: Test with --version (long option if supported) ==="
echo "Testing: $GCOV_DUMP --version"
$GCOV_DUMP --version 2>&1 || true

echo -e "\n=== Test Case 20: Test with flag and non-existent file ==="
echo "Testing: $GCOV_DUMP -l nonexistent.gcda"
$GCOV_DUMP -l nonexistent.gcda 2>&1 || true

# Step 6: Create a summary of what was tested
echo -e "\n=== Test Summary ==="
echo "The following switch cases in gcov-dump.cc were exercised:"
echo "  -h: Help/usage information"
echo "  -v: Version information"
echo "  -l: Dump contents flag"
echo "  -p: Dump positions flag"
echo "  -r: Dump raw flag"
echo "  -s: Dump stable flag"
echo "  default: Invalid flags (-x, -z, -xyz)"
echo ""
echo "Additional test scenarios:"
echo "  - Combined flags (-lp, -rl, -spr)"
echo "  - Missing required filename arguments"
echo "  - Different file types (.gcda and .gcno)"
echo "  - Non-existent files"
echo "  - Long options (--help, --version) if supported"

# Step 7: Cleanup (optional - comment out to keep files for inspection)
echo -e "\n=== Cleaning up ==="
cd ..
rm -rf "$TEST_DIR"
echo "Test directory removed: $TEST_DIR"

echo -e "\n=== All tests completed successfully! ==="
```

This test script provides comprehensive coverage of the uncovered switch statement in `gcov-dump.cc`:

1. **Valid single-character flags**: Tests each case (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) individually
2. **Invalid flags**: Tests `-x`, `-z`, and `-xyz` to trigger the `default` case
3. **Combined flags**: Tests `-lp`, `-rl`, `-spr` to ensure the option parsing loop processes each character
4. **Missing arguments**: Tests `-l` and `-p` without filenames
5. **File handling**: Tests with both `.gcda` and `.gcno` files, and with non-existent files
6. **Long options**: Tests `--help` and `--version` if supported
7. **Edge cases**: Tests with no arguments, multiple files

The script creates a minimal C program, compiles it with coverage instrumentation, runs it to generate coverage data, then systematically tests all the command-line options. Each test case redirects output to limit verbosity while ensuring all code paths are executed.
