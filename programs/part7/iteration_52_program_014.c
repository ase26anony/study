Looking at the uncovered lines in `gcov-dump.cc`, I need to create a comprehensive test that exercises all the command-line flag handling logic. Here's a shell script that systematically tests each uncovered case:

```bash
#!/bin/bash
# Test script for gcov-dump command-line argument parsing
# This script tests the uncovered switch cases in gcov-dump.cc lines 111-130

set -e  # Exit on any error

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    cd /
    rm -rf "$TEST_DIR"
    echo "Test completed. Temporary directory $TEST_DIR removed."
}

# Trap to ensure cleanup on exit
trap cleanup EXIT

echo "Testing gcov-dump command-line argument parsing..."
echo "Test directory: $TEST_DIR"

# Create a simple C test program
cat > test1.c << 'EOF'
#include <stdio.h>

int main() {
    printf("Hello from test1\n");
    return 0;
}
EOF

cat > test2.c << 'EOF'
#include <stdio.h>

int helper() {
    return 42;
}

int main() {
    printf("Result: %d\n", helper());
    return 0;
}
EOF

# Compile test programs with coverage instrumentation
echo "Compiling test programs with coverage flags..."
gcc -fprofile-arcs -ftest-coverage -O0 test1.c -o test1_prog
gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test2_prog

# Run the programs to generate .gcda files
echo "Running test programs to generate coverage data..."
./test1_prog > /dev/null
./test2_prog > /dev/null

# Verify that coverage files were created
if [ ! -f test1.gcda ] || [ ! -f test1.gcno ]; then
    echo "ERROR: test1 coverage files not created!"
    exit 1
fi

if [ ! -f test2.gcda ] || [ ! -f test2.gcno ]; then
    echo "ERROR: test2 coverage files not created!"
    exit 1
fi

echo "Coverage files created successfully."
echo ""

# Test 1: -h flag (case 'h')
echo "=== Test 1: Testing -h flag (should show usage) ==="
gcov-dump -h 2>&1 | head -5
echo ""

# Test 2: --help flag (should also trigger -h case)
echo "=== Test 2: Testing --help flag ==="
gcov-dump --help 2>&1 | head -5
echo ""

# Test 3: -v flag (case 'v')
echo "=== Test 3: Testing -v flag (should show version) ==="
gcov-dump -v 2>&1
echo ""

# Test 4: --version flag (should also trigger -v case)
echo "=== Test 4: Testing --version flag ==="
gcov-dump --version 2>&1
echo ""

# Test 5: -l flag (case 'l')
echo "=== Test 5: Testing -l flag (dump contents) ==="
gcov-dump -l test1.gcda 2>&1 | head -10
echo ""

# Test 6: -p flag (case 'p')
echo "=== Test 6: Testing -p flag (dump positions) ==="
gcov-dump -p test1.gcda 2>&1 | head -10
echo ""

# Test 7: -r flag (case 'r')
echo "=== Test 7: Testing -r flag (dump raw) ==="
gcov-dump -r test1.gcda 2>&1 | head -10
echo ""

# Test 8: -s flag (case 's')
echo "=== Test 8: Testing -s flag (dump stable) ==="
gcov-dump -s test1.gcda 2>&1 | head -10
echo ""

# Test 9: Combination of all flags
echo "=== Test 9: Testing combination of all flags (-l -p -r -s) ==="
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -10
echo ""

# Test 10: Multiple input files with flags
echo "=== Test 10: Testing multiple input files with -l flag ==="
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -5
echo ""

# Test 11: .gcno file without flags
echo "=== Test 11: Testing .gcno file without flags ==="
gcov-dump test1.gcno 2>&1 | head -5
echo ""

# Test 12: .gcno file with -l flag
echo "=== Test 12: Testing .gcno file with -l flag ==="
gcov-dump -l test1.gcno 2>&1 | head -5
echo ""

# Test 13: Multiple files with different types
echo "=== Test 13: Testing multiple files (gcda and gcno) with -l flag ==="
gcov-dump -l test1.gcda test1.gcno 2>&1 | head -5
echo ""

# Test 14: Invalid flag (should trigger default case)
echo "=== Test 14: Testing invalid flag -x (should show error) ==="
gcov-dump -x test1.gcda 2>&1 || true  # Continue even if command fails
echo ""

# Test 15: Another invalid flag
echo "=== Test 15: Testing invalid flag -z ==="
gcov-dump -z test1.gcda 2>&1 || true
echo ""

# Test 16: Valid flag with invalid file
echo "=== Test 16: Testing -l flag with non-existent file ==="
gcov-dump -l nonexistent.gcda 2>&1 || true
echo ""

# Test 17: No arguments (should show usage or error)
echo "=== Test 17: Testing no arguments ==="
gcov-dump 2>&1 | head -5 || true
echo ""

# Test 18: Flag with no file argument
echo "=== Test 18: Testing -l flag with no file ==="
gcov-dump -l 2>&1 | head -5 || true
echo ""

# Test 19: Long form invalid flag
echo "=== Test 19: Testing invalid long flag ==="
gcov-dump --invalid-flag test1.gcda 2>&1 | head -5 || true
echo ""

# Test 20: Mixed valid and invalid flags
echo "=== Test 20: Testing mixed valid (-l) and invalid (-x) flags ==="
gcov-dump -l -x test1.gcda 2>&1 | head -5 || true
echo ""

echo "All tests executed successfully!"
echo "The following switch cases were tested:"
echo "  -h / --help (print_usage)"
echo "  -v / --version (print_version)"
echo "  -l (flag_dump_contents = 1)"
echo "  -p (flag_dump_positions = 1)"
echo "  -r (flag_dump_raw = 1)"
echo "  -s (flag_dump_stable = 1)"
echo "  default case (unknown flag)"
```

This test script systematically exercises all the uncovered lines:

1. **Creates test coverage files**: Compiles and runs simple C programs to generate `.gcda` and `.gcno` files
2. **Tests each individual flag**: `-h`, `-v`, `-l`, `-p`, `-r`, `-s`
3. **Tests flag combinations**: `-l -p -r -s` together
4. **Tests different input scenarios**: Single `.gcda`, multiple `.gcda`, `.gcno` files, mixed file types
5. **Triggers the default case**: Uses invalid flags `-x` and `-z` to reach the `default:` case
6. **Tests edge cases**: No arguments, flags without files, invalid files

To run this test, you need to:
1. Ensure `gcov-dump` is in your PATH or provide the full path
2. Have GCC installed with coverage support
3. Make the script executable: `chmod +x test_gcov_dump.sh`
4. Run it: `./test_gcov_dump.sh`

The script will create a temporary directory, generate the necessary coverage files, run all the test cases, and clean up automatically. Each test case exercises specific parts of the switch statement in the uncovered lines.
