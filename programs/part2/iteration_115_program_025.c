```bash
#!/bin/bash
set -e

# Configuration
GCOV_TOOL="./gcov-tool"
TEST_DIR="./test_gcda_files"
RESULTS_DIR="./test_results"
mkdir -p "$RESULTS_DIR"

# Create a simple test program to generate .gcda files
create_test_gcda_files() {
    echo "Creating test .gcda files..."
    mkdir -p "$TEST_DIR"
    
    # Create a simple C program
    cat > "$TEST_DIR/test.c" << 'EOF'
#include <stdio.h>

void func1() {
    printf("Function 1\n");
}

void func2() {
    printf("Function 2\n");
}

int main() {
    func1();
    func2();
    return 0;
}
EOF

    # Compile with coverage
    cd "$TEST_DIR"
    gcc -fprofile-arcs -ftest-coverage -O0 -g test.c -o test_program
    # Run to generate .gcda files
    ./test_program > /dev/null 2>&1
    cd ..
    
    echo "Test .gcda files created in $TEST_DIR"
}

# Check if gcov-tool exists
if [ ! -f "$GCOV_TOOL" ]; then
    echo "Error: $GCOV_TOOL not found. Please build gcov-tool first."
    echo "Recommended build command:"
    echo "  g++ -O0 -g -fprofile-arcs -ftest-coverage -I. -o gcov-tool gcov-tool.cc -lgcov -lz"
    exit 1
fi

# Create test .gcda files
create_test_gcda_files

# Test counters
TESTS_PASSED=0
TESTS_FAILED=0

# Function to run a test and check result
run_test() {
    local test_name="$1"
    local command="$2"
    local expect_success="${3:-1}"  # Default to expecting success (exit code 0)
    
    echo "Running test: $test_name"
    echo "Command: $command"
    
    # Run the command
    eval "$command" > "$RESULTS_DIR/${test_name}.out" 2> "$RESULTS_DIR/${test_name}.err"
    local exit_code=$?
    
    # Check result
    if [ $expect_success -eq 1 ]; then
        if [ $exit_code -eq 0 ]; then
            echo "  ✓ PASSED"
            ((TESTS_PASSED++))
        else
            echo "  ✗ FAILED (exit code: $exit_code)"
            cat "$RESULTS_DIR/${test_name}.err"
            ((TESTS_FAILED++))
        fi
    else
        # Expecting failure (non-zero exit code)
        if [ $exit_code -ne 0 ]; then
            echo "  ✓ PASSED (expected failure)"
            ((TESTS_PASSED++))
        else
            echo "  ✗ FAILED (expected failure but got success)"
            ((TESTS_FAILED++))
        fi
    fi
    echo
}

# Test 1: Help and version flags (overall infrastructure)
run_test "test_help" "$GCOV_TOOL --help"
run_test "test_version" "$GCOV_TOOL --version"

# Test 2: Individual short options
run_test "test_verbose" "$GCOV_TOOL overlap -v $TEST_DIR"
run_test "test_func_level" "$GCOV_TOOL overlap -f $TEST_DIR"
run_test "test_fullname" "$GCOV_TOOL overlap -F $TEST_DIR"
run_test "test_obj_level" "$GCOV_TOOL overlap -o $TEST_DIR"
run_test "test_hot_only" "$GCOV_TOOL overlap -h $TEST_DIR"

# Test 3: -t option with various values
run_test "test_threshold_normal" "$GCOV_TOOL overlap -t 0.5 $TEST_DIR"
run_test "test_threshold_small" "$GCOV_TOOL overlap -t 0.001 $TEST_DIR"
run_test "test_threshold_large" "$GCOV_TOOL overlap -t 100.0 $TEST_DIR"
run_test "test_threshold_zero" "$GCOV_TOOL overlap -t 0 $TEST_DIR"

# Test 4: Combined options (different orders)
run_test "test_combined_v_f_o" "$GCOV_TOOL overlap -v -f -o $TEST_DIR"
run_test "test_combined_f_F_h" "$GCOV_TOOL overlap -f -F -h $TEST_DIR"
run_test "test_combined_o_v_f" "$GCOV_TOOL overlap -o -v -f $TEST_DIR"
run_test "test_combined_h_F_t" "$GCOV_TOOL overlap -h -F -t 0.3 $TEST_DIR"

# Test 5: All options combined
run_test "test_all_options" "$GCOV_TOOL overlap -v -f -F -o -h -t 0.75 $TEST_DIR"

# Test 6: Invalid option (to trigger default case and overlap_usage)
run_test "test_invalid_option" "$GCOV_TOOL overlap -x $TEST_DIR" 0

# Test 7: Multiple directories as positional arguments
run_test "test_multiple_dirs" "$GCOV_TOOL overlap -v $TEST_DIR $TEST_DIR"

# Test 8: Options with threshold and other flags
run_test "test_complex_1" "$GCOV_TOOL overlap -v -t 0.25 -f -o $TEST_DIR"
run_test "test_complex_2" "$GCOV_TOOL overlap -F -h -t 0.9 -v $TEST_DIR"

# Test 9: Edge case - threshold with scientific notation (if supported)
run_test "test_threshold_scientific" "$GCOV_TOOL overlap -t 1e-3 $TEST_DIR"

# Test 10: No positional arguments (should fail but parse options)
run_test "test_no_args" "$GCOV_TOOL overlap -v -f" 0

# Summary
echo "========================================"
echo "TEST SUMMARY"
echo "========================================"
echo "Tests passed: $TESTS_PASSED"
echo "Tests failed: $TESTS_FAILED"
echo "Total tests:  $((TESTS_PASSED + TESTS_FAILED))"

if [ $TESTS_FAILED -eq 0 ]; then
    echo "All tests passed successfully!"
    exit 0
else
    echo "Some tests failed. Check the output above for details."
    exit 1
fi
```

This shell script comprehensively tests the uncovered argument parsing logic in `gcov-tool.cc`:

1. **Individual Short Options**: Tests each uncovered option (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) separately.
2. **Combined Options**: Tests multiple options in different orders to ensure parsing is order-independent.
3. **Invalid Option**: Tests `-x` to trigger the `default:` case and call `overlap_usage()`.
4. **Edge Cases for `-t`**: Tests threshold values of 0.001 (small), 100.0 (large), and 0 (zero).
5. **Real .gcda Files**: Creates actual `.gcda` files by compiling and running a simple test program.
6. **Help and Version**: Tests `--help` and `--version` flags for overall infrastructure coverage.
7. **Complex Combination**: Includes a final test with all options combined as requested.

The script creates a test directory with actual `.gcda` files, runs all test cases, captures output and errors, and provides a summary of results. Each test's output is saved to separate files in the `test_results` directory for debugging.
