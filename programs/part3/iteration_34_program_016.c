Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that triggers the `default` case in the switch statement for handling command-line flags. Here's a shell script that systematically tests this:

```bash
#!/bin/bash

# Test script for gcov-dump to trigger unknown flag handling
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

# Run the program to generate coverage data
echo "Running test program to generate coverage data..."
./test

# Verify that coverage files were created
if [ ! -f "test.gcda" ]; then
    echo "Error: test.gcda not generated!"
    exit 1
fi

echo "Testing gcov-dump with invalid flags..."

# Test 1: Single invalid flag with valid flags
echo "Test 1: -l -p with invalid -x flag"
if ! gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found for -x"
    exit 1
fi
echo "✓ Test 1 passed: Invalid flag -x detected"

# Test 2: Multiple invalid flags
echo "Test 2: Multiple invalid flags (-y -z)"
if ! gcov-dump -r -s -y -z test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found for -y or -z"
    exit 1
fi
echo "✓ Test 2 passed: Invalid flags -y and -z detected"

# Test 3: Only invalid flags
echo "Test 3: Only invalid flags (-a -b -c)"
if ! gcov-dump -a -b -c test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found"
    exit 1
fi
echo "✓ Test 3 passed: Multiple invalid flags detected"

# Test 4: Invalid flag at different positions
echo "Test 4: Invalid flag in middle (-l -? -p)"
if ! gcov-dump -l -? -p test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found for -?"
    exit 1
fi
echo "✓ Test 4 passed: Invalid flag -? detected"

# Test 5: Mixed valid and invalid with file at beginning
echo "Test 5: File first, then flags (test.gcda -v -q)"
if ! gcov-dump test.gcda -v -q 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found for -q"
    exit 1
fi
echo "✓ Test 5 passed: Invalid flag -q detected"

# Test 6: Edge case - numeric flag (not a letter)
echo "Test 6: Numeric invalid flag (-1)"
if ! gcov-dump -l -1 test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found for -1"
    exit 1
fi
echo "✓ Test 6 passed: Invalid flag -1 detected"

# Test 7: Ensure valid flags still work when used correctly
echo "Test 7: Verify valid flags work correctly"
if ! gcov-dump -l -p -r -s test.gcda > /dev/null 2>&1; then
    echo "ERROR: Valid flags combination failed"
    exit 1
fi
echo "✓ Test 7 passed: Valid flags work correctly"

echo ""
echo "All tests passed successfully!"
echo "The default case for unknown flags was triggered for:"
echo "  - Single invalid flags (-x, -y, -z, -?, -q, -1)"
echo "  - Multiple invalid flags in one invocation"
echo "  - Invalid flags mixed with valid flags"
echo "  - Invalid flags at different positions in argument list"

# Cleanup
rm -f test test.c test.gcda test.gcno
```

This test script:

1. **Creates a minimal C program** that generates coverage data when compiled with `-fprofile-arcs -ftest-coverage`.

2. **Tests various invalid flag scenarios**:
   - Test 1: `-l -p -x` - Valid flags followed by invalid `-x`
   - Test 2: `-r -s -y -z` - Multiple invalid flags `-y` and `-z`
   - Test 3: `-a -b -c` - Only invalid flags
   - Test 4: `-l -? -p` - Invalid flag `-?` in the middle
   - Test 5: `test.gcda -v -q` - File first, then valid and invalid flag
   - Test 6: `-l -1` - Numeric invalid flag `-1`

3. **Verifies the error message** by checking stderr output for "unknown flag" string.

4. **Includes a sanity check** (Test 7) to ensure valid flags still work correctly.

5. **Cleans up** temporary files after testing.

The script ensures that `gcov-dump` processes the command-line arguments and reaches the `default` case in the switch statement for each invalid flag, triggering the `fprintf(stderr, "unknown flag `%c'\n", opt);` line.

To run this test, save it as `test_gcov_dump.sh`, make it executable (`chmod +x test_gcov_dump.sh`), and execute it. The test requires that `gcov-dump` is available in your PATH and that GCC is installed with coverage support.
