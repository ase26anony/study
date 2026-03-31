Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that triggers the default case in the switch statement when an invalid single-character flag is provided. Here's a comprehensive shell script that meets all the requirements:

```bash
#!/bin/bash

# Test script for gcov-dump to trigger uncovered lines (111-130)
# Specifically tests the default case for unknown command-line flags

set -e  # Exit on any error

# Create a temporary directory for our test
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

echo "=== Creating test C program ==="

# Create a minimal C program that will generate coverage data
cat > test.c << 'EOF'
#include <stdio.h>

int main(int argc, char *argv[]) {
    int result = argc * 2;
    printf("Result: %d\n", result);
    
    // Add some branching for coverage
    if (argc > 1) {
        printf("Arguments provided: %d\n", argc - 1);
    } else {
        printf("No arguments provided\n");
    }
    
    return result;
}
EOF

echo "=== Compiling with coverage flags ==="
# Compile with coverage enabled
gcc -O0 -fprofile-arcs -ftest-coverage -o test test.c

echo "=== Running the program to generate .gcda file ==="
# Execute to generate coverage data
./test
./test arg1 arg2  # Run with arguments to generate more coverage data

echo "=== Testing gcov-dump with invalid flags ==="

# Test 1: Single invalid flag with valid flags
echo "Test 1: -l -p with invalid -x flag"
if gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag error for -x"
else
    echo "✗ Failed to trigger unknown flag error for -x"
    exit 1
fi

echo ""
echo "=== Test 2: Multiple invalid flags ==="
# Test 2: Multiple invalid flags with valid flags
echo "Test 2: -r -s with invalid -y and -z flags"
if gcov-dump -r -s -y -z test.gcda 2>&1 | grep -c "unknown flag" | grep -q "2"; then
    echo "✓ Successfully triggered two unknown flag errors"
else
    echo "✗ Failed to trigger multiple unknown flag errors"
    exit 1
fi

echo ""
echo "=== Test 3: Only invalid flags ==="
# Test 3: Only invalid flags
echo "Test 3: Only invalid flags -a -b -c"
if gcov-dump -a -b -c test.gcda 2>&1 | grep -c "unknown flag" | grep -q "3"; then
    echo "✓ Successfully triggered three unknown flag errors"
else
    echo "✗ Failed to trigger three unknown flag errors"
    exit 1
fi

echo ""
echo "=== Test 4: Invalid flag at different positions ==="
# Test 4: Invalid flag in different positions
echo "Test 4: Invalid flag at beginning -? test.gcda"
if gcov-dump -? test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag error for -?"
else
    echo "✗ Failed to trigger unknown flag error for -?"
    exit 1
fi

echo ""
echo "=== Test 5: Mixed valid and invalid flags with file at beginning ==="
# Test 5: File argument at beginning (should still parse flags after)
echo "Test 5: test.gcda -l -x (file first)"
if gcov-dump test.gcda -l -x 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag error with file first"
else
    echo "✗ Failed to trigger unknown flag error with file first"
    exit 1
fi

echo ""
echo "=== Test 6: Test with .gcno file as well ==="
# Test 6: Also test with .gcno file
echo "Test 6: Testing with test.gcno file and invalid flag -q"
if gcov-dump -l -q test.gcno 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag error for -q with .gcno"
else
    echo "✗ Failed to trigger unknown flag error for -q with .gcno"
    exit 1
fi

echo ""
echo "=== Test 7: Edge case - invalid flag that's almost valid ==="
# Test 7: Test a flag that's close to valid ones
echo "Test 7: Testing with -m (close to -l) and -t (close to -r)"
if gcov-dump -m -t test.gcda 2>&1 | grep -c "unknown flag" | grep -q "2"; then
    echo "✓ Successfully triggered unknown flag errors for -m and -t"
else
    echo "✗ Failed to trigger unknown flag errors for -m and -t"
    exit 1
fi

echo ""
echo "=== All tests completed successfully ==="
echo "The uncovered lines in gcov-dump.cc (111-130) should now be covered."

# Clean up
cd /
rm -rf "$TEST_DIR"

echo "=== Test directory cleaned up ==="
```

This script provides comprehensive testing of the uncovered lines by:

1. **Creating a valid coverage data file**: Compiles and runs a simple C program to generate `.gcda` and `.gcno` files.

2. **Testing various invalid flag scenarios**:
   - Single invalid flag with valid flags (`-l -p -x`)
   - Multiple invalid flags (`-r -s -y -z`)
   - Only invalid flags (`-a -b -c`)
   - Invalid flag at different positions
   - File argument at beginning with flags after
   - Testing with both `.gcda` and `.gcno` files
   - Edge cases with flags close to valid ones

3. **Verifying error messages**: Each test checks that the "unknown flag" error message is printed, confirming the default case was triggered.

4. **Clean execution**: Uses a temporary directory and cleans up after itself.

The script tests the exact uncovered code by ensuring:
- The `default` case in the switch statement is triggered for invalid single-character flags
- The `fprintf(stderr, "unknown flag `%c'\n", opt);` line is executed
- Multiple invalid flags each trigger the error message
- The program still parses flags even when a valid input file is provided

To run this test, ensure:
1. GCC with gcov-dump is installed and in PATH
2. The script has execute permissions (`chmod +x test_gcov_dump.sh`)
3. Run from a directory where you have write permissions
