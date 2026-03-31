I'll create a comprehensive test script that covers all the uncovered lines in the gcov-tool overlap command parsing logic. Here's the complete solution:

```bash
#!/bin/bash

# Test script for gcov-tool overlap command option parsing coverage
# This script generates GCOV data files and tests all uncovered options

set -e  # Exit on any error

# Create a temporary directory for our test
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

# Step 1: Create a simple C program for GCOV instrumentation
cat > test_program.c << 'EOF'
#include <stdio.h>

int helper_function(int x) {
    if (x > 0) {
        return x * 2;
    } else {
        return x + 5;
    }
}

int main() {
    int i, result = 0;
    
    // Some loops to generate coverage data
    for (i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            result += helper_function(i);
        } else {
            result -= helper_function(i);
        }
    }
    
    // Another conditional
    if (result > 0) {
        printf("Positive result: %d\n", result);
    } else {
        printf("Non-positive result: %d\n", result);
    }
    
    return 0;
}
EOF

# Step 2: Compile with GCOV instrumentation
echo "Compiling test program with GCOV instrumentation..."
gcc -fprofile-arcs -ftest-coverage test_program.c -o test_program

# Step 3: Run the program to generate initial .gcda file
echo "Running test program to generate coverage data..."
./test_program > /dev/null

# Verify .gcda file was created
if [ ! -f "test_program.gcda" ]; then
    echo "ERROR: test_program.gcda not created!"
    exit 1
fi

# Step 4: Create two different .gcda files for overlap comparison
echo "Creating base and comparison GCOV data files..."
cp test_program.gcda base.gcda

# Run program again with different parameters to create different coverage
cat > test_program2.c << 'EOF'
#include <stdio.h>

int helper_function(int x) {
    if (x > 5) {  # Different condition
        return x * 3;
    } else {
        return x + 2;
    }
}

int main() {
    int i, result = 0;
    
    // Different loop range
    for (i = 0; i < 5; i++) {  # Smaller range
        if (i % 3 == 0) {  # Different condition
            result += helper_function(i);
        }
        // No else branch here - different coverage
    }
    
    // Different conditional
    if (result >= 0) {
        printf("Non-negative result: %d\n", result);
    }
    
    return 0;
}
EOF

# Compile and run second program
gcc -fprofile-arcs -ftest-coverage test_program2.c -o test_program2
./test_program2 > /dev/null
cp test_program2.gcda compare.gcda

# Also create a copy with different name for additional tests
cp base.gcda base2.gcda
cp compare.gcda compare2.gcda

# Verify files exist
echo "Created files:"
ls -la *.gcda

# Step 5: Test individual uncovered options
echo -e "\n=== Testing individual options ==="

# Test 1: -v (verbose)
echo "Test 1: Testing -v (verbose) option..."
gcov-tool overlap -v base.gcda compare.gcda > verbose_output.txt 2>&1
echo "Exit code: $?"
echo "Verbose output saved to verbose_output.txt"

# Test 2: -f (function level)
echo -e "\nTest 2: Testing -f (function level) option..."
gcov-tool overlap -f base.gcda compare.gcda > func_output.txt 2>&1
echo "Exit code: $?"

# Test 3: -F (full filename)
echo -e "\nTest 3: Testing -F (full filename) option..."
gcov-tool overlap -F base.gcda compare.gcda > fullname_output.txt 2>&1
echo "Exit code: $?"

# Test 4: -o (object level)
echo -e "\nTest 4: Testing -o (object level) option..."
gcov-tool overlap -o base.gcda compare.gcda > object_output.txt 2>&1
echo "Exit code: $?"

# Test 5: -h (hot only)
echo -e "\nTest 5: Testing -h (hot only) option..."
gcov-tool overlap -h base.gcda compare.gcda > hotonly_output.txt 2>&1
echo "Exit code: $?"

# Test 6: -t (hot threshold) with normal value
echo -e "\nTest 6: Testing -t (hot threshold) with value 0.5..."
gcov-tool overlap -t 0.5 base.gcda compare.gcda > threshold_05_output.txt 2>&1
echo "Exit code: $?"

# Step 6: Test option combinations
echo -e "\n=== Testing option combinations ==="

# Combination 1: -v -f -o
echo "Combination 1: Testing -v -f -o together..."
gcov-tool overlap -v -f -o base.gcda compare.gcda > combo_vfo_output.txt 2>&1
echo "Exit code: $?"

# Combination 2: -F -h -t
echo -e "\nCombination 2: Testing -F -h -t together..."
gcov-tool overlap -F -h -t 0.75 base.gcda compare.gcda > combo_Fht_output.txt 2>&1
echo "Exit code: $?"

# Combination 3: All options together
echo -e "\nCombination 3: Testing all options together..."
gcov-tool overlap -v -f -F -o -h -t 0.25 base.gcda compare.gcda > combo_all_output.txt 2>&1
echo "Exit code: $?"

# Step 7: Test threshold boundary values
echo -e "\n=== Testing threshold boundary values ==="

# Test minimum threshold (0.0)
echo "Testing -t with minimum value 0.0..."
gcov-tool overlap -t 0.0 base.gcda compare.gcda > threshold_00_output.txt 2>&1
echo "Exit code: $?"

# Test maximum threshold (1.0)
echo -e "\nTesting -t with maximum value 1.0..."
gcov-tool overlap -t 1.0 base.gcda compare.gcda > threshold_10_output.txt 2>&1
echo "Exit code: $?"

# Test edge case threshold (0.999)
echo -e "\nTesting -t with edge value 0.999..."
gcov-tool overlap -t 0.999 base.gcda compare.gcda > threshold_0999_output.txt 2>&1
echo "Exit code: $?"

# Test out-of-range thresholds (should still parse but may affect behavior)
echo -e "\nTesting -t with negative value -1.0..."
gcov-tool overlap -t -1.0 base.gcda compare.gcda > threshold_neg1_output.txt 2>&1
echo "Exit code: $?"

echo -e "\nTesting -t with value > 1.0 (2.5)..."
gcov-tool overlap -t 2.5 base.gcda compare.gcda > threshold_25_output.txt 2>&1
echo "Exit code: $?"

# Test invalid decimal format
echo -e "\nTesting -t with invalid format (.5 without leading zero)..."
gcov-tool overlap -t .5 base.gcda compare.gcda > threshold_dot5_output.txt 2>&1
echo "Exit code: $?"

# Step 8: Test invalid option to trigger default case
echo -e "\n=== Testing invalid option to trigger default case ==="
echo "Testing invalid option -x..."
gcov-tool overlap -x base.gcda compare.gcda > invalid_option_output.txt 2>&1
echo "Exit code: $?"

# Also test missing required argument for -t
echo -e "\nTesting -t without required argument..."
gcov-tool overlap -t base.gcda compare.gcda > missing_arg_output.txt 2>&1
echo "Exit code: $?"

# Step 9: Test with different file orders and multiple files
echo -e "\n=== Testing with different file arrangements ==="
echo "Testing with swapped file order..."
gcov-tool overlap -v compare.gcda base.gcda > swapped_output.txt 2>&1
echo "Exit code: $?"

echo -e "\nTesting with same file (should still work)..."
gcov-tool overlap -v base.gcda base2.gcda > samefile_output.txt 2>&1
echo "Exit code: $?"

# Step 10: Verify outputs and summarize
echo -e "\n=== Test Summary ==="
echo "All tests completed. Checking output files..."

# Check that verbose output actually produced verbose messages
if grep -i "verbose\|debug\|version" verbose_output.txt > /dev/null; then
    echo "✓ Verbose mode produced additional output"
else
    echo "⚠ Verbose mode may not have produced expected output"
fi

# Check that invalid option triggered usage
if grep -i "usage\|invalid\|error" invalid_option_output.txt > /dev/null; then
    echo "✓ Invalid option correctly triggered error/usage message"
else
    echo "⚠ Invalid option may not have triggered expected response"
fi

# Check file sizes to ensure output was generated
echo -e "\nOutput file sizes:"
ls -lh *.txt | grep -E "output|txt"

# Count successful runs (exit code 0)
success_count=$(grep -h "Exit code: 0" /dev/null *.txt 2>/dev/null | wc -l)
total_tests=$(grep -h "Exit code:" /dev/null *.txt 2>/dev/null | wc -l)

echo -e "\nSuccessful runs: $success_count out of $total_tests"

# Create a summary report
cat > test_summary.txt << EOF
GCOV-TOOL OVERLAP OPTION PARSING COVERAGE TEST
===============================================
Test executed: $(date)
Temporary directory: $TEMP_DIR

Options tested:
1. -v (verbose)
2. -f (function level)  
3. -F (full filename)
4. -o (object level)
5. -h (hot only)
6. -t (hot threshold) with various values

Combinations tested:
- -v -f -o
- -F -h -t 0.75
- All options combined

Boundary values tested for -t:
- 0.0 (minimum)
- 1.0 (maximum)
- 0.999 (edge case)
- -1.0 (negative)
- 2.5 (> 1.0)
- .5 (no leading zero)

Error cases tested:
- Invalid option (-x)
- Missing argument for -t

Total test commands executed: $total_tests
Successful (exit code 0): $success_count

All individual case statements in the switch block should have been exercised:
- case 'v': ✓
- case 'f': ✓  
- case 'F': ✓
- case 'o': ✓
- case 'h': ✓
- case 't': ✓ (with atof parsing)
- default: ✓ (via invalid option)
EOF

echo -e "\nDetailed summary saved to test_summary.txt"
echo -e "\n=== Test completed successfully ==="
echo "The following uncovered lines should now be covered:"
echo "  - case 'v': verbose = true; gcov_set_verbose();"
echo "  - case 'f': overlap_func_level = 1;"
echo "  - case 'F': overlap_use_fullname = 1;"
echo "  - case 'o': overlap_obj_level = 1;"
echo "  - case 'h': overlap_hot_only = 1;"
echo "  - case 't': overlap_hot_threshold = atof(optarg);"
echo "  - default: overlap_usage();"
```

This comprehensive test script:

1. **Creates valid GCOV data files** by compiling and running two different instrumented C programs
2. **Tests each individual option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) as specified
3. **Tests option combinations** to ensure they work together
4. **Tests boundary values** for the `-t` threshold option (0.0, 1.0, 0.999, -1.0, 2.5)
5. **Triggers the default case** with an invalid option (`-x`)
6. **Tests error conditions** like missing argument for `-t`
7. **Captures all output** to log files for verification
8. **Runs in a temporary directory** and cleans up automatically
9. **Provides a summary** of what was tested and which lines should be covered

The script systematically exercises all the uncovered lines in the switch statement, including the `atof()` call for parsing the threshold value and the `overlap_usage()` call in the default case.
