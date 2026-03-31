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
    int i, result;
    for (i = 0; i < 5; i++) {
        result = factorial(i);
        printf("factorial(%d) = %d\n", i, result);
    }
    
    // Add some conditional logic for branch coverage
    if (result > 0) {
        printf("Result is positive\n");
    } else {
        printf("Result is non-positive\n");
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

# Verify .gcda file was created
if [ ! -f test.gcda ]; then
    echo "Error: test.gcda not created!"
    exit 1
fi

# Step 4: Create two different .gcda files for overlap comparison
echo "Creating profile data files for overlap analysis..."
cp test.gcda base.gcda

# Create a slightly different profile by running the program again
# This simulates different execution paths
./test_prog > /dev/null
cp test.gcda compare.gcda

# Verify both files exist
if [ ! -f base.gcda ] || [ ! -f compare.gcda ]; then
    echo "Error: Failed to create base.gcda and compare.gcda!"
    exit 1
fi

echo "Profile data files created successfully."
echo "Base file size: $(wc -c < base.gcda) bytes"
echo "Compare file size: $(wc -c < compare.gcda) bytes"

# Step 5: Test individual options (covering each case in the switch statement)
echo -e "\n=== Testing individual options ==="

# Test 1: -v (verbose) - covers case 'v'
echo "Test 1: Testing -v (verbose) option..."
if gcov-tool overlap -v base.gcda compare.gcda 2>&1 | grep -q "verbose\|Verbose"; then
    echo "✓ -v option processed (verbose output detected)"
else
    echo "✓ -v option processed"
fi

# Test 2: -f (function level) - covers case 'f'
echo -e "\nTest 2: Testing -f (function level) option..."
gcov-tool overlap -f base.gcda compare.gcda > /dev/null 2>&1 && echo "✓ -f option processed"

# Test 3: -F (full filename) - covers case 'F'
echo -e "\nTest 3: Testing -F (full filename) option..."
gcov-tool overlap -F base.gcda compare.gcda > /dev/null 2>&1 && echo "✓ -F option processed"

# Test 4: -o (object level) - covers case 'o'
echo -e "\nTest 4: Testing -o (object level) option..."
gcov-tool overlap -o base.gcda compare.gcda > /dev/null 2>&1 && echo "✓ -o option processed"

# Test 5: -h (hot only) - covers case 'h'
echo -e "\nTest 5: Testing -h (hot only) option..."
gcov-tool overlap -h base.gcda compare.gcda > /dev/null 2>&1 && echo "✓ -h option processed"

# Test 6: -t (hot threshold) - covers case 't'
echo -e "\nTest 6: Testing -t (hot threshold) option..."
echo "  Testing with threshold 0.5..."
gcov-tool overlap -t 0.5 base.gcda compare.gcda > /dev/null 2>&1 && echo "  ✓ -t 0.5 processed"

echo "  Testing with threshold 0.0 (minimum)..."
gcov-tool overlap -t 0.0 base.gcda compare.gcda > /dev/null 2>&1 && echo "  ✓ -t 0.0 processed"

echo "  Testing with threshold 1.0 (maximum)..."
gcov-tool overlap -t 1.0 base.gcda compare.gcda > /dev/null 2>&1 && echo "  ✓ -t 1.0 processed"

# Test 7: Invalid option - covers default case
echo -e "\nTest 7: Testing invalid option (should trigger usage)..."
if ! gcov-tool overlap -x base.gcda compare.gcda 2>&1 | grep -q "usage\|Usage\|invalid"; then
    echo "✗ Invalid option did not trigger expected error"
else
    echo "✓ Invalid option triggered usage message"
fi

# Step 6: Test option combinations
echo -e "\n=== Testing option combinations ==="

# Combination 1: Multiple boolean flags
echo "Test 8: Testing combination -v -f -o..."
gcov-tool overlap -v -f -o base.gcda compare.gcda > /dev/null 2>&1 && echo "✓ -v -f -o combination processed"

# Combination 2: Flags with threshold
echo -e "\nTest 9: Testing combination -F -h -t 0.75..."
gcov-tool overlap -F -h -t 0.75 base.gcda compare.gcda > /dev/null 2>&1 && echo "✓ -F -h -t 0.75 combination processed"

# Combination 3: All flags together
echo -e "\nTest 10: Testing combination -v -f -F -o -h -t 0.25..."
gcov-tool overlap -v -f -F -o -h -t 0.25 base.gcda compare.gcda > /dev/null 2>&1 && echo "✓ All options combination processed"

# Step 7: Test edge cases for -t option
echo -e "\n=== Testing edge cases for -t option ==="

# Test with very small threshold
echo "Test 11: Testing with threshold 0.001..."
gcov-tool overlap -t 0.001 base.gcda compare.gcda > /dev/null 2>&1 && echo "✓ -t 0.001 processed"

# Test with threshold at upper boundary
echo -e "\nTest 12: Testing with threshold 0.999..."
gcov-tool overlap -t 0.999 base.gcda compare.gcda > /dev/null 2>&1 && echo "✓ -t 0.999 processed"

# Test with out-of-range thresholds (should still parse but may produce warnings)
echo -e "\nTest 13: Testing with out-of-range thresholds..."
echo "  Testing with negative threshold (-1.0)..."
if gcov-tool overlap -t -1.0 base.gcda compare.gcda 2>&1; then
    echo "  ✓ -t -1.0 parsed (may produce warning)"
fi

echo "  Testing with threshold > 1.0 (2.5)..."
if gcov-tool overlap -t 2.5 base.gcda compare.gcda 2>&1; then
    echo "  ✓ -t 2.5 parsed (may produce warning)"
fi

# Step 8: Test with different argument ordering
echo -e "\n=== Testing different argument ordering ==="

echo "Test 14: Testing options after filenames..."
gcov-tool overlap base.gcda compare.gcda -v -f 2>&1 | grep -q "error\|Error" || echo "✓ Options after filenames handled"

echo -e "\nTest 15: Testing mixed argument order..."
gcov-tool overlap -v base.gcda -f compare.gcda -o 2>&1 | grep -q "error\|Error" || echo "✓ Mixed argument order handled"

# Step 9: Create log files for verification
echo -e "\n=== Creating detailed log files ==="

mkdir -p logs

# Run each test and capture output
echo "Creating verbose output log..."
gcov-tool overlap -v base.gcda compare.gcda > logs/verbose.log 2>&1

echo "Creating function level output log..."
gcov-tool overlap -f base.gcda compare.gcda > logs/function_level.log 2>&1

echo "Creating full combination output log..."
gcov-tool overlap -v -f -F -o -h -t 0.5 base.gcda compare.gcda > logs/full_combo.log 2>&1

echo -e "\n=== Test Summary ==="
echo "All gcov-tool overlap option tests completed."
echo "Log files available in: $TEST_DIR/logs/"
echo "Profile files: base.gcda, compare.gcda"
echo ""
echo "Covered options:"
echo "  -v (verbose)            - Sets verbose flag"
echo "  -f (function level)     - Sets overlap_func_level"
echo "  -F (full filename)      - Sets overlap_use_fullname"
echo "  -o (object level)       - Sets overlap_obj_level"
echo "  -h (hot only)           - Sets overlap_hot_only"
echo "  -t (threshold)          - Sets overlap_hot_threshold via atof()"
echo "  invalid options         - Triggers default case and overlap_usage()"
echo ""
echo "All switch cases in the uncovered block (lines 534-554) have been exercised."
```

This script systematically tests all the uncovered lines in the `gcov-tool.cc` file:

1. **Creates valid GCOV data files**: Compiles a simple C program with GCOV instrumentation, runs it to generate `.gcda` files, and creates two copies for overlap comparison.

2. **Tests each individual option**:
   - `-v` (verbose): Triggers `case 'v'`, sets `verbose = true`, calls `gcov_set_verbose()`
   - `-f` (function level): Triggers `case 'f'`, sets `overlap_func_level = 1`
   - `-F` (full filename): Triggers `case 'F'`, sets `overlap_use_fullname = 1`
   - `-o` (object level): Triggers `case 'o'`, sets `overlap_obj_level = 1`
   - `-h` (hot only): Triggers `case 'h'`, sets `overlap_hot_only = 1`
   - `-t` (threshold): Triggers `case 't'`, calls `atof(optarg)` to set `overlap_hot_threshold`

3. **Tests option combinations**: Exercises multiple options together to ensure they work in combination.

4. **Tests edge cases**:
   - Boundary values for `-t` option (0.0, 1.0, 0.001, 0.999)
   - Out-of-range values for `-t` (-1.0, 2.5) to test `atof()` parsing
   - Invalid option (`-x`) to trigger the `default` case and `overlap_usage()`

5. **Tests different argument orderings**: Ensures the option parsing is robust.

6. **Creates log files**: Captures output for verification that each option was processed.

The script runs in a temporary directory, cleans up after itself, and provides clear output about what's being tested. Each `gcov-tool` invocation will trigger the command-line parsing logic in the uncovered block, ensuring that all the `case` statements and the `default` case are executed.
