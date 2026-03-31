Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that triggers the default case when an invalid command-line flag is provided. Here's a comprehensive shell script that meets all the requirements:

```bash
#!/bin/bash

# Test script for gcov-dump to trigger uncovered lines 111-130
# Specifically tests the default case for unknown command-line flags

set -e

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d)
cd "$TEST_DIR"

# Create a minimal C program for coverage testing
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

echo "2. Running test program to generate coverage data..."
./test

echo "3. Testing gcov-dump with valid and invalid flags..."

# Test 1: Single invalid flag with valid flags
echo "Test 1: -l -p -x (where -x is invalid)"
if gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag error for -x"
else
    echo "✗ Failed to trigger unknown flag error for -x"
    exit 1
fi

# Test 2: Multiple invalid flags
echo -e "\nTest 2: -r -s -y -z (where -y and -z are invalid)"
output=$(gcov-dump -r -s -y -z test.gcda 2>&1)
if echo "$output" | grep -q "unknown flag 'y'" && \
   echo "$output" | grep -q "unknown flag 'z'"; then
    echo "✓ Successfully triggered unknown flag errors for -y and -z"
else
    echo "✗ Failed to trigger unknown flag errors for -y and -z"
    exit 1
fi

# Test 3: Only invalid flags
echo -e "\nTest 3: -? -@ (both invalid)"
output=$(gcov-dump -\? -\@ test.gcda 2>&1)
if echo "$output" | grep -q "unknown flag '\?'" && \
   echo "$output" | grep -q "unknown flag '@'"; then
    echo "✓ Successfully triggered unknown flag errors for -? and -@"
else
    echo "✗ Failed to trigger unknown flag errors for -? and -@"
    exit 1
fi

# Test 4: Invalid flag at different positions
echo -e "\nTest 4: -x test.gcda -l (invalid flag first)"
if gcov-dump -x test.gcda -l 2>&1 | grep -q "unknown flag 'x'"; then
    echo "✓ Successfully triggered unknown flag error when -x is first"
else
    echo "✗ Failed to trigger unknown flag error when -x is first"
    exit 1
fi

# Test 5: Mixed case invalid flag (uppercase)
echo -e "\nTest 5: -l -P -X (where -P and -X are invalid uppercase)"
output=$(gcov-dump -l -P -X test.gcda 2>&1)
if echo "$output" | grep -q "unknown flag 'P'" && \
   echo "$output" | grep -q "unknown flag 'X'"; then
    echo "✓ Successfully triggered unknown flag errors for uppercase -P and -X"
else
    echo "✗ Failed to trigger unknown flag errors for uppercase -P and -X"
    exit 1
fi

# Test 6: Long option (should not trigger the default case but included for completeness)
echo -e "\nTest 6: --invalid-long-option"
if gcov-dump --invalid-long-option test.gcda 2>&1 | grep -q "unrecognized option"; then
    echo "✓ Long option triggered different error path (as expected)"
else
    echo "Note: Long option handling may vary"
fi

echo -e "\n7. Verifying valid flags still work..."
# Verify that valid flags work correctly
echo "Testing valid flag -h (help):"
if gcov-dump -h 2>&1 | head -1 | grep -q "Usage:"; then
    echo "✓ -h flag works correctly"
else
    echo "✗ -h flag not working"
    exit 1
fi

echo -e "\nTesting valid flag -v (version):"
if gcov-dump -v 2>&1 | grep -q "gcov-dump"; then
    echo "✓ -v flag works correctly"
else
    echo "✗ -v flag not working"
    exit 1
fi

# Clean up
cd ..
rm -rf "$TEST_DIR"

echo -e "\nAll tests completed successfully!"
echo "The uncovered lines in gcov-dump.cc (111-130) should now be covered."
```

This script provides comprehensive testing of the uncovered lines by:

1. **Creating a valid coverage file**: Compiles and runs a simple C program to generate `.gcda` files that `gcov-dump` can process.

2. **Testing invalid flags in various scenarios**:
   - Single invalid flag with valid flags (`-l -p -x`)
   - Multiple invalid flags (`-r -s -y -z`)
   - Only invalid flags (`-? -@`)
   - Invalid flag at different positions in the argument list
   - Uppercase invalid flags (`-P -X`)

3. **Ensuring the program reaches the parsing loop**: By providing a valid input file (`test.gcda`), the program will parse all flags before potentially exiting.

4. **Verifying valid flags still work**: Tests `-h` and `-v` to ensure the switch statement handles valid cases correctly.

5. **Testing edge cases**: Includes a test with long options to show they follow a different code path.

The script captures stderr output and checks for the "unknown flag" error message, confirming that the default case in the switch statement was executed. Each invalid flag should trigger the `fprintf(stderr, "unknown flag `%c'\n", opt);` line, covering the uncovered code.
