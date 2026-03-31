I'll create a comprehensive test script that covers all the uncovered lines in the gcov-tool overlap command parsing logic.

```bash
#!/bin/bash

# Test script for gcov-tool overlap command line parsing coverage
# This script generates GCOV data files and tests all uncovered options

set -e

# Create a temporary directory for our test
TEMP_DIR=$(mktemp -d /tmp/gcov_tool_test_XXXXXX)
echo "Working in temporary directory: $TEMP_DIR"
cd "$TEMP_DIR"

# Cleanup function
cleanup() {
    echo "Cleaning up temporary directory: $TEMP_DIR"
    cd /
    rm -rf "$TEMP_DIR"
}

# Trap EXIT to ensure cleanup
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
    
    // Conditional branches
    if (sum > 20) {
        printf("Sum is large: %d\n", sum);
    } else {
        printf("Sum is small: %d\n", sum);
    }
    
    // Function call
    int fact = factorial(5);
    printf("Factorial of 5 is: %d\n", fact);
    
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
    echo "Error: test.gcda file not created!"
    exit 1
fi

# Step 4: Create two different .gcda files for overlap analysis
# First file: base profile
cp test.gcda base.gcda

# Run program again with different behavior to create different profile
cat > test2.c << 'EOF'
#include <stdio.h>

int main() {
    int i, sum = 0;
    
    // Different loop count
    for (i = 0; i < 5; i++) {
        sum += i;
    }
    
    // Different branch taken
    if (sum < 20) {
        printf("Different sum: %d\n", sum);
    }
    
    return 0;
}
EOF

gcc -fprofile-arcs -ftest-coverage test2.c -o test2_prog
./test2_prog > /dev/null
cp test2.gcda compare.gcda

# Verify both files exist
if [ ! -f base.gcda ] || [ ! -f compare.gcda ]; then
    echo "Error: Required .gcda files not created!"
    exit 1
fi

echo "Created base.gcda and compare.gcda for overlap analysis"

# Step 5: Test individual uncovered options
echo -e "\n=== Testing individual options ==="

# Test 1: -v (verbose)
echo "Test 1: Testing -v (verbose) option..."
gcov-tool overlap -v base.gcda compare.gcda 2>&1 | tee test_v.log
echo "Exit code: $?"

# Test 2: -f (function level)
echo -e "\nTest 2: Testing -f (function level) option..."
gcov-tool overlap -f base.gcda compare.gcda 2>&1 | tee test_f.log
echo "Exit code: $?"

# Test 3: -F (full filename)
echo -e "\nTest 3: Testing -F (full filename) option..."
gcov-tool overlap -F base.gcda compare.gcda 2>&1 | tee test_F.log
echo "Exit code: $?"

# Test 4: -o (object level)
echo -e "\nTest 4: Testing -o (object level) option..."
gcov-tool overlap -o base.gcda compare.gcda 2>&1 | tee test_o.log
echo "Exit code: $?"

# Test 5: -h (hot only)
echo -e "\nTest 5: Testing -h (hot only) option..."
gcov-tool overlap -h base.gcda compare.gcda 2>&1 | tee test_h.log
echo "Exit code: $?"

# Test 6: -t (hot threshold) with valid argument
echo -e "\nTest 6: Testing -t (hot threshold) with argument 0.5..."
gcov-tool overlap -t 0.5 base.gcda compare.gcda 2>&1 | tee test_t_0.5.log
echo "Exit code: $?"

# Step 6: Test option combinations
echo -e "\n=== Testing option combinations ==="

# Combination 1: -v -f -o
echo "Combination 1: Testing -v -f -o..."
gcov-tool overlap -v -f -o base.gcda compare.gcda 2>&1 | tee test_vfo.log
echo "Exit code: $?"

# Combination 2: -F -h -t
echo -e "\nCombination 2: Testing -F -h -t 0.75..."
gcov-tool overlap -F -h -t 0.75 base.gcda compare.gcda 2>&1 | tee test_Fht.log
echo "Exit code: $?"

# Combination 3: All options together
echo -e "\nCombination 3: Testing all options together..."
gcov-tool overlap -v -f -F -o -h -t 0.3 base.gcda compare.gcda 2>&1 | tee test_all.log
echo "Exit code: $?"

# Step 7: Test threshold boundary values
echo -e "\n=== Testing threshold boundary values ==="

# Test minimum threshold (0.0)
echo "Testing -t 0.0 (minimum)..."
gcov-tool overlap -t 0.0 base.gcda compare.gcda 2>&1 | tee test_t_0.0.log
echo "Exit code: $?"

# Test maximum threshold (1.0)
echo -e "\nTesting -t 1.0 (maximum)..."
gcov-tool overlap -t 1.0 base.gcda compare.gcda 2>&1 | tee test_t_1.0.log
echo "Exit code: $?"

# Test out-of-range thresholds (should still parse but may produce warnings)
echo -e "\nTesting -t -1.0 (negative)..."
gcov-tool overlap -t -1.0 base.gcda compare.gcda 2>&1 | tee test_t_neg1.0.log
echo "Exit code: $?"

echo -e "\nTesting -t 2.5 (greater than 1.0)..."
gcov-tool overlap -t 2.5 base.gcda compare.gcda 2>&1 | tee test_t_2.5.log
echo "Exit code: $?"

# Test invalid threshold format
echo -e "\nTesting -t invalid (not a number)..."
gcov-tool overlap -t invalid base.gcda compare.gcda 2>&1 | tee test_t_invalid.log
echo "Exit code: $?"

# Step 8: Test invalid option to trigger default case
echo -e "\n=== Testing invalid option to trigger default case ==="
echo "Testing invalid option -x..."
gcov-tool overlap -x base.gcda compare.gcda 2>&1 | tee test_invalid.log
echo "Exit code: $?"

# Step 9: Test missing required argument for -t
echo -e "\nTesting -t without argument..."
gcov-tool overlap -t base.gcda compare.gcda 2>&1 | tee test_t_noarg.log
echo "Exit code: $?"

# Step 10: Test with different file order and additional arguments
echo -e "\n=== Testing additional scenarios ==="
echo "Testing with swapped file order..."
gcov-tool overlap -v compare.gcda base.gcda 2>&1 | tee test_swapped.log
echo "Exit code: $?"

# Step 11: Verify that verbose option actually produces verbose output
echo -e "\n=== Verifying verbose output ==="
echo "Comparing verbose vs non-verbose output size..."

# Get output sizes
NON_VERBOSE_SIZE=$(gcov-tool overlap base.gcda compare.gcda 2>&1 | wc -l)
VERBOSE_SIZE=$(gcov-tool overlap -v base.gcda compare.gcda 2>&1 | wc -l)

echo "Non-verbose output lines: $NON_VERBOSE_SIZE"
echo "Verbose output lines: $VERBOSE_SIZE"

if [ "$VERBOSE_SIZE" -gt "$NON_VERBOSE_SIZE" ]; then
    echo "✓ Verbose option produces more output (as expected)"
else
    echo "⚠ Verbose option may not be working as expected"
fi

# Step 12: Summary
echo -e "\n=== Test Summary ==="
echo "All gcov-tool overlap option tests completed."
echo "Log files created in: $TEMP_DIR"
echo "Check the log files for any error messages or unexpected behavior."

# List all generated log files
echo -e "\nGenerated log files:"
ls -la *.log

echo -e "\nTest completed successfully!"
```

This comprehensive test script:

1. **Creates valid GCOV data files** by compiling and running instrumented C programs
2. **Tests each individual uncovered option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`)
3. **Tests option combinations** to ensure they work together
4. **Tests boundary values** for the `-t` threshold option (0.0, 1.0, negative, >1.0)
5. **Tests invalid options** to trigger the `default` case and `overlap_usage()`
6. **Tests edge cases** like missing arguments for `-t`
7. **Verifies verbose output** by comparing output sizes
8. **Captures all output** to log files for analysis
9. **Cleans up automatically** using a temporary directory

To run this test:

1. Ensure `gcov-tool` is in your PATH (it's usually in the GCC build directory)
2. Make the script executable: `chmod +x run_gcov_tool_test.sh`
3. Run it: `./run_gcov_tool_test.sh`

The script will systematically exercise all the uncovered lines in the switch statement, ensuring that each case is executed and the corresponding global variables are set. The coverage instrumentation in `gcov-tool` will record that these code paths were executed.
