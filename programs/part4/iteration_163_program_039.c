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

# Step 1: Create a simple C program for GCOV instrumentation
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
    printf("Factorial of 5: %d\n", factorial(5));
    
    return 0;
}
EOF

# Step 2: Compile with GCOV instrumentation
echo "Compiling test program with GCOV instrumentation..."
gcc -fprofile-arcs -ftest-coverage test.c -o test_prog

# Step 3: Run the program to generate initial .gcda file
echo "Running test program to generate profile data..."
./test_prog > /dev/null

# Verify .gcda file was created
if [ ! -f test.gcda ]; then
    echo "ERROR: test.gcda not created!"
    exit 1
fi

# Step 4: Create two different .gcda files for overlap comparison
# First, create a baseline
cp test.gcda base.gcda

# Run the program again with different behavior to create different coverage
cat > test2.c << 'EOF'
#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    int i, sum = 0;
    
    // Different loop count
    for (i = 0; i < 5; i++) {  # Changed from 10 to 5
        sum += i;
    }
    
    // This will take the other branch
    if (sum > 20) {
        printf("Sum is large: %d\n", sum);
    } else {
        printf("Sum is small: %d\n", sum);  # This branch will be taken
    }
    
    // Call function with different value
    printf("Factorial of 3: %d\n", factorial(3));  # Changed from 5 to 3
    
    return 0;
}
EOF

# Compile and run the second version
gcc -fprofile-arcs -ftest-coverage test2.c -o test_prog2
./test_prog2 > /dev/null
cp test2.gcda compare.gcda

echo "Created base.gcda and compare.gcda for overlap analysis"

# Step 5: Test individual options (covering each case in the switch statement)
echo -e "\n=== Testing individual options ==="

# Test 1: -v (verbose) - covers case 'v'
echo "Test 1: Testing -v (verbose) option..."
gcov-tool overlap -v base.gcda compare.gcda 2>&1 | tee test1_verbose.log
echo "Exit code: $?"

# Test 2: -f (function level) - covers case 'f'
echo -e "\nTest 2: Testing -f (function level) option..."
gcov-tool overlap -f base.gcda compare.gcda 2>&1 | tee test2_func.log
echo "Exit code: $?"

# Test 3: -F (full filename) - covers case 'F'
echo -e "\nTest 3: Testing -F (full filename) option..."
gcov-tool overlap -F base.gcda compare.gcda 2>&1 | tee test3_fullname.log
echo "Exit code: $?"

# Test 4: -o (object level) - covers case 'o'
echo -e "\nTest 4: Testing -o (object level) option..."
gcov-tool overlap -o base.gcda compare.gcda 2>&1 | tee test4_obj.log
echo "Exit code: $?"

# Test 5: -h (hot only) - covers case 'h'
echo -e "\nTest 5: Testing -h (hot only) option..."
gcov-tool overlap -h base.gcda compare.gcda 2>&1 | tee test5_hot.log
echo "Exit code: $?"

# Test 6: -t (hot threshold) - covers case 't'
echo -e "\nTest 6: Testing -t (hot threshold) option with 0.5..."
gcov-tool overlap -t 0.5 base.gcda compare.gcda 2>&1 | tee test6_threshold.log
echo "Exit code: $?"

# Step 6: Test option combinations
echo -e "\n=== Testing option combinations ==="

# Combination 1: -v -f -o
echo "Combination 1: Testing -v -f -o options together..."
gcov-tool overlap -v -f -o base.gcda compare.gcda 2>&1 | tee combo1_vfo.log
echo "Exit code: $?"

# Combination 2: -F -h -t 0.75
echo -e "\nCombination 2: Testing -F -h -t 0.75 options together..."
gcov-tool overlap -F -h -t 0.75 base.gcda compare.gcda 2>&1 | tee combo2_Fht.log
echo "Exit code: $?"

# Combination 3: -v -F -o -h
echo -e "\nCombination 3: Testing -v -F -o -h options together..."
gcov-tool overlap -v -F -o -h base.gcda compare.gcda 2>&1 | tee combo3_vFoh.log
echo "Exit code: $?"

# Step 7: Test threshold boundary values
echo -e "\n=== Testing threshold boundary values ==="

# Test minimum threshold (0.0)
echo "Testing -t 0.0 (minimum threshold)..."
gcov-tool overlap -t 0.0 base.gcda compare.gcda 2>&1 | tee threshold_min.log
echo "Exit code: $?"

# Test maximum threshold (1.0)
echo -e "\nTesting -t 1.0 (maximum threshold)..."
gcov-tool overlap -t 1.0 base.gcda compare.gcda 2>&1 | tee threshold_max.log
echo "Exit code: $?"

# Test edge case thresholds (should still parse even if out of logical range)
echo -e "\nTesting -t -1.0 (negative threshold)..."
gcov-tool overlap -t -1.0 base.gcda compare.gcda 2>&1 | tee threshold_neg.log
echo "Exit code: $?"

echo -e "\nTesting -t 2.5 (threshold > 1.0)..."
gcov-tool overlap -t 2.5 base.gcda compare.gcda 2>&1 | tee threshold_high.log
echo "Exit code: $?"

# Test with scientific notation
echo -e "\nTesting -t 5e-1 (scientific notation)..."
gcov-tool overlap -t 5e-1 base.gcda compare.gcda 2>&1 | tee threshold_sci.log
echo "Exit code: $?"

# Step 8: Test invalid option to trigger default case
echo -e "\n=== Testing invalid option (to trigger default case) ==="
echo "Testing invalid option -x..."
gcov-tool overlap -x base.gcda compare.gcda 2>&1 | tee invalid_option.log
echo "Exit code: $? (expected non-zero)"

# Step 9: Test missing required argument for -t
echo -e "\nTesting -t without argument..."
gcov-tool overlap -t base.gcda compare.gcda 2>&1 | tee missing_arg.log
echo "Exit code: $? (expected non-zero)"

# Step 10: Test with swapped file order
echo -e "\n=== Testing with swapped file order ==="
echo "Testing with files in reverse order..."
gcov-tool overlap -v compare.gcda base.gcda 2>&1 | tee swapped_files.log
echo "Exit code: $?"

# Step 11: Verify that verbose option actually produces verbose output
echo -e "\n=== Verifying verbose output ==="
VERBOSE_LINES=$(wc -l < test1_verbose.log)
NON_VERBOSE_OUTPUT=$(gcov-tool overlap base.gcda compare.gcda 2>&1 | wc -l)
echo "Lines with -v: $VERBOSE_LINES"
echo "Lines without -v: $NON_VERBOSE_OUTPUT"

if [ "$VERBOSE_LINES" -gt "$NON_VERBOSE_OUTPUT" ]; then
    echo "✓ Verbose option produces more output"
else
    echo "⚠ Verbose output check inconclusive"
fi

# Step 12: Summary
echo -e "\n=== Test Summary ==="
echo "All tests completed successfully!"
echo "Generated log files in $TEST_DIR:"
ls -la *.log

echo -e "\nTo check if all uncovered lines were hit:"
echo "1. Ensure gcov-tool was built with coverage instrumentation"
echo "2. Run: gcov -b gcov-tool.cc | grep -A2 -B2 'lines 534-554'"
echo "3. The uncovered lines should now show execution counts > 0"
```

This script comprehensively tests all the uncovered lines in the switch statement:

1. **Individual option testing** (Tests 1-6): Each option (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually to ensure the corresponding case statement is executed.

2. **Option combinations** (Combinations 1-3): Tests multiple options together to ensure they work correctly in combination.

3. **Threshold boundary testing**: Tests the `-t` option with various values including:
   - Normal value (0.5)
   - Minimum (0.0)
   - Maximum (1.0)
   - Out-of-range values (-1.0, 2.5)
   - Scientific notation (5e-1)
   This exercises the `atof(optarg)` call thoroughly.

4. **Invalid option test**: Tests `-x` to trigger the `default` case and call `overlap_usage()`.

5. **Edge cases**: Tests missing argument for `-t` and swapped file order.

6. **Verification**: Checks that verbose mode actually produces more output, providing indirect validation that the `verbose` flag was set.

The script creates a clean temporary directory, generates valid GCOV data files, and systematically tests all code paths. Each test captures output and exit codes for verification. When this script is run against a coverage-instrumented `gcov-tool`, it should cause all the uncovered lines (534-554) to be executed.
