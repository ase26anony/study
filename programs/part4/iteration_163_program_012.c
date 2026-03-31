I'll create a comprehensive test script that covers all the uncovered lines in the gcov-tool overlap command parsing logic.

```bash
#!/bin/bash

# Test script for gcov-tool overlap command line parsing coverage
# This script tests the specific uncovered lines in gcov-tool.cc (lines 534-554)

set -e  # Exit on error

# Create a temporary directory for our test
TEST_DIR=$(mktemp -d /tmp/gcov_tool_test_XXXXXX)
echo "Test directory: $TEST_DIR"

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

cd "$TEST_DIR"

# Step 1: Create a simple C program for GCOV instrumentation
cat > test.c << 'EOF'
#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    int i, result;
    
    // Some loops to generate coverage data
    for (i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            result = factorial(i);
        } else {
            result = i * 2;
        }
        printf("Iteration %d: %d\n", i, result);
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

# Step 4: Create two different .gcda files for overlap analysis
echo "Creating base and comparison profile files..."
cp test.gcda base.gcda

# Run program again with different parameters to create different coverage
cat > test2.c << 'EOF'
#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    int i, result;
    
    // Different loop to generate different coverage
    for (i = 0; i < 5; i++) {  // Only 5 iterations
        if (i % 3 == 0) {      // Different condition
            result = factorial(i + 1);
        } else {
            result = i * 3;    // Different calculation
        }
        printf("Iteration %d: %d\n", i, result);
    }
    
    return 0;
}
EOF

gcc -fprofile-arcs -ftest-coverage test2.c -o test2_prog
./test2_prog > /dev/null
cp test2.gcda compare.gcda

echo "Profile files created:"
ls -la *.gcda

# Step 5: Test individual options from the uncovered switch block
echo -e "\n=== Testing individual options ==="

# Test 1: -v (verbose)
echo "Test 1: Testing -v (verbose) option..."
if gcov-tool overlap -v base.gcda compare.gcda 2>&1 | tee test1.log; then
    echo "✓ -v option processed successfully"
else
    echo "✗ -v option failed"
fi

# Test 2: -f (function level)
echo -e "\nTest 2: Testing -f (function level) option..."
if gcov-tool overlap -f base.gcda compare.gcda 2>&1 | tee test2.log; then
    echo "✓ -f option processed successfully"
else
    echo "✗ -f option failed"
fi

# Test 3: -F (full filename)
echo -e "\nTest 3: Testing -F (full filename) option..."
if gcov-tool overlap -F base.gcda compare.gcda 2>&1 | tee test3.log; then
    echo "✓ -F option processed successfully"
else
    echo "✗ -F option failed"
fi

# Test 4: -o (object level)
echo -e "\nTest 4: Testing -o (object level) option..."
if gcov-tool overlap -o base.gcda compare.gcda 2>&1 | tee test4.log; then
    echo "✓ -o option processed successfully"
else
    echo "✗ -o option failed"
fi

# Test 5: -h (hot only)
echo -e "\nTest 5: Testing -h (hot only) option..."
if gcov-tool overlap -h base.gcda compare.gcda 2>&1 | tee test5.log; then
    echo "✓ -h option processed successfully"
else
    echo "✗ -h option failed"
fi

# Test 6: -t (hot threshold) with valid argument
echo -e "\nTest 6: Testing -t (hot threshold) option with value 0.5..."
if gcov-tool overlap -t 0.5 base.gcda compare.gcda 2>&1 | tee test6.log; then
    echo "✓ -t 0.5 option processed successfully"
else
    echo "✗ -t 0.5 option failed"
fi

# Step 6: Test option combinations
echo -e "\n=== Testing option combinations ==="

# Test 7: Multiple options together
echo "Test 7: Testing -v -f -o combination..."
if gcov-tool overlap -v -f -o base.gcda compare.gcda 2>&1 | tee test7.log; then
    echo "✓ -v -f -o combination processed successfully"
else
    echo "✗ -v -f -o combination failed"
fi

# Test 8: Different combination
echo -e "\nTest 8: Testing -F -h -t 0.75 combination..."
if gcov-tool overlap -F -h -t 0.75 base.gcda compare.gcda 2>&1 | tee test8.log; then
    echo "✓ -F -h -t 0.75 combination processed successfully"
else
    echo "✗ -F -h -t 0.75 combination failed"
fi

# Test 9: All options together
echo -e "\nTest 9: Testing all options together..."
if gcov-tool overlap -v -f -F -o -h -t 0.9 base.gcda compare.gcda 2>&1 | tee test9.log; then
    echo "✓ All options combination processed successfully"
else
    echo "✗ All options combination failed"
fi

# Step 7: Test threshold boundary values
echo -e "\n=== Testing threshold boundary values ==="

# Test 10: Minimum threshold (0.0)
echo "Test 10: Testing -t 0.0 (minimum threshold)..."
if gcov-tool overlap -t 0.0 base.gcda compare.gcda 2>&1 | tee test10.log; then
    echo "✓ -t 0.0 option processed successfully"
else
    echo "✗ -t 0.0 option failed"
fi

# Test 11: Maximum threshold (1.0)
echo -e "\nTest 11: Testing -t 1.0 (maximum threshold)..."
if gcov-tool overlap -t 1.0 base.gcda compare.gcda 2>&1 | tee test11.log; then
    echo "✓ -t 1.0 option processed successfully"
else
    echo "✗ -t 1.0 option failed"
fi

# Test 12: Edge case threshold (0.001)
echo -e "\nTest 12: Testing -t 0.001 (small value)..."
if gcov-tool overlap -t 0.001 base.gcda compare.gcda 2>&1 | tee test12.log; then
    echo "✓ -t 0.001 option processed successfully"
else
    echo "✗ -t 0.001 option failed"
fi

# Test 13: Edge case threshold (0.999)
echo -e "\nTest 13: Testing -t 0.999 (large value)..."
if gcov-tool overlap -t 0.999 base.gcda compare.gcda 2>&1 | tee test13.log; then
    echo "✓ -t 0.999 option processed successfully"
else
    echo "✗ -t 0.999 option failed"
fi

# Step 8: Test invalid threshold values (should still parse but may fail later)
echo -e "\n=== Testing invalid threshold values ==="

# Test 14: Negative threshold
echo "Test 14: Testing -t -1.0 (negative threshold)..."
if gcov-tool overlap -t -1.0 base.gcda compare.gcda 2>&1 | tee test14.log; then
    echo "✓ -t -1.0 parsed (may produce warnings)"
else
    echo "✗ -t -1.0 failed to parse"
fi

# Test 15: Threshold > 1.0
echo -e "\nTest 15: Testing -t 2.5 (threshold > 1.0)..."
if gcov-tool overlap -t 2.5 base.gcda compare.gcda 2>&1 | tee test15.log; then
    echo "✓ -t 2.5 parsed (may produce warnings)"
else
    echo "✗ -t 2.5 failed to parse"
fi

# Step 9: Test invalid option to trigger default case
echo -e "\n=== Testing invalid option (to trigger default case) ==="

# Test 16: Invalid option -x
echo "Test 16: Testing invalid option -x (should trigger usage)..."
if gcov-tool overlap -x base.gcda compare.gcda 2>&1 | tee test16.log; then
    echo "✗ Invalid option -x should have failed"
else
    echo "✓ Invalid option -x correctly triggered error/usage"
fi

# Test 17: Missing argument for -t
echo -e "\nTest 17: Testing -t without argument..."
if gcov-tool overlap -t base.gcda compare.gcda 2>&1 | tee test17.log; then
    echo "✗ Missing argument should have failed"
else
    echo "✓ Missing argument correctly triggered error"
fi

# Step 10: Test with different file orders and additional arguments
echo -e "\n=== Testing additional edge cases ==="

# Test 18: Options after filenames
echo "Test 18: Testing options after filenames..."
if gcov-tool overlap base.gcda compare.gcda -v 2>&1 | tee test18.log; then
    echo "✓ Options after filenames processed"
else
    echo "✗ Options after filenames failed"
fi

# Test 19: Multiple -t options (last one should win)
echo -e "\nTest 19: Testing multiple -t options..."
if gcov-tool overlap -t 0.1 -t 0.9 base.gcda compare.gcda 2>&1 | tee test19.log; then
    echo "✓ Multiple -t options processed"
else
    echo "✗ Multiple -t options failed"
fi

# Step 11: Verify that verbose mode actually produces verbose output
echo -e "\n=== Verifying verbose output ==="
VERBOSE_OUTPUT=$(gcov-tool overlap -v base.gcda compare.gcda 2>&1 | wc -l)
NORMAL_OUTPUT=$(gcov-tool overlap base.gcda compare.gcda 2>&1 | wc -l)

echo "Verbose output lines: $VERBOSE_OUTPUT"
echo "Normal output lines: $NORMAL_OUTPUT"

if [ "$VERBOSE_OUTPUT" -gt "$NORMAL_OUTPUT" ]; then
    echo "✓ Verbose mode produces more output as expected"
else
    echo "✗ Verbose mode may not be working correctly"
fi

# Step 12: Summary
echo -e "\n=== Test Summary ==="
echo "All tests completed. Check individual log files for details:"
ls -la test*.log

echo -e "\nGenerated files in $TEST_DIR:"
ls -la

echo -e "\nTo examine coverage of the specific switch block, run:"
echo "gcov -b <path-to-gcov-tool.cc> | grep -A 20 -B 5 'case.*[vFfoht]'"

exit 0
```

This comprehensive test script covers all the requirements:

1. **Creates valid GCOV data files**: Generates two different C programs, compiles them with `-fprofile-arcs -ftest-coverage`, runs them to create `.gcda` files, and copies them for overlap analysis.

2. **Tests all individual options**: Each uncovered `case` statement is tested individually:
   - `-v` (verbose)
   - `-f` (function level)
   - `-F` (full filename)
   - `-o` (object level)
   - `-h` (hot only)
   - `-t` (hot threshold)

3. **Tests option combinations**: Multiple tests combine 2-3 options together to ensure they work in combination.

4. **Tests threshold boundary values**: Tests `-t` with values 0.0, 0.001, 0.5, 0.75, 0.9, 0.999, 1.0, -1.0, and 2.5 to exercise the `atof` parsing.

5. **Tests invalid option**: Tests `-x` to trigger the `default` case and `overlap_usage()`.

6. **Includes edge cases**: Tests options after filenames, multiple `-t` options, and missing arguments.

7. **Automates the process**: The script creates a temporary directory, generates all necessary files, runs all tests, and cleans up.

8. **Captures output**: Each test redirects output to log files for verification.

9. **Validates behavior**: Checks that verbose mode produces more output and that invalid options trigger errors.

To run this test, save it as `run_gcov_tool_test.sh`, make it executable (`chmod +x run_gcov_tool_test.sh`), and execute it. The script assumes `gcov-tool` is in your PATH and was built with GCOV instrumentation for coverage analysis.
