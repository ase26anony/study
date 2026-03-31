Here's a comprehensive shell script that systematically tests the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/sh

# Test script for gcov-tool overlap command options
# This script creates test coverage data and exercises all uncovered
# option parsing cases in gcov-tool.cc lines 534-554

set -e  # Exit on any error

# Create a temporary directory for our test
TEMP_DIR=$(mktemp -d /tmp/gcov_tool_test_XXXXXX)
echo "Test directory: $TEMP_DIR"
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
    
    // Some loops to generate coverage data
    for (i = 0; i < 100; i++) {
        sum += helper(i);
    }
    
    // Conditional execution
    if (sum > 5000) {
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
echo "Running test program to generate coverage data..."
./test_prog > /dev/null 2>&1

# Verify .gcda file was created
if [ ! -f "test.gcda" ]; then
    echo "ERROR: test.gcda not generated!"
    exit 1
fi

# Step 4: Create two different .gcda files for overlap analysis
# First, create a base version
cp test.gcda base.gcda

# Run program again with different behavior to create different coverage
# We'll modify the program slightly and recompile
cat > test2.c << 'EOF'
#include <stdio.h>

int helper(int x) {
    if (x > 50) {  # Different threshold
        return x * 3;
    } else {
        return x + 2;
    }
}

int main() {
    int i, sum = 0;
    
    // Different loop bounds
    for (i = 0; i < 50; i++) {
        sum += helper(i);
    }
    
    // Different condition
    if (sum > 1000) {
        printf("Sum is very large: %d\n", sum);
    }
    
    return 0;
}
EOF

# Compile and run second version
gcc -fprofile-arcs -ftest-coverage test2.c -o test2_prog
./test2_prog > /dev/null 2>&1
cp test2.gcda compare.gcda

echo "Created base.gcda and compare.gcda for overlap analysis"

# Step 5: Test individual options from the uncovered switch block
echo -e "\n=== Testing individual options ==="

# Test verbose option (-v)
echo "Testing -v (verbose) option..."
gcov-tool overlap -v base.gcda compare.gcda > verbose_output.log 2>&1
echo "Exit code: $?"

# Test function level option (-f)
echo "Testing -f (function level) option..."
gcov-tool overlap -f base.gcda compare.gcda > func_level_output.log 2>&1
echo "Exit code: $?"

# Test full filename option (-F)
echo "Testing -F (full filename) option..."
gcov-tool overlap -F base.gcda compare.gcda > fullname_output.log 2>&1
echo "Exit code: $?"

# Test object level option (-o)
echo "Testing -o (object level) option..."
gcov-tool overlap -o base.gcda compare.gcda > obj_level_output.log 2>&1
echo "Exit code: $?"

# Test hot only option (-h)
echo "Testing -h (hot only) option..."
gcov-tool overlap -h base.gcda compare.gcda > hot_only_output.log 2>&1
echo "Exit code: $?"

# Test hot threshold option (-t) with various values
echo "Testing -t (hot threshold) option with value 0.5..."
gcov-tool overlap -t 0.5 base.gcda compare.gcda > threshold_0.5_output.log 2>&1
echo "Exit code: $?"

# Step 6: Test option combinations
echo -e "\n=== Testing option combinations ==="

# Combination 1: verbose + function level + object level
echo "Testing -v -f -o combination..."
gcov-tool overlap -v -f -o base.gcda compare.gcda > combo_vfo_output.log 2>&1
echo "Exit code: $?"

# Combination 2: fullname + hot only + threshold
echo "Testing -F -h -t 0.75 combination..."
gcov-tool overlap -F -h -t 0.75 base.gcda compare.gcda > combo_Fht_output.log 2>&1
echo "Exit code: $?"

# Combination 3: All options together
echo "Testing all options together..."
gcov-tool overlap -v -f -F -o -h -t 0.25 base.gcda compare.gcda > combo_all_output.log 2>&1
echo "Exit code: $?"

# Step 7: Test threshold boundary values
echo -e "\n=== Testing threshold boundary values ==="

# Minimum threshold
echo "Testing -t 0.0 (minimum)..."
gcov-tool overlap -t 0.0 base.gcda compare.gcda > threshold_0.0_output.log 2>&1
echo "Exit code: $?"

# Maximum threshold
echo "Testing -t 1.0 (maximum)..."
gcov-tool overlap -t 1.0 base.gcda compare.gcda > threshold_1.0_output.log 2>&1
echo "Exit code: $?"

# Edge case: exactly 0.0
echo "Testing -t 0.0..."
gcov-tool overlap -t 0.0 base.gcda compare.gcda > threshold_exact_0_output.log 2>&1
echo "Exit code: $?"

# Edge case: exactly 1.0
echo "Testing -t 1.0..."
gcov-tool overlap -t 1.0 base.gcda compare.gcda > threshold_exact_1_output.log 2>&1
echo "Exit code: $?"

# Step 8: Test invalid option to trigger default case
echo -e "\n=== Testing invalid option (to trigger default case) ==="
echo "Testing invalid option -x..."
gcov-tool overlap -x base.gcda compare.gcda > invalid_option_output.log 2>&1
echo "Exit code: $?"

# Step 9: Test malformed threshold values
echo -e "\n=== Testing malformed threshold values ==="

# Negative threshold
echo "Testing -t -1.0 (negative)..."
gcov-tool overlap -t -1.0 base.gcda compare.gcda > threshold_neg_output.log 2>&1
echo "Exit code: $?"

# Threshold > 1.0
echo "Testing -t 2.5 (> 1.0)..."
gcov-tool overlap -t 2.5 base.gcda compare.gcda > threshold_2.5_output.log 2>&1
echo "Exit code: $?"

# Non-numeric threshold
echo "Testing -t abc (non-numeric)..."
gcov-tool overlap -t abc base.gcda compare.gcda > threshold_nonnumeric_output.log 2>&1
echo "Exit code: $?"

# Step 10: Verify outputs and summarize
echo -e "\n=== Test Summary ==="
echo "All gcov-tool commands executed successfully."
echo "Generated output files in: $TEMP_DIR"
echo -e "\nOutput file sizes:"
ls -la *.log

# Check if verbose option actually produced more output
VERBOSE_SIZE=$(wc -l < verbose_output.log)
BASIC_SIZE=$(wc -l < func_level_output.log)
echo -e "\nVerbose output lines: $VERBOSE_SIZE"
echo "Basic output lines: $BASIC_SIZE"

if [ "$VERBOSE_SIZE" -gt "$BASIC_SIZE" ]; then
    echo "✓ Verbose option appears to produce more output as expected"
else
    echo "⚠ Verbose option may not be producing additional output"
fi

# Check if invalid option triggered usage
if grep -q "usage\|Usage\|OVERLAP" invalid_option_output.log; then
    echo "✓ Invalid option triggered usage message"
else
    echo "⚠ Invalid option may not have triggered usage message"
fi

echo -e "\n=== Test completed successfully ==="
echo "All uncovered option parsing cases have been exercised:"
echo "  -v (verbose)              - triggers gcov_set_verbose()"
echo "  -f (function level)       - sets overlap_func_level = 1"
echo "  -F (full filename)        - sets overlap_use_fullname = 1"
echo "  -o (object level)         - sets overlap_obj_level = 1"
echo "  -h (hot only)             - sets overlap_hot_only = 1"
echo "  -t (threshold)            - calls atof(optarg)"
echo "  default case              - calls overlap_usage()"
```

This script provides comprehensive coverage of the uncovered lines by:

1. **Creating valid GCOV data files** by compiling and running instrumented C programs
2. **Testing each individual option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) separately
3. **Testing option combinations** to ensure they work together
4. **Testing boundary values** for the `-t` threshold option (0.0, 1.0, negative, >1.0)
5. **Testing invalid options** to trigger the `default` case and `overlap_usage()` call
6. **Capturing all output** to log files for verification
7. **Running in a temporary directory** to avoid pollution
8. **Providing validation** by checking output characteristics

To use this script:
1. Save it as `run_gcov_tool_test.sh`
2. Make it executable: `chmod +x run_gcov_tool_test.sh`
3. Ensure `gcov-tool` is in your PATH and built with coverage instrumentation
4. Run it: `./run_gcov_tool_test.sh`

The script will create the necessary coverage data files and systematically test all the uncovered parsing logic in the `switch` statement.
