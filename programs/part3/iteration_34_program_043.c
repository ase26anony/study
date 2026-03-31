Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that triggers the default case for unknown command-line flags. Here's a comprehensive shell script that meets all the requirements:

```bash
#!/bin/bash

# Test script for gcov-dump to trigger unknown flag handling
# This script creates a simple C program, compiles it with coverage,
# runs it to generate .gcda files, then invokes gcov-dump with invalid flags

set -e

# Create a temporary directory for our test
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

# 3. Execute the program to generate .gcda data
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
if ! gcov-dump -l -p -x test.gcda 2>&1; then
    echo "Test 1 executed (expected to fail due to invalid flag -x)"
fi

# 5. Test 2: Multiple invalid flags
echo -e "\n=== Test 2: Multiple invalid flags ==="
echo "Command: gcov-dump -r -s -y -z test.gcda"
if ! gcov-dump -r -s -y -z test.gcda 2>&1; then
    echo "Test 2 executed (expected to fail due to invalid flags -y and -z)"
fi

# 6. Test 3: Only invalid flags
echo -e "\n=== Test 3: Only invalid flags ==="
echo "Command: gcov-dump -a -b -c test.gcda"
if ! gcov-dump -a -b -c test.gcda 2>&1; then
    echo "Test 3 executed (expected to fail due to invalid flags -a, -b, -c)"
fi

# 7. Test 4: Invalid flag at different positions
echo -e "\n=== Test 4: Invalid flag in different positions ==="
echo "Command: gcov-dump -x -l -p test.gcda"
if ! gcov-dump -x -l -p test.gcda 2>&1; then
    echo "Test 4 executed (invalid flag -x at beginning)"
fi

echo -e "\nCommand: gcov-dump -l -x -p test.gcda"
if ! gcov-dump -l -x -p test.gcda 2>&1; then
    echo "Test 4 executed (invalid flag -x in middle)"
fi

echo -e "\nCommand: gcov-dump -l -p -x test.gcda"
if ! gcov-dump -l -p -x test.gcda 2>&1; then
    echo "Test 4 executed (invalid flag -x at end)"
fi

# 8. Test 5: Test with .gcno file as well (different input file type)
echo -e "\n=== Test 5: Testing with .gcno file ==="
if [ -f test.gcno ]; then
    echo "Command: gcov-dump -v -q test.gcno"
    if ! gcov-dump -v -q test.gcno 2>&1; then
        echo "Test 5 executed (invalid flag -q with .gcno file)"
    fi
else
    echo "Note: test.gcno file not found (normal if compiled elsewhere)"
fi

# 9. Test 6: Mixed case invalid flags (case sensitivity test)
echo -e "\n=== Test 6: Case-sensitive invalid flags ==="
echo "Command: gcov-dump -L -P -X test.gcda"
if ! gcov-dump -L -P -X test.gcda 2>&1; then
    echo "Test 6 executed (uppercase invalid flags)"
fi

# 10. Test 7: Special character invalid flag
echo -e "\n=== Test 7: Special character invalid flag ==="
echo "Command: gcov-dump -l -? test.gcda"
if ! gcov-dump -l -? test.gcda 2>&1; then
    echo "Test 7 executed (special character invalid flag -?)"
fi

echo -e "\n=== All tests completed ==="
echo "The default case for unknown flags should have been triggered multiple times."
echo "Check the output above for 'unknown flag' error messages."
```

This script comprehensively tests the uncovered lines by:

1. **Creating a valid C program** and compiling it with coverage flags to generate `.gcda` files
2. **Testing various invalid flag scenarios**:
   - Single invalid flag combined with valid flags (`-x` with `-l -p`)
   - Multiple invalid flags (`-y -z` with `-r -s`)
   - Only invalid flags (`-a -b -c`)
   - Invalid flags in different positions
   - Testing with both `.gcda` and `.gcno` files
   - Case-sensitive invalid flags (uppercase)
   - Special character invalid flag (`-?`)

3. **Ensuring the program reaches the parsing loop** by providing valid input files
4. **Triggering the default case multiple times** with different invalid flags
5. **Capturing stderr output** where the "unknown flag" error message should appear

The script uses `2>&1` to capture both stdout and stderr, allowing you to see the "unknown flag" error messages that indicate the default case was triggered. Each test is designed to ensure `gcov-dump` parses the command line and reaches the switch statement's default case for invalid single-character flags.
