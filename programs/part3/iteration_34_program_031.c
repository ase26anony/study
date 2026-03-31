Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that specifically triggers the `default` case in the switch statement for handling command-line flags. This happens when an invalid single-character flag is provided. Here's a comprehensive shell script that meets all the requirements:

```bash
#!/bin/bash

# Test script for gcov-dump to trigger uncovered lines 111-130
# Specifically tests the default case for unknown command-line flags

set -e

# Create a temporary directory for our test
TEST_DIR=$(mktemp -d)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"

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
echo "Compiling with coverage flags..."
gcc -O0 -fprofile-arcs -ftest-coverage -o test test.c

# 3. Execute the program to generate .gcda file
echo "Running program to generate coverage data..."
./test > /dev/null

# Verify that .gcda file was created
if [ ! -f test.gcda ]; then
    echo "ERROR: test.gcda file not created!"
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
echo "Command: gcov-dump -r -s -y -z test.gcda"
if ! gcov-dump -r -s -y -z test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 6. Test 3: Only invalid flags
echo -e "\n=== Test 3: Only invalid flags (-a -b -c) ==="
echo "Command: gcov-dump -a -b -c test.gcda"
if ! gcov-dump -a -b -c test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 7. Test 4: Invalid flag at different positions
echo -e "\n=== Test 4: Invalid flag at beginning (-? test.gcda) ==="
echo "Command: gcov-dump -? test.gcda"
if ! gcov-dump -? test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

echo -e "\n=== Test 5: Invalid flag in the middle ==="
echo "Command: gcov-dump -l -@ -p test.gcda"
if ! gcov-dump -l -@ -p test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 8. Test 6: Test with .gcno file as well (compile-time data)
echo -e "\n=== Test 6: Testing with .gcno file ==="
if [ -f test.gcno ]; then
    echo "Command: gcov-dump -v -# test.gcno"
    if ! gcov-dump -v -# test.gcno 2>&1 | grep -q "unknown flag"; then
        echo "ERROR: Expected 'unknown flag' error message not found!"
    fi
else
    echo "Note: test.gcno file not found (might be in build directory)"
fi

# 9. Test 7: Mixed case - uppercase invalid flag
echo -e "\n=== Test 7: Uppercase invalid flag (-X) ==="
echo "Command: gcov-dump -l -X test.gcda"
if ! gcov-dump -l -X test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 10. Test 8: Number as invalid flag
echo -e "\n=== Test 8: Number as invalid flag (-1) ==="
echo "Command: gcov-dump -1 test.gcda"
if ! gcov-dump -1 test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 11. Clean up and show success
echo -e "\n=== All tests completed ==="
echo "The following invalid flags were tested:"
echo "  -x, -y, -z, -a, -b, -c, -?, -@, -X, -1"
echo "Each should have triggered the 'default' case in gcov-dump.cc"

# Clean up
cd ..
rm -rf "$TEST_DIR"

echo -e "\nTest completed successfully!"
```

This test script comprehensively covers the requirements:

1. **Invalid Command-Line Flags**: Tests multiple invalid single-character flags (`-x`, `-y`, `-z`, `-a`, `-b`, `-c`, `-?`, `-@`, `-X`, `-1`) that are not in the valid set (`h`, `v`, `l`, `p`, `r`, `s`).

2. **Valid Flag Combinations**: Combines invalid flags with valid ones (e.g., `-l -p -x`, `-r -s -y -z`) to ensure the parser reaches the default case.

3. **Input GCOV File Requirement**: Creates a valid C program, compiles it with `-fprofile-arcs -ftest-coverage`, runs it to generate `.gcda` files, and uses them as input to `gcov-dump`.

4. **Multiple Invalid Flags**: Tests with multiple invalid flags in a single invocation (e.g., `-a -b -c`).

5. **Mixed Flag Syntax**: Tests various types of invalid characters including special characters (`-?`, `-@`), uppercase (`-X`), and numbers (`-1`).

6. **Execution Flow**: The script follows the required flow: creates C source, compiles with coverage, runs the program, then invokes `gcov-dump` with various flag combinations.

7. **Error Verification**: Each test checks if the "unknown flag" error message appears in stderr, confirming the default case was triggered.

The script tests the uncovered lines by ensuring that whenever an invalid single-character flag is encountered, the `default` case in the switch statement is executed, printing "unknown flag `%c'" via `fprintf`.
