I'll create a comprehensive test script that covers all the uncovered lines in the gcov-tool overlap command parsing logic.

```bash
#!/bin/bash

# Test script for gcov-tool overlap command options coverage
# This script tests the uncovered lines in gcov-tool.cc (lines 534-554)

set -e  # Exit on error

# Create a temporary directory for our test
TEMP_DIR=$(mktemp -d /tmp/gcov_tool_test_XXXXXX)
echo "Working in temporary directory: $TEMP_DIR"
cd "$TEMP_DIR"

# Cleanup function
cleanup() {
    echo "Cleaning up temporary directory: $TEMP_DIR"
    rm -rf "$TEMP_DIR"
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

echo "Created test.c source file"

# Step 2: Compile with GCOV instrumentation
echo "Compiling test.c with GCOV instrumentation..."
gcc -fprofile-arcs -ftest-coverage test.c -o test_prog

if [ ! -f test_prog ]; then
    echo "Error: Failed to compile test program"
    exit 1
fi

# Step 3: Run the program to generate initial .gcda file
echo "Running test program to generate coverage data..."
./test_prog > /dev/null

if [ ! -f test.gcda ]; then
    echo "Error: No .gcda file generated"
    exit 1
fi

echo "Generated test.gcda file"

# Step 4: Create two different .gcda files for overlap analysis
# First, create a base version
cp test.gcda base.gcda

# Run the program again with different input to create variation
# We'll modify the program slightly and recompile to get different coverage
cat > test2.c << 'EOF'
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
    
    // Only test factorial with different value
    printf("Factorial of 3: %d\n", factorial(3));
    
    // Skip fibonacci test
    
    // Loop with different range
    for (i = 0; i < 5; i++) {
        if (i % 3 == 0) {
            printf("%d divisible by 3\n", i);
        }
    }
    
    return 0;
}
EOF

# Compile and run second version
gcc -fprofile-arcs -ftest-coverage test2.c -o test2_prog
./test2_prog > /dev/null
cp test2.gcda compare.gcda

echo "Created base.gcda and compare.gcda files for overlap analysis"

# Step 5: Test individual options from the uncovered block
echo -e "\n=== Testing individual options ==="

# Test 1: -v (verbose)
echo "Test 1: Testing -v (verbose) option..."
gcov-tool overlap -v base.gcda compare.gcda > verbose_output.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ -v option executed successfully"
    # Check if verbose output was produced
    if [ -s verbose_output.txt ]; then
        echo "  Verbose output generated"
    fi
else
    echo "✗ -v option failed"
fi

# Test 2: -f (function level)
echo -e "\nTest 2: Testing -f (function level) option..."
gcov-tool overlap -f base.gcda compare.gcda > func_output.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ -f option executed successfully"
else
    echo "✗ -f option failed"
fi

# Test 3: -F (full filename)
echo -e "\nTest 3: Testing -F (full filename) option..."
gcov-tool overlap -F base.gcda compare.gcda > fullname_output.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ -F option executed successfully"
else
    echo "✗ -F option failed"
fi

# Test 4: -o (object level)
echo -e "\nTest 4: Testing -o (object level) option..."
gcov-tool overlap -o base.gcda compare.gcda > obj_output.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ -o option executed successfully"
else
    echo "✗ -o option failed"
fi

# Test 5: -h (hot only)
echo -e "\nTest 5: Testing -h (hot only) option..."
gcov-tool overlap -h base.gcda compare.gcda > hot_output.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ -h option executed successfully"
else
    echo "✗ -h option failed"
fi

# Test 6: -t (hot threshold) with various values
echo -e "\nTest 6: Testing -t (hot threshold) option..."

# Test 6a: -t with normal value
echo "  Testing -t 0.5..."
gcov-tool overlap -t 0.5 base.gcda compare.gcda > threshold_0.5_output.txt 2>&1
if [ $? -eq 0 ]; then
    echo "  ✓ -t 0.5 executed successfully"
else
    echo "  ✗ -t 0.5 failed"
fi

# Test 6b: -t with minimum value (0.0)
echo "  Testing -t 0.0..."
gcov-tool overlap -t 0.0 base.gcda compare.gcda > threshold_0.0_output.txt 2>&1
if [ $? -eq 0 ]; then
    echo "  ✓ -t 0.0 executed successfully"
else
    echo "  ✗ -t 0.0 failed"
fi

# Test 6c: -t with maximum value (1.0)
echo "  Testing -t 1.0..."
gcov-tool overlap -t 1.0 base.gcda compare.gcda > threshold_1.0_output.txt 2>&1
if [ $? -eq 0 ]; then
    echo "  ✓ -t 1.0 executed successfully"
else
    echo "  ✗ -t 1.0 failed"
fi

# Test 6d: -t with out-of-range negative value
echo "  Testing -t -1.0 (should trigger error handling)..."
gcov-tool overlap -t -1.0 base.gcda compare.gcda > threshold_neg1.0_output.txt 2>&1
# This might fail, which is expected for invalid threshold
echo "  -t -1.0 tested (exit code: $?)"

# Test 6e: -t with out-of-range positive value
echo "  Testing -t 2.5 (should trigger error handling)..."
gcov-tool overlap -t 2.5 base.gcda compare.gcda > threshold_2.5_output.txt 2>&1
echo "  -t 2.5 tested (exit code: $?)"

# Step 6: Test option combinations
echo -e "\n=== Testing option combinations ==="

# Combination 1: -v -f -o
echo "Test 7: Testing combination -v -f -o..."
gcov-tool overlap -v -f -o base.gcda compare.gcda > combo_vfo_output.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ Combination -v -f -o executed successfully"
else
    echo "✗ Combination -v -f -o failed"
fi

# Combination 2: -F -h -t 0.75
echo -e "\nTest 8: Testing combination -F -h -t 0.75..."
gcov-tool overlap -F -h -t 0.75 base.gcda compare.gcda > combo_Fht_output.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ Combination -F -h -t 0.75 executed successfully"
else
    echo "✗ Combination -F -h -t 0.75 failed"
fi

# Combination 3: -v -F -o -h
echo -e "\nTest 9: Testing combination -v -F -o -h..."
gcov-tool overlap -v -F -o -h base.gcda compare.gcda > combo_vFoh_output.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ Combination -v -F -o -h executed successfully"
else
    echo "✗ Combination -v -F -o -h failed"
fi

# Combination 4: -f -o -t 0.25
echo -e "\nTest 10: Testing combination -f -o -t 0.25..."
gcov-tool overlap -f -o -t 0.25 base.gcda compare.gcda > combo_fot_output.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ Combination -f -o -t 0.25 executed successfully"
else
    echo "✗ Combination -f -o -t 0.25 failed"
fi

# Step 7: Test invalid option to trigger default case
echo -e "\n=== Testing invalid option (to trigger default case) ==="
echo "Test 11: Testing invalid option -x..."
gcov-tool overlap -x base.gcda compare.gcda > invalid_option_output.txt 2>&1
INVALID_EXIT_CODE=$?
if [ $INVALID_EXIT_CODE -ne 0 ]; then
    echo "✓ Invalid option -x triggered error (exit code: $INVALID_EXIT_CODE)"
    # Check if usage was printed
    if grep -q "usage\|Usage\|OVERLOP" invalid_option_output.txt; then
        echo "  Usage message was displayed"
    fi
else
    echo "✗ Invalid option -x did not trigger expected error"
fi

# Step 8: Test missing required argument for -t
echo -e "\nTest 12: Testing -t without argument..."
gcov-tool overlap -t base.gcda compare.gcda > missing_arg_output.txt 2>&1
MISSING_ARG_EXIT_CODE=$?
if [ $MISSING_ARG_EXIT_CODE -ne 0 ]; then
    echo "✓ Missing argument for -t triggered error (exit code: $MISSING_ARG_EXIT_CODE)"
else
    echo "✗ Missing argument for -t did not trigger expected error"
fi

# Step 9: Test with no options (baseline)
echo -e "\n=== Testing baseline (no options) ==="
echo "Test 13: Testing with no options..."
gcov-tool overlap base.gcda compare.gcda > baseline_output.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ Baseline execution successful"
else
    echo "✗ Baseline execution failed"
fi

# Step 10: Summary and verification
echo -e "\n=== Test Summary ==="
echo "All tests completed. Output files in $TEMP_DIR:"
ls -la *.txt 2>/dev/null | head -20

echo -e "\nChecking if all target options were tested:"
echo "✓ -v (verbose)"
echo "✓ -f (function level)" 
echo "✓ -F (full filename)"
echo "✓ -o (object level)"
echo "✓ -h (hot only)"
echo "✓ -t (hot threshold) with various values"
echo "✓ Option combinations"
echo "✓ Invalid option (default case)"
echo "✓ Missing argument test"

echo -e "\nTo verify coverage was achieved:"
echo "1. Ensure gcov-tool was built with coverage instrumentation"
echo "2. Run this script with a coverage-instrumented gcov-tool"
echo "3. Check that the switch cases for 'v', 'f', 'F', 'o', 'h', 't' were executed"
echo "4. Verify the default case was triggered by the invalid option test"

echo -e "\nTest script completed successfully!"
```

This comprehensive test script:

1. **Creates a test environment** with temporary directory and cleanup
2. **Generates two C programs** with different execution paths to create meaningful coverage data
3. **Compiles with GCOV instrumentation** to generate `.gcda` files
4. **Tests each individual option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) from the uncovered block
5. **Tests boundary values** for `-t` option (0.0, 1.0, -1.0, 2.5)
6. **Tests option combinations** to ensure they work together
7. **Tests invalid option** (`-x`) to trigger the `default` case and `overlap_usage()`
8. **Tests missing argument** for `-t` option
9. **Captures all output** to files for verification
10. **Provides clear feedback** about test success/failure

To run this test:

1. Ensure `gcov-tool` is in your PATH and was built with coverage instrumentation
2. Make the script executable: `chmod +x run_gcov_tool_test.sh`
3. Run the script: `./run_gcov_tool_test.sh`

The script will systematically exercise all the uncovered lines in the switch statement, ensuring that each case is executed and the corresponding global variables are set.
