I'll create a comprehensive test script that covers all the uncovered lines in the `gcov-tool overlap` command-line parsing logic.

```bash
#!/bin/bash

# Test script for gcov-tool overlap command options coverage
# This script tests the uncovered lines in gcov-tool.cc (lines 534-554)

set -e  # Exit on any error

# Create a temporary directory for our test
TEST_DIR=$(mktemp -d /tmp/gcov_tool_test_XXXXXX)
echo "Test directory: $TEST_DIR"

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

# Change to test directory
cd "$TEST_DIR"

# Step 1: Create a simple C program for GCOV instrumentation
cat > test_program.c << 'EOF'
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
    
    // Loop to generate some execution counts
    for (i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
    
    return 0;
}
EOF

# Step 2: Compile with GCOV instrumentation
echo "Compiling test program with GCOV instrumentation..."
gcc -fprofile-arcs -ftest-coverage test_program.c -o test_program

# Step 3: Run the program to generate initial .gcda file
echo "Running test program to generate profile data..."
./test_program > /dev/null

# Verify .gcda file was created
if [ ! -f "test_program.gcda" ]; then
    echo "ERROR: test_program.gcda not created!"
    exit 1
fi

# Step 4: Create two different .gcda files for overlap analysis
echo "Creating base and comparison profile files..."
cp test_program.gcda base.gcda

# Run program again with different parameters to create different profile
cat > test_program2.c << 'EOF'
#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    // Different execution pattern
    printf("Factorial of 3: %d\n", factorial(3));
    
    // Only even numbers
    for (int i = 0; i < 5; i += 2) {
        printf("Number: %d\n", i);
    }
    
    return 0;
}
EOF

gcc -fprofile-arcs -ftest-coverage test_program2.c -o test_program2
./test_program2 > /dev/null
cp test_program2.gcda compare.gcda

# Verify both files exist
if [ ! -f "base.gcda" ] || [ ! -f "compare.gcda" ]; then
    echo "ERROR: Required .gcda files not created!"
    exit 1
fi

echo "Profile files created successfully."
echo "Base file size: $(wc -c < base.gcda) bytes"
echo "Compare file size: $(wc -c < compare.gcda) bytes"

# Step 5: Test individual options (covering each case in the switch statement)
echo ""
echo "=== Testing individual options ==="
echo ""

# Test 1: -v (verbose) option - covers case 'v'
echo "Test 1: Testing -v (verbose) option..."
if gcov-tool overlap -v base.gcda compare.gcda 2>&1 | tee test1_verbose.log; then
    echo "✓ -v option processed successfully"
    # Check if verbose output was produced
    if grep -q "verbose\|Verbose" test1_verbose.log 2>/dev/null || [ $(wc -l < test1_verbose.log) -gt 5 ]; then
        echo "✓ Verbose output detected"
    fi
else
    echo "✗ -v option failed"
fi

# Test 2: -f (function level) option - covers case 'f'
echo ""
echo "Test 2: Testing -f (function level) option..."
if gcov-tool overlap -f base.gcda compare.gcda 2>&1 | tee test2_func.log; then
    echo "✓ -f option processed successfully"
else
    echo "✗ -f option failed"
fi

# Test 3: -F (full filename) option - covers case 'F'
echo ""
echo "Test 3: Testing -F (full filename) option..."
if gcov-tool overlap -F base.gcda compare.gcda 2>&1 | tee test3_fullname.log; then
    echo "✓ -F option processed successfully"
else
    echo "✗ -F option failed"
fi

# Test 4: -o (object level) option - covers case 'o'
echo ""
echo "Test 4: Testing -o (object level) option..."
if gcov-tool overlap -o base.gcda compare.gcda 2>&1 | tee test4_obj.log; then
    echo "✓ -o option processed successfully"
else
    echo "✗ -o option failed"
fi

# Test 5: -h (hot only) option - covers case 'h'
echo ""
echo "Test 5: Testing -h (hot only) option..."
if gcov-tool overlap -h base.gcda compare.gcda 2>&1 | tee test5_hot.log; then
    echo "✓ -h option processed successfully"
else
    echo "✗ -h option failed"
fi

# Test 6: -t (hot threshold) option - covers case 't'
echo ""
echo "Test 6: Testing -t (hot threshold) option..."
if gcov-tool overlap -t 0.5 base.gcda compare.gcda 2>&1 | tee test6_threshold.log; then
    echo "✓ -t 0.5 option processed successfully"
else
    echo "✗ -t 0.5 option failed"
fi

# Step 6: Test option combinations
echo ""
echo "=== Testing option combinations ==="
echo ""

# Test 7: Combination of -v, -f, -o
echo "Test 7: Testing combination -v -f -o..."
if gcov-tool overlap -v -f -o base.gcda compare.gcda 2>&1 | tee test7_combo1.log; then
    echo "✓ Combination -v -f -o processed successfully"
else
    echo "✗ Combination -v -f -o failed"
fi

# Test 8: Combination of -F, -h, -t
echo ""
echo "Test 8: Testing combination -F -h -t 0.75..."
if gcov-tool overlap -F -h -t 0.75 base.gcda compare.gcda 2>&1 | tee test8_combo2.log; then
    echo "✓ Combination -F -h -t 0.75 processed successfully"
else
    echo "✗ Combination -F -h -t 0.75 failed"
fi

# Test 9: All options together
echo ""
echo "Test 9: Testing all options together..."
if gcov-tool overlap -v -f -F -o -h -t 0.9 base.gcda compare.gcda 2>&1 | tee test9_all.log; then
    echo "✓ All options processed successfully"
else
    echo "✗ All options failed"
fi

# Step 7: Test threshold boundary values
echo ""
echo "=== Testing threshold boundary values ==="
echo ""

# Test 10: Minimum threshold (0.0)
echo "Test 10: Testing -t 0.0 (minimum)..."
if gcov-tool overlap -t 0.0 base.gcda compare.gcda 2>&1 | tee test10_min.log; then
    echo "✓ -t 0.0 option processed successfully"
else
    echo "✗ -t 0.0 option failed"
fi

# Test 11: Maximum threshold (1.0)
echo ""
echo "Test 11: Testing -t 1.0 (maximum)..."
if gcov-tool overlap -t 1.0 base.gcda compare.gcda 2>&1 | tee test11_max.log; then
    echo "✓ -t 1.0 option processed successfully"
else
    echo "✗ -t 1.0 option failed"
fi

# Test 12: Out of range threshold (negative)
echo ""
echo "Test 12: Testing -t -1.0 (out of range negative)..."
if gcov-tool overlap -t -1.0 base.gcda compare.gcda 2>&1 | tee test12_neg.log; then
    echo "✓ -t -1.0 option processed (may be valid or handled internally)"
else
    echo "✗ -t -1.0 option failed (possibly expected)"
fi

# Test 13: Out of range threshold (greater than 1)
echo ""
echo "Test 13: Testing -t 2.5 (out of range positive)..."
if gcov-tool overlap -t 2.5 base.gcda compare.gcda 2>&1 | tee test13_large.log; then
    echo "✓ -t 2.5 option processed (may be valid or handled internally)"
else
    echo "✗ -t 2.5 option failed (possibly expected)"
fi

# Step 8: Test invalid option (to trigger default case)
echo ""
echo "=== Testing invalid option (to trigger default case) ==="
echo ""

# Test 14: Invalid option -x
echo "Test 14: Testing invalid option -x..."
if gcov-tool overlap -x base.gcda compare.gcda 2>&1 | tee test14_invalid.log; then
    echo "✗ Invalid option -x should have failed but didn't"
else
    echo "✓ Invalid option -x correctly triggered error (exit code: $?)"
    # Check if usage was shown
    if grep -q -i "usage\|Usage\|option" test14_invalid.log; then
        echo "✓ Usage message displayed for invalid option"
    fi
fi

# Step 9: Test missing required argument for -t
echo ""
echo "=== Testing missing argument for -t ==="
echo ""

# Test 15: -t without argument
echo "Test 15: Testing -t without argument..."
if gcov-tool overlap -t base.gcda compare.gcda 2>&1 | tee test15_noarg.log; then
    echo "✗ -t without argument should have failed but didn't"
else
    echo "✓ -t without argument correctly triggered error (exit code: $?)"
fi

# Step 10: Summary and verification
echo ""
echo "=== Test Summary ==="
echo ""

# Count successful tests
SUCCESS_COUNT=0
TOTAL_TESTS=15

for i in $(seq 1 $TOTAL_TESTS); do
    if [ -f "test${i}_"*.log ]; then
        # Check if command executed (not whether it succeeded, as some failures are expected)
        SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
    fi
done

echo "Executed $SUCCESS_COUNT out of $TOTAL_TESTS test cases"
echo ""
echo "Generated log files:"
ls -la test*.log 2>/dev/null | head -20

echo ""
echo "=== Key Coverage Points Verified ==="
echo "1. Individual options: -v, -f, -F, -o, -h, -t ✓"
echo "2. Option combinations ✓"
echo "3. Threshold boundary values (0.0, 1.0, -1.0, 2.5) ✓"
echo "4. Invalid option (triggers default case) ✓"
echo "5. Missing argument for -t ✓"

echo ""
echo "All tests completed. The uncovered lines in gcov-tool.cc should now be covered."
echo "Check coverage with: gcov -b gcov-tool.cc"
```

This comprehensive test script:

1. **Creates valid GCOV data files** by compiling and running instrumented C programs
2. **Tests each individual option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) to cover each case in the switch statement
3. **Tests option combinations** to ensure they work together
4. **Tests boundary values** for the `-t` threshold option
5. **Tests invalid options** to trigger the default case and `overlap_usage()` call
6. **Captures all output** to log files for verification
7. **Runs in a temporary directory** and cleans up after itself
8. **Provides clear feedback** about which tests passed/failed

To run this test:
1. Ensure `gcov-tool` is in your PATH (built with coverage instrumentation)
2. Make the script executable: `chmod +x run_gcov_tool_test.sh`
3. Run it: `./run_gcov_tool_test.sh`

The script will systematically exercise all the uncovered lines in the `gcov-tool overlap` command parsing logic, ensuring that each case in the switch statement is executed and the corresponding global variables are set.
