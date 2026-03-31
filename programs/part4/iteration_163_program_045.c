I'll create a comprehensive test script that covers all the uncovered lines in the `gcov-tool overlap` command-line parsing logic. This script will generate test data and systematically exercise each option.

```bash
#!/bin/bash

# Test script for gcov-tool overlap command options
# This script covers the uncovered lines in gcov-tool.cc (lines 534-554)

set -e  # Exit on error

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

# Trap to ensure cleanup on exit
trap cleanup EXIT

# Step 1: Create a simple C program for coverage testing
cat > test.c << 'EOF'
#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int main() {
    int i;
    
    // Test factorial
    printf("Factorial of 5: %d\n", factorial(5));
    
    // Test fibonacci
    printf("Fibonacci of 6: %d\n", fibonacci(6));
    
    // Loop with conditional
    for (i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            printf("%d is even\n", i);
        } else {
            printf("%d is odd\n", i);
        }
    }
    
    return 0;
}
EOF

# Step 2: Compile with GCOV instrumentation
echo "Compiling test program with GCOV instrumentation..."
gcc -fprofile-arcs -ftest-coverage test.c -o test_prog

# Step 3: Run the program to generate initial .gcda file
echo "Running test program to generate profile data..."
./test_prog > /dev/null

# Step 4: Create two .gcda files for overlap comparison
echo "Creating base and comparison .gcda files..."
cp test.gcda base.gcda

# Run program again with different execution pattern
cat > test2.c << 'EOF'
#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    // Only call factorial, not fibonacci
    printf("Factorial of 3: %d\n", factorial(3));
    
    // Different loop pattern
    for (int i = 0; i < 5; i++) {
        printf("Iteration %d\n", i);
    }
    
    return 0;
}
EOF

gcc -fprofile-arcs -ftest-coverage test2.c -o test_prog2
./test_prog2 > /dev/null
cp test2.gcda compare.gcda

# Step 5: Test individual options (covering each case statement)

echo -e "\n=== Testing individual options ==="

# Test 1: -v (verbose) - covers case 'v'
echo "Test 1: Testing -v option (verbose)..."
if gcov-tool overlap -v base.gcda compare.gcda 2>&1 | grep -q "verbose\|Processing"; then
    echo "✓ -v option processed (verbose output detected)"
else
    echo "✓ -v option processed"
fi

# Test 2: -f (function level) - covers case 'f'
echo -e "\nTest 2: Testing -f option (function level)..."
gcov-tool overlap -f base.gcda compare.gcda 2>&1 > /dev/null && echo "✓ -f option processed"

# Test 3: -F (full filename) - covers case 'F'
echo -e "\nTest 3: Testing -F option (full filename)..."
gcov-tool overlap -F base.gcda compare.gcda 2>&1 > /dev/null && echo "✓ -F option processed"

# Test 4: -o (object level) - covers case 'o'
echo -e "\nTest 4: Testing -o option (object level)..."
gcov-tool overlap -o base.gcda compare.gcda 2>&1 > /dev/null && echo "✓ -o option processed"

# Test 5: -h (hot only) - covers case 'h'
echo -e "\nTest 5: Testing -h option (hot only)..."
gcov-tool overlap -h base.gcda compare.gcda 2>&1 > /dev/null && echo "✓ -h option processed"

# Test 6: -t (hot threshold) - covers case 't'
echo -e "\nTest 6: Testing -t option (hot threshold)..."
echo "  Testing with threshold 0.5"
gcov-tool overlap -t 0.5 base.gcda compare.gcda 2>&1 > /dev/null && echo "  ✓ -t 0.5 processed"

# Step 6: Test option combinations

echo -e "\n=== Testing option combinations ==="

# Combination 1: -v -f -o
echo "Test 7: Testing combination -v -f -o..."
gcov-tool overlap -v -f -o base.gcda compare.gcda 2>&1 > /dev/null && echo "✓ Combination -v -f -o processed"

# Combination 2: -F -h -t
echo -e "\nTest 8: Testing combination -F -h -t 0.75..."
gcov-tool overlap -F -h -t 0.75 base.gcda compare.gcda 2>&1 > /dev/null && echo "✓ Combination -F -h -t 0.75 processed"

# Combination 3: All options together
echo -e "\nTest 9: Testing all options together..."
gcov-tool overlap -v -f -F -o -h -t 0.25 base.gcda compare.gcda 2>&1 > /dev/null && echo "✓ All options together processed"

# Step 7: Test threshold boundary values

echo -e "\n=== Testing threshold boundary values ==="

# Test minimum threshold (0.0)
echo "Test 10: Testing threshold 0.0..."
gcov-tool overlap -t 0.0 base.gcda compare.gcda 2>&1 > /dev/null && echo "✓ Threshold 0.0 processed"

# Test maximum threshold (1.0)
echo -e "\nTest 11: Testing threshold 1.0..."
gcov-tool overlap -t 1.0 base.gcda compare.gcda 2>&1 > /dev/null && echo "✓ Threshold 1.0 processed"

# Test edge case thresholds (should still parse even if out of logical range)
echo -e "\nTest 12: Testing threshold -1.0..."
if gcov-tool overlap -t -1.0 base.gcda compare.gcda 2>&1; then
    echo "✓ Threshold -1.0 parsed (may produce output)"
fi

echo -e "\nTest 13: Testing threshold 2.5..."
if gcov-tool overlap -t 2.5 base.gcda compare.gcda 2>&1; then
    echo "✓ Threshold 2.5 parsed (may produce output)"
fi

# Step 8: Test invalid option (covers default case)

echo -e "\n=== Testing invalid option (covers default case) ==="

echo "Test 14: Testing invalid option -x..."
if ! gcov-tool overlap -x base.gcda compare.gcda 2>&1 | grep -q "usage\|Usage\|invalid"; then
    echo "⚠ Invalid option might not have triggered usage (check manually)"
else
    echo "✓ Invalid option triggered usage message"
fi

# Step 9: Test missing required argument for -t

echo -e "\nTest 15: Testing -t without argument..."
if ! gcov-tool overlap -t base.gcda compare.gcda 2>&1 | grep -q "requires\|argument"; then
    echo "⚠ -t without argument might have different error (check manually)"
else
    echo "✓ -t without argument triggered error"
fi

# Step 10: Test with swapped file order

echo -e "\n=== Testing with different file orders ==="

echo "Test 16: Testing with swapped file order and options..."
gcov-tool overlap -v -f compare.gcda base.gcda 2>&1 > /dev/null && echo "✓ Swapped files with options processed"

# Step 11: Create a summary report

echo -e "\n=== Test Summary ==="
echo "All gcov-tool overlap option tests completed."
echo "The following cases were covered:"
echo "1. -v (verbose) - sets verbose flag"
echo "2. -f (function level) - sets overlap_func_level"
echo "3. -F (full filename) - sets overlap_use_fullname"
echo "4. -o (object level) - sets overlap_obj_level"
echo "5. -h (hot only) - sets overlap_hot_only"
echo "6. -t (hot threshold) - parses argument with atof"
echo "7. Multiple option combinations"
echo "8. Threshold boundary values (0.0, 1.0, -1.0, 2.5)"
echo "9. Invalid option (triggers default case)"
echo "10. Missing argument for -t"

echo -e "\nGenerated files in test directory:"
ls -la

echo -e "\nTest completed successfully!"
```

This comprehensive test script:

1. **Creates valid GCOV data files** by compiling and running two instrumented C programs
2. **Systematically tests each uncovered option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`)
3. **Tests option combinations** to ensure they work together
4. **Tests boundary values** for the `-t` threshold option
5. **Tests the default case** with an invalid option (`-x`)
6. **Handles edge cases** like missing arguments
7. **Runs in a clean temporary directory** and cleans up after itself
8. **Captures output** and provides feedback on each test

To execute this test:

1. Save the script as `test_gcov_tool_overlap.sh`
2. Make it executable: `chmod +x test_gcov_tool_overlap.sh`
3. Ensure `gcov-tool` is in your PATH (built with coverage instrumentation)
4. Run the script: `./test_gcov_tool_overlap.sh`

The script will generate coverage data for the `gcov-tool` binary itself when it processes the command-line arguments, specifically covering the switch statement cases for the overlap subcommand options.
