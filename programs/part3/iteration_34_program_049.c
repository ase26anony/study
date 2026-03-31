Here's a shell script that generates a test program, compiles it with coverage, and invokes `gcov-dump` with invalid flags to trigger the uncovered lines:

```bash
#!/bin/bash

# Test script for gcov-dump uncovered lines (111-130)
# Specifically tests the default case for unknown command-line flags

set -e

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

# Create a minimal C program
cat > test.c << 'EOF'
#include <stdio.h>

int main(int argc, char *argv[]) {
    int result = argc * 2;
    printf("Result: %d\n", result);
    return result;
}
EOF

echo "1. Compiling test program with coverage flags..."
gcc -O0 -fprofile-arcs -ftest-coverage -o test test.c

echo "2. Running test program to generate .gcda file..."
./test

echo "3. Testing gcov-dump with valid and invalid flags (single invalid flag)..."
# This should trigger the default case for '-x'
echo "Command: gcov-dump -l -p -x test.gcda"
if gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag error for '-x'"
    gcov-dump -l -p -x test.gcda 2>&1 | grep "unknown flag"
else
    echo "✗ Failed to trigger unknown flag error"
    gcov-dump -l -p -x test.gcda 2>&1
fi

echo -e "\n4. Testing gcov-dump with multiple invalid flags..."
# This should trigger the default case for both '-y' and '-z'
echo "Command: gcov-dump -r -s -y -z test.gcda"
if gcov-dump -r -s -y -z test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag errors for '-y' and '-z'"
    gcov-dump -r -s -y -z test.gcda 2>&1 | grep "unknown flag"
else
    echo "✗ Failed to trigger unknown flag errors"
    gcov-dump -r -s -y -z test.gcda 2>&1
fi

echo -e "\n5. Testing gcov-dump with only invalid flags..."
# This should trigger the default case for '-a', '-b', and '-c'
echo "Command: gcov-dump -a -b -c test.gcda"
if gcov-dump -a -b -c test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag errors for '-a', '-b', and '-c'"
    gcov-dump -a -b -c test.gcda 2>&1 | grep "unknown flag"
else
    echo "✗ Failed to trigger unknown flag errors"
    gcov-dump -a -b -c test.gcda 2>&1
fi

echo -e "\n6. Testing gcov-dump with invalid flag at different positions..."
# Test invalid flag at beginning
echo "Command: gcov-dump -q -l -p test.gcda"
gcov-dump -q -l -p test.gcda 2>&1 | grep "unknown flag" || true

# Test invalid flag in middle
echo "Command: gcov-dump -l -? -p test.gcda"
gcov-dump -l -? -p test.gcda 2>&1 | grep "unknown flag" || true

# Test invalid flag at end
echo "Command: gcov-dump -l -p -! test.gcda"
gcov-dump -l -p -! test.gcda 2>&1 | grep "unknown flag" || true

echo -e "\n7. Testing with .gcno file as well..."
# Also test with .gcno file
echo "Command: gcov-dump -l -p -x test.gcno"
gcov-dump -l -p -x test.gcno 2>&1 | grep "unknown flag" || true

echo -e "\n8. Verifying valid flags still work..."
# Quick check that valid flags work normally
echo "Command: gcov-dump -h"
gcov-dump -h 2>&1 | head -5

echo -e "\n9. Testing edge case: uppercase invalid flag..."
# Test uppercase letter (not in switch)
echo "Command: gcov-dump -L -P -X test.gcda"
gcov-dump -L -P -X test.gcda 2>&1 | grep "unknown flag" || true

echo -e "\nTest completed. Cleaning up..."
cd ..
rm -rf "$TEST_DIR"

echo "All tests executed. The uncovered lines should have been triggered by:"
echo "  - Invalid flags: x, y, z, a, b, c, q, ?, !, L, P, X"
echo "  - Mixed with valid flags: l, p, r, s"
echo "  - With both .gcda and .gcno files"
```

This script comprehensively tests the uncovered lines by:

1. **Creating a valid GCOV input file**: Compiles a simple C program with coverage flags and runs it to generate `.gcda` and `.gcno` files.

2. **Using invalid single-character flags**: Tests `-x`, `-y`, `-z`, `-a`, `-b`, `-c`, `-q`, `-?`, `-!`, `-L`, `-P`, `-X` (all not in the switch statement).

3. **Combining valid and invalid flags**: Uses combinations like `-l -p -x` to ensure the parser reaches the default case after processing valid flags.

4. **Multiple invalid flags**: Tests `-y -z` and `-a -b -c` to trigger the error message multiple times.

5. **Different flag positions**: Tests invalid flags at beginning, middle, and end of argument list.

6. **Both file types**: Tests with both `.gcda` and `.gcno` files.

7. **Edge cases**: Tests uppercase letters and special characters as invalid flags.

The script captures stderr output and checks for the "unknown flag" message to confirm the uncovered lines were executed. Each test case is designed to ensure the `default` case in the switch statement (lines 111-130) is triggered, covering the `fprintf(stderr, "unknown flag `%c'\n", opt);` line.
