Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that triggers the default case in the switch statement for handling command-line flags. Here's a comprehensive shell script that meets all the requirements:

```bash
#!/bin/bash

# Test script for gcov-dump to trigger uncovered lines 111-130
# Specifically tests the default case for unknown command-line flags

set -e

# Create a temporary directory for our test
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

echo "2. Running the test program to generate coverage data..."
./test

echo "3. Testing gcov-dump with invalid flag -x (should trigger default case)..."
# Test 1: Single invalid flag with valid input file
# This should trigger the default case for unknown flag 'x'
echo "Test 1: gcov-dump -x test.gcda"
if gcov-dump -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag error for -x"
else
    echo "✗ Failed to trigger unknown flag error for -x"
    exit 1
fi

echo "4. Testing gcov-dump with combination of valid and invalid flags..."
# Test 2: Valid flags followed by invalid flag
# This ensures the parser reaches the invalid flag after processing valid ones
echo "Test 2: gcov-dump -l -p -x test.gcda"
if gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag.*x"; then
    echo "✓ Successfully triggered unknown flag error in combination"
else
    echo "✗ Failed to trigger unknown flag error in combination"
    exit 1
fi

echo "5. Testing gcov-dump with multiple invalid flags..."
# Test 3: Multiple invalid flags to trigger the error multiple times
echo "Test 3: gcov-dump -y -z test.gcda"
output=$(gcov-dump -y -z test.gcda 2>&1)
unknown_flag_count=$(echo "$output" | grep -c "unknown flag")
if [ "$unknown_flag_count" -ge 2 ]; then
    echo "✓ Successfully triggered multiple unknown flag errors (count: $unknown_flag_count)"
else
    echo "✗ Expected at least 2 unknown flag errors, got $unknown_flag_count"
    exit 1
fi

echo "6. Testing gcov-dump with invalid flag in middle of valid flags..."
# Test 4: Invalid flag sandwiched between valid flags
echo "Test 4: gcov-dump -r -? -s test.gcda"
if gcov-dump -r -? -s test.gcda 2>&1 | grep -q "unknown flag.*\?"; then
    echo "✓ Successfully triggered unknown flag error for -?"
else
    echo "✗ Failed to trigger unknown flag error for -?"
    exit 1
fi

echo "7. Testing gcov-dump with only invalid flags..."
# Test 5: Only invalid flags (no valid ones)
echo "Test 5: gcov-dump -a -b -c test.gcda"
output=$(gcov-dump -a -b -c test.gcda 2>&1)
unknown_flag_count=$(echo "$output" | grep -c "unknown flag")
if [ "$unknown_flag_count" -ge 3 ]; then
    echo "✓ Successfully triggered 3 unknown flag errors for -a -b -c"
else
    echo "✗ Expected 3 unknown flag errors, got $unknown_flag_count"
    exit 1
fi

echo "8. Testing with .gcno file as well..."
# Test 6: Also test with .gcno file
echo "Test 6: gcov-dump -q test.gcno"
if gcov-dump -q test.gcno 2>&1 | grep -q "unknown flag.*q"; then
    echo "✓ Successfully triggered unknown flag error with .gcno file"
else
    echo "✗ Failed to trigger unknown flag error with .gcno file"
    exit 1
fi

echo "9. Testing edge case: invalid flag after filename..."
# Test 7: Invalid flag after filename (should still be processed)
echo "Test 7: gcov-dump test.gcda -w"
if gcov-dump test.gcda -w 2>&1 | grep -q "unknown flag.*w"; then
    echo "✓ Successfully triggered unknown flag error when flag comes after filename"
else
    echo "✗ Failed to trigger unknown flag error when flag comes after filename"
    exit 1
fi

echo "10. Testing mixed valid/invalid flags in complex pattern..."
# Test 8: Complex pattern of valid and invalid flags
echo "Test 8: gcov-dump -l -m -p -n -r -o -s test.gcda"
output=$(gcov-dump -l -m -p -n -r -o -s test.gcda 2>&1)
# Count occurrences of unknown flags m, n, o
m_count=$(echo "$output" | grep -c "unknown flag.*m")
n_count=$(echo "$output" | grep -c "unknown flag.*n")
o_count=$(echo "$output" | grep -c "unknown flag.*o")
total_invalid=$((m_count + n_count + o_count))

if [ "$total_invalid" -ge 3 ]; then
    echo "✓ Successfully triggered unknown flag errors for -m, -n, -o in complex pattern"
else
    echo "✗ Expected 3 unknown flag errors in complex pattern, got $total_invalid"
    exit 1
fi

echo ""
echo "========================================="
echo "All tests passed successfully!"
echo "The default case in gcov-dump.cc (lines 111-130)"
echo "has been triggered for various invalid flags:"
echo "- Single invalid flags (-x, -y, -z, -?, -q, -w)"
echo "- Multiple invalid flags (-a -b -c)"
echo "- Invalid flags mixed with valid flags"
echo "- Invalid flags with both .gcda and .gcno files"
echo "========================================="

# Clean up
cd ..
rm -rf "$TEST_DIR"
```

This test script comprehensively covers all the requirements:

1. **Invalid Command-Line Flags**: Tests multiple invalid flags (`-x`, `-y`, `-z`, `-?`, `-a`, `-b`, `-c`, `-m`, `-n`, `-o`, `-q`, `-w`) that should trigger the default case.

2. **Valid Flag Combinations**: Tests invalid flags mixed with valid flags (`-l -p -x`, `-r -? -s`, `-l -m -p -n -r -o -s`) to ensure the parser processes multiple arguments.

3. **Input GCOV File Requirement**: Creates a simple C program, compiles it with `-fprofile-arcs -ftest-coverage`, runs it to generate `.gcda` files, and uses both `.gcda` and `.gcno` files as input.

4. **Multiple Invalid Flags**: Tests with multiple invalid flags in a single invocation (`-y -z`, `-a -b -c`) to trigger the error message multiple times.

5. **Execution Flow**: The script follows the specified flow: creates C source, compiles with coverage, runs the program, and invokes `gcov-dump` with various flag combinations.

6. **Error Verification**: Each test checks that the "unknown flag" error message is printed to stderr, confirming the path was taken.

The script tests various scenarios including:
- Single invalid flag
- Invalid flag in combination with valid flags
- Multiple invalid flags
- Invalid flag after filename
- Complex patterns of valid and invalid flags
- Testing with both `.gcda` and `.gcno` files

This should provide comprehensive coverage of the uncovered lines in `gcov-dump.cc`.
