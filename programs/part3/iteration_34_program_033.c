Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that triggers the default case in the switch statement for handling command-line flags. Here's a comprehensive shell script that meets all the requirements:

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

echo "Created test.c"

# 2. Compile with coverage enabled
echo "Compiling test.c with coverage flags..."
gcc -O0 -fprofile-arcs -ftest-coverage -o test test.c

# 3. Execute the program to generate .gcda file
echo "Running test program to generate coverage data..."
./test > /dev/null

# Verify that .gcda file was created
if [ ! -f test.gcda ]; then
    echo "ERROR: test.gcda not created!"
    exit 1
fi
echo "Generated test.gcda file"

# 4. Test 1: Single invalid flag with valid flags
echo -e "\n=== Test 1: Single invalid flag (-x) with valid flags ==="
echo "Command: gcov-dump -l -p -x test.gcda"
if ! gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
    # Continue anyway to test other cases
fi

# 5. Test 2: Multiple invalid flags
echo -e "\n=== Test 2: Multiple invalid flags (-y -z) ==="
echo "Command: gcov-dump -y -z test.gcda"
if ! gcov-dump -y -z test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 6. Test 3: Invalid flag at different positions
echo -e "\n=== Test 3: Invalid flag in middle of arguments ==="
echo "Command: gcov-dump -r -? -s test.gcda"
if ! gcov-dump -r -? -s test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 7. Test 4: Only invalid flags
echo -e "\n=== Test 4: Only invalid flags ==="
echo "Command: gcov-dump -a -b -c test.gcda"
if ! gcov-dump -a -b -c test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 8. Test 5: Mixed valid and multiple invalid flags
echo -e "\n=== Test 5: Complex mix of valid and invalid flags ==="
echo "Command: gcov-dump -l -p -m -n -o -r test.gcda"
if ! gcov-dump -l -p -m -n -o -r test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 9. Test 6: Invalid flag with .gcno file (alternative input)
echo -e "\n=== Test 6: Invalid flag with .gcno file ==="
echo "Command: gcov-dump -q test.gcno"
if ! gcov-dump -q test.gcno 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 10. Test 7: Edge case - dash only (should be treated as file input)
echo -e "\n=== Test 7: Testing edge case ==="
echo "Command: gcov-dump -l -p - test.gcda 2>&1"
# Note: '-' as a flag might be interpreted differently, but let's test it
gcov-dump -l -p - test.gcda 2>&1 || true

echo -e "\n=== All tests completed ==="
echo "Check the output above for 'unknown flag' messages."
echo "If you see 'unknown flag' errors, the uncovered lines were triggered successfully."

# Optional: Verify coverage data exists
echo -e "\n=== Verifying coverage data ==="
gcov -a test.c 2>&1 | head -20
```

This script comprehensively tests the uncovered lines by:

1. **Creating a valid C program** and compiling it with coverage flags to generate the necessary `.gcda` file
2. **Testing various invalid flag scenarios**:
   - Single invalid flag with valid flags (`-x`)
   - Multiple invalid flags (`-y -z`)
   - Invalid flag in different positions (`-?` in the middle)
   - Only invalid flags (`-a -b -c`)
   - Complex mix of valid and invalid flags
   - Invalid flag with `.gcno` file (alternative input)
   - Edge cases

3. **Ensuring the program reaches the parsing loop** by providing a valid input file (`test.gcda`)

4. **Using single-character invalid flags** that will trigger the `default` case in the switch statement

The script captures stderr output and checks for "unknown flag" messages to confirm the path was taken. Each test case is designed to exercise different aspects of the flag parsing logic while ensuring the invalid flags trigger the uncovered code.
