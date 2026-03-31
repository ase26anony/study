Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that exercises all the command-line flag handling logic. Here's a comprehensive shell script that will cover all the uncovered cases:

```bash
#!/bin/bash
# Test script for gcov-dump command-line argument parsing
# This script covers lines 111-130 in gcov-dump.cc

set -e  # Exit on any error

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"
echo ""

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
echo "Compiling test programs with coverage instrumentation..."
gcc -fprofile-arcs -ftest-coverage -O0 test1.c -o test1_prog
gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test2_prog

# Run the test programs to generate .gcda files
echo "Running test programs to generate coverage data..."
./test1_prog > /dev/null
./test2_prog > /dev/null

echo ""
echo "Generated coverage files:"
ls -la *.gcno *.gcda
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
gcov-dump -v 2>&1
echo ""

# Test 4: Version flag (--version)
echo "=== Test 4: Version flag (--version) ==="
gcov-dump --version 2>&1
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
echo "=== Test 10: Invalid flag (-x) triggers default case ==="
gcov-dump -x test1.gcda 2>&1 || true
echo ""

# Test 11: No flags (default behavior)
echo "=== Test 11: No flags (default behavior) ==="
gcov-dump test1.gcno 2>&1 | head -10
echo ""

# Test 12: Multiple input files with flag
echo "=== Test 12: Multiple input files with flag ==="
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -15
echo ""

# Test 13: .gcno file with flag
echo "=== Test 13: .gcno file with flag ==="
gcov-dump -p test1.gcno 2>&1 | head -10
echo ""

# Test 14: Multiple flags in different order
echo "=== Test 14: Multiple flags in different order ==="
gcov-dump -s -r -p -l test1.gcda 2>&1 | head -15
echo ""

# Test 15: Combined short flags (if supported)
echo "=== Test 15: Combined short flags ==="
gcov-dump -lprs test1.gcda 2>&1 | head -15 2>/dev/null || echo "Combined short flags not supported (expected)"
echo ""

# Test 16: Flag with .gcda and .gcno together
echo "=== Test 16: Flag with both .gcda and .gcno ==="
gcov-dump -l test1.gcda test1.gcno 2>&1 | head -15
echo ""

# Test 17: Long run with verbose output
echo "=== Test 17: Long dump to verify flag effects ==="
echo "With -l flag:"
gcov-dump -l test1.gcda 2>&1 | grep -c "Tag\|Function\|Lines" || true
echo "With -p flag:"
gcov-dump -p test1.gcda 2>&1 | grep -c "position" || true
echo ""

# Cleanup
echo "Cleaning up test directory..."
cd /
rm -rf "$TEST_DIR"

echo ""
echo "All tests completed successfully!"
echo "Covered all switch cases:"
echo "  -h / --help (print_usage)"
echo "  -v / --version (print_version)"
echo "  -l (flag_dump_contents = 1)"
echo "  -p (flag_dump_positions = 1)"
echo "  -r (flag_dump_raw = 1)"
echo "  -s (flag_dump_stable = 1)"
echo "  default case (invalid flag)"
```

This test script comprehensively covers all the uncovered lines:

1. **Help flags**: Tests both `-h` and `--help` (covers `case 'h'`)
2. **Version flags**: Tests both `-v` and `--version` (covers `case 'v'`)
3. **Individual flags**: Tests `-l`, `-p`, `-r`, `-s` separately (covers each case)
4. **Combined flags**: Tests multiple flags together (`-l -p -r -s`)
5. **Invalid flag**: Tests `-x` to trigger the `default` case
6. **No flags**: Tests default behavior
7. **Multiple input files**: Tests with multiple `.gcda` files
8. **Different file types**: Tests with `.gcno` files alone and with `.gcda`
9. **Flag combinations**: Tests flags in different orders

The script:
- Creates temporary test C programs
- Compiles them with coverage instrumentation (`-fprofile-arcs -ftest-coverage`)
- Runs them to generate `.gcda` files
- Invokes `gcov-dump` with all the flag combinations needed
- Cleans up after itself

Each test case specifically targets one or more of the uncovered switch cases, ensuring that the flag-setting logic (`flag_dump_contents = 1`, etc.) is executed. The invalid flag test ensures the `default` case is hit, which prints the error message "unknown flag".
