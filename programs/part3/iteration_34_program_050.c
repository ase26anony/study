Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that triggers the `default` case in the switch statement for handling command-line flags. This happens when an invalid single-character flag is provided. Here's a comprehensive shell script that meets all the requirements:

```bash
#!/bin/bash

# Test script for gcov-dump to trigger uncovered lines 111-130
# Specifically tests the default case for unknown command-line flags

set -e

# Create a temporary directory for our test
TEST_DIR=$(mktemp -d)
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
echo "Compiling test program with coverage..."
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
    echo "Test 4b executed (invalid flag -x in middle)"
fi

# 8. Test 5: Edge case - question mark as invalid flag
echo -e "\n=== Test 5: Question mark as invalid flag ==="
echo "Command: gcov-dump -l -p -\? test.gcda"
if ! gcov-dump -l -p -\? test.gcda 2>&1; then
    echo "Test 5 executed (invalid flag -?)"
fi

# 9. Test 6: Mixed valid and invalid flags with file at beginning
echo -e "\n=== Test 6: File argument at different positions ==="
echo "Command: gcov-dump test.gcda -l -x"
if ! gcov-dump test.gcda -l -x 2>&1; then
    echo "Test 6 executed (file first, then invalid flag)"
fi

echo -e "\nAll tests completed. The 'unknown flag' messages above"
echo "confirm that the default case in the switch statement was triggered."
```

This script provides comprehensive testing of the uncovered lines by:

1. **Creating a valid .gcda file**: Compiles and runs a simple C program with coverage flags to generate the required input file for `gcov-dump`.

2. **Testing various invalid flag scenarios**:
   - Test 1: Valid flags (`-l -p`) with one invalid flag (`-x`)
   - Test 2: Valid flags (`-r -s`) with multiple invalid flags (`-y -z`)
   - Test 3: Only invalid flags (`-a -b -c`)
   - Test 4: Invalid flag at different positions in the argument list
   - Test 5: Edge case with question mark as invalid flag
   - Test 6: Different file argument positions

3. **Ensuring the parsing loop executes**: By including valid flags in most tests, we ensure the program enters the flag parsing loop and reaches the `default` case for invalid flags.

4. **Handling multiple invalid flags**: Tests 2 and 3 specifically test multiple invalid flags to ensure each triggers the error message.

The script captures stderr output (via `2>&1`) where the "unknown flag" error messages from the `fprintf` call in the `default` case should appear. Each test is designed to fail (return non-zero exit code) due to the invalid flags, which is expected behavior.

To run this test, ensure:
1. GCC with coverage support is installed
2. `gcov-dump` is in your PATH (usually comes with GCC)
3. The script has execute permissions: `chmod +x test_gcov_dump.sh`
4. Run it: `./test_gcov_dump.sh`

The script will create a temporary directory, compile and run a test program, then execute multiple `gcov-dump` invocations with various flag combinations to thoroughly exercise the uncovered code paths.
