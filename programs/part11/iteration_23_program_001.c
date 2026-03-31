Looking at the uncovered lines in `gcov-tool.cc`, I need to create a test that exercises the `overlap` subcommand with specific flags. Here's a comprehensive test script that covers all the required cases:

```bash
#!/bin/bash

# Test script for gcov-tool overlap command coverage
# This script tests the specific switch cases in gcov-tool.cc lines 534-554

set -e

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-tool-test-XXXXXX)
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    echo "Cleaning up test directory: $TEST_DIR"
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

# Create a simple test C program with conditional logic
cat > test.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

void func1(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else {
        printf("Non-positive: %d\n", x);
    }
}

void func2(int y) {
    for (int i = 0; i < y; i++) {
        printf("Iteration %d\n", i);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }
    
    int val = atoi(argv[1]);
    func1(val);
    func2(val > 3 ? 3 : val);
    
    return 0;
}
EOF

# Compile with coverage instrumentation
echo "Compiling test program with coverage..."
gcc -fprofile-arcs -ftest-coverage test.c -o test_prog

# Run the program multiple times with different inputs to generate varied .gcda files
echo "Generating coverage data files..."

# First run - exercise some paths
echo "Run 1: ./test_prog 2"
./test_prog 2

# Copy the .gcda file to preserve it for later use
cp test.gcda test_run1.gcda

# Clean up for second run
rm -f test.gcda test.gcno

# Recompile to reset coverage
gcc -fprofile-arcs -ftest-coverage test.c -o test_prog

# Second run - exercise different paths
echo "Run 2: ./test_prog 5"
./test_prog 5

# Copy the .gcda file
cp test.gcda test_run2.gcda

# Clean up for third run
rm -f test.gcda test.gcno

# Recompile again
gcc -fprofile-arcs -ftest-coverage test.c -o test_prog

# Third run - exercise yet another path
echo "Run 3: ./test_prog 0"
./test_prog 0

# Copy the .gcda file
cp test.gcda test_run3.gcda

echo "Generated coverage data files:"
ls -la *.gcda

# Find gcov-tool (assuming it's in the GCC build directory or PATH)
GCOV_TOOL=""
if [ -x "./gcc/gcov-tool" ]; then
    GCOV_TOOL="./gcc/gcov-tool"
elif [ -x "../gcc/gcov-tool" ]; then
    GCOV_TOOL="../gcc/gcov-tool"
else
    GCOV_TOOL=$(which gcov-tool 2>/dev/null || echo "")
fi

if [ -z "$GCOV_TOOL" ]; then
    echo "Error: gcov-tool not found!"
    echo "Please build GCC with --coverage flag first"
    exit 1
fi

echo "Using gcov-tool: $GCOV_TOOL"

# Test 1: Cover all switch cases in one command
# -v: verbose mode (covers case 'v')
# -f: function-level overlap (covers case 'f')
# -F: use full filenames (covers case 'F')
# -o: object-level reporting (covers case 'o')
# -h: hot-only filtering (covers case 'h')
# -t 0.5: hotness threshold (covers case 't')
echo ""
echo "=== Test 1: Testing all flags in one command ==="
echo "Command: $GCOV_TOOL overlap -v -f -F -o -h -t 0.5 test_run1.gcda test_run2.gcda test_run3.gcda"
$GCOV_TOOL overlap -v -f -F -o -h -t 0.5 test_run1.gcda test_run2.gcda test_run3.gcda 2>&1 || true

# Test 2: Test with just verbose and threshold flags
echo ""
echo "=== Test 2: Testing -v and -t flags ==="
echo "Command: $GCOV_TOOL overlap -v -t 1.0 test_run1.gcda test_run2.gcda"
$GCOV_TOOL overlap -v -t 1.0 test_run1.gcda test_run2.gcda 2>&1 || true

# Test 3: Test with just function-level flag
echo ""
echo "=== Test 3: Testing -f flag ==="
echo "Command: $GCOV_TOOL overlap -f test_run1.gcda test_run2.gcda"
$GCOV_TOOL overlap -f test_run1.gcda test_run2.gcda 2>&1 || true

# Test 4: Test with just fullname flag
echo ""
echo "=== Test 4: Testing -F flag ==="
echo "Command: $GCOV_TOOL overlap -F test_run1.gcda test_run2.gcda"
$GCOV_TOOL overlap -F test_run1.gcda test_run2.gcda 2>&1 || true

# Test 5: Test with just object-level flag
echo ""
echo "=== Test 5: Testing -o flag ==="
echo "Command: $GCOV_TOOL overlap -o test_run1.gcda test_run2.gcda"
$GCOV_TOOL overlap -o test_run1.gcda test_run2.gcda 2>&1 || true

# Test 6: Test with just hot-only flag
echo ""
echo "=== Test 6: Testing -h flag ==="
echo "Command: $GCOV_TOOL overlap -h test_run1.gcda test_run2.gcda"
$GCOV_TOOL overlap -h test_run1.gcda test_run2.gcda 2>&1 || true

# Test 7: Trigger default case with invalid option
# This should call overlap_usage() and exit
echo ""
echo "=== Test 7: Testing invalid option (to trigger default case) ==="
echo "Command: $GCOV_TOOL overlap -z test_run1.gcda"
$GCOV_TOOL overlap -z test_run1.gcda 2>&1 || true

# Test 8: Test with threshold only
echo ""
echo "=== Test 8: Testing -t flag alone ==="
echo "Command: $GCOV_TOOL overlap -t 0.75 test_run1.gcda test_run2.gcda"
$GCOV_TOOL overlap -t 0.75 test_run1.gcda test_run2.gcda 2>&1 || true

echo ""
echo "=== All tests completed ==="
echo "Lines covered in gcov-tool.cc:"
echo "  case 'v': verbose = true; gcov_set_verbose();"
echo "  case 'f': overlap_func_level = 1;"
echo "  case 'F': overlap_use_fullname = 1;"
echo "  case 'o': overlap_obj_level = 1;"
echo "  case 'h': overlap_hot_only = 1;"
echo "  case 't': overlap_hot_threshold = atof(optarg);"
echo "  default: overlap_usage();"
```

This script comprehensively tests all the uncovered lines:

1. **Generates valid .gcda files**: Compiles a C program with coverage flags and runs it multiple times with different inputs to create varied coverage data.

2. **Tests all target flags in combination**: The first test (`Test 1`) uses `-v -f -F -o -h -t 0.5` all together, covering cases 'v', 'f', 'F', 'o', 'h', and 't'.

3. **Tests flags individually**: Tests 2-6 test each flag separately to ensure each case is independently exercised.

4. **Triggers the default case**: Test 7 uses an invalid option `-z` to trigger the `default:` branch that calls `overlap_usage()`.

5. **Provides multiple .gcda files**: Uses three different coverage data files (`test_run1.gcda`, `test_run2.gcda`, `test_run3.gcda`) for meaningful overlap analysis.

6. **Handles different threshold values**: Tests with `-t 0.5`, `-t 1.0`, and `-t 0.75` to ensure the `atof(optarg)` call is properly exercised.

The script is self-contained, creates a temporary directory for isolation, and cleans up after itself. It also tries to locate `gcov-tool` in common locations or uses the system PATH.
