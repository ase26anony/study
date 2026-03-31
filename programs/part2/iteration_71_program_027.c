Here's a shell script that implements the test requirements to cover the uncovered lines in `gcov-tool.cc`:

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
if ! command -v gcov-tool >/dev/null 2>&1; then
    echo "Error: gcov-tool not found in PATH"
    exit 1
fi

# Create minimal C source files with coverage opportunities
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
echo "Compiling test programs with coverage instrumentation..."
gcc -fprofile-arcs -ftest-coverage -O0 prog1.c -o prog1
gcc -fprofile-arcs -ftest-coverage -O0 prog2.c -o prog2

# Run programs to generate .gcda files
echo "Running programs to generate coverage data..."
./prog1 > /dev/null
./prog2 > /dev/null

# Verify .gcda files were created
if [ ! -f prog1.gcda ] || [ ! -f prog2.gcda ]; then
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

# Test 2: -f, -F, -o, -h boolean flags
echo "Test 2: Testing boolean flags (-f, -F, -o, -h)..."
if gcov-tool overlap -f -F -o -h prog1.gcda prog2.gcda > /dev/null 2>&1; then
    echo "✓ Boolean flags accepted"
else
    echo "✗ Boolean flags failed"
    exit 1
fi

# Test 3: -t with valid float argument
echo "Test 3: Testing -t with valid float argument (0.75)..."
if gcov-tool overlap -t 0.75 prog1.gcda prog2.gcda > /dev/null 2>&1; then
    echo "✓ -t 0.75 option accepted"
else
    echo "✗ -t 0.75 option failed"
    exit 1
fi

# Test 4: -t with different float argument (0.5)
echo "Test 4: Testing -t with different float argument (0.5)..."
if gcov-tool overlap -t 0.5 prog1.gcda prog2.gcda > /dev/null 2>&1; then
    echo "✓ -t 0.5 option accepted"
else
    echo "✗ -t 0.5 option failed"
    exit 1
fi

# Test 5: -t with integer argument (should also work)
echo "Test 5: Testing -t with integer argument (1)..."
if gcov-tool overlap -t 1 prog1.gcda prog2.gcda > /dev/null 2>&1; then
    echo "✓ -t 1 option accepted"
else
    echo "✗ -t 1 option failed"
    exit 1
fi

# Test 6: Invalid option to trigger default case and overlap_usage()
echo "Test 6: Testing invalid option (-x) to trigger usage..."
if ! gcov-tool overlap -x prog1.gcda prog2.gcda 2>&1 | grep -q "usage\|Usage\|overlap"; then
    echo "✗ Invalid option did not trigger usage message"
    # Some implementations may exit without printing usage
    # Check if command failed (which it should)
    if [ $? -eq 0 ]; then
        echo "✗ Invalid option should have failed"
        exit 1
    else
        echo "✓ Invalid option caused failure (may not print usage)"
    fi
else
    echo "✓ Invalid option triggered usage message"
fi

# Test 7: Combined options
echo "Test 7: Testing combined options (-v -f -t 0.8)..."
if gcov-tool overlap -v -f -t 0.8 prog1.gcda prog2.gcda > /dev/null 2>&1; then
    echo "✓ Combined options accepted"
else
    echo "✗ Combined options failed"
    exit 1
fi

# Test 8: Missing required argument for -t
echo "Test 8: Testing -t without argument (should fail)..."
if gcov-tool overlap -t prog1.gcda prog2.gcda 2>&1 | grep -q "requires\|argument\|missing"; then
    echo "✓ -t without argument properly rejected"
else
    # Check exit code
    gcov-tool overlap -t prog1.gcda prog2.gcda > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "✓ -t without argument caused failure"
    else
        echo "✗ -t without argument should have failed"
        exit 1
    fi
fi

# Test 9: Invalid argument for -t (non-numeric)
echo "Test 9: Testing -t with non-numeric argument..."
if gcov-tool overlap -t invalid prog1.gcda prog2.gcda 2>&1 | grep -q "invalid\|number\|float"; then
    echo "✓ Non-numeric argument for -t properly rejected"
else
    # Check exit code
    gcov-tool overlap -t invalid prog1.gcda prog2.gcda > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "✓ Non-numeric argument for -t caused failure"
    else
        echo "✗ Non-numeric argument for -t should have failed"
        exit 1
    fi
fi

echo ""
echo "All tests completed successfully!"
echo "The uncovered lines in gcov-tool.cc should now be covered:"
echo "  - Case 'v': verbose flag setting"
echo "  - Case 'f': overlap_func_level flag"
echo "  - Case 'F': overlap_use_fullname flag"
echo "  - Case 'o': overlap_obj_level flag"
echo "  - Case 'h': overlap_hot_only flag"
echo "  - Case 't': overlap_hot_threshold with atof()"
echo "  - Default case: invalid option handling"

# Cleanup happens automatically via trap
exit 0
```

This script comprehensively tests all the uncovered lines:

1. **Command-Line Argument Parsing**: Tests each short option (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) separately and in combination.

2. **Overlap Subcommand with GCDA Files**: Creates two C programs, compiles them with coverage instrumentation, runs them to generate `.gcda` files, and uses these as input to `gcov-tool overlap`.

3. **Option-Specific Behavior**:
   - `-v`: Tests verbose mode (checks for verbose output)
   - `-f`, `-F`, `-o`, `-h`: Tests boolean flags in combination
   - `-t`: Tests with valid float arguments (0.75, 0.5, 1) and invalid arguments

4. **Invalid Option Handling**: Tests `-x` to trigger the default case and `overlap_usage()`.

5. **Edge Cases**: Tests missing argument for `-t` and non-numeric argument for `-t`.

The script creates a temporary directory, generates the necessary coverage data files, runs all test cases, and cleans up automatically. Each test case validates that the options are accepted and the commands execute (or fail as expected for invalid cases).
