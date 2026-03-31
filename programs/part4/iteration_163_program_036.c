Here's a comprehensive shell script that systematically tests the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/sh

# Test script for gcov-tool overlap command options coverage
# This script tests the specific switch case block handling options:
# -v, -f, -F, -o, -h, -t

set -e

# Create a temporary directory for our test
TESTDIR=$(mktemp -d /tmp/gcov_tool_test_XXXXXX)
echo "Test directory: $TESTDIR"
cd "$TESTDIR"

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    cd /
    rm -rf "$TESTDIR"
}
trap cleanup EXIT

# Step 1: Create a simple C program for coverage testing
cat > test.c << 'EOF'
#include <stdio.h>

int helper(int x) {
    if (x > 0) {
        return x * 2;
    } else {
        return x + 1;
    }
}

int main() {
    int i, sum = 0;
    
    // Some loops to generate coverage data
    for (i = 0; i < 10; i++) {
        sum += helper(i);
    }
    
    for (i = -5; i < 5; i++) {
        if (i % 2 == 0) {
            sum += 1;
        } else {
            sum -= 1;
        }
    }
    
    printf("Result: %d\n", sum);
    return 0;
}
EOF

# Step 2: Compile with coverage instrumentation
echo "Compiling test program with coverage..."
gcc -fprofile-arcs -ftest-coverage test.c -o test_prog

# Step 3: Run the program to generate initial .gcda file
echo "Running test program to generate coverage data..."
./test_prog > /dev/null

# Step 4: Create two .gcda files for overlap comparison
echo "Creating base and comparison coverage files..."
cp test.gcda base.gcda

# Run program again with slightly different path
./test_prog > /dev/null
cp test.gcda compare.gcda

# Also create a third version for more variation
echo "int extra() { return 42; }" >> test.c
gcc -fprofile-arcs -ftest-coverage test.c -o test_prog2
./test_prog2 > /dev/null 2>&1
cp test.gcda compare2.gcda

# Step 5: Test individual options (covering each case statement)
echo ""
echo "=== Testing individual options ==="
echo ""

# Test verbose option (-v)
echo "Testing -v (verbose) option..."
if gcov-tool overlap -v base.gcda compare.gcda > verbose.log 2>&1; then
    echo "✓ -v option executed successfully"
    if [ -s verbose.log ]; then
        echo "  Output generated: $(wc -l < verbose.log) lines"
    fi
else
    echo "✗ -v option failed"
fi

# Test function level option (-f)
echo "Testing -f (function level) option..."
if gcov-tool overlap -f base.gcda compare.gcda > func_level.log 2>&1; then
    echo "✓ -f option executed successfully"
else
    echo "✗ -f option failed"
fi

# Test full filename option (-F)
echo "Testing -F (full filename) option..."
if gcov-tool overlap -F base.gcda compare.gcda > fullname.log 2>&1; then
    echo "✓ -F option executed successfully"
else
    echo "✗ -F option failed"
fi

# Test object level option (-o)
echo "Testing -o (object level) option..."
if gcov-tool overlap -o base.gcda compare.gcda > obj_level.log 2>&1; then
    echo "✓ -o option executed successfully"
else
    echo "✗ -o option failed"
fi

# Test hot only option (-h)
echo "Testing -h (hot only) option..."
if gcov-tool overlap -h base.gcda compare.gcda > hot_only.log 2>&1; then
    echo "✓ -h option executed successfully"
else
    echo "✗ -h option failed"
fi

# Test hot threshold option (-t) with various values
echo "Testing -t (hot threshold) option with value 0.5..."
if gcov-tool overlap -t 0.5 base.gcda compare.gcda > threshold_0.5.log 2>&1; then
    echo "✓ -t 0.5 option executed successfully"
else
    echo "✗ -t 0.5 option failed"
fi

# Step 6: Test option combinations
echo ""
echo "=== Testing option combinations ==="
echo ""

# Combination 1: verbose + function level + object level
echo "Testing combination: -v -f -o"
if gcov-tool overlap -v -f -o base.gcda compare.gcda > combo1.log 2>&1; then
    echo "✓ Combination -v -f -o executed successfully"
else
    echo "✗ Combination -v -f -o failed"
fi

# Combination 2: fullname + hot only + threshold
echo "Testing combination: -F -h -t 0.75"
if gcov-tool overlap -F -h -t 0.75 base.gcda compare.gcda > combo2.log 2>&1; then
    echo "✓ Combination -F -h -t 0.75 executed successfully"
else
    echo "✗ Combination -F -h -t 0.75 failed"
fi

# Combination 3: all options together
echo "Testing combination: -v -f -F -o -h -t 0.25"
if gcov-tool overlap -v -f -F -o -h -t 0.25 base.gcda compare.gcda > combo3.log 2>&1; then
    echo "✓ All options combination executed successfully"
else
    echo "✗ All options combination failed"
fi

# Step 7: Test threshold boundary values
echo ""
echo "=== Testing threshold boundary values ==="
echo ""

# Minimum threshold (0.0)
echo "Testing threshold: 0.0 (minimum)"
if gcov-tool overlap -t 0.0 base.gcda compare.gcda > threshold_0.0.log 2>&1; then
    echo "✓ Threshold 0.0 executed successfully"
else
    echo "✗ Threshold 0.0 failed"
fi

# Maximum threshold (1.0)
echo "Testing threshold: 1.0 (maximum)"
if gcov-tool overlap -t 1.0 base.gcda compare.gcda > threshold_1.0.log 2>&1; then
    echo "✓ Threshold 1.0 executed successfully"
else
    echo "✗ Threshold 1.0 failed"
fi

# Edge case: very small positive value
echo "Testing threshold: 0.001 (small positive)"
if gcov-tool overlap -t 0.001 base.gcda compare.gcda > threshold_0.001.log 2>&1; then
    echo "✓ Threshold 0.001 executed successfully"
else
    echo "✗ Threshold 0.001 failed"
fi

# Edge case: near maximum
echo "Testing threshold: 0.999 (near maximum)"
if gcov-tool overlap -t 0.999 base.gcda compare.gcda > threshold_0.999.log 2>&1; then
    echo "✓ Threshold 0.999 executed successfully"
else
    echo "✗ Threshold 0.999 failed"
fi

# Step 8: Test invalid options (to trigger default case)
echo ""
echo "=== Testing invalid option (to trigger default case) ==="
echo ""

# Invalid option -x
echo "Testing invalid option: -x"
if ! gcov-tool overlap -x base.gcda compare.gcda > invalid_option.log 2>&1; then
    echo "✓ Invalid option -x triggered error (as expected)"
    # Check if usage was shown
    if grep -q "Usage:" invalid_option.log || grep -q "overlap" invalid_option.log; then
        echo "  Usage message displayed"
    fi
else
    echo "✗ Invalid option -x did not trigger error (unexpected)"
fi

# Invalid threshold value (non-numeric)
echo "Testing invalid threshold: not_a_number"
if ! gcov-tool overlap -t not_a_number base.gcda compare.gcda > invalid_threshold.log 2>&1; then
    echo "✓ Invalid threshold triggered error (as expected)"
else
    echo "✗ Invalid threshold did not trigger error"
fi

# Out of range threshold (negative)
echo "Testing out-of-range threshold: -1.0"
if gcov-tool overlap -t -1.0 base.gcda compare.gcda > threshold_neg.log 2>&1; then
    echo "✓ Threshold -1.0 executed (behavior depends on implementation)"
else
    echo "✗ Threshold -1.0 failed"
fi

# Out of range threshold (> 1.0)
echo "Testing out-of-range threshold: 2.5"
if gcov-tool overlap -t 2.5 base.gcda compare.gcda > threshold_2.5.log 2>&1; then
    echo "✓ Threshold 2.5 executed (behavior depends on implementation)"
else
    echo "✗ Threshold 2.5 failed"
fi

# Step 9: Test with different file combinations
echo ""
echo "=== Testing with different file combinations ==="
echo ""

# Test with the third .gcda file
echo "Testing with different comparison file..."
if gcov-tool overlap -v base.gcda compare2.gcda > diff_file.log 2>&1; then
    echo "✓ Different file combination executed successfully"
else
    echo "✗ Different file combination failed"
fi

# Test with swapped files
echo "Testing with swapped input files..."
if gcov-tool overlap -v compare.gcda base.gcda > swapped.log 2>&1; then
    echo "✓ Swapped files executed successfully"
else
    echo "✗ Swapped files failed"
fi

# Step 10: Summary
echo ""
echo "=== Test Summary ==="
echo "All gcov-tool overlap option tests completed."
echo "Log files created in: $TESTDIR"
echo "Check individual log files for detailed output."
echo ""
echo "The following cases were tested to cover the switch block:"
echo "1. -v (verbose) - triggers 'case 'v':'"
echo "2. -f (function level) - triggers 'case 'f':'"
echo "3. -F (full filename) - triggers 'case 'F':'"
echo "4. -o (object level) - triggers 'case 'o':'"
echo "5. -h (hot only) - triggers 'case 'h':'"
echo "6. -t (threshold) - triggers 'case 't':' with atof(optarg)"
echo "7. Invalid option - triggers 'default:' case"
echo "8. Various threshold values - exercises atof parsing"
echo "9. Option combinations - tests multiple flag processing"
```

This script provides comprehensive coverage of the target code block by:

1. **Creating valid GCOV data files** through compilation and execution of an instrumented C program
2. **Testing each individual option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) to trigger each case statement
3. **Testing option combinations** to ensure flags work together correctly
4. **Testing threshold boundary values** (0.0, 0.001, 0.5, 0.75, 0.999, 1.0) to exercise `atof` parsing
5. **Testing invalid options** to trigger the default case and `overlap_usage()` call
6. **Testing edge cases** like negative thresholds and values >1.0
7. **Using different file combinations** to ensure robust execution

The script captures all output to log files for verification and cleans up after itself. Each test case is designed to ensure the corresponding lines in the switch block are executed, with the `-t` option specifically testing the `atof(optarg)` call.
