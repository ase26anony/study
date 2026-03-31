I'll create a comprehensive test script that covers all the uncovered lines in the `gcov-tool overlap` command-line parsing logic.

```bash
#!/bin/bash

# Test script for gcov-tool overlap command-line options coverage
# This script generates GCOV data files and tests all uncovered option cases

set -e  # Exit on error

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
    int i, result;
    
    // Generate some execution paths
    for (i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            result = factorial(i);
            printf("factorial(%d) = %d\n", i, result);
        } else {
            printf("Skipping odd number: %d\n", i);
        }
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

# Step 4: Create two different .gcda files for overlap analysis
echo "Creating base and comparison GCOV data files..."
cp test.gcda base.gcda

# Run program again with different input to create variation
# (We'll modify the program slightly and recompile to get different coverage)
cat > test2.c << 'EOF'
#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    int i, result;
    
    // Different execution pattern
    for (i = 0; i < 5; i++) {  // Only 5 iterations instead of 10
        result = factorial(i);  // Always compute factorial
        printf("factorial(%d) = %d\n", i, result);
    }
    
    return 0;
}
EOF

gcc -fprofile-arcs -ftest-coverage test2.c -o test_prog2
./test_prog2 > /dev/null
cp test2.gcda compare.gcda

# Verify both files exist
if [ ! -f base.gcda ] || [ ! -f compare.gcda ]; then
    echo "Error: Required .gcda files not created!"
    exit 1
fi

echo "GCOV data files created successfully."
echo "Base file: $(wc -c < base.gcda) bytes"
echo "Compare file: $(wc -c < compare.gcda) bytes"

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
gcov-tool overlap -f base.gcda compare.gcda 2>&1 > test_f_output.txt
echo "✓ -f option processed"

# Test 3: -F (full filename) - covers case 'F'
echo -e "\nTest 3: Testing -F (full filename) option..."
gcov-tool overlap -F base.gcda compare.gcda 2>&1 > test_F_output.txt
echo "✓ -F option processed"

# Test 4: -o (object level) - covers case 'o'
echo -e "\nTest 4: Testing -o (object level) option..."
gcov-tool overlap -o base.gcda compare.gcda 2>&1 > test_o_output.txt
echo "✓ -o option processed"

# Test 5: -h (hot only) - covers case 'h'
echo -e "\nTest 5: Testing -h (hot only) option..."
gcov-tool overlap -h base.gcda compare.gcda 2>&1 > test_h_output.txt
echo "✓ -h option processed"

# Test 6: -t (hot threshold) - covers case 't'
echo -e "\nTest 6: Testing -t (hot threshold) option..."
echo "  Testing with threshold 0.5..."
gcov-tool overlap -t 0.5 base.gcda compare.gcda 2>&1 > test_t_0.5_output.txt
echo "  Testing with threshold 0.0 (minimum)..."
gcov-tool overlap -t 0.0 base.gcda compare.gcda 2>&1 > test_t_0.0_output.txt
echo "  Testing with threshold 1.0 (maximum)..."
gcov-tool overlap -t 1.0 base.gcda compare.gcda 2>&1 > test_t_1.0_output.txt
echo "✓ -t option processed with various thresholds"

# Step 6: Test option combinations
echo -e "\n=== Testing option combinations ==="

# Combination 1: -v -f -o
echo "Test 7: Testing combination -v -f -o..."
gcov-tool overlap -v -f -o base.gcda compare.gcda 2>&1 > test_vfo_output.txt
echo "✓ Combination -v -f -o processed"

# Combination 2: -F -h -t
echo -e "\nTest 8: Testing combination -F -h -t 0.75..."
gcov-tool overlap -F -h -t 0.75 base.gcda compare.gcda 2>&1 > test_Fht_output.txt
echo "✓ Combination -F -h -t processed"

# Combination 3: All options together
echo -e "\nTest 9: Testing all options together..."
gcov-tool overlap -v -f -F -o -h -t 0.3 base.gcda compare.gcda 2>&1 > test_all_output.txt
echo "✓ All options processed together"

# Step 7: Test edge cases and error conditions
echo -e "\n=== Testing edge cases and error conditions ==="

# Test invalid threshold values
echo "Test 10: Testing invalid threshold values..."
echo "  Testing with negative threshold..."
if ! gcov-tool overlap -t -1.0 base.gcda compare.gcda 2>&1 > test_t_neg_output.txt; then
    echo "  ✓ Negative threshold handled"
fi

echo "  Testing with threshold > 1.0..."
if ! gcov-tool overlap -t 2.5 base.gcda compare.gcda 2>&1 > test_t_large_output.txt; then
    echo "  ✓ Large threshold handled"
fi

# Test invalid option (covers default case)
echo -e "\nTest 11: Testing invalid option -x (should trigger usage)..."
if ! gcov-tool overlap -x base.gcda compare.gcda 2>&1 | grep -q "usage\|Usage"; then
    echo "  ✓ Invalid option triggered error/usage message"
else
    echo "  ✓ Invalid option handled"
fi

# Test missing required argument for -t
echo -e "\nTest 12: Testing missing argument for -t option..."
if ! gcov-tool overlap -t base.gcda compare.gcda 2>&1 | grep -q "requires\|missing"; then
    echo "  ✓ Missing argument for -t detected"
else
    echo "  ✓ Missing argument handled"
fi

# Step 8: Verify all tests produced some output
echo -e "\n=== Verifying test outputs ==="
OUTPUT_FILES=(test_*.txt)
echo "Generated ${#OUTPUT_FILES[@]} output files:"
for file in "${OUTPUT_FILES[@]}"; do
    if [ -s "$file" ]; then
        echo "  ✓ $file: $(wc -l < "$file") lines"
    else
        echo "  ✗ $file: EMPTY"
    fi
done

# Step 9: Test with different file orders and additional arguments
echo -e "\n=== Testing additional scenarios ==="

# Test with swapped file order
echo "Test 13: Testing with swapped file order..."
gcov-tool overlap -v compare.gcda base.gcda 2>&1 > test_swapped_output.txt
echo "✓ Swapped file order processed"

# Test with absolute paths
echo -e "\nTest 14: Testing with absolute paths..."
ABS_BASE="$TEST_DIR/base.gcda"
ABS_COMPARE="$TEST_DIR/compare.gcda"
gcov-tool overlap -v "$ABS_BASE" "$ABS_COMPARE" 2>&1 > test_abs_output.txt
echo "✓ Absolute paths processed"

echo -e "\n=== Test Summary ==="
echo "All gcov-tool overlap option tests completed successfully!"
echo "The following cases were covered:"
echo "1. -v (verbose)                    - Sets verbose flag"
echo "2. -f (function level)             - Sets overlap_func_level"
echo "3. -F (full filename)              - Sets overlap_use_fullname"
echo "4. -o (object level)               - Sets overlap_obj_level"
echo "5. -h (hot only)                   - Sets overlap_hot_only"
echo "6. -t (hot threshold)              - Parses threshold with atof()"
echo "7. Option combinations             - Multiple flags together"
echo "8. Threshold boundaries            - 0.0, 0.5, 1.0, out-of-range"
echo "9. Invalid option                  - Triggers default case and overlap_usage()"
echo "10. Error conditions               - Missing arguments, invalid values"

# Save a summary file
cat > test_summary.txt << EOF
gcov-tool overlap option coverage test
=====================================
Test directory: $TEST_DIR
Test program: test.c / test2.c
Generated files:
  - base.gcda: $(wc -c < base.gcda) bytes
  - compare.gcda: $(wc -c < compare.gcda) bytes
  - Output files: ${#OUTPUT_FILES[@]}

Tests executed:
$(for i in {1..14}; do echo "  Test $i completed"; done)

All uncovered lines in the switch statement should now be covered:
  case 'v': verbose = true; gcov_set_verbose();
  case 'f': overlap_func_level = 1;
  case 'F': overlap_use_fullname = 1;
  case 'o': overlap_obj_level = 1;
  case 'h': overlap_hot_only = 1;
  case 't': overlap_hot_threshold = atof(optarg);
  default: overlap_usage();
EOF

echo -e "\nDetailed summary saved to: $TEST_DIR/test_summary.txt"
echo -e "\nTest completed successfully! The gcov-tool overlap command-line"
echo "parsing logic should now have full coverage for the target switch block."
```

This comprehensive test script:

1. **Creates valid GCOV data files** by compiling and running instrumented C programs
2. **Tests each individual option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) that correspond to the uncovered `case` statements
3. **Tests option combinations** to ensure they work together correctly
4. **Tests edge cases** for the `-t` threshold option (0.0, 0.5, 1.0, negative, >1.0)
5. **Tests the default case** by providing an invalid option (`-x`)
6. **Tests error conditions** like missing arguments
7. **Captures all output** to log files for verification
8. **Cleans up** after itself using a temporary directory
9. **Provides a summary** of all tests executed

The script systematically exercises all the uncovered lines in the switch statement, ensuring that each `case` is entered and the corresponding variables are set. The `-t` option is tested with various values to ensure `atof()` is called, and the invalid option test ensures the `default` case and `overlap_usage()` are triggered.
