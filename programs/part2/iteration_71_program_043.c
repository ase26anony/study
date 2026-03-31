Here's a shell script that implements the test requirements to cover the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash
# test_gcov_tool_overlap.sh - Test gcov-tool overlap command-line options

set -e

# Create temporary directory for test files
TEST_DIR=$(mktemp -d)
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    cd /
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

# Check if gcov-tool exists
if ! command -v gcov-tool >/dev/null 2>&1; then
    echo "Error: gcov-tool not found in PATH"
    exit 1
fi

# Create minimal C source files with coverage opportunities
cat > prog1.c << 'EOF'
#include <stdio.h>

int main() {
    int i, sum = 0;
    
    // Generate some arcs for coverage
    for (i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            sum += i;
        } else {
            sum -= i;
        }
    }
    
    printf("Result: %d\n", sum);
    return 0;
}
EOF

cat > prog2.c << 'EOF'
#include <stdio.h>

int main() {
    int i, product = 1;
    
    // Different control flow for overlap comparison
    for (i = 1; i <= 5; i++) {
        if (i > 3) {
            product *= i * 2;
        } else {
            product *= i;
        }
    }
    
    printf("Result: %d\n", product);
    return 0;
}
EOF

# Compile with coverage instrumentation
echo "Compiling test programs with coverage instrumentation..."
gcc -fprofile-arcs -ftest-coverage -O0 prog1.c -o prog1
gcc -fprofile-arcs -ftest-coverage -O0 prog2.c -o prog2

# Run programs to generate .gcda files
echo "Running programs to generate coverage data..."
./prog1 > /dev/null
./prog2 > /dev/null

# Verify .gcda files were created
if [[ ! -f prog1.gcda ]] || [[ ! -f prog2.gcda ]]; then
    echo "Error: Failed to generate .gcda files"
    exit 1
fi

echo "Testing gcov-tool overlap command with various options..."

# Test 1: Verbose mode (-v)
echo "Test 1: Testing -v (verbose) option..."
if gcov-tool overlap -v prog1.gcda prog2.gcda 2>&1 | grep -q "verbose\|Verbose"; then
    echo "✓ Verbose mode triggered"
else
    # Even if no verbose output is printed, the flag should be set
    gcov-tool overlap -v prog1.gcda prog2.gcda > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "✓ -v option accepted"
    else
        echo "✗ -v option failed"
        exit 1
    fi
fi

# Test 2: All boolean flags (-f -F -o -h)
echo "Test 2: Testing boolean flags (-f -F -o -h)..."
if gcov-tool overlap -f -F -o -h prog1.gcda prog2.gcda > /dev/null 2>&1; then
    echo "✓ Boolean flags accepted"
else
    echo "✗ Boolean flags failed"
    exit 1
fi

# Test 3: Threshold option with valid float (-t 0.75)
echo "Test 3: Testing -t with valid float argument..."
if gcov-tool overlap -t 0.75 prog1.gcda prog2.gcda > /dev/null 2>&1; then
    echo "✓ -t 0.75 accepted"
else
    echo "✗ -t 0.75 failed"
    exit 1
fi

# Test 4: Threshold option with different float value (-t 0.5)
echo "Test 4: Testing -t with 0.5 threshold..."
if gcov-tool overlap -t 0.5 prog1.gcda prog2.gcda > /dev/null 2>&1; then
    echo "✓ -t 0.5 accepted"
else
    echo "✗ -t 0.5 failed"
    exit 1
fi

# Test 5: Threshold option with integer value (-t 1)
echo "Test 5: Testing -t with integer argument..."
if gcov-tool overlap -t 1 prog1.gcda prog2.gcda > /dev/null 2>&1; then
    echo "✓ -t 1 accepted"
else
    echo "✗ -t 1 failed"
    exit 1
fi

# Test 6: Invalid option to trigger default case and overlap_usage()
echo "Test 6: Testing invalid option to trigger usage..."
if ! gcov-tool overlap -x prog1.gcda prog2.gcda 2>&1 | grep -q "usage\|Usage\|overlap"; then
    # Some versions might exit without printing usage
    if [ $? -ne 0 ]; then
        echo "✓ Invalid option rejected (exit code $?)"
    else
        echo "✗ Invalid option should have been rejected"
        exit 1
    fi
else
    echo "✓ Usage message displayed for invalid option"
fi

# Test 7: Combined options
echo "Test 7: Testing combined options..."
if gcov-tool overlap -v -f -o -t 0.8 prog1.gcda prog2.gcda > /dev/null 2>&1; then
    echo "✓ Combined options accepted"
else
    echo "✗ Combined options failed"
    exit 1
fi

# Test 8: Missing required argument for -t (should trigger error)
echo "Test 8: Testing -t without argument..."
if ! gcov-tool overlap -t prog1.gcda prog2.gcda 2>&1 | grep -q "requires\|argument\|missing"; then
    # Check exit code instead
    gcov-tool overlap -t prog1.gcda prog2.gcda > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "✓ -t without argument correctly rejected"
    else
        echo "✗ -t without argument should have failed"
        exit 1
    fi
else
    echo "✓ -t without argument triggered error message"
fi

# Test 9: Invalid argument for -t (non-numeric)
echo "Test 9: Testing -t with non-numeric argument..."
if ! gcov-tool overlap -t invalid prog1.gcda prog2.gcda 2>&1 | grep -q "invalid\|numeric\|conversion"; then
    gcov-tool overlap -t invalid prog1.gcda prog2.gcda > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "✓ Non-numeric argument to -t rejected"
    else
        echo "✗ Non-numeric argument to -t should have failed"
        exit 1
    fi
else
    echo "✓ Non-numeric argument to -t triggered error"
fi

echo ""
echo "All tests completed successfully!"
echo "Covered command-line options:"
echo "  -v (verbose)            - Sets verbose flag"
echo "  -f (func_level)         - Sets overlap_func_level"
echo "  -F (fullname)           - Sets overlap_use_fullname"
echo "  -o (obj_level)          - Sets overlap_obj_level"
echo "  -h (hot_only)           - Sets overlap_hot_only"
echo "  -t (threshold)          - Sets overlap_hot_threshold with argument"
echo "  default case            - Triggers overlap_usage() for invalid options"

# Cleanup happens automatically via trap
exit 0
```

This script provides comprehensive testing of the uncovered lines:

1. **Command-Line Argument Parsing**: Tests each short option (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) as required.

2. **Overlap Subcommand with GCDA Files**: Creates two C programs, compiles them with `-fprofile-arcs -ftest-coverage`, runs them to generate `.gcda` files, and uses these as input to `gcov-tool overlap`.

3. **Option-Specific Behavior**:
   - `-v`: Tests verbose mode
   - `-f`, `-F`, `-o`, `-h`: Tests boolean flags individually and combined
   - `-t`: Tests with valid floats (0.75, 0.5), integer (1), and invalid arguments

4. **Invalid Option Handling**: Tests `-x` to trigger the `default:` case and call `overlap_usage()`.

5. **Additional Edge Cases**:
   - Combined options test
   - Missing argument for `-t`
   - Non-numeric argument for `-t`

The script creates a temporary directory, generates the necessary coverage data files, runs all test cases, validates exit codes, and cleans up after itself. Each test case is designed to exercise specific code paths in the uncovered switch statement.
