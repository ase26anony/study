Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that exercises all the command-line flag handling logic. Here's a comprehensive shell script that will cover all the uncovered cases:

```bash
#!/bin/bash
# Test script for gcov-dump command-line argument parsing
# This script covers the switch statement cases for flags: h, v, l, p, r, s, and default

set -e  # Exit on any error

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"

# Create a simple C test program
cat > test1.c << 'EOF'
#include <stdio.h>

int main() {
    int x = 5;
    int y = 10;
    int sum = x + y;
    printf("Sum: %d\n", sum);
    return 0;
}
EOF

# Create another test program for multiple file testing
cat > test2.c << 'EOF'
#include <stdio.h>

void helper() {
    printf("Helper function\n");
}

int main() {
    helper();
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
    echo "ERROR: Coverage files not created for test1"
    exit 1
fi

if [ ! -f test2.gcda ] || [ ! -f test2.gcno ]; then
    echo "ERROR: Coverage files not created for test2"
    exit 1
fi

echo "Coverage files created successfully"

# Test 1: Help flag (-h) - covers case 'h'
echo "=== Test 1: Help flag (-h) ==="
gcov-dump -h 2>&1 | head -5

# Test 2: Help flag (--help) - also covers case 'h'
echo -e "\n=== Test 2: Help flag (--help) ==="
gcov-dump --help 2>&1 | head -5

# Test 3: Version flag (-v) - covers case 'v'
echo -e "\n=== Test 3: Version flag (-v) ==="
gcov-dump -v 2>&1

# Test 4: Version flag (--version) - also covers case 'v'
echo -e "\n=== Test 4: Version flag (--version) ==="
gcov-dump --version 2>&1

# Test 5: Dump contents flag (-l) - covers case 'l'
echo -e "\n=== Test 5: Dump contents flag (-l) ==="
gcov-dump -l test1.gcda 2>&1 | head -10

# Test 6: Dump positions flag (-p) - covers case 'p'
echo -e "\n=== Test 6: Dump positions flag (-p) ==="
gcov-dump -p test1.gcda 2>&1 | head -10

# Test 7: Dump raw flag (-r) - covers case 'r'
echo -e "\n=== Test 7: Dump raw flag (-r) ==="
gcov-dump -r test1.gcda 2>&1 | head -10

# Test 8: Dump stable flag (-s) - covers case 's'
echo -e "\n=== Test 8: Dump stable flag (-s) ==="
gcov-dump -s test1.gcda 2>&1 | head -10

# Test 9: Multiple flags combined - covers multiple cases sequentially
echo -e "\n=== Test 9: Multiple flags combined (-l -p -r -s) ==="
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -10

# Test 10: Invalid flag (-x) - covers default case
echo -e "\n=== Test 10: Invalid flag (-x) - should trigger default case ==="
gcov-dump -x test1.gcda 2>&1 || true  # Continue even if command fails

# Test 11: No flags (default behavior) on .gcno file
echo -e "\n=== Test 11: No flags on .gcno file ==="
gcov-dump test1.gcno 2>&1 | head -10

# Test 12: Single flag on .gcno file
echo -e "\n=== Test 12: -l flag on .gcno file ==="
gcov-dump -l test1.gcno 2>&1 | head -10

# Test 13: Multiple input files with flag
echo -e "\n=== Test 13: Multiple input files with -l flag ==="
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -10

# Test 14: Multiple input files (mixed .gcda and .gcno) with flag
echo -e "\n=== Test 14: Mixed files with -p flag ==="
gcov-dump -p test1.gcda test1.gcno 2>&1 | head -10

# Test 15: Long form flags with equals sign (if supported)
echo -e "\n=== Test 15: Testing various flag combinations ==="
# Test -lprs as combined short options (if supported)
gcov-dump -lprs test1.gcda 2>&1 | head -5

# Test 16: Empty input (should show usage or error)
echo -e "\n=== Test 16: No input files ==="
gcov-dump -l 2>&1 || true

# Test 17: Non-existent file
echo -e "\n=== Test 17: Non-existent file ==="
gcov-dump -l nonexistent.gcda 2>&1 || true

echo -e "\n=== All tests completed ==="

# Cleanup
cd ..
rm -rf "$TEST_DIR"
echo "Cleaned up test directory"
```

This test script comprehensively covers all the uncovered lines:

1. **Lines 111-112 (`case 'h'`)**: Covered by tests 1 and 2 with `-h` and `--help`
2. **Lines 113-114 (`case 'v'`)**: Covered by tests 3 and 4 with `-v` and `--version`
3. **Lines 115-116 (`case 'l'`)**: Covered by tests 5, 12, 13, 14, and 15
4. **Lines 117-118 (`case 'p'`)**: Covered by tests 6, 9, 14, and 15
5. **Lines 119-120 (`case 'r'`)**: Covered by tests 7, 9, and 15
6. **Lines 121-122 (`case 's'`)**: Covered by tests 8, 9, and 15
7. **Lines 123-124 (`default` case)**: Covered by test 10 with invalid flag `-x`

The script also tests various combinations and edge cases:
- Multiple flags in one invocation (test 9)
- Different types of input files (`.gcda` and `.gcno`)
- Multiple input files (tests 13-14)
- Error conditions (tests 16-17)

To run this test, you need to:
1. Ensure `gcov-dump` is in your PATH or provide the full path
2. Have GCC installed with coverage support
3. Make the script executable: `chmod +x test_gcov_dump.sh`
4. Run it: `./test_gcov_dump.sh`

The script creates a temporary directory, generates test coverage files, runs all the test cases, and cleans up afterwards. Each test case exercises different code paths in the switch statement to ensure full coverage of the uncovered lines.
