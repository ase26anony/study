Here's a shell script that specifically targets the uncovered lines in `gcov-dump.cc` by generating coverage data and then invoking `gcov-dump` with invalid command-line flags:

```bash
#!/bin/bash
set -e

# Create a minimal C program for coverage testing
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
./test

# Verify that .gcda file was created
if [ ! -f test.gcda ]; then
    echo "ERROR: test.gcda not generated!"
    exit 1
fi

echo "Generated test.gcda successfully."

# Test 1: Single invalid flag with valid flags
echo -e "\n=== Test 1: Valid flags with one invalid flag ==="
echo "Command: gcov-dump -l -p -x test.gcda"
if ! gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found in Test 1"
    # Continue anyway to test other cases
fi

# Test 2: Multiple invalid flags
echo -e "\n=== Test 2: Multiple invalid flags ==="
echo "Command: gcov-dump -r -s -y -z test.gcda"
if ! gcov-dump -r -s -y -z test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found in Test 2"
fi

# Test 3: Only invalid flags
echo -e "\n=== Test 3: Only invalid flags ==="
echo "Command: gcov-dump -a -b -c test.gcda"
if ! gcov-dump -a -b -c test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found in Test 3"
fi

# Test 4: Invalid flag at different positions
echo -e "\n=== Test 4: Invalid flag at beginning ==="
echo "Command: gcov-dump -? -l -p test.gcda"
if ! gcov-dump -? -l -p test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found in Test 4"
fi

# Test 5: Invalid flag at end
echo -e "\n=== Test 5: Invalid flag at end ==="
echo "Command: gcov-dump -l -p -v -q test.gcda"
if ! gcov-dump -l -p -v -q test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found in Test 5"
fi

# Test 6: Test with .gcno file as well (compiler-generated notes)
echo -e "\n=== Test 6: Testing with .gcno file ==="
echo "Command: gcov-dump -x test.gcno"
if ! gcov-dump -x test.gcno 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found in Test 6"
fi

# Test 7: Mixed valid and invalid flags in complex order
echo -e "\n=== Test 7: Complex flag ordering ==="
echo "Command: gcov-dump -l -x -p -y -r -z test.gcda"
if ! gcov-dump -l -x -p -y -r -z test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found in Test 7"
fi

echo -e "\n=== All tests completed ==="
echo "The uncovered lines in gcov-dump.cc (lines 111-130) should now be triggered."
echo "Specifically, the 'default' case handling unknown flags should have executed multiple times."

# Cleanup
echo -e "\nCleaning up..."
rm -f test test.c test.gcno test.gcda test.gcov
```

This script systematically tests the uncovered lines by:

1. **Generating valid coverage data**: Creates a simple C program, compiles it with `-fprofile-arcs -ftest-coverage`, and runs it to produce `.gcda` files.

2. **Testing invalid flags**: Uses various combinations of valid and invalid flags:
   - `-x`, `-y`, `-z`, `-?`, `-a`, `-b`, `-c`, `-q` (all invalid single-character flags)
   - Combined with valid flags `-l`, `-p`, `-r`, `-s`, `-v`, `-h`

3. **Multiple test cases**: Each test case is designed to ensure the `getopt` loop processes the flags and reaches the `default` case for invalid flags.

4. **Verification**: Each test checks if the "unknown flag" error message appears in stderr, confirming the path was taken.

The script tests:
- Invalid flags mixed with valid flags
- Multiple invalid flags together
- Invalid flags at different positions (beginning, middle, end)
- Both `.gcda` and `.gcno` files as input
- Complex flag ordering

This should comprehensively cover the `default` case in the switch statement (lines 111-130) and ensure the `fprintf(stderr, "unknown flag `%c'\n", opt);` line is executed multiple times.
