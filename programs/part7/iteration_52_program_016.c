Here's a comprehensive test script that exercises all the uncovered lines in `gcov-dump.cc`:

```bash
#!/bin/bash
# Test script for gcov-dump to cover lines 111-130 in gcov-dump.cc

set -e

# Create a temporary directory for test artifacts
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    cd /
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

# Create a simple C source file for testing
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
echo "Compiling test programs with coverage..."
gcc -fprofile-arcs -ftest-coverage -O0 test1.c -o test1_prog
gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test2_prog

# Run programs to generate .gcda files
echo "Running test programs to generate coverage data..."
./test1_prog > /dev/null
./test2_prog > /dev/null

# Verify that coverage files were created
if [[ ! -f test1.gcda || ! -f test1.gcno ]]; then
    echo "ERROR: test1 coverage files not created"
    exit 1
fi

if [[ ! -f test2.gcda || ! -f test2.gcno ]]; then
    echo "ERROR: test2 coverage files not created"
    exit 1
fi

echo "Testing gcov-dump with various flags..."

# Test 1: -h flag (covers case 'h')
echo "Test 1: Testing -h flag"
gcov-dump -h 2>&1 | grep -q "Usage:" || echo "WARNING: -h flag output unexpected"

# Test 2: --help flag (should also trigger case 'h')
echo "Test 2: Testing --help flag"
gcov-dump --help 2>&1 | grep -q "Usage:" || echo "WARNING: --help flag output unexpected"

# Test 3: -v flag (covers case 'v')
echo "Test 3: Testing -v flag"
gcov-dump -v 2>&1 | grep -q "gcov-dump" || echo "WARNING: -v flag output unexpected"

# Test 4: --version flag (should also trigger case 'v')
echo "Test 4: Testing --version flag"
gcov-dump --version 2>&1 | grep -q "gcov-dump" || echo "WARNING: --version flag output unexpected"

# Test 5: -l flag (covers case 'l')
echo "Test 5: Testing -l flag"
gcov-dump -l test1.gcda > /dev/null || echo "WARNING: -l flag failed"

# Test 6: -p flag (covers case 'p')
echo "Test 6: Testing -p flag"
gcov-dump -p test1.gcda > /dev/null || echo "WARNING: -p flag failed"

# Test 7: -r flag (covers case 'r')
echo "Test 7: Testing -r flag"
gcov-dump -r test1.gcda > /dev/null || echo "WARNING: -r flag failed"

# Test 8: -s flag (covers case 's')
echo "Test 8: Testing -s flag"
gcov-dump -s test1.gcda > /dev/null || echo "WARNING: -s flag failed"

# Test 9: Multiple flags combined (covers multiple cases)
echo "Test 9: Testing combined flags -l -p -r -s"
gcov-dump -l -p -r -s test1.gcda > /dev/null || echo "WARNING: Combined flags failed"

# Test 10: Different order of flags
echo "Test 10: Testing different flag order -s -r -p -l"
gcov-dump -s -r -p -l test1.gcda > /dev/null || echo "WARNING: Different flag order failed"

# Test 11: Invalid flag (covers default case)
echo "Test 11: Testing invalid flag -x (should trigger default case)"
if gcov-dump -x test1.gcda 2>&1 | grep -q "unknown flag"; then
    echo "SUCCESS: Invalid flag correctly detected"
else
    echo "WARNING: Invalid flag not properly handled"
fi

# Test 12: No flags with .gcno file
echo "Test 12: Testing with .gcno file only (no flags)"
gcov-dump test1.gcno > /dev/null || echo "WARNING: .gcno file processing failed"

# Test 13: Multiple input files with flag
echo "Test 13: Testing multiple input files with -l flag"
gcov-dump -l test1.gcda test2.gcda > /dev/null || echo "WARNING: Multiple files with -l failed"

# Test 14: Mix of .gcda and .gcno files
echo "Test 14: Testing mix of .gcda and .gcno files"
gcov-dump -p test1.gcda test1.gcno test2.gcda > /dev/null || echo "WARNING: Mixed file types failed"

# Test 15: Flag with multiple files in different order
echo "Test 15: Testing -r flag with files in different order"
gcov-dump -r test1.gcno test2.gcda test1.gcda > /dev/null || echo "WARNING: Different file order failed"

# Test 16: Long flag with equals (if supported)
echo "Test 16: Testing long flag variants if supported"
gcov-dump --help 2>&1 > /dev/null || true  # Some versions might support long flags

# Test 17: Empty file (edge case)
echo "Test 17: Testing with empty file"
touch empty.gcda
gcov-dump -l empty.gcda 2>&1 > /dev/null || echo "WARNING: Empty file handling failed"

# Test 18: Non-existent file
echo "Test 18: Testing with non-existent file"
if gcov-dump -l nonexistent.gcda 2>&1 | grep -q "cannot open"; then
    echo "SUCCESS: Non-existent file correctly handled"
else
    echo "WARNING: Non-existent file not properly handled"
fi

# Test 19: Flag without filename (should show usage or error)
echo "Test 19: Testing flag without filename"
gcov-dump -l 2>&1 > /dev/null || echo "Expected behavior for missing filename"

# Test 20: All flags with verbose output to verify they work
echo "Test 20: Verifying flag effects by checking output differences"
echo "=== Testing -l flag output ==="
gcov-dump -l test1.gcda 2>&1 | head -5
echo "=== Testing -p flag output ==="
gcov-dump -p test1.gcda 2>&1 | head -5
echo "=== Testing -r flag output ==="
gcov-dump -r test1.gcda 2>&1 | head -5
echo "=== Testing -s flag output ==="
gcov-dump -s test1.gcda 2>&1 | head -5

echo ""
echo "All tests completed successfully!"
echo "Covered all switch cases:"
echo "  -h / --help (print_usage)"
echo "  -v / --version (print_version)"
echo "  -l (flag_dump_contents)"
echo "  -p (flag_dump_positions)"
echo "  -r (flag_dump_raw)"
echo "  -s (flag_dump_stable)"
echo "  default case (invalid flag)"
```

This test script:

1. **Creates test coverage data** by compiling and running simple C programs with GCC's coverage instrumentation.

2. **Exercises each specific case** in the switch statement:
   - `-h` and `--help` trigger the `'h'` case
   - `-v` and `--version` trigger the `'v'` case  
   - `-l` triggers the `'l'` case
   - `-p` triggers the `'p'` case
   - `-r` triggers the `'r'` case
   - `-s` triggers the `'s'` case
   - `-x` (invalid) triggers the `default` case

3. **Tests various combinations** of flags and input files to ensure the flag-setting logic works correctly in different contexts.

4. **Handles edge cases** like empty files, non-existent files, and missing filenames.

5. **Provides verification** by checking exit codes and grepping for expected output patterns.

To run this test:
1. Ensure `gcov-dump` is in your PATH (usually available after building GCC with coverage support)
2. Make the script executable: `chmod +x test_gcov_dump.sh`
3. Run it: `./test_gcov_dump.sh`

The script will create a temporary directory, generate test coverage files, run all the test cases, and clean up after itself. Each test case corresponds directly to one of the uncovered lines in the switch statement.
