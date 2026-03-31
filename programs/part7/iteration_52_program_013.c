Looking at the uncovered lines in `gcov-dump.cc`, I need to create a comprehensive test that exercises all the command-line option parsing logic. Here's a shell script that will generate the necessary GCOV data files and test all the uncovered switch cases:

```bash
#!/bin/bash
set -e

# Create a temporary directory for test artifacts
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    cd /
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

# Create a simple C test program
cat > test1.c << 'EOF'
#include <stdio.h>

int main() {
    printf("Test program 1\n");
    return 0;
}
EOF

cat > test2.c << 'EOF'
#include <stdio.h>

int helper() {
    return 42;
}

int main() {
    printf("Test program 2: %d\n", helper());
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

# Verify that GCOV files were created
if [ ! -f test1.gcda ] || [ ! -f test1.gcno ]; then
    echo "ERROR: GCOV files not created for test1"
    exit 1
fi

if [ ! -f test2.gcda ] || [ ! -f test2.gcno ]; then
    echo "ERROR: GCOV files not created for test2"
    exit 1
fi

echo "GCOV files created successfully"
echo ""

# Test 1: Help flag (-h)
echo "=== Test 1: Help flag (-h) ==="
gcov-dump -h 2>&1 | head -5
echo "Exit code: $?"
echo ""

# Test 2: Help flag (--help)
echo "=== Test 2: Help flag (--help) ==="
gcov-dump --help 2>&1 | head -5
echo "Exit code: $?"
echo ""

# Test 3: Version flag (-v)
echo "=== Test 3: Version flag (-v) ==="
gcov-dump -v 2>&1
echo "Exit code: $?"
echo ""

# Test 4: Version flag (--version)
echo "=== Test 4: Version flag (--version) ==="
gcov-dump --version 2>&1
echo "Exit code: $?"
echo ""

# Test 5: Long dump flag (-l)
echo "=== Test 5: Long dump flag (-l) ==="
gcov-dump -l test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 6: Positions dump flag (-p)
echo "=== Test 6: Positions dump flag (-p) ==="
gcov-dump -p test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 7: Raw dump flag (-r)
echo "=== Test 7: Raw dump flag (-r) ==="
gcov-dump -r test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 8: Stable dump flag (-s)
echo "=== Test 8: Stable dump flag (-s) ==="
gcov-dump -s test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 9: Multiple flags combined
echo "=== Test 9: Multiple flags combined (-l -p -r -s) ==="
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 10: Invalid flag (triggers default case)
echo "=== Test 10: Invalid flag (-x) ==="
gcov-dump -x test1.gcda 2>&1
echo "Exit code: $?"
echo ""

# Test 11: No flags (default behavior)
echo "=== Test 11: No flags (default behavior) ==="
gcov-dump test1.gcno 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 12: Multiple input files with flag
echo "=== Test 12: Multiple input files with flag (-l) ==="
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 13: .gcno file with flag
echo "=== Test 13: .gcno file with flag (-p) ==="
gcov-dump -p test1.gcno 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 14: Mixed .gcda and .gcno files
echo "=== Test 14: Mixed .gcda and .gcno files ==="
gcov-dump -r test1.gcda test1.gcno 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 15: All flags with multiple files
echo "=== Test 15: All flags with multiple files ==="
gcov-dump -l -p -r -s test1.gcda test2.gcda test1.gcno 2>&1 | head -10
echo "Exit code: $?"
echo ""

echo "All tests completed successfully!"
```

This test script comprehensively covers all the uncovered lines:

1. **Lines 111-112 (`case 'h'`)**: Covered by tests 1 and 2 with `-h` and `--help`
2. **Lines 113-115 (`case 'v'`)**: Covered by tests 3 and 4 with `-v` and `--version`
3. **Lines 116-118 (`case 'l'`)**: Covered by tests 5, 9, 12, 14, and 15
4. **Lines 119-121 (`case 'p'`)**: Covered by tests 6, 9, 13, 14, and 15
5. **Lines 122-124 (`case 'r'`)**: Covered by tests 7, 9, 14, and 15
6. **Lines 125-127 (`case 's'`)**: Covered by tests 8, 9, 14, and 15
7. **Lines 128-130 (`default` case)**: Covered by test 10 with invalid flag `-x`

The script also tests various combinations and edge cases:
- Single flags with single files
- Multiple flags combined
- Multiple input files
- Different file types (.gcda and .gcno)
- Mixed file types in single invocation
- Default behavior with no flags

To run this test, you need to:
1. Ensure `gcov-dump` is in your PATH or provide the full path
2. Have GCC installed with coverage support
3. Make the script executable: `chmod +x test_gcov_dump.sh`
4. Run it: `./test_gcov_dump.sh`

The script creates a temporary directory, generates test programs, compiles them with coverage, runs them to produce .gcda files, and then exercises all the command-line options of `gcov-dump`. Each test shows a snippet of output and the exit code for verification.
