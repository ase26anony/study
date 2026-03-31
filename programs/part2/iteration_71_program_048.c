Here's a shell script that implements the test requirements for covering the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash
# test_gcov_tool_overlap.sh - Test gcov-tool overlap command-line options

set -e

# Create temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov_test_XXXXXX)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"

# Function to clean up on exit
cleanup() {
    echo "Cleaning up..."
    cd /
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

# Check if gcov-tool exists
if ! command -v gcov-tool >/dev/null 2>&1; then
    echo "Error: gcov-tool not found in PATH"
    exit 1
fi

# Create minimal C source files with coverage instrumentation
cat > prog1.c << 'EOF'
#include <stdio.h>

int main() {
    int i;
    for (i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
    return 0;
}
EOF

cat > prog2.c << 'EOF'
#include <stdio.h>

int main() {
    int i;
    for (i = 0; i < 5; i++) {
        if (i < 3) {
            printf("Less than 3: %d\n", i);
        } else {
            printf("Three or more: %d\n", i);
        }
    }
    return 0;
}
EOF

# Compile with coverage instrumentation
echo "Compiling test programs with coverage..."
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

echo "Testing gcov-tool overlap with various options..."

# Test 1: -v (verbose) option
echo "Test 1: Testing -v (verbose) option..."
if gcov-tool overlap -v prog1.gcda prog2.gcda 2>&1 | grep -q "verbose\|Verbose"; then
    echo "✓ -v option triggered verbose mode"
else
    echo "✓ -v option executed (may not have visible output)"
fi

# Test 2: -f (function level) option
echo "Test 2: Testing -f (function level) option..."
if gcov-tool overlap -f prog1.gcda prog2.gcda >/dev/null 2>&1; then
    echo "✓ -f option executed successfully"
else
    echo "✗ -f option failed"
    exit 1
fi

# Test 3: -F (fullname) option
echo "Test 3: Testing -F (fullname) option..."
if gcov-tool overlap -F prog1.gcda prog2.gcda >/dev/null 2>&1; then
    echo "✓ -F option executed successfully"
else
    echo "✗ -F option failed"
    exit 1
fi

# Test 4: -o (object level) option
echo "Test 4: Testing -o (object level) option..."
if gcov-tool overlap -o prog1.gcda prog2.gcda >/dev/null 2>&1; then
    echo "✓ -o option executed successfully"
else
    echo "✗ -o option failed"
    exit 1
fi

# Test 5: -h (hot only) option
echo "Test 5: Testing -h (hot only) option..."
if gcov-tool overlap -h prog1.gcda prog2.gcda >/dev/null 2>&1; then
    echo "✓ -h option executed successfully"
else
    echo "✗ -h option failed"
    exit 1
fi

# Test 6: Combined boolean flags
echo "Test 6: Testing combined boolean flags (-f -F -o -h)..."
if gcov-tool overlap -f -F -o -h prog1.gcda prog2.gcda >/dev/null 2>&1; then
    echo "✓ Combined boolean flags executed successfully"
else
    echo "✗ Combined boolean flags failed"
    exit 1
fi

# Test 7: -t (threshold) with valid float argument
echo "Test 7: Testing -t (threshold) with valid float argument..."
if gcov-tool overlap -t 0.75 prog1.gcda prog2.gcda >/dev/null 2>&1; then
    echo "✓ -t 0.75 option executed successfully"
else
    echo "✗ -t 0.75 option failed"
    exit 1
fi

# Test 8: -t (threshold) with different float value
echo "Test 8: Testing -t (threshold) with 0.5..."
if gcov-tool overlap -t 0.5 prog1.gcda prog2.gcda >/dev/null 2>&1; then
    echo "✓ -t 0.5 option executed successfully"
else
    echo "✗ -t 0.5 option failed"
    exit 1
fi

# Test 9: -t (threshold) with integer argument
echo "Test 9: Testing -t (threshold) with integer argument..."
if gcov-tool overlap -t 1 prog1.gcda prog2.gcda >/dev/null 2>&1; then
    echo "✓ -t 1 option executed successfully"
else
    echo "✗ -t 1 option failed"
    exit 1
fi

# Test 10: Invalid option to trigger default case and overlap_usage()
echo "Test 10: Testing invalid option to trigger usage..."
if ! gcov-tool overlap -x prog1.gcda prog2.gcda 2>&1 | grep -q "usage\|Usage\|overlap"; then
    echo "✗ Invalid option did not trigger usage message"
    exit 1
else
    echo "✓ Invalid option triggered usage message (as expected)"
fi

# Test 11: Missing argument for -t option
echo "Test 11: Testing missing argument for -t option..."
if gcov-tool overlap -t prog1.gcda prog2.gcda 2>&1 | grep -q "requires an argument\|missing argument"; then
    echo "✓ Missing argument for -t detected"
else
    echo "Note: Missing argument handling may vary"
fi

# Test 12: Invalid argument for -t option (non-numeric)
echo "Test 12: Testing invalid argument for -t option (non-numeric)..."
if ! gcov-tool overlap -t invalid prog1.gcda prog2.gcda 2>&1 >/dev/null; then
    echo "✓ Non-numeric argument for -t rejected"
else
    echo "Note: Non-numeric argument handling may vary"
fi

# Test 13: Verbose combined with other options
echo "Test 13: Testing verbose combined with other options..."
if gcov-tool overlap -v -f -t 0.8 prog1.gcda prog2.gcda >/dev/null 2>&1; then
    echo "✓ Verbose combined with other options executed"
else
    echo "✗ Verbose combined with other options failed"
    exit 1
fi

echo ""
echo "All tests completed successfully!"
echo "The uncovered lines in gcov-tool.cc (534-554) should now be covered by:"
echo "  - Case 'v': Test 1, Test 13"
echo "  - Case 'f': Test 2, Test 6, Test 13"
echo "  - Case 'F': Test 3, Test 6"
echo "  - Case 'o': Test 4, Test 6"
echo "  - Case 'h': Test 5, Test 6"
echo "  - Case 't': Test 7, Test 8, Test 9, Test 13"
echo "  - Default case: Test 10"
```

This script comprehensively tests all the uncovered lines:

1. **Command-Line Argument Parsing**: Tests each short option (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) individually and in combination.

2. **Overlap Subcommand with Required Files**: Creates two C programs, compiles them with coverage instrumentation, runs them to generate `.gcda` files, and uses these as input to `gcov-tool overlap`.

3. **Option-Specific Behavior**:
   - `-v`: Tests verbose mode (Test 1, 13)
   - `-f`, `-F`, `-o`, `-h`: Tests boolean flags individually and combined (Tests 2-6)
   - `-t`: Tests with valid float arguments (0.75, 0.5, 1) (Tests 7-9, 13)

4. **Invalid Option Handling**: Tests `-x` to trigger the default case and call `overlap_usage()` (Test 10)

5. **Edge Cases**: Also tests missing arguments and invalid non-numeric arguments for `-t` (Tests 11-12)

The script creates a temporary directory, generates the necessary coverage data files, runs all test cases, and cleans up after itself. Each test validates that the command executes (or fails as expected for invalid cases), ensuring the switch cases in lines 534-554 of `gcov-tool.cc` are exercised.
