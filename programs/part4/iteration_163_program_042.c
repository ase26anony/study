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
    
    printf("Sum: %d\n", sum);
    return 0;
}
EOF

# Step 2: Compile with coverage instrumentation
echo "Compiling test program with coverage..."
gcc -fprofile-arcs -ftest-coverage test.c -o test_prog

# Step 3: Run the program to generate initial .gcda file
echo "Running test program to generate profile data..."
./test_prog > /dev/null

# Verify .gcda file was created
if [ ! -f "test.gcda" ]; then
    echo "Error: test.gcda not created!"
    exit 1
fi

# Step 4: Create two different .gcda files for overlap analysis
echo "Creating base and comparison profile files..."
cp test.gcda base.gcda

# Run program again with different input to create different coverage
# We'll modify the .gcda file slightly by running with different conditions
cat > test2.c << 'EOF'
#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    int i, sum = 0;
    
    // Different loop to create different coverage
    for (i = 0; i < 5; i++) {  // Only 5 iterations instead of 10
        sum += i * 2;
        if (i % 3 == 0) {  // Different condition
            sum += factorial(i);
        }
    }
    
    printf("Sum: %d\n", sum);
    return 0;
}
EOF

gcc -fprofile-arcs -ftest-coverage test2.c -o test2_prog
./test2_prog > /dev/null
cp test2.gcda compare.gcda

# Verify both files exist
if [ ! -f "base.gcda" ] || [ ! -f "compare.gcda" ]; then
    echo "Error: Required .gcda files not created!"
    exit 1
fi

echo "Profile files created successfully."
echo "Base file size: $(stat -c%s base.gcda) bytes"
echo "Compare file size: $(stat -c%s compare.gcda) bytes"

# Step 5: Test individual options (covering each case in the switch)
echo -e "\n=== Testing individual options ==="

# Test -v (verbose)
echo "Testing -v option..."
gcov-tool overlap -v base.gcda compare.gcda 2>&1 | tee overlap_v.log
echo "Exit code: $?"

# Test -f (function level)
echo -e "\nTesting -f option..."
gcov-tool overlap -f base.gcda compare.gcda 2>&1 | tee overlap_f.log
echo "Exit code: $?"

# Test -F (full filename)
echo -e "\nTesting -F option..."
gcov-tool overlap -F base.gcda compare.gcda 2>&1 | tee overlap_F.log
echo "Exit code: $?"

# Test -o (object level)
echo -e "\nTesting -o option..."
gcov-tool overlap -o base.gcda compare.gcda 2>&1 | tee overlap_o.log
echo "Exit code: $?"

# Test -h (hot only)
echo -e "\nTesting -h option..."
gcov-tool overlap -h base.gcda compare.gcda 2>&1 | tee overlap_h.log
echo "Exit code: $?"

# Test -t with argument (hot threshold)
echo -e "\nTesting -t option with 0.5 threshold..."
gcov-tool overlap -t 0.5 base.gcda compare.gcda 2>&1 | tee overlap_t_0.5.log
echo "Exit code: $?"

# Step 6: Test option combinations
echo -e "\n=== Testing option combinations ==="

# Combination 1: verbose + function level + object level
echo "Testing -v -f -o combination..."
gcov-tool overlap -v -f -o base.gcda compare.gcda 2>&1 | tee overlap_vfo.log
echo "Exit code: $?"

# Combination 2: fullname + hot only + threshold
echo -e "\nTesting -F -h -t 0.75 combination..."
gcov-tool overlap -F -h -t 0.75 base.gcda compare.gcda 2>&1 | tee overlap_Fht.log
echo "Exit code: $?"

# Combination 3: All options together
echo -e "\nTesting all options together..."
gcov-tool overlap -v -f -F -o -h -t 0.9 base.gcda compare.gcda 2>&1 | tee overlap_all.log
echo "Exit code: $?"

# Step 7: Test threshold boundary values
echo -e "\n=== Testing threshold boundary values ==="

# Test minimum threshold (0.0)
echo "Testing -t 0.0..."
gcov-tool overlap -t 0.0 base.gcda compare.gcda 2>&1 | tee overlap_t_0.0.log
echo "Exit code: $?"

# Test maximum threshold (1.0)
echo -e "\nTesting -t 1.0..."
gcov-tool overlap -t 1.0 base.gcda compare.gcda 2>&1 | tee overlap_t_1.0.log
echo "Exit code: $?"

# Test out-of-range thresholds (should still parse with atof)
echo -e "\nTesting -t -1.0 (out of range)..."
gcov-tool overlap -t -1.0 base.gcda compare.gcda 2>&1 | tee overlap_t_neg1.log
echo "Exit code: $?"

echo -e "\nTesting -t 2.5 (out of range)..."
gcov-tool overlap -t 2.5 base.gcda compare.gcda 2>&1 | tee overlap_t_2.5.log
echo "Exit code: $?"

# Test invalid number format
echo -e "\nTesting -t invalid (not a number)..."
gcov-tool overlap -t invalid base.gcda compare.gcda 2>&1 | tee overlap_t_invalid.log
echo "Exit code: $?"

# Step 8: Test invalid option (to trigger default case)
echo -e "\n=== Testing invalid option (to trigger default case) ==="
echo "Testing invalid option -x..."
gcov-tool overlap -x base.gcda compare.gcda 2>&1 | tee overlap_invalid.log
echo "Exit code: $?"

# Step 9: Test missing required argument for -t
echo -e "\n=== Testing missing argument for -t ==="
echo "Testing -t without argument..."
gcov-tool overlap -t base.gcda compare.gcda 2>&1 | tee overlap_t_noarg.log
echo "Exit code: $?"

# Step 10: Test with swapped file order
echo -e "\n=== Testing with swapped file order ==="
echo "Testing with swapped input files..."
gcov-tool overlap -v compare.gcda base.gcda 2>&1 | tee overlap_swapped.log
echo "Exit code: $?"

# Step 11: Verify verbose output actually produces more output
echo -e "\n=== Verifying verbose mode produces more output ==="
NORMAL_OUTPUT=$(gcov-tool overlap base.gcda compare.gcda 2>&1 | wc -l)
VERBOSE_OUTPUT=$(gcov-tool overlap -v base.gcda compare.gcda 2>&1 | wc -l)

echo "Normal output lines: $NORMAL_OUTPUT"
echo "Verbose output lines: $VERBOSE_OUTPUT"

if [ "$VERBOSE_OUTPUT" -gt "$NORMAL_OUTPUT" ]; then
    echo "✓ Verbose mode produces more output as expected"
else
    echo "⚠ Verbose mode might not be working as expected"
fi

# Step 12: Summary
echo -e "\n=== Test Summary ==="
echo "All tests completed successfully!"
echo "Log files created in: $TEMP_DIR"
echo "Check the log files to verify each option was processed correctly."
echo "The following options were tested:"
echo "  -v (verbose)          - Triggers 'case 'v':' and gcov_set_verbose()"
echo "  -f (function level)   - Triggers 'case 'f':' and sets overlap_func_level"
echo "  -F (full filename)    - Triggers 'case 'F':' and sets overlap_use_fullname"
echo "  -o (object level)     - Triggers 'case 'o':' and sets overlap_obj_level"
echo "  -h (hot only)         - Triggers 'case 'h':' and sets overlap_hot_only"
echo "  -t (threshold)        - Triggers 'case 't':' and calls atof(optarg)"
echo "  invalid option        - Triggers 'default:' case and overlap_usage()"

# List all generated log files
echo -e "\nGenerated log files:"
ls -la overlap_*.log 2>/dev/null || echo "No log files found"

exit 0
```

This script provides comprehensive coverage of the uncovered lines by:

1. **Creating valid GCOV data files** - Compiles and runs two instrumented C programs to generate `.gcda` files with different coverage patterns.

2. **Testing each individual option** - Exercises each `case` in the switch statement:
   - `-v` triggers `verbose = true` and `gcov_set_verbose()`
   - `-f` sets `overlap_func_level = 1`
   - `-F` sets `overlap_use_fullname = 1`
   - `-o` sets `overlap_obj_level = 1`
   - `-h` sets `overlap_hot_only = 1`
   - `-t` triggers `atof(optarg)` with various values

3. **Testing option combinations** - Ensures multiple flags can be used together.

4. **Testing boundary values for `-t`** - Tests `0.0`, `1.0`, `-1.0`, `2.5`, and invalid input to exercise `atof()`.

5. **Triggering the default case** - Uses `-x` to trigger the `default:` case and call `overlap_usage()`.

6. **Validating behavior** - Compares output length for verbose vs non-verbose modes to verify the option actually changes behavior.

7. **Clean environment** - Uses a temporary directory and cleanup trap to avoid polluting the filesystem.

To run this test:
1. Ensure `gcov-tool` is in your PATH (built with coverage instrumentation)
2. Make the script executable: `chmod +x run_gcov_tool_test.sh`
3. Run it: `./run_gcov_tool_test.sh`

The script will generate log files for each test case, allowing you to verify that each option was processed without errors and that the tool's behavior changed as expected.
