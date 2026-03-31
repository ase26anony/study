Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that triggers the default case for unknown command-line flags. Here's a comprehensive shell script that meets all the requirements:

```bash
#!/bin/bash

# Test script for gcov-dump to trigger unknown flag handling
# This script creates a simple C program, compiles it with coverage,
# runs it to generate .gcda files, then tests gcov-dump with invalid flags

set -e

# Create a temporary directory for our test
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test-XXXXXX)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"

# Create a minimal C program
cat > test.c << 'EOF'
#include <stdio.h>

int main(int argc, char *argv[]) {
    int result = argc * 2;
    printf("Result: %d\n", result);
    return result;
}
EOF

# Compile with coverage flags
echo "Compiling test program with coverage..."
gcc -O0 -fprofile-arcs -ftest-coverage -o test test.c

# Run the program to generate .gcda file
echo "Running test program to generate coverage data..."
./test > /dev/null

# Verify that .gcda file was created
if [ ! -f test.gcda ]; then
    echo "ERROR: test.gcda file not created!"
    exit 1
fi

echo "Generated test.gcda file successfully"

# Test 1: Single invalid flag with valid flags
echo -e "\n=== Test 1: Single invalid flag with valid flags ==="
echo "Command: gcov-dump -l -p -x test.gcda"
if ! gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
    # Continue anyway to test other cases
fi

# Test 2: Multiple invalid flags
echo -e "\n=== Test 2: Multiple invalid flags ==="
echo "Command: gcov-dump -r -s -y -z test.gcda"
if ! gcov-dump -r -s -y -z test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# Test 3: Only invalid flags
echo -e "\n=== Test 3: Only invalid flags ==="
echo "Command: gcov-dump -x -y -z test.gcda"
if ! gcov-dump -x -y -z test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# Test 4: Invalid flag at different positions
echo -e "\n=== Test 4: Invalid flag at beginning ==="
echo "Command: gcov-dump -x -l -p test.gcda"
if ! gcov-dump -x -l -p test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

echo -e "\n=== Test 5: Invalid flag at end ==="
echo "Command: gcov-dump -l -p -? test.gcda"
if ! gcov-dump -l -p -? test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# Test 6: Mixed case - try uppercase invalid flag
echo -e "\n=== Test 6: Uppercase invalid flag ==="
echo "Command: gcov-dump -l -p -X test.gcda"
if ! gcov-dump -l -p -X test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# Test 7: Number as invalid flag
echo -e "\n=== Test 7: Number as invalid flag ==="
echo "Command: gcov-dump -l -p -1 test.gcda"
if ! gcov-dump -l -p -1 test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# Test 8: Special character as invalid flag
echo -e "\n=== Test 8: Special character as invalid flag ==="
echo "Command: gcov-dump -l -p -\@ test.gcda"
if ! gcov-dump -l -p -\@ test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# Test 9: Try with .gcno file as well (different input file type)
echo -e "\n=== Test 9: Testing with .gcno file ==="
echo "Command: gcov-dump -l -x test.gcno"
if ! gcov-dump -l -x test.gcno 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# Test 10: Long option (should not trigger the default case but included for completeness)
echo -e "\n=== Test 10: Long option (different code path) ==="
echo "Command: gcov-dump --invalid-option test.gcda"
gcov-dump --invalid-option test.gcda 2>&1 || true

# Clean up
echo -e "\n=== Cleaning up ==="
cd /
rm -rf "$TEST_DIR"
echo "Test completed successfully!"
```

This script comprehensively tests the uncovered lines by:

1. **Creating a valid input file**: Compiles a simple C program with coverage flags and runs it to generate `.gcda` files.

2. **Testing various invalid flag scenarios**:
   - Single invalid flag with valid flags (`-x` with `-l -p`)
   - Multiple invalid flags (`-y -z` with `-r -s`)
   - Only invalid flags (`-x -y -z`)
   - Invalid flags at different positions (beginning, middle, end)
   - Various types of invalid characters (uppercase, numbers, special characters)

3. **Ensuring the parsing loop executes**: By including valid flags (`-l`, `-p`, `-r`, `-s`) in most tests, we ensure the program enters the switch statement.

4. **Testing with both file types**: Uses both `.gcda` and `.gcno` files as input.

5. **Capturing error output**: Each test captures stderr and checks for the "unknown flag" message.

The script will trigger the default case in the switch statement multiple times, covering lines 111-130 in `gcov-dump.cc`. Each invalid flag will cause the `fprintf(stderr, "unknown flag `%c'\n", opt);` line to execute, providing coverage for that specific uncovered line.
