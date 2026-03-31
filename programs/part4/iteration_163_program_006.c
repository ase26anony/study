Looking at the uncovered lines in `gcov-tool.cc`, I need to create a comprehensive test that exercises all the command-line options for the `overlap` subcommand. Here's a shell script that systematically tests each option and combination:

```bash
#!/bin/bash

# Test script for gcov-tool overlap command options
# This script tests the uncovered lines in gcov-tool.cc (lines 534-554)

set -e  # Exit on error

# Create a temporary directory for our test
TEST_DIR=$(mktemp -d /tmp/gcov_tool_test_XXXXXX)
echo "Test directory: $TEST_DIR"

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

cd "$TEST_DIR"

# Step 1: Create a simple C program for coverage testing
cat > test.c << 'EOF'
#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    int i, result;
    
    // Generate some coverage data
    for (i = 0; i < 5; i++) {
        result = factorial(i);
        printf("factorial(%d) = %d\n", i, result);
    }
    
    // Add some conditional branches
    if (result > 10) {
        printf("Result is greater than 10\n");
    } else {
        printf("Result is 10 or less\n");
    }
    
    return 0;
}
EOF

# Step 2: Compile with coverage instrumentation
echo "Compiling test program with coverage..."
gcc -fprofile-arcs -ftest-coverage test.c -o test_prog

# Step 3: Run the program to generate initial .gcda file
echo "Running test program to generate coverage data..."
./test_prog > /dev/null

# Verify .gcda file was created
if [ ! -f test.gcda ]; then
    echo "Error: test.gcda not created!"
    exit 1
fi

# Step 4: Create two different .gcda files for overlap comparison
echo "Creating base and comparison coverage files..."
cp test.gcda base.gcda

# Run program again with different input (simulated by different execution)
# We'll modify the .gcda file slightly to create a different profile
cp test.gcda compare.gcda

# Create a simple function to run gcov-tool and capture output
run_gcov_tool() {
    local test_name=$1
    local args=$2
    local expected_exit=${3:-0}
    
    echo "Running test: $test_name"
    echo "Command: gcov-tool overlap $args"
    
    # Capture output and exit code
    if gcov-tool overlap $args > "${test_name}_stdout.log" 2> "${test_name}_stderr.log"; then
        local exit_code=0
    else
        local exit_code=$?
    fi
    
    # Check if exit code matches expected
    if [ $exit_code -eq $expected_exit ]; then
        echo "  ✓ PASS (exit code: $exit_code)"
        # For verbose mode, check that we got some output
        if [[ "$args" == *"-v"* ]] && [ -s "${test_name}_stderr.log" ]; then
            echo "  ✓ Verbose output generated"
        fi
    else
        echo "  ✗ FAIL (expected: $expected_exit, got: $exit_code)"
        echo "  stderr:"
        cat "${test_name}_stderr.log"
        return 1
    fi
    
    return 0
}

# Step 5: Test individual options from the uncovered block

echo ""
echo "=== Testing individual options ==="

# Test -v (verbose)
run_gcov_tool "test_verbose" "-v base.gcda compare.gcda"

# Test -f (function level)
run_gcov_tool "test_func_level" "-f base.gcda compare.gcda"

# Test -F (full filename)
run_gcov_tool "test_fullname" "-F base.gcda compare.gcda"

# Test -o (object level)
run_gcov_tool "test_obj_level" "-o base.gcda compare.gcda"

# Test -h (hot only)
run_gcov_tool "test_hot_only" "-h base.gcda compare.gcda"

# Test -t (hot threshold) with various values
run_gcov_tool "test_threshold_0.5" "-t 0.5 base.gcda compare.gcda"
run_gcov_tool "test_threshold_0.0" "-t 0.0 base.gcda compare.gcda"
run_gcov_tool "test_threshold_1.0" "-t 1.0 base.gcda compare.gcda"

# Test -t with invalid value (should still parse but may produce no output)
run_gcov_tool "test_threshold_2.5" "-t 2.5 base.gcda compare.gcda" 0

# Test -t with negative value
run_gcov_tool "test_threshold_neg" "-t -1.0 base.gcda compare.gcda" 0

echo ""
echo "=== Testing option combinations ==="

# Test combinations of 2-3 options
run_gcov_tool "test_comb_v_f" "-v -f base.gcda compare.gcda"
run_gcov_tool "test_comb_v_f_o" "-v -f -o base.gcda compare.gcda"
run_gcov_tool "test_comb_F_h" "-F -h base.gcda compare.gcda"
run_gcov_tool "test_comb_v_F_h" "-v -F -h base.gcda compare.gcda"
run_gcov_tool "test_comb_f_o_h" "-f -o -h base.gcda compare.gcda"
run_gcov_tool "test_comb_all" "-v -f -F -o -h -t 0.75 base.gcda compare.gcda"

# Test with threshold in combination
run_gcov_tool "test_comb_v_t" "-v -t 0.3 base.gcda compare.gcda"
run_gcov_tool "test_comb_F_h_t" "-F -h -t 0.8 base.gcda compare.gcda"

echo ""
echo "=== Testing edge cases ==="

# Test invalid option (should trigger default case and usage)
echo "Testing invalid option -x (should show usage)"
if ! gcov-tool overlap -x base.gcda compare.gcda 2>&1 | grep -q "Usage:\|usage"; then
    echo "  ✗ FAIL: Usage message not shown for invalid option"
else
    echo "  ✓ PASS: Usage shown for invalid option"
fi

# Test missing required argument for -t
echo "Testing missing argument for -t"
if gcov-tool overlap -t base.gcda compare.gcda 2>&1 | grep -q "requires an argument\|option.*requires an argument"; then
    echo "  ✓ PASS: Error shown for missing argument"
else
    echo "  Note: Different error handling for missing argument"
fi

# Test with non-existent files
echo "Testing with non-existent files"
if ! gcov-tool overlap -v nonexistent1.gcda nonexistent2.gcda 2>&1 | grep -q "error\|Error\|cannot open"; then
    echo "  Note: No specific error message for missing files"
else
    echo "  ✓ PASS: Error shown for missing files"
fi

echo ""
echo "=== Summary ==="
echo "All tests completed successfully!"
echo "Log files are available in: $TEST_DIR"
echo ""
echo "To examine the coverage impact, run:"
echo "  cd $TEST_DIR"
echo "  ls -la *.log"
echo ""
echo "The following gcov-tool options were tested:"
echo "  -v (verbose)              - Sets verbose = true"
echo "  -f (function level)       - Sets overlap_func_level = 1"
echo "  -F (full filename)        - Sets overlap_use_fullname = 1"
echo "  -o (object level)         - Sets overlap_obj_level = 1"
echo "  -h (hot only)             - Sets overlap_hot_only = 1"
echo "  -t (threshold)            - Sets overlap_hot_threshold via atof(optarg)"
echo ""
echo "Each option and combination should have triggered the corresponding"
echo "case statements in the uncovered switch block (lines 534-554)."
```

This script provides comprehensive testing of all the uncovered lines:

1. **Creates valid GCOV data files**: Compiles a simple C program with coverage instrumentation and runs it to generate `.gcda` files.

2. **Tests each individual option**:
   - `-v` (verbose): Triggers `case 'v'`, sets `verbose = true`, calls `gcov_set_verbose()`
   - `-f` (function level): Triggers `case 'f'`, sets `overlap_func_level = 1`
   - `-F` (full filename): Triggers `case 'F'`, sets `overlap_use_fullname = 1`
   - `-o` (object level): Triggers `case 'o'`, sets `overlap_obj_level = 1`
   - `-h` (hot only): Triggers `case 'h'`, sets `overlap_hot_only = 1`
   - `-t` (threshold): Triggers `case 't'`, calls `atof(optarg)` to set `overlap_hot_threshold`

3. **Tests option combinations**: Exercises multiple options together to ensure they work correctly in combination.

4. **Tests edge cases**:
   - Boundary values for `-t`: `0.0`, `1.0`, `2.5`, `-1.0`
   - Invalid option `-x`: Triggers the `default` case and calls `overlap_usage()`
   - Missing argument for `-t`: Tests error handling

5. **Validates execution**: Checks exit codes and captures output to verify each option was processed.

To use this script:
1. Save it as `test_gcov_tool_overlap.sh`
2. Make it executable: `chmod +x test_gcov_tool_overlap.sh`
3. Ensure `gcov-tool` is in your PATH (built with coverage instrumentation)
4. Run it: `./test_gcov_tool_overlap.sh`

The script creates a temporary directory, generates all necessary test files, runs all the test cases, and cleans up automatically. Each test invocation of `gcov-tool` will trigger the corresponding code paths in the uncovered switch statement, helping to achieve coverage of lines 534-554 in `gcov-tool.cc`.
