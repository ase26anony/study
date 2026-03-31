Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that triggers the default case when an invalid single-character flag is provided. Here's a comprehensive shell script that meets all requirements:

```bash
#!/bin/bash

# Test script for gcov-dump to trigger uncovered lines 111-130
# Specifically tests the default case for unknown command-line flags

set -e

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test-XXXXXX)
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    cd /
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

# 1. Create a minimal C program
cat > test.c << 'EOF'
#include <stdio.h>

int main(int argc, char *argv[]) {
    int result = argc * 2;
    printf("Result: %d\n", result);
    return result;
}
EOF

# 2. Compile with coverage enabled
echo "Compiling test program with coverage flags..."
gcc -O0 -fprofile-arcs -ftest-coverage -o test test.c

# 3. Execute the program to generate .gcda file
echo "Running test program to generate coverage data..."
./test > /dev/null

# Verify that .gcda file was created
if [ ! -f test.gcda ]; then
    echo "ERROR: test.gcda file not created!"
    exit 1
fi

echo "Generated test.gcda file successfully."

# 4. Test 1: Single invalid flag with valid flags
echo -e "\n=== Test 1: Valid flags with one invalid flag ==="
echo "Command: gcov-dump -l -p -x test.gcda"
if ! gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
    # Continue anyway to try other tests
fi

# 5. Test 2: Multiple invalid flags
echo -e "\n=== Test 2: Multiple invalid flags ==="
echo "Command: gcov-dump -r -s -y -z test.gcda"
if ! gcov-dump -r -s -y -z test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 6. Test 3: Only invalid flags
echo -e "\n=== Test 3: Only invalid flags ==="
echo "Command: gcov-dump -x -y -z test.gcda"
if ! gcov-dump -x -y -z test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 7. Test 4: Invalid flag at different positions
echo -e "\n=== Test 4: Invalid flag in middle of arguments ==="
echo "Command: gcov-dump -l -x -p test.gcda"
if ! gcov-dump -l -x -p test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 8. Test 5: Invalid flag with long options (mixed syntax)
echo -e "\n=== Test 5: Mixed short and long options ==="
echo "Command: gcov-dump -l --invalid-long -x test.gcda"
# Note: --invalid-long might trigger different error path, but -x should still trigger default case
if ! gcov-dump -l --invalid-long -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "Note: Different error handling for long options is expected"
fi

# 9. Test 6: Edge case - question mark as invalid flag
echo -e "\n=== Test 6: Question mark as invalid flag ==="
echo "Command: gcov-dump -l -p -\? test.gcda"
if ! gcov-dump -l -p -\? test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 10. Test 7: All valid flags to ensure they work
echo -e "\n=== Test 7: All valid flags (sanity check) ==="
echo "Command: gcov-dump -l -p -r -s test.gcda"
if gcov-dump -l -p -r -s test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Valid flags triggered unknown flag error!"
fi

echo -e "\n=== Test completed ==="
echo "All tests executed. Check output above for 'unknown flag' messages."
echo "If 'unknown flag' messages appeared, the default case in gcov-dump.cc was triggered."
```

This script provides comprehensive testing of the uncovered lines by:

1. **Creating a valid GCOV file**: Compiles and runs a simple C program with coverage flags to generate `test.gcda`.

2. **Testing invalid flags in various scenarios**:
   - Test 1: Valid flags (`-l -p`) with one invalid flag (`-x`)
   - Test 2: Valid flags (`-r -s`) with multiple invalid flags (`-y -z`)
   - Test 3: Only invalid flags (`-x -y -z`)
   - Test 4: Invalid flag in the middle (`-l -x -p`)
   - Test 5: Mixed short and long options (though long options may use different error paths)
   - Test 6: Edge case with question mark as invalid flag

3. **Ensuring the parser reaches the default case**: By providing valid flags along with invalid ones, the script ensures `getopt` processes multiple arguments and reaches the switch statement's default case for invalid flags.

4. **Verifying error messages**: The script checks for "unknown flag" messages in stderr output, confirming the `fprintf(stderr, "unknown flag '%c'\n", opt)` line was executed.

The script uses `grep -q` to check for error messages but continues testing even if one test fails, ensuring maximum coverage of the target code paths.
