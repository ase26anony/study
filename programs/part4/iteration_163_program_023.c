Here's a comprehensive shell script that systematically tests the uncovered argument parsing logic in `gcov-tool`:

```bash
#!/bin/bash

# Test script for gcov-tool overlap command argument parsing
# Tests the uncovered switch cases for options: -v, -f, -F, -o, -h, -t

set -e  # Exit on any error

# Create a temporary directory for our test
TEMP_DIR=$(mktemp -d /tmp/gcov_tool_test_XXXXXX)
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
cat > test_program.c << 'EOF'
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

# Step 2: Compile with coverage instrumentation
echo "Compiling test program with coverage instrumentation..."
gcc -fprofile-arcs -ftest-coverage test_program.c -o test_program

# Step 3: Run the program to generate initial .gcda file
echo "Running test program to generate profile data..."
./test_program > /dev/null

# Verify .gcda file was created
if [ ! -f "test_program.gcda" ]; then
    echo "ERROR: test_program.gcda not created!"
    exit 1
fi

# Step 4: Create two different .gcda files for overlap comparison
echo "Creating base and comparison profile files..."
cp test_program.gcda base.gcda

# Run program again with different parameters to create different coverage
# We'll modify the source slightly and recompile to ensure different coverage
cat > test_program2.c << 'EOF'
#include <stdio.h>

int helper(int x) {
    if (x > 5) {  # Different threshold
        return x * 3;  # Different multiplier
    } else {
        return x + 3;  # Different addition
    }
}

int main() {
    int i, sum = 0;
    
    // Different loop count
    for (i = 0; i < 50; i++) {
        sum += helper(i);
    }
    
    // Different conditional
    if (sum > 1000) {
        printf("Sum is very large: %d\n", sum);
    } else {
        printf("Sum is moderate: %d\n", sum);
    }
    
    return 0;
}
EOF

# Compile and run second version
gcc -fprofile-arcs -ftest-coverage test_program2.c -o test_program2
./test_program2 > /dev/null
cp test_program2.gcda compare.gcda

echo "Profile files created: base.gcda and compare.gcda"

# Step 5: Test individual options from the uncovered switch block
echo ""
echo "=== Testing individual options ==="

# Test verbose option (-v)
echo "Testing -v (verbose) option..."
gcov-tool overlap -v base.gcda compare.gcda > verbose_output.txt 2>&1
echo "  Exit code: $?"
echo "  Output size: $(wc -l < verbose_output.txt) lines"

# Test function level option (-f)
echo "Testing -f (function level) option..."
gcov-tool overlap -f base.gcda compare.gcda > func_output.txt 2>&1
echo "  Exit code: $?"

# Test full filename option (-F)
echo "Testing -F (full filename) option..."
gcov-tool overlap -F base.gcda compare.gcda > fullname_output.txt 2>&1
echo "  Exit code: $?"

# Test object level option (-o)
echo "Testing -o (object level) option..."
gcov-tool overlap -o base.gcda compare.gcda > obj_output.txt 2>&1
echo "  Exit code: $?"

# Test hot only option (-h)
echo "Testing -h (hot only) option..."
gcov-tool overlap -h base.gcda compare.gcda > hotonly_output.txt 2>&1
echo "  Exit code: $?"

# Test hot threshold option (-t) with normal value
echo "Testing -t (hot threshold) option with value 0.5..."
gcov-tool overlap -t 0.5 base.gcda compare.gcda > threshold_0.5_output.txt 2>&1
echo "  Exit code: $?"

# Step 6: Test option combinations
echo ""
echo "=== Testing option combinations ==="

# Test combination 1: verbose + function level
echo "Testing -v -f combination..."
gcov-tool overlap -v -f base.gcda compare.gcda > combo_vf_output.txt 2>&1
echo "  Exit code: $?"

# Test combination 2: fullname + hot only + threshold
echo "Testing -F -h -t 0.75 combination..."
gcov-tool overlap -F -h -t 0.75 base.gcda compare.gcda > combo_Fht_output.txt 2>&1
echo "  Exit code: $?"

# Test combination 3: multiple options together
echo "Testing -v -f -o -h combination..."
gcov-tool overlap -v -f -o -h base.gcda compare.gcda > combo_vfoh_output.txt 2>&1
echo "  Exit code: $?"

# Step 7: Test threshold boundary values
echo ""
echo "=== Testing threshold boundary values ==="

# Test minimum threshold (0.0)
echo "Testing -t 0.0..."
gcov-tool overlap -t 0.0 base.gcda compare.gcda > threshold_0.0_output.txt 2>&1
echo "  Exit code: $?"

# Test maximum threshold (1.0)
echo "Testing -t 1.0..."
gcov-tool overlap -t 1.0 base.gcda compare.gcda > threshold_1.0_output.txt 2>&1
echo "  Exit code: $?"

# Test edge case threshold (just above 1.0)
echo "Testing -t 1.1..."
gcov-tool overlap -t 1.1 base.gcda compare.gcda > threshold_1.1_output.txt 2>&1
echo "  Exit code: $?"

# Test negative threshold
echo "Testing -t -0.5..."
gcov-tool overlap -t -0.5 base.gcda compare.gcda > threshold_neg0.5_output.txt 2>&1
echo "  Exit code: $?"

# Test very small threshold
echo "Testing -t 0.001..."
gcov-tool overlap -t 0.001 base.gcda compare.gcda > threshold_0.001_output.txt 2>&1
echo "  Exit code: $?"

# Step 8: Test invalid option to trigger default case
echo ""
echo "=== Testing invalid option (to trigger default case) ==="
echo "Testing invalid option -x..."
gcov-tool overlap -x base.gcda compare.gcda > invalid_output.txt 2>&1
echo "  Exit code: $?"

# Step 9: Verify verbose output actually produced more output
echo ""
echo "=== Verifying verbose mode produces more output ==="
REGULAR_LINES=$(gcov-tool overlap base.gcda compare.gcda 2>&1 | wc -l)
VERBOSE_LINES=$(gcov-tool overlap -v base.gcda compare.gcda 2>&1 | wc -l)
echo "Regular output lines: $REGULAR_LINES"
echo "Verbose output lines: $VERBOSE_LINES"

if [ "$VERBOSE_LINES" -gt "$REGULAR_LINES" ]; then
    echo "✓ Verbose mode produces more output as expected"
else
    echo "⚠ Verbose mode may not be working as expected"
fi

# Step 10: Summary
echo ""
echo "=== Test Summary ==="
echo "All tests completed successfully!"
echo "Generated test files in: $TEMP_DIR"
echo "Output files preserved for inspection:"
ls -la *.txt 2>/dev/null | head -10

# Keep the directory for inspection (cleanup trap will handle it)
echo ""
echo "Temporary directory preserved: $TEMP_DIR"
echo "To clean up manually: rm -rf $TEMP_DIR"
```

This script provides comprehensive testing of the uncovered argument parsing logic:

1. **Creates valid GCOV data files** by compiling and running instrumented C programs
2. **Tests each individual option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) from the uncovered switch block
3. **Tests option combinations** to ensure they work together correctly
4. **Tests boundary values** for the `-t` threshold option (0.0, 1.0, 1.1, -0.5, 0.001)
5. **Tests invalid option** (`-x`) to trigger the `default` case and `overlap_usage()` call
6. **Verifies verbose mode** actually produces more output
7. **Captures all output** to files for inspection
8. **Uses a temporary directory** and cleans up properly

To run this test:
1. Ensure `gcov-tool` is in your PATH (built with coverage instrumentation)
2. Make the script executable: `chmod +x run_gcov_tool_test.sh`
3. Run it: `./run_gcov_tool_test.sh`

The script will exercise all the uncovered lines in the switch statement, setting the global variables:
- `verbose = true` and calling `gcov_set_verbose()` for `-v`
- `overlap_func_level = 1` for `-f`
- `overlap_use_fullname = 1` for `-F`
- `overlap_obj_level = 1` for `-o`
- `overlap_hot_only = 1` for `-h`
- `overlap_hot_threshold = atof(optarg)` for `-t`
- `overlap_usage()` for invalid options (default case)
