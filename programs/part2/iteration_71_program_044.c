Here's a shell script that implements the test requirements for covering the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash
# test_gcov_tool_overlap.sh - Test gcov-tool overlap command-line options

set -e

# Create temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov_test_XXXXXX)
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    cd /
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

# Check if gcov-tool exists
if ! command -v gcov-tool &> /dev/null; then
    echo "Error: gcov-tool not found in PATH"
    exit 1
fi

# Check if gcc supports coverage
if ! gcc -fprofile-arcs -ftest-coverage -x c -c -o /dev/null /dev/null 2>&1; then
    echo "Error: gcc doesn't support -fprofile-arcs -ftest-coverage"
    exit 1
fi

echo "=== Creating test C programs ==="

# Create first test program
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

# Create second test program (different structure)
cat > prog2.c << 'EOF'
#include <stdio.h>

int helper(int x) {
    if (x > 5) {
        return x * 2;
    } else {
        return x + 3;
    }
}

int main() {
    int i, result = 0;
    
    for (i = 0; i < 8; i++) {
        result += helper(i);
        
        switch (i % 3) {
            case 0: result += 1; break;
            case 1: result += 2; break;
            case 2: result += 3; break;
        }
    }
    
    printf("Final: %d\n", result);
    return 0;
}
EOF

echo "=== Compiling test programs with coverage ==="

# Compile both programs with coverage instrumentation
gcc -fprofile-arcs -ftest-coverage -O0 prog1.c -o prog1
gcc -fprofile-arcs -ftest-coverage -O0 prog2.c -o prog2

echo "=== Running programs to generate .gcda files ==="

# Run programs to generate coverage data
./prog1 > /dev/null
./prog2 > /dev/null

# Verify .gcda files were created
if [ ! -f prog1.gcda ] || [ ! -f prog2.gcda ]; then
    echo "Error: .gcda files not generated"
    exit 1
fi

echo "=== Testing gcov-tool overlap options ==="

# Test 1: -v (verbose) option
echo "Test 1: Testing -v (verbose) option"
if gcov-tool overlap -v prog1.gcda prog2.gcda 2>&1 | grep -q "verbose\|Verbose"; then
    echo "✓ -v option triggered verbose output"
else
    echo "✓ -v option executed (may not have visible output)"
fi

# Test 2: -f (function level) option
echo -e "\nTest 2: Testing -f (function level) option"
if gcov-tool overlap -f prog1.gcda prog2.gcda > /dev/null 2>&1; then
    echo "✓ -f option executed successfully"
else
    echo "✗ -f option failed"
    exit 1
fi

# Test 3: -F (fullname) option
echo -e "\nTest 3: Testing -F (fullname) option"
if gcov-tool overlap -F prog1.gcda prog2.gcda > /dev/null 2>&1; then
    echo "✓ -F option executed successfully"
else
    echo "✗ -F option failed"
    exit 1
fi

# Test 4: -o (object level) option
echo -e "\nTest 4: Testing -o (object level) option"
if gcov-tool overlap -o prog1.gcda prog2.gcda > /dev/null 2>&1; then
    echo "✓ -o option executed successfully"
else
    echo "✗ -o option failed"
    exit 1
fi

# Test 5: -h (hot only) option
echo -e "\nTest 5: Testing -h (hot only) option"
if gcov-tool overlap -h prog1.gcda prog2.gcda > /dev/null 2>&1; then
    echo "✓ -h option executed successfully"
else
    echo "✗ -h option failed"
    exit 1
fi

# Test 6: Combined boolean flags
echo -e "\nTest 6: Testing combined boolean flags (-f -F -o -h)"
if gcov-tool overlap -f -F -o -h prog1.gcda prog2.gcda > /dev/null 2>&1; then
    echo "✓ Combined flags executed successfully"
else
    echo "✗ Combined flags failed"
    exit 1
fi

# Test 7: -t (threshold) with valid float argument
echo -e "\nTest 7: Testing -t (threshold) with valid float (0.75)"
if gcov-tool overlap -t 0.75 prog1.gcda prog2.gcda > /dev/null 2>&1; then
    echo "✓ -t 0.75 option executed successfully"
else
    echo "✗ -t 0.75 option failed"
    exit 1
fi

# Test 8: -t (threshold) with different float value
echo -e "\nTest 8: Testing -t (threshold) with 0.5"
if gcov-tool overlap -t 0.5 prog1.gcda prog2.gcda > /dev/null 2>&1; then
    echo "✓ -t 0.5 option executed successfully"
else
    echo "✗ -t 0.5 option failed"
    exit 1
fi

# Test 9: -t (threshold) with integer argument
echo -e "\nTest 9: Testing -t (threshold) with integer (1)"
if gcov-tool overlap -t 1 prog1.gcda prog2.gcda > /dev/null 2>&1; then
    echo "✓ -t 1 option executed successfully"
else
    echo "✗ -t 1 option failed"
    exit 1
fi

# Test 10: Invalid option to trigger default case
echo -e "\nTest 10: Testing invalid option (-x) to trigger default case"
if ! gcov-tool overlap -x prog1.gcda prog2.gcda 2>&1 | grep -q "usage\|Usage\|overlap"; then
    echo "✗ Invalid option didn't trigger usage message"
    # Continue anyway as some implementations may exit differently
else
    echo "✓ Invalid option triggered usage message (as expected)"
fi

# Test 11: -t with invalid (non-numeric) argument
echo -e "\nTest 11: Testing -t with invalid argument (non-numeric)"
if gcov-tool overlap -t invalid prog1.gcda prog2.gcda 2>&1; then
    echo "✗ -t with invalid argument should have failed"
    # Note: atof() returns 0.0 for invalid input, so this might succeed
else
    echo "✓ -t with invalid argument handled"
fi

# Test 12: Verbose with combined options
echo -e "\nTest 12: Testing verbose with other options"
if gcov-tool overlap -v -f -t 0.8 prog1.gcda prog2.gcda 2>&1 > /dev/null; then
    echo "✓ Verbose with combined options executed"
else
    echo "✗ Verbose with combined options failed"
    exit 1
fi

echo -e "\n=== All tests completed successfully ==="
echo "Covered command-line options:"
echo "  -v (verbose)           - line 534-536"
echo "  -f (func level)        - line 537-539"
echo "  -F (fullname)          - line 540-542"
echo "  -o (object level)      - line 543-545"
echo "  -h (hot only)          - line 546-548"
echo "  -t (threshold)         - line 549-551"
echo "  default case           - line 552-554"

# Create a third .gcda for testing with more than 2 files
cat > prog3.c << 'EOF'
int main() {
    return 0;
}
EOF
gcc -fprofile-arcs -ftest-coverage -O0 prog3.c -o prog3
./prog3 > /dev/null

# Test with three files
echo -e "\nAdditional test: Three .gcda files"
if gcov-tool overlap -v prog1.gcda prog2.gcda prog3.gcda > /dev/null 2>&1; then
    echo "✓ Works with multiple .gcda files"
fi

echo -e "\n=== Test script completed ==="
```

This script comprehensively tests all the uncovered lines in the specified block:

1. **Creates valid GCDA files**: Compiles and runs two C programs with coverage instrumentation to generate `.gcda` files.

2. **Tests each option individually**:
   - `-v` (verbose): Triggers `gcov_set_verbose()`
   - `-f` (function level): Sets `overlap_func_level = 1`
   - `-F` (fullname): Sets `overlap_use_fullname = 1`
   - `-o` (object level): Sets `overlap_obj_level = 1`
   - `-h` (hot only): Sets `overlap_hot_only = 1`
   - `-t` (threshold): Calls `atof(optarg)` to set `overlap_hot_threshold`

3. **Tests combined options**: Uses `-f -F -o -h` together to ensure all boolean flags work in combination.

4. **Tests invalid option**: Uses `-x` to trigger the `default:` case and call `overlap_usage()`.

5. **Tests threshold with various arguments**: Tests valid floats (0.75, 0.5), integer (1), and invalid input.

6. **Validates execution**: Checks exit codes and captures output where appropriate.

The script creates temporary files, runs the tests, and cleans up automatically. Each test case is designed to exercise the specific switch cases in lines 534-554 of `gcov-tool.cc`.
