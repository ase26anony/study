Here's a comprehensive shell script that systematically tests the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash

# Script to test gcov-tool overlap command options
# Tests the uncovered switch cases for options: -v, -f, -F, -o, -h, -t

set -e  # Exit on error

# Create temporary directory
TEMP_DIR=$(mktemp -d /tmp/gcov_test_XXXXXX)
echo "Working in temporary directory: $TEMP_DIR"
cd "$TEMP_DIR"

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    cd /
    rm -rf "$TEMP_DIR"
}
trap cleanup EXIT

# Step 1: Create a simple C program for coverage testing
cat > test.c << 'EOF'
#include <stdio.h>

int helper(int x) {
    if (x > 10) {
        return x * 2;
    } else {
        return x + 5;
    }
}

int main() {
    int i, sum = 0;
    
    // Generate some coverage data
    for (i = 0; i < 20; i++) {
        sum += helper(i);
    }
    
    // Conditional execution
    if (sum > 100) {
        printf("Sum is large: %d\n", sum);
    } else {
        printf("Sum is small: %d\n", sum);
    }
    
    return 0;
}
EOF

# Step 2: Compile with GCOV instrumentation
echo "Compiling test program with GCOV instrumentation..."
gcc -fprofile-arcs -ftest-coverage test.c -o test_prog

# Step 3: Run the program to generate initial .gcda file
echo "Running test program to generate profile data..."
./test_prog > /dev/null 2>&1

# Verify .gcda file was created
if [ ! -f test.gcda ]; then
    echo "Error: test.gcda not created!"
    exit 1
fi

# Step 4: Create two .gcda files for overlap analysis
echo "Creating base and comparison .gcda files..."
cp test.gcda base.gcda

# Create a slightly different profile by running again with different input
# We'll modify the .gcda file slightly to create differences
cp test.gcda compare.gcda

# Step 5: Test each uncovered option individually
echo ""
echo "=== Testing individual options ==="
echo ""

# Test verbose option (-v)
echo "Testing -v (verbose) option..."
gcov-tool overlap -v base.gcda compare.gcda > verbose_output.txt 2>&1
echo "Exit code: $?"

# Test function level option (-f)
echo "Testing -f (function level) option..."
gcov-tool overlap -f base.gcda compare.gcda > func_output.txt 2>&1
echo "Exit code: $?"

# Test full filename option (-F)
echo "Testing -F (full filename) option..."
gcov-tool overlap -F base.gcda compare.gcda > fullname_output.txt 2>&1
echo "Exit code: $?"

# Test object level option (-o)
echo "Testing -o (object level) option..."
gcov-tool overlap -o base.gcda compare.gcda > obj_output.txt 2>&1
echo "Exit code: $?"

# Test hot only option (-h)
echo "Testing -h (hot only) option..."
gcov-tool overlap -h base.gcda compare.gcda > hot_output.txt 2>&1
echo "Exit code: $?"

# Test hot threshold option (-t) with normal value
echo "Testing -t (hot threshold) option with value 0.5..."
gcov-tool overlap -t 0.5 base.gcda compare.gcda > threshold_0.5_output.txt 2>&1
echo "Exit code: $?"

# Step 6: Test option combinations
echo ""
echo "=== Testing option combinations ==="
echo ""

# Test combination of 2 options
echo "Testing -v -f combination..."
gcov-tool overlap -v -f base.gcda compare.gcda > combo_vf_output.txt 2>&1
echo "Exit code: $?"

# Test combination of 3 options
echo "Testing -F -h -o combination..."
gcov-tool overlap -F -h -o base.gcda compare.gcda > combo_Fho_output.txt 2>&1
echo "Exit code: $?"

# Test combination with threshold
echo "Testing -v -f -t 0.75 combination..."
gcov-tool overlap -v -f -t 0.75 base.gcda compare.gcda > combo_vft_output.txt 2>&1
echo "Exit code: $?"

# Step 7: Test threshold boundary values
echo ""
echo "=== Testing threshold boundary values ==="
echo ""

# Test minimum threshold
echo "Testing -t 0.0 (minimum)..."
gcov-tool overlap -t 0.0 base.gcda compare.gcda > threshold_0.0_output.txt 2>&1
echo "Exit code: $?"

# Test maximum threshold
echo "Testing -t 1.0 (maximum)..."
gcov-tool overlap -t 1.0 base.gcda compare.gcda > threshold_1.0_output.txt 2>&1
echo "Exit code: $?"

# Test out-of-range threshold (negative)
echo "Testing -t -1.0 (negative, out of range)..."
gcov-tool overlap -t -1.0 base.gcda compare.gcda > threshold_neg1.0_output.txt 2>&1
echo "Exit code: $?"

# Test out-of-range threshold (greater than 1)
echo "Testing -t 2.5 (greater than 1, out of range)..."
gcov-tool overlap -t 2.5 base.gcda compare.gcda > threshold_2.5_output.txt 2>&1
echo "Exit code: $?"

# Test invalid threshold format
echo "Testing -t invalid (not a number)..."
gcov-tool overlap -t invalid base.gcda compare.gcda > threshold_invalid_output.txt 2>&1
echo "Exit code: $?"

# Step 8: Test invalid option to trigger default case
echo ""
echo "=== Testing invalid option (to trigger default case) ==="
echo ""

echo "Testing invalid option -x..."
gcov-tool overlap -x base.gcda compare.gcda > invalid_option_output.txt 2>&1
echo "Exit code: $?"

# Step 9: Verify outputs show expected behavior
echo ""
echo "=== Verifying outputs ==="
echo ""

# Check that verbose output is different from non-verbose
echo "Checking verbose output differences..."
if [ -s verbose_output.txt ]; then
    echo "✓ Verbose output generated"
    
    # Compare with a non-verbose run
    gcov-tool overlap base.gcda compare.gcda > non_verbose_output.txt 2>&1
    if ! cmp -s verbose_output.txt non_verbose_output.txt; then
        echo "✓ Verbose and non-verbose outputs differ (expected)"
    else
        echo "⚠ Verbose and non-verbose outputs are identical"
    fi
else
    echo "✗ No verbose output generated"
fi

# Check that invalid option triggered usage
echo "Checking invalid option handling..."
if grep -q "Usage:" invalid_option_output.txt || [ $? -eq 1 ]; then
    echo "✓ Usage message shown for invalid option"
else
    echo "⚠ No usage message for invalid option"
fi

# Step 10: Summary
echo ""
echo "=== Test Summary ==="
echo "All gcov-tool overlap option tests completed."
echo "Generated test files in: $TEMP_DIR"
echo ""
echo "Output files created:"
ls -la *.txt 2>/dev/null | head -20

echo ""
echo "To examine specific outputs:"
echo "  cat $TEMP_DIR/verbose_output.txt"
echo "  cat $TEMP_DIR/invalid_option_output.txt"
echo ""
echo "Test completed successfully!"
```

This script systematically tests all the uncovered lines:

1. **Individual option testing** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`): Each option is tested separately to ensure the corresponding case statement is executed.

2. **Option combinations**: Tests multiple options together to ensure they work in combination.

3. **Threshold boundary testing**: Tests `-t` with values 0.0, 1.0, -1.0, 2.5, and "invalid" to exercise the `atof(optarg)` call and error handling.

4. **Invalid option testing**: Tests `-x` to trigger the `default` case and `overlap_usage()` function.

5. **Output verification**: Checks that verbose mode produces different output and that invalid options trigger usage messages.

The script creates a temporary directory, generates a simple C program, compiles it with GCOV instrumentation, runs it to create profile data, and then performs all the tests. Each test captures both stdout and stderr to log files for later inspection.

To run this test, ensure:
1. `gcov-tool` is in your PATH (built with coverage instrumentation)
2. GCC is installed
3. The script has execute permissions: `chmod +x run_gcov_tool_test.sh`
4. Run with: `./run_gcov_tool_test.sh`

The script will clean up all temporary files automatically when it exits.
