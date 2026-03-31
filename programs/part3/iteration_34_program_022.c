Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that triggers the default case when an invalid single-character flag is provided. Here's a comprehensive shell script that meets all the requirements:

```bash
#!/bin/bash

# Test script for gcov-dump to trigger uncovered lines 111-130
# Specifically tests the default case for unknown command-line flags

set -e

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test-XXXXXX)
cd "$TEST_DIR"

echo "=== Creating test C program ==="

# Create a minimal C program that will generate coverage data
cat > test.c << 'EOF'
#include <stdio.h>

int main(int argc, char *argv[]) {
    int result = argc * 2;
    printf("Result: %d\n", result);
    return result;
}
EOF

echo "=== Compiling with coverage flags ==="
# Compile with coverage enabled
gcc -O0 -fprofile-arcs -ftest-coverage -o test test.c

echo "=== Running program to generate .gcda file ==="
# Execute to generate coverage data
./test
./test arg1 arg2  # Run with arguments to generate different coverage

echo "=== Testing gcov-dump with invalid flags ==="

# Test 1: Single invalid flag with valid input file
echo "Test 1: Single invalid flag '-x'"
if gcov-dump -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag error for '-x'"
else
    echo "✗ Failed to trigger unknown flag error for '-x'"
    exit 1
fi

# Test 2: Combination of valid and invalid flags
echo -e "\nTest 2: Valid flags '-l -p' with invalid flag '-x'"
if gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag error in combination"
else
    echo "✗ Failed to trigger unknown flag error in combination"
    exit 1
fi

# Test 3: Multiple invalid flags
echo -e "\nTest 3: Multiple invalid flags '-y -z'"
output=$(gcov-dump -y -z test.gcda 2>&1)
if echo "$output" | grep -q "unknown flag 'y'" && \
   echo "$output" | grep -q "unknown flag 'z'"; then
    echo "✓ Successfully triggered multiple unknown flag errors"
else
    echo "✗ Failed to trigger multiple unknown flag errors"
    exit 1
fi

# Test 4: Invalid flag with other valid flags
echo -e "\nTest 4: Mixed valid and invalid flags '-r -s -?'"
if gcov-dump -r -s -\? test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag error for '-?'"
else
    echo "✗ Failed to trigger unknown flag error for '-?'"
    exit 1
fi

# Test 5: Invalid flag at different positions
echo -e "\nTest 5: Invalid flag at the end '-l -p -a'"
if gcov-dump -l -p -a test.gcda 2>&1 | grep -q "unknown flag 'a'"; then
    echo "✓ Successfully triggered unknown flag error at end position"
else
    echo "✗ Failed to trigger unknown flag error at end position"
    exit 1
fi

# Test 6: Test with .gcno file as well
echo -e "\nTest 6: Testing with .gcno file and invalid flag"
if gcov-dump -b test.gcno 2>&1 | grep -q "unknown flag 'b'"; then
    echo "✓ Successfully triggered unknown flag error with .gcno file"
else
    echo "✗ Failed to trigger unknown flag error with .gcno file"
    exit 1
fi

# Test 7: Edge case - uppercase invalid flag
echo -e "\nTest 7: Uppercase invalid flag '-X'"
if gcov-dump -X test.gcda 2>&1 | grep -q "unknown flag 'X'"; then
    echo "✓ Successfully triggered unknown flag error for uppercase"
else
    echo "✗ Failed to trigger unknown flag error for uppercase"
    exit 1
fi

# Test 8: Combination with version flag (valid) and invalid flag
echo -e "\nTest 8: Valid version flag with invalid flag '-v -q'"
# Note: -v prints version and exits, so invalid flag might not be reached
# This tests the parsing order
output=$(gcov-dump -v -q test.gcda 2>&1)
if echo "$output" | grep -q "unknown flag" || echo "$output" | grep -q "gcov-dump"; then
    echo "✓ Version information or error message printed"
else
    echo "✗ No output from version + invalid flag combination"
fi

echo -e "\n=== All tests completed successfully ==="

# Cleanup
cd /
rm -rf "$TEST_DIR"

echo "Test directory cleaned up: $TEST_DIR"
```

This script provides comprehensive testing of the uncovered lines by:

1. **Creating a valid coverage data file**: Compiles and runs a simple C program to generate `.gcda` and `.gcno` files that `gcov-dump` can process.

2. **Testing various invalid flag scenarios**:
   - Single invalid flag (`-x`)
   - Valid flags combined with invalid flags (`-l -p -x`)
   - Multiple invalid flags (`-y -z`)
   - Special character invalid flag (`-?`)
   - Invalid flag at different positions
   - Invalid flag with `.gcno` files
   - Uppercase invalid flag (`-X`)
   - Mixed with version flag

3. **Ensuring the default case is triggered**: Each test checks for the "unknown flag" error message that should be printed by the `fprintf` statement in the default case.

4. **Using proper execution flow**: The script follows the required steps of creating a C program, compiling with coverage flags, running it, and then invoking `gcov-dump` with the test flags.

The script also includes proper error checking and cleanup to ensure it doesn't leave temporary files behind. Each test verifies that the expected error message is produced, confirming that the uncovered code path was executed.
