Here's a comprehensive shell script that covers all the specified requirements for testing the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash

# test_gcov_tool.sh - Comprehensive test for gcov-tool overlap analysis flags

set -e  # Exit on any error

echo "=== Starting gcov-tool overlap analysis flag testing ==="

# Clean up from previous runs
cleanup() {
    echo "Cleaning up..."
    rm -f test.c test_prog test_prog_opt test_prog_lto
    rm -f *.gcda *.gcno *.gcov gcov_output*.txt
    rm -rf profile_data_* gcov_data_*
}

cleanup

# Step 1: Generate a minimal C source file with conditional branches
cat > test.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

int calculate(int a, int b, int mode) {
    int result = 0;
    
    if (mode == 1) {
        result = a + b;
    } else if (mode == 2) {
        result = a * b;
    } else {
        result = a - b;
    }
    
    for (int i = 0; i < a; i++) {
        result += i;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int mode = 1;
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    int x = 10;
    int y = 5;
    
    int res = calculate(x, y, mode);
    printf("Result: %d (mode: %d)\n", res, mode);
    
    // Another conditional
    if (res > 20) {
        printf("Large result\n");
    } else {
        printf("Small result\n");
    }
    
    return 0;
}
EOF

echo "Generated test.c"

# Step 2: Compile with different optimization levels to generate varied GCOV data
echo "Compiling test programs with GCOV instrumentation..."

# Basic compilation
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog

# With optimization
gcc -fprofile-arcs -ftest-coverage -O2 test.c -o test_prog_opt

# With LTO (if supported)
if gcc -fprofile-arcs -ftest-coverage -flto -O2 test.c -o test_prog_lto 2>/dev/null; then
    HAS_LTO=1
else
    HAS_LTO=0
    echo "Note: LTO compilation failed (may not be supported), continuing without it"
fi

# Step 3: Generate multiple sets of profile data
echo "Generating profile data with different execution paths..."

# First run - mode 1
echo "Run 1: mode 1"
./test_prog 1
mv test_prog.gcda test_prog_run1.gcda
cp test_prog.gcno test_prog_run1.gcno

# Second run - mode 2 (different path)
echo "Run 2: mode 2"
./test_prog 2
mv test_prog.gcda test_prog_run2.gcda

# Third run - mode 3 (another different path)
echo "Run 3: mode 3"
./test_prog 3
mv test_prog.gcda test_prog_run3.gcda

# Run optimized version
echo "Run 4: optimized version mode 1"
./test_prog_opt 1
mv test_prog_opt.gcda test_prog_opt_run1.gcda
cp test_prog_opt.gcno test_prog_opt_run1.gcno

# Create directories with different GCOV data structures
mkdir -p profile_data_1
mkdir -p profile_data_2

cp test_prog_run1.gcda profile_data_1/
cp test_prog_run1.gcno profile_data_1/
cp test_prog_run2.gcda profile_data_2/
cp test_prog_run2.gcno profile_data_2/

# Step 4: Test individual flags for overlap analysis
echo -e "\n=== Testing individual flags ==="

# Test verbose flag (-v)
echo "Testing: -v flag"
gcov-tool overlap -v test_prog_run1.gcda test_prog_run2.gcda > gcov_output_verbose.txt 2>&1
echo "  Verbose output saved to gcov_output_verbose.txt"

# Test function level flag (-f)
echo "Testing: -f flag"
gcov-tool overlap -f test_prog_run1.gcda test_prog_run2.gcda > gcov_output_func.txt 2>&1

# Test fullname flag (-F)
echo "Testing: -F flag"
gcov-tool overlap -F test_prog_run1.gcda test_prog_run2.gcda > gcov_output_fullname.txt 2>&1

# Test object level flag (-o)
echo "Testing: -o flag"
gcov-tool overlap -o test_prog_run1.gcda test_prog_run2.gcda > gcov_output_obj.txt 2>&1

# Test hot only flag (-h)
echo "Testing: -h flag"
gcov-tool overlap -h test_prog_run1.gcda test_prog_run2.gcda > gcov_output_hot.txt 2>&1

# Test threshold flag (-t) with various values
echo "Testing: -t flag with 0.5"
gcov-tool overlap -t 0.5 test_prog_run1.gcda test_prog_run2.gcda > gcov_output_thresh_0.5.txt 2>&1

echo "Testing: -t flag with 1.0"
gcov-tool overlap -t 1.0 test_prog_run1.gcda test_prog_run2.gcda > gcov_output_thresh_1.0.txt 2>&1

echo "Testing: -t flag with 10.5"
gcov-tool overlap -t 10.5 test_prog_run1.gcda test_prog_run2.gcda > gcov_output_thresh_10.5.txt 2>&1

# Step 5: Test flag combinations
echo -e "\n=== Testing flag combinations ==="

# Combination 1: -f -o
echo "Testing: -f -o combination"
gcov-tool overlap -f -o test_prog_run1.gcda test_prog_run2.gcda test_prog_run3.gcda > gcov_output_combo1.txt 2>&1

# Combination 2: -F -h -t 1.0
echo "Testing: -F -h -t 1.0 combination"
gcov-tool overlap -F -h -t 1.0 test_prog_run1.gcda test_prog_run2.gcda > gcov_output_combo2.txt 2>&1

# Combination 3: -v -f -F -o -h -t 5.0 (all flags)
echo "Testing: -v -f -F -o -h -t 5.0 (all flags)"
gcov-tool overlap -v -f -F -o -h -t 5.0 test_prog_run1.gcda test_prog_run2.gcda > gcov_output_combo3.txt 2>&1

# Combination 4: -v -t 0.8 with optimized program data
echo "Testing: -v -t 0.8 with optimized program"
gcov-tool overlap -v -t 0.8 test_prog_opt_run1.gcda test_prog_run1.gcda > gcov_output_combo4.txt 2>&1

# Combination 5: -f -o -t 0.8 (as specified in requirements)
echo "Testing: -f -o -t 0.8 combination"
gcov-tool overlap -f -o -t 0.8 test_prog_run1.gcda test_prog_run2.gcda > gcov_output_combo5.txt 2>&1

# Step 6: Test with multiple input files in different directories
echo -e "\n=== Testing with multiple input files and directories ==="

echo "Testing: -f flag with directory inputs"
gcov-tool overlap -f profile_data_1/*.gcda profile_data_2/*.gcda > gcov_output_multi_dir.txt 2>&1

echo "Testing: -v -o with wildcard pattern"
gcov-tool overlap -v -o *.gcda > gcov_output_wildcard.txt 2>&1

# Step 7: Test error case - invalid flag to trigger overlap_usage()
echo -e "\n=== Testing error case (invalid flag) ==="
echo "Testing: Invalid flag -Z (should trigger usage)"
gcov-tool overlap -Z test_prog_run1.gcda > gcov_output_error.txt 2>&1 || true

# Also test with valid flags followed by invalid flag
echo "Testing: Valid flag followed by invalid flag"
gcov-tool overlap -f -Z test_prog_run1.gcda > gcov_output_error2.txt 2>&1 || true

# Step 8: Test edge cases
echo -e "\n=== Testing edge cases ==="

# Test with threshold 0.0
echo "Testing: -t 0.0 (edge case)"
gcov-tool overlap -t 0.0 test_prog_run1.gcda test_prog_run2.gcda > gcov_output_edge1.txt 2>&1

# Test with threshold 100.0
echo "Testing: -t 100.0 (edge case)"
gcov-tool overlap -t 100.0 test_prog_run1.gcda test_prog_run2.gcda > gcov_output_edge2.txt 2>&1

# Test with single input file (should still work)
echo "Testing: Single input file with -v -f"
gcov-tool overlap -v -f test_prog_run1.gcda > gcov_output_single.txt 2>&1

# Step 9: Verify all required flags were tested
echo -e "\n=== Test Summary ==="
echo "The following individual flags were tested:"
echo "  -v (verbose)"
echo "  -f (function level)"
echo "  -F (fullname)"
echo "  -o (object level)"
echo "  -h (hot only)"
echo "  -t (threshold with values: 0.5, 1.0, 10.5, 0.0, 100.0, 5.0, 0.8)"

echo -e "\nFlag combinations tested:"
echo "  -f -o"
echo "  -F -h -t 1.0"
echo "  -v -f -F -o -h -t 5.0"
echo "  -v -t 0.8"
echo "  -f -o -t 0.8"

echo -e "\nError case tested:"
echo "  Invalid flag -Z (triggers overlap_usage())"

echo -e "\nMultiple input scenarios:"
echo "  Multiple .gcda files"
echo "  Files from different directories"
echo "  Wildcard patterns"

# Step 10: Display sample outputs to verify execution
echo -e "\n=== Sample output verification ==="
echo "Checking if tests produced output..."

if [ -s gcov_output_verbose.txt ]; then
    echo "✓ Verbose test produced output"
    echo "  First 3 lines of verbose output:"
    head -3 gcov_output_verbose.txt | sed 's/^/    /'
fi

if [ -s gcov_output_error.txt ]; then
    echo "✓ Error test produced output (likely usage message)"
    echo "  First 2 lines of error output:"
    head -2 gcov_output_error.txt | sed 's/^/    /'
fi

# Count test files
TEST_COUNT=$(ls gcov_output_*.txt 2>/dev/null | wc -l)
echo -e "\nTotal test output files generated: $TEST_COUNT"

echo -e "\n=== Testing complete ==="
echo "All gcov-tool overlap flag combinations have been exercised."
echo "The uncovered lines in gcov-tool.cc (534-554) should now be covered."

# Optional: Cleanup
# Uncomment the following line to clean up after testing
# cleanup
```

This script provides comprehensive testing of the uncovered lines in `gcov-tool.cc` by:

1. **Generating a test C program** with conditional branches to create meaningful GCOV data
2. **Compiling with GCOV instrumentation** at different optimization levels
3. **Generating multiple profile runs** with different execution paths
4. **Testing all individual flags** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t` with various values)
5. **Testing flag combinations** including the specific `-f -o -t 0.8` mentioned
6. **Testing error cases** with invalid flag `-Z` to trigger `overlap_usage()`
7. **Testing with multiple input files** and different directory structures
8. **Providing verification** that tests executed successfully

The script ensures that every case in the switch statement (lines 534-554) is exercised, including the `default` case that calls `overlap_usage()`.
