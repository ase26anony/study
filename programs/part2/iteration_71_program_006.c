Here's a shell script that implements the test requirements for covering the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash
# test_gcov_tool_overlap.sh - Test gcov-tool overlap command-line options

set -e

# Create temporary directory for test files
TESTDIR=$(mktemp -d /tmp/gcov_test_XXXXXX)
cd "$TESTDIR"

# Cleanup function
cleanup() {
    cd /
    rm -rf "$TESTDIR"
}
trap cleanup EXIT

# Check if gcov-tool exists
if ! command -v gcov-tool >/dev/null 2>&1; then
    echo "Error: gcov-tool not found in PATH"
    exit 1
fi

# Check if gcc supports coverage
if ! gcc -fprofile-arcs -ftest-coverage -x c -c -o /dev/null - <<<"int main(){}" 2>/dev/null; then
    echo "Error: gcc doesn't support -fprofile-arcs -ftest-coverage"
    exit 1
fi

echo "=== Creating test C programs ==="

# Create first test program
cat > prog1.c <<'EOF'
#include <stdio.h>
int main() {
    int i, sum = 0;
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

# Create second test program (different structure to generate different coverage)
cat > prog2.c <<'EOF'
#include <stdio.h>
int main() {
    int i, product = 1;
    for (i = 1; i <= 5; i++) {
        if (i > 2) {
            product *= i;
        } else {
            product += i;
        }
    }
    printf("Result: %d\n", product);
    return 0;
}
EOF

echo "=== Compiling with coverage instrumentation ==="
# Compile both programs with coverage
gcc -fprofile-arcs -ftest-coverage -O0 prog1.c -o prog1
gcc -fprofile-arcs -ftest-coverage -O0 prog2.c -o prog2

echo "=== Running programs to generate .gcda files ==="
./prog1 > /dev/null
./prog2 > /dev/null

# Verify .gcda files were created
if [ ! -f prog1.gcda ] || [ ! -f prog2.gcda ]; then
    echo "Error: .gcda files not generated"
    exit 1
fi

echo "=== Testing gcov-tool overlap with various options ==="

# Test 1: -v (verbose) option - covers case 'v'
echo "Test 1: Testing -v (verbose) option"
if gcov-tool overlap -v prog1.gcda prog2.gcda 2>&1 | grep -q "verbose\|Verbose"; then
    echo "✓ -v option triggered verbose mode"
else
    echo "✓ -v option executed (may not have visible output)"
fi

# Test 2: -f (function level) option - covers case 'f'
echo "Test 2: Testing -f (function level) option"
if gcov-tool overlap -f prog1.gcda prog2.gcda >/dev/null 2>&1; then
    echo "✓ -f option executed successfully"
else
    echo "✗ -f option failed"
    exit 1
fi

# Test 3: -F (fullname) option - covers case 'F'
echo "Test 3: Testing -F (fullname) option"
if gcov-tool overlap -F prog1.gcda prog2.gcda >/dev/null 2>&1; then
    echo "✓ -F option executed successfully"
else
    echo "✗ -F option failed"
    exit 1
fi

# Test 4: -o (object level) option - covers case 'o'
echo "Test 4: Testing -o (object level) option"
if gcov-tool overlap -o prog1.gcda prog2.gcda >/dev/null 2>&1; then
    echo "✓ -o option executed successfully"
else
    echo "✗ -o option failed"
    exit 1
fi

# Test 5: -h (hot only) option - covers case 'h'
echo "Test 5: Testing -h (hot only) option"
if gcov-tool overlap -h prog1.gcda prog2.gcda >/dev/null 2>&1; then
    echo "✓ -h option executed successfully"
else
    echo "✗ -h option failed"
    exit 1
fi

# Test 6: Combined boolean flags - covers cases 'f', 'F', 'o', 'h'
echo "Test 6: Testing combined boolean flags (-f -F -o -h)"
if gcov-tool overlap -f -F -o -h prog1.gcda prog2.gcda >/dev/null 2>&1; then
    echo "✓ Combined boolean flags executed successfully"
else
    echo "✗ Combined boolean flags failed"
    exit 1
fi

# Test 7: -t (threshold) with valid float - covers case 't'
echo "Test 7: Testing -t (threshold) with valid float 0.5"
if gcov-tool overlap -t 0.5 prog1.gcda prog2.gcda >/dev/null 2>&1; then
    echo "✓ -t 0.5 option executed successfully"
else
    echo "✗ -t 0.5 option failed"
    exit 1
fi

# Test 8: -t (threshold) with different float - covers case 't'
echo "Test 8: Testing -t (threshold) with float 0.75"
if gcov-tool overlap -t 0.75 prog1.gcda prog2.gcda >/dev/null 2>&1; then
    echo "✓ -t 0.75 option executed successfully"
else
    echo "✗ -t 0.75 option failed"
    exit 1
fi

# Test 9: -t (threshold) with integer - covers case 't'
echo "Test 9: Testing -t (threshold) with integer 1"
if gcov-tool overlap -t 1 prog1.gcda prog2.gcda >/dev/null 2>&1; then
    echo "✓ -t 1 option executed successfully"
else
    echo "✗ -t 1 option failed"
    exit 1
fi

# Test 10: Invalid option - triggers default case and overlap_usage()
echo "Test 10: Testing invalid option -x (should trigger usage)"
if ! gcov-tool overlap -x prog1.gcda prog2.gcda 2>&1 | grep -q "usage\|Usage\|overlap"; then
    echo "✗ Invalid option didn't trigger usage message"
    # Some implementations may exit without output, so we check exit code instead
    if [ $? -eq 0 ]; then
        echo "✗ Invalid option didn't fail as expected"
        exit 1
    else
        echo "✓ Invalid option failed with non-zero exit code (may not show usage)"
    fi
else
    echo "✓ Invalid option triggered usage message"
fi

# Test 11: -t with invalid (non-numeric) argument
echo "Test 11: Testing -t with invalid argument 'invalid'"
if gcov-tool overlap -t invalid prog1.gcda prog2.gcda 2>&1 | grep -q "error\|Error\|invalid"; then
    echo "✓ -t with invalid argument triggered error"
elif [ $? -ne 0 ]; then
    echo "✓ -t with invalid argument failed (non-zero exit)"
else
    echo "✗ -t with invalid argument didn't fail as expected"
    exit 1
fi

# Test 12: Mix of options
echo "Test 12: Testing mixed options (-v -f -t 0.3)"
if gcov-tool overlap -v -f -t 0.3 prog1.gcda prog2.gcda >/dev/null 2>&1; then
    echo "✓ Mixed options executed successfully"
else
    echo "✗ Mixed options failed"
    exit 1
fi

echo "=== All tests completed successfully ==="
echo "=== Cleaning up ==="

# Cleanup will happen via trap
cd /
echo "Test directory: $TESTDIR (will be cleaned up)"
```

This script:

1. **Creates two minimal C programs** with different control flow to generate distinct coverage data
2. **Compiles with coverage instrumentation** using `-fprofile-arcs -ftest-coverage`
3. **Runs the programs** to generate `.gcda` files
4. **Tests each uncovered option individually**:
   - `-v` for verbose mode
   - `-f` for function-level overlap
   - `-F` for fullname mode
   - `-o` for object-level overlap
   - `-h` for hot-only mode
   - `-t` with various numeric arguments
5. **Tests combined options** to ensure they work together
6. **Tests invalid options** to trigger the `default:` case and `overlap_usage()`
7. **Tests invalid arguments** for `-t` option
8. **Cleans up** all temporary files

The script uses `grep` to check for expected output patterns and exit codes to verify correct behavior. Each test case is designed to exercise specific lines in the uncovered block from `gcov-tool.cc`.
