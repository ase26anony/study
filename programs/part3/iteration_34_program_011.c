Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that triggers the default case when an invalid single-character flag is provided. Here's a comprehensive shell script that meets all the requirements:

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

# Verify .gcda file was created
if [ ! -f test.gcda ]; then
    echo "ERROR: test.gcda file not created!"
    exit 1
fi

echo "Testing gcov-dump with invalid flags..."

# Test 1: Single invalid flag with valid flags
echo "Test 1: -l -p with invalid -x flag"
if ! gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error for -x"
    exit 1
fi
echo "✓ Test 1 passed: Invalid flag -x detected"

# Test 2: Multiple invalid flags
echo "Test 2: -r -s with invalid -y and -z flags"
output=$(gcov-dump -r -s -y -z test.gcda 2>&1)
if ! echo "$output" | grep -q "unknown flag.*y" || ! echo "$output" | grep -q "unknown flag.*z"; then
    echo "ERROR: Expected 'unknown flag' errors for both -y and -z"
    exit 1
fi
echo "✓ Test 2 passed: Invalid flags -y and -z detected"

# Test 3: Only invalid flags
echo "Test 3: Only invalid flags -a -b -c"
output=$(gcov-dump -a -b -c test.gcda 2>&1)
invalid_count=$(echo "$output" | grep -c "unknown flag")
if [ "$invalid_count" -lt 3 ]; then
    echo "ERROR: Expected 3 'unknown flag' errors, got $invalid_count"
    exit 1
fi
echo "✓ Test 3 passed: All invalid flags detected"

# Test 4: Invalid flag at different positions
echo "Test 4: Invalid flag in middle of valid flags"
if ! gcov-dump -l -q -p test.gcda 2>&1 | grep -q "unknown flag.*q"; then
    echo "ERROR: Expected 'unknown flag' error for -q"
    exit 1
fi
echo "✓ Test 4 passed: Invalid flag -q detected in middle position"

# Test 5: Mixed case invalid flags (testing different characters)
echo "Test 5: Testing various invalid characters"
for flag in d e f g i j k m n o t u w; do
    if gcov-dump -$flag test.gcda 2>&1 | grep -q "unknown flag.*$flag"; then
        echo "  ✓ Flag -$flag correctly identified as invalid"
    else
        echo "  WARNING: Flag -$flag might not trigger error"
    fi
done

# Test 6: Edge case - question mark (special character)
echo "Test 6: Testing special character -?"
if ! gcov-dump -\? test.gcda 2>&1 | grep -q "unknown flag.*\?"; then
    echo "Note: -? might be handled differently by getopt"
fi

# Test 7: Ensure valid flags still work when used correctly
echo "Test 7: Verifying valid flags work correctly"
if gcov-dump -l -p -r -s test.gcda > /dev/null 2>&1; then
    echo "✓ Valid flags work correctly"
else
    echo "ERROR: Valid flags combination failed"
    exit 1
fi

echo ""
echo "All tests completed successfully!"
echo "The uncovered lines in gcov-dump.cc (111-130) should now be covered."
echo "Specifically, the 'default' case handling unknown flags was triggered multiple times."

# Cleanup
rm -f test test.c test.gcda test.gcno
```

This test script:

1. **Creates a minimal C program** that generates coverage data when compiled with `-fprofile-arcs -ftest-coverage`.

2. **Tests various invalid flag scenarios**:
   - Test 1: Valid flags (`-l -p`) with one invalid flag (`-x`)
   - Test 2: Valid flags (`-r -s`) with multiple invalid flags (`-y -z`)
   - Test 3: Only invalid flags (`-a -b -c`)
   - Test 4: Invalid flag in the middle of valid flags (`-l -q -p`)
   - Test 5: Systematic testing of various invalid characters
   - Test 6: Special character testing (`-?`)
   - Test 7: Verification that valid flags still work correctly

3. **Ensures the program reaches the parsing loop** by providing a valid `.gcda` file as input.

4. **Captures stderr output** and verifies that the "unknown flag" error message is printed for each invalid flag.

5. **Tests the default case multiple times** with different invalid flags to ensure thorough coverage.

The script will exit with an error if any test fails, making it suitable for automated testing environments. Each test case is designed to trigger the `default` case in the switch statement, which contains the `fprintf(stderr, "unknown flag '%c'\n", opt);` line that needs to be covered.
