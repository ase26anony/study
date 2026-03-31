Here's a comprehensive shell script that systematically tests the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash

# Test script for gcov-tool overlap command options
# Tests the uncovered lines in gcov-tool.cc (lines 534-554)

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

# Step 1: Create instrumented source file
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
        if (i % 2 == 0) {
            sum += factorial(i);
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
echo "Running test program to generate profile data..."
./test_prog > /dev/null

# Step 4: Create two .gcda files for overlap analysis
echo "Creating base and comparison profile files..."
cp test.gcda base.gcda
cp test.gcda compare.gcda

# Create a slightly different profile for more interesting overlap
# Run program with different input to generate different profile
cat > test2.c << 'EOF'
#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    int i, sum = 0;
    
    // Different loop to generate different coverage
    for (i = 5; i < 15; i++) {
        sum += i * 2;
        if (i % 3 == 0) {
            sum += factorial(i % 5);
        }
    }
    
    printf("Result: %d\n", sum);
    return 0;
}
EOF

gcc -fprofile-arcs -ftest-coverage test2.c -o test2_prog
./test2_prog > /dev/null
cp test2.gcda compare.gcda  # Overwrite with different profile

# Step 5: Test individual options (uncovered lines)
echo -e "\n=== Testing individual options ==="

# Test verbose option (-v)
echo "Testing -v (verbose) option..."
gcov-tool overlap -v base.gcda compare.gcda > verbose_output.log 2>&1
echo "Exit code: $?"

# Test function level option (-f)
echo "Testing -f (function level) option..."
gcov-tool overlap -f base.gcda compare.gcda > func_output.log 2>&1
echo "Exit code: $?"

# Test full filename option (-F)
echo "Testing -F (full filename) option..."
gcov-tool overlap -F base.gcda compare.gcda > fullname_output.log 2>&1
echo "Exit code: $?"

# Test object level option (-o)
echo "Testing -o (object level) option..."
gcov-tool overlap -o base.gcda compare.gcda > obj_output.log 2>&1
echo "Exit code: $?"

# Test hot only option (-h)
echo "Testing -h (hot only) option..."
gcov-tool overlap -h base.gcda compare.gcda > hotonly_output.log 2>&1
echo "Exit code: $?"

# Test hot threshold option (-t) with normal value
echo "Testing -t (hot threshold) option with 0.5..."
gcov-tool overlap -t 0.5 base.gcda compare.gcda > threshold_0.5_output.log 2>&1
echo "Exit code: $?"

# Step 6: Test option combinations
echo -e "\n=== Testing option combinations ==="

# Test combination 1: verbose + function level + object level
echo "Testing -v -f -o combination..."
gcov-tool overlap -v -f -o base.gcda compare.gcda > combo1_output.log 2>&1
echo "Exit code: $?"

# Test combination 2: fullname + hot only + threshold
echo "Testing -F -h -t 0.75 combination..."
gcov-tool overlap -F -h -t 0.75 base.gcda compare.gcda > combo2_output.log 2>&1
echo "Exit code: $?"

# Test combination 3: all options together
echo "Testing all options together..."
gcov-tool overlap -v -f -F -o -h -t 0.9 base.gcda compare.gcda > combo3_output.log 2>&1
echo "Exit code: $?"

# Step 7: Test threshold boundary values
echo -e "\n=== Testing threshold boundary values ==="

# Test minimum threshold (0.0)
echo "Testing -t 0.0..."
gcov-tool overlap -t 0.0 base.gcda compare.gcda > threshold_0.0_output.log 2>&1
echo "Exit code: $?"

# Test maximum threshold (1.0)
echo "Testing -t 1.0..."
gcov-tool overlap -t 1.0 base.gcda compare.gcda > threshold_1.0_output.log 2>&1
echo "Exit code: $?"

# Test out-of-range thresholds (should still parse but may produce warnings)
echo "Testing -t -1.0 (out of range)..."
gcov-tool overlap -t -1.0 base.gcda compare.gcda > threshold_neg1.0_output.log 2>&1
echo "Exit code: $?"

echo "Testing -t 2.5 (out of range)..."
gcov-tool overlap -t 2.5 base.gcda compare.gcda > threshold_2.5_output.log 2>&1
echo "Exit code: $?"

# Test with scientific notation
echo "Testing -t 5e-1 (scientific notation)..."
gcov-tool overlap -t 5e-1 base.gcda compare.gcda > threshold_sci_output.log 2>&1
echo "Exit code: $?"

# Step 8: Test invalid option (triggers default case)
echo -e "\n=== Testing invalid option (triggers default case) ==="
echo "Testing invalid option -x..."
gcov-tool overlap -x base.gcda compare.gcda > invalid_option_output.log 2>&1
echo "Exit code: $?"

# Step 9: Test missing required argument for -t
echo -e "\n=== Testing missing argument for -t ==="
echo "Testing -t without argument..."
gcov-tool overlap -t base.gcda compare.gcda > missing_arg_output.log 2>&1
echo "Exit code: $?"

# Step 10: Test with non-existent files (error handling)
echo -e "\n=== Testing with non-existent files ==="
echo "Testing with non-existent .gcda files..."
gcov-tool overlap -v nonexistent1.gcda nonexistent2.gcda > nonexistent_output.log 2>&1
echo "Exit code: $?"

# Step 11: Verify verbose output actually produced more output
echo -e "\n=== Verifying verbose mode produces more output ==="
NORMAL_SIZE=$(wc -l < func_output.log)
VERBOSE_SIZE=$(wc -l < verbose_output.log)
echo "Normal output lines: $NORMAL_SIZE"
echo "Verbose output lines: $VERBOSE_SIZE"

if [ "$VERBOSE_SIZE" -gt "$NORMAL_SIZE" ]; then
    echo "✓ Verbose mode produced more output as expected"
else
    echo "⚠ Verbose mode may not be working as expected"
fi

# Step 12: Show summary of log files
echo -e "\n=== Generated log files ==="
ls -la *.log

echo -e "\n=== Test completed successfully ==="
echo "All gcov-tool overlap options have been exercised."
echo "Check the generated *.log files for command outputs."
```

This script comprehensively tests all the uncovered lines:

1. **Individual option testing** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) - directly exercises each case statement
2. **Option combinations** - tests multiple options together
3. **Threshold boundary values** (`0.0`, `1.0`, `-1.0`, `2.5`, scientific notation) - exercises `atof(optarg)` parsing
4. **Invalid option** (`-x`) - triggers the `default` case and `overlap_usage()`
5. **Missing argument** for `-t` - tests error handling
6. **Non-existent files** - tests general error handling

To run this test:
1. Ensure `gcov-tool` is in your PATH (built with coverage instrumentation)
2. Ensure `gcc` is available
3. Make the script executable: `chmod +x run_gcov_tool_test.sh`
4. Run it: `./run_gcov_tool_test.sh`

The script creates a clean temporary environment, generates valid GCOV data files, and systematically tests all the uncovered code paths. Each test captures both stdout and stderr to log files for verification.
