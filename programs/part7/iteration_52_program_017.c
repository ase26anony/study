Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that exercises the command-line argument parsing logic. Here's a comprehensive shell script that will cover all the specified cases:

```bash
#!/bin/bash
# Test script for gcov-dump command-line argument parsing
# This script covers lines 111-130 in gcov-dump.cc

set -e  # Exit on any error

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"

# Create a simple C source file for testing
cat > test1.c << 'EOF'
#include <stdio.h>

int main() {
    int x = 5;
    int y = 10;
    int sum = x + y;
    
    if (sum > 10) {
        printf("Sum is greater than 10\n");
    } else {
        printf("Sum is 10 or less\n");
    }
    
    for (int i = 0; i < 3; i++) {
        printf("Iteration %d\n", i);
    }
    
    return 0;
}
EOF

# Create another test file for multiple file testing
cat > test2.c << 'EOF'
#include <stdio.h>

void helper_function() {
    printf("Helper function called\n");
}

int main() {
    printf("Test program 2\n");
    helper_function();
    return 0;
}
EOF

# Compile test programs with coverage instrumentation
echo "Compiling test programs with coverage..."
gcc -fprofile-arcs -ftest-coverage -O0 test1.c -o test1_prog
gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test2_prog

# Run the programs to generate .gcda files
echo "Running test programs to generate coverage data..."
./test1_prog > /dev/null
./test2_prog > /dev/null

# Verify that coverage files were created
if [ ! -f test1.gcda ] || [ ! -f test1.gcno ]; then
    echo "ERROR: Coverage files not created for test1"
    exit 1
fi

if [ ! -f test2.gcda ] || [ ! -f test2.gcno ]; then
    echo "ERROR: Coverage files not created for test2"
    exit 1
fi

echo "Coverage files created successfully"
echo ""

# Test 1: Help flag (-h)
echo "=== Test 1: Help flag (-h) ==="
gcov-dump -h 2>&1 | head -5
echo ""

# Test 2: Help flag (--help)
echo "=== Test 2: Help flag (--help) ==="
gcov-dump --help 2>&1 | head -5
echo ""

# Test 3: Version flag (-v)
echo "=== Test 3: Version flag (-v) ==="
gcov-dump -v
echo ""

# Test 4: Version flag (--version)
echo "=== Test 4: Version flag (--version) ==="
gcov-dump --version
echo ""

# Test 5: Dump contents flag (-l)
echo "=== Test 5: Dump contents flag (-l) ==="
gcov-dump -l test1.gcda 2>&1 | head -10
echo ""

# Test 6: Dump positions flag (-p)
echo "=== Test 6: Dump positions flag (-p) ==="
gcov-dump -p test1.gcda 2>&1 | head -10
echo ""

# Test 7: Dump raw flag (-r)
echo "=== Test 7: Dump raw flag (-r) ==="
gcov-dump -r test1.gcda 2>&1 | head -10
echo ""

# Test 8: Dump stable flag (-s)
echo "=== Test 8: Dump stable flag (-s) ==="
gcov-dump -s test1.gcda 2>&1 | head -10
echo ""

# Test 9: Multiple flags combined
echo "=== Test 9: Multiple flags combined (-l -p -r -s) ==="
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -15
echo ""

# Test 10: Invalid flag (triggers default case)
echo "=== Test 10: Invalid flag (-x) ==="
gcov-dump -x test1.gcda 2>&1 || true  # Continue even if command fails
echo ""

# Test 11: No flags (default behavior)
echo "=== Test 11: No flags (default behavior) ==="
gcov-dump test1.gcno 2>&1 | head -10
echo ""

# Test 12: Multiple input files with flag
echo "=== Test 12: Multiple input files with flag (-l) ==="
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -15
echo ""

# Test 13: .gcno file with flag
echo "=== Test 13: .gcno file with flag (-p) ==="
gcov-dump -p test1.gcno 2>&1 | head -10
echo ""

# Test 14: Multiple flags in different order
echo "=== Test 14: Multiple flags in different order (-s -r -p) ==="
gcov-dump -s -r -p test1.gcda 2>&1 | head -15
echo ""

# Test 15: Combined short options (if supported)
echo "=== Test 15: Combined short options (-lpr) ==="
gcov-dump -lpr test1.gcda 2>&1 | head -15 || true
echo ""

# Test 16: Flag with .gcda and .gcno together
echo "=== Test 16: Flag with both .gcda and .gcno ==="
gcov-dump -l test1.gcda test1.gcno 2>&1 | head -15
echo ""

# Test 17: Long flag with equals (if supported)
echo "=== Test 17: Long form flags (if supported) ==="
gcov-dump --long 2>&1 | head -5 || true
echo ""

# Clean up
echo "Cleaning up test directory..."
cd /
rm -rf "$TEST_DIR"

echo ""
echo "All tests completed successfully!"
echo "Covered command-line options:"
echo "  -h / --help      (print_usage)"
echo "  -v / --version   (print_version)"
echo "  -l               (flag_dump_contents)"
echo "  -p               (flag_dump_positions)"
echo "  -r               (flag_dump_raw)"
echo "  -s               (flag_dump_stable)"
echo "  default case     (invalid flag)"
echo ""
echo "The uncovered lines in gcov-dump.cc (111-130) should now be covered."
```

This test script comprehensively covers all the specified requirements:

1. **Generates valid GCOV data files**: Compiles two test programs with `-fprofile-arcs -ftest-coverage` and runs them to produce `.gcda` files.

2. **Exercises all flag cases**:
   - `-h` and `--help` (calls `print_usage()`)
   - `-v` and `--version` (calls `print_version()`)
   - `-l` (sets `flag_dump_contents = 1`)
   - `-p` (sets `flag_dump_positions = 1`)
   - `-r` (sets `flag_dump_raw = 1`)
   - `-s` (sets `flag_dump_stable = 1`)
   - Invalid flag `-x` (triggers `default` case)

3. **Tests flag combinations**: Includes tests with multiple flags combined (`-l -p -r -s`) and in different orders.

4. **Tests various file inputs**:
   - Single `.gcda` file
   - Single `.gcno` file
   - Multiple `.gcda` files
   - Both `.gcda` and `.gcno` files together

5. **Handles edge cases**: Tests combined short options (`-lpr`), long flags, and ensures the script continues even when commands fail (like with invalid flags).

The script uses `head` to limit output for readability, but you can remove those pipes if you need to see full output. Each test section is clearly labeled, making it easy to verify that all target lines are being exercised.

To run this test, save it as `test_gcov_dump.sh`, make it executable (`chmod +x test_gcov_dump.sh`), and execute it. The script will create a temporary directory, generate all necessary test files, run `gcov-dump` with all the specified flag combinations, and clean up after itself.
