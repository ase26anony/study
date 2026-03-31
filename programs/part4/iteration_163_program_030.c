Looking at the uncovered lines in `gcov-tool.cc`, I need to create a comprehensive test that exercises all the option parsing logic for the `overlap` subcommand. Here's a shell script that systematically tests each uncovered case:

```bash
#!/bin/bash

# Test script for gcov-tool overlap command options
# This script tests the uncovered lines in gcov-tool.cc (lines 534-554)

set -e  # Exit on any error

# Create a temporary directory for our test
TEST_DIR=$(mktemp -d /tmp/gcov_tool_test_XXXXXX)
echo "Test directory: $TEST_DIR"
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    cd /
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

# Step 1: Create a simple C program for coverage testing
cat > test.c << 'EOF'
#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    int i, sum = 0;
    
    // Some loops to generate coverage data
    for (i = 0; i < 10; i++) {
        sum += i;
    }
    
    // Conditional branch
    if (sum > 20) {
        printf("Sum is large: %d\n", sum);
    } else {
        printf("Sum is small: %d\n", sum);
    }
    
    // Call a function
    int fact = factorial(5);
    printf("Factorial of 5 is: %d\n", fact);
    
    return 0;
}
EOF

# Step 2: Compile with GCOV instrumentation
echo "Compiling test program with GCOV instrumentation..."
gcc -fprofile-arcs -ftest-coverage test.c -o test_prog

# Step 3: Run the program to generate initial .gcda file
echo "Running test program to generate coverage data..."
./test_prog > /dev/null

# Verify .gcda file was created
if [ ! -f test.gcda ]; then
    echo "Error: test.gcda not created!"
    exit 1
fi

# Step 4: Create two different .gcda files for overlap comparison
echo "Creating base and comparison .gcda files..."
cp test.gcda base.gcda

# Run program again with different input to create different coverage
# We'll modify the .gcda file slightly by running with different conditions
cat > test2.c << 'EOF'
#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    int i, sum = 0;
    
    // Different loop count
    for (i = 0; i < 5; i++) {  // Changed from 10 to 5
        sum += i;
    }
    
    // This will take the other branch
    if (sum > 20) {
        printf("Sum is large: %d\n", sum);
    } else {
        printf("Sum is small: %d\n", sum);  // This branch will be taken
    }
    
    // Call function with different value
    int fact = factorial(3);  // Changed from 5 to 3
    printf("Factorial of 3 is: %d\n", fact);
    
    return 0;
}
EOF

gcc -fprofile-arcs -ftest-coverage test2.c -o test_prog2
./test_prog2 > /dev/null
cp test2.gcda compare.gcda

# Verify both files exist
if [ ! -f base.gcda ] || [ ! -f compare.gcda ]; then
    echo "Error: Required .gcda files not created!"
    exit 1
fi

echo "Created base.gcda and compare.gcda for overlap analysis"

# Step 5: Test individual options (covering each case in the switch statement)
echo ""
echo "=== Testing individual options ==="

# Test 1: -v (verbose) - covers case 'v'
echo "Test 1: Testing -v (verbose) option..."
if gcov-tool overlap -v base.gcda compare.gcda 2>&1 | grep -q "verbose\|Processing"; then
    echo "✓ -v option processed successfully"
else
    echo "✗ -v option may not have worked"
fi

# Test 2: -f (function level) - covers case 'f'
echo "Test 2: Testing -f (function level) option..."
if gcov-tool overlap -f base.gcda compare.gcda 2>&1 > /dev/null; then
    echo "✓ -f option processed successfully"
else
    echo "✗ -f option failed"
fi

# Test 3: -F (full filename) - covers case 'F'
echo "Test 3: Testing -F (full filename) option..."
if gcov-tool overlap -F base.gcda compare.gcda 2>&1 > /dev/null; then
    echo "✓ -F option processed successfully"
else
    echo "✗ -F option failed"
fi

# Test 4: -o (object level) - covers case 'o'
echo "Test 4: Testing -o (object level) option..."
if gcov-tool overlap -o base.gcda compare.gcda 2>&1 > /dev/null; then
    echo "✓ -o option processed successfully"
else
    echo "✗ -o option failed"
fi

# Test 6: -t (hot threshold) - covers case 't'
echo "Test 6: Testing -t (hot threshold) option..."
if gcov-tool overlap -t 0.5 base.gcda compare.gcda 2>&1 > /dev/null; then
    echo "✓ -t 0.5 option processed successfully"
else
    echo "✗ -t 0.5 option failed"
fi

# Step 6: Test option combinations
echo ""
echo "=== Testing option combinations ==="

# Combination 1: -v -f -o
echo "Test 7: Testing combination -v -f -o..."
if gcov-tool overlap -v -f -o base.gcda compare.gcda 2>&1 > /dev/null; then
    echo "✓ Combination -v -f -o processed successfully"
else
    echo "✗ Combination -v -f -o failed"
fi

# Combination 2: -F -h -t 0.75
echo "Test 8: Testing combination -F -h -t 0.75..."
if gcov-tool overlap -F -h -t 0.75 base.gcda compare.gcda 2>&1 > /dev/null; then
    echo "✓ Combination -F -h -t 0.75 processed successfully"
else
    echo "✗ Combination -F -h -t 0.75 failed"
fi

# Combination 3: -v -F -o -h
echo "Test 9: Testing combination -v -F -o -h..."
if gcov-tool overlap -v -F -o -h base.gcda compare.gcda 2>&1 > /dev/null; then
    echo "✓ Combination -v -F -o -h processed successfully"
else
    echo "✗ Combination -v -F -o -h failed"
fi

# Step 7: Test threshold boundary values
echo ""
echo "=== Testing threshold boundary values ==="

# Test minimum threshold
echo "Test 10: Testing -t 0.0 (minimum)..."
if gcov-tool overlap -t 0.0 base.gcda compare.gcda 2>&1 > /dev/null; then
    echo "✓ -t 0.0 option processed successfully"
else
    echo "✗ -t 0.0 option failed"
fi

# Test maximum threshold
echo "Test 11: Testing -t 1.0 (maximum)..."
if gcov-tool overlap -t 1.0 base.gcda compare.gcda 2>&1 > /dev/null; then
    echo "✓ -t 1.0 option processed successfully"
else
    echo "✗ -t 1.0 option failed"
fi

# Test out-of-range threshold (should still parse but may produce warnings)
echo "Test 12: Testing -t 2.5 (out of range)..."
if gcov-tool overlap -t 2.5 base.gcda compare.gcda 2>&1 > /dev/null; then
    echo "✓ -t 2.5 option parsed (may produce warning)"
else
    echo "✗ -t 2.5 option failed"
fi

# Test negative threshold
echo "Test 13: Testing -t -1.0 (negative)..."
if gcov-tool overlap -t -1.0 base.gcda compare.gcda 2>&1 > /dev/null; then
    echo "✓ -t -1.0 option parsed (may produce warning)"
else
    echo "✗ -t -1.0 option failed"
fi

# Step 8: Test invalid option to trigger default case
echo ""
echo "=== Testing invalid option (to trigger default case) ==="

# Test invalid option -x
echo "Test 14: Testing invalid option -x..."
if ! gcov-tool overlap -x base.gcda compare.gcda 2>&1 | grep -q "usage\|Usage\|invalid"; then
    echo "✗ Invalid option -x did not trigger expected error"
else
    echo "✓ Invalid option -x triggered usage/error message"
fi

# Test missing argument for -t
echo "Test 15: Testing -t without argument..."
if ! gcov-tool overlap -t base.gcda compare.gcda 2>&1 | grep -q "requires\|argument\|missing"; then
    echo "✗ Missing argument for -t did not trigger expected error"
else
    echo "✓ Missing argument for -t triggered error message"
fi

# Step 9: Test with no arguments to trigger usage
echo ""
echo "=== Testing edge cases ==="

echo "Test 16: Testing with no arguments..."
if ! gcov-tool overlap 2>&1 | grep -q "usage\|Usage"; then
    echo "✗ No arguments did not trigger usage"
else
    echo "✓ No arguments triggered usage message"
fi

echo ""
echo "=== Summary ==="
echo "All tests completed. The following options were tested:"
echo "  -v (verbose)              - case 'v'"
echo "  -f (function level)       - case 'f'"
echo "  -F (full filename)        - case 'F'"
echo "  -o (object level)         - case 'o'"
echo "  -h (hot only)             - case 'h'"
echo "  -t (hot threshold)        - case 't'"
echo ""
echo "Also tested:"
echo "  - Multiple option combinations"
echo "  - Threshold boundary values (0.0, 1.0, 2.5, -1.0)"
echo "  - Invalid option (-x) to trigger default case"
echo "  - Missing argument for -t"
echo "  - No arguments case"
echo ""
echo "If gcov-tool was built with coverage instrumentation, these tests"
echo "should cover the switch statement at lines 534-554 in gcov-tool.cc"
```

This script comprehensively tests all the uncovered lines:

1. **Creates valid GCOV data files**: It generates two C programs, compiles them with `-fprofile-arcs -ftest-coverage`, runs them to create `.gcda` files, and copies them for use as `base.gcda` and `compare.gcda`.

2. **Tests each individual option**:
   - `-v` (verbose): Tests case 'v'
   - `-f` (function level): Tests case 'f'
   - `-F` (full filename): Tests case 'F'
   - `-o` (object level): Tests case 'o'
   - `-h` (hot only): Tests case 'h'
   - `-t` (hot threshold): Tests case 't'

3. **Tests option combinations**: Exercises multiple options together to ensure they work in combination.

4. **Tests threshold boundary values**: Tests `-t` with 0.0, 1.0, 2.5, and -1.0 to exercise the `atof(optarg)` parsing.

5. **Tests invalid option**: Uses `-x` to trigger the `default` case and call `overlap_usage()`.

6. **Tests edge cases**: Missing argument for `-t` and no arguments.

The script creates a clean temporary directory, generates all necessary test files, runs the tests, and cleans up after itself. Each test checks the exit status of `gcov-tool` to ensure the options were parsed correctly (even if some produce warnings for invalid threshold values).

To run this test, save it as `test_gcov_tool_overlap.sh`, make it executable (`chmod +x test_gcov_tool_overlap.sh`), and execute it. The script assumes `gcov-tool` is in your PATH and was built with GCOV instrumentation for coverage testing.
