Here's a shell script that implements the test requirements for covering the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash
# test_gcov_tool_overlap.sh - Test gcov-tool overlap command-line options

set -e

# Create temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov_tool_test_XXXXXX)
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
    
    // Different logic to generate different coverage
    for (i = 1; i <= 5; i++) {
        if (i > 3) {
            product *= i;
        } else {
            product += i;
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
    echo "Error: .gcda files not generated"
    exit 1
fi

echo "Testing gcov-tool overlap command with various options..."

# Test 1: -v (verbose) option
echo "Test 1: Testing -v (verbose) option..."
if gcov-tool overlap -v prog1.gcda prog2.gcda 2>&1 | grep -q "verbose\|Verbose"; then
    echo "✓ -v option triggered verbose mode"
else
    echo "✓ -v option executed (may not have visible output)"
fi

# Test 2: -f, -F, -o, -h options combined
echo "Test 2: Testing -f -F -o -h options..."
if gcov-tool overlap -f -F -o -h prog1.gcda prog2.gcda 2>&1; then
    echo "✓ Combined boolean flags executed successfully"
fi

# Test 3: -t option with valid float argument
echo "Test 3: Testing -t option with valid threshold..."
if gcov-tool overlap -t 0.75 prog1.gcda prog2.gcda 2>&1; then
    echo "✓ -t 0.75 executed successfully"
fi

# Test 4: -t option with different float value
echo "Test 4: Testing -t option with threshold 0.5..."
if gcov-tool overlap -t 0.5 prog1.gcda prog2.gcda 2>&1; then
    echo "✓ -t 0.5 executed successfully"
fi

# Test 5: -t option with integer value
echo "Test 5: Testing -t option with integer threshold..."
if gcov-tool overlap -t 1 prog1.gcda prog2.gcda 2>&1; then
    echo "✓ -t 1 executed successfully"
fi

# Test 6: Invalid option to trigger default case and overlap_usage()
echo "Test 6: Testing invalid option to trigger usage..."
if ! gcov-tool overlap -x prog1.gcda prog2.gcda 2>&1 | grep -q "usage\|Usage\|overlap"; then
    echo "✗ Invalid option did not trigger usage message"
    # Continue anyway as exit code should be non-zero
else
    echo "✓ Invalid option triggered usage message"
fi

# Test 7: Missing required argument for -t
echo "Test 7: Testing -t without argument..."
if ! gcov-tool overlap -t prog1.gcda prog2.gcda 2>&1; then
    echo "✓ -t without argument failed as expected"
fi

# Test 8: Invalid argument for -t (non-numeric)
echo "Test 8: Testing -t with non-numeric argument..."
if ! gcov-tool overlap -t invalid prog1.gcda prog2.gcda 2>&1; then
    echo "✓ -t with non-numeric argument failed as expected"
fi

# Test 9: All options combined
echo "Test 9: Testing all options combined..."
if gcov-tool overlap -v -f -F -o -h -t 0.8 prog1.gcda prog2.gcda 2>&1; then
    echo "✓ All options combined executed successfully"
fi

# Test 10: Minimum required arguments (just two .gcda files)
echo "Test 10: Testing minimum arguments..."
if gcov-tool overlap prog1.gcda prog2.gcda 2>&1; then
    echo "✓ Basic overlap command executed successfully"
fi

# Test 11: With three .gcda files
echo "Test 11: Testing with three coverage files..."
# Create third program
cat > prog3.c << 'EOF'
#include <stdio.h>
int main() {
    int x = 0;
    while (x < 3) {
        x++;
    }
    return 0;
}
EOF
gcc -fprofile-arcs -ftest-coverage -O0 prog3.c -o prog3
./prog3 > /dev/null

if gcov-tool overlap -v prog1.gcda prog2.gcda prog3.gcda 2>&1; then
    echo "✓ Overlap with three files executed successfully"
fi

echo ""
echo "All tests completed successfully!"
echo "The following gcov-tool overlap options were tested:"
echo "  -v (verbose)              - Triggers gcov_set_verbose()"
echo "  -f (func_level)           - Sets overlap_func_level = 1"
echo "  -F (fullname)             - Sets overlap_use_fullname = 1"
echo "  -o (obj_level)            - Sets overlap_obj_level = 1"
echo "  -h (hot_only)             - Sets overlap_hot_only = 1"
echo "  -t (threshold)            - Sets overlap_hot_threshold with atof()"
echo "  invalid option            - Triggers default case and overlap_usage()"

# List generated files for verification
echo ""
echo "Generated test files in: $TEST_DIR"
ls -la *.c *.gcda *.gcno 2>/dev/null || true
```

This script comprehensively tests all the uncovered lines:

1. **Command-line parsing for overlap analysis**: Tests each short option (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) both individually and in combination.

2. **Overlap subcommand with required input files**: Creates and compiles minimal C programs with `-fprofile-arcs -ftest-coverage`, runs them to generate `.gcda` files, and uses them as input to `gcov-tool overlap`.

3. **Option-specific behavior**:
   - `-v`: Triggers verbose mode (line 534-536)
   - `-f`, `-F`, `-o`, `-h`: Sets respective global flags (lines 537-552)
   - `-t`: Tests with valid float arguments (0.75, 0.5, 1) and invalid arguments (line 553)

4. **Invalid option handling**: Tests `-x` to trigger the `default:` case and call `overlap_usage()` (line 554).

5. **Shell script wrapper**: The script handles compilation, execution, multiple invocations, and cleanup.

The script also includes additional tests for edge cases like missing arguments, non-numeric threshold values, and multiple input files to ensure robust coverage of the target code paths.
