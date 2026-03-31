Here's a comprehensive test script that exercises all the uncovered lines in `gcov-dump.cc`:

```bash
#!/bin/bash
# Test script for gcov-dump to cover lines 111-130 in gcov-dump.cc

set -e

# Create temporary directory for test artifacts
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"

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
if [ ! -f test1.gcda ] || [ ! -f test1.gcno ]; then
    echo "ERROR: Coverage files not created for test1"
    exit 1
fi

if [ ! -f test2.gcda ] || [ ! -f test2.gcno ]; then
    echo "ERROR: Coverage files not created for test2"
    exit 1
fi

echo "Coverage files created successfully"

# Test 1: Help flag (-h)
echo "=== Test 1: Help flag (-h) ==="
gcov-dump -h 2>&1 | head -5
echo "Exit code: $?"

# Test 2: Help flag (--help)
echo -e "\n=== Test 2: Help flag (--help) ==="
gcov-dump --help 2>&1 | head -5
echo "Exit code: $?"

# Test 3: Version flag (-v)
echo -e "\n=== Test 3: Version flag (-v) ==="
gcov-dump -v 2>&1
echo "Exit code: $?"

# Test 4: Version flag (--version)
echo -e "\n=== Test 4: Version flag (--version) ==="
gcov-dump --version 2>&1
echo "Exit code: $?"

# Test 5: Dump contents flag (-l)
echo -e "\n=== Test 5: Dump contents flag (-l) ==="
gcov-dump -l test1.gcda 2>&1 | head -10
echo "Exit code: $?"

# Test 6: Dump positions flag (-p)
echo -e "\n=== Test 6: Dump positions flag (-p) ==="
gcov-dump -p test1.gcda 2>&1 | head -10
echo "Exit code: $?"

# Test 7: Dump raw flag (-r)
echo -e "\n=== Test 7: Dump raw flag (-r) ==="
gcov-dump -r test1.gcda 2>&1 | head -10
echo "Exit code: $?"

# Test 8: Dump stable flag (-s)
echo -e "\n=== Test 8: Dump stable flag (-s) ==="
gcov-dump -s test1.gcda 2>&1 | head -10
echo "Exit code: $?"

# Test 9: Multiple flags combined
echo -e "\n=== Test 9: Multiple flags combined (-l -p -r -s) ==="
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -15
echo "Exit code: $?"

# Test 10: Invalid flag (triggers default case)
echo -e "\n=== Test 10: Invalid flag (-x) triggers default case ==="
gcov-dump -x test1.gcda 2>&1
echo "Exit code: $?"

# Test 11: No flags, just .gcda file
echo -e "\n=== Test 11: No flags, .gcda file ==="
gcov-dump test1.gcda 2>&1 | head -5
echo "Exit code: $?"

# Test 12: No flags, just .gcno file
echo -e "\n=== Test 12: No flags, .gcno file ==="
gcov-dump test1.gcno 2>&1 | head -5
echo "Exit code: $?"

# Test 13: Flag with .gcno file
echo -e "\n=== Test 13: Flag with .gcno file (-l test.gcno) ==="
gcov-dump -l test1.gcno 2>&1 | head -5
echo "Exit code: $?"

# Test 14: Multiple input files
echo -e "\n=== Test 14: Multiple input files (-l test1.gcda test2.gcda) ==="
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -5
echo "Exit code: $?"

# Test 15: Mixed file types
echo -e "\n=== Test 15: Mixed file types (-l test1.gcda test1.gcno) ==="
gcov-dump -l test1.gcda test1.gcno 2>&1 | head -5
echo "Exit code: $?"

# Test 16: Long form of combined flags
echo -e "\n=== Test 16: Long form flags (--help --version should show help) ==="
gcov-dump --help --version 2>&1 | head -5
echo "Exit code: $?"

# Test 17: Flag with multiple files including invalid
echo -e "\n=== Test 17: Flag with valid and non-existent files ==="
gcov-dump -l test1.gcda nonexistent.gcda 2>&1 | head -5
echo "Exit code: $?"

# Test 18: Empty arguments
echo -e "\n=== Test 18: No arguments (should show usage or error) ==="
gcov-dump 2>&1 | head -5
echo "Exit code: $?"

# Test 19: Another invalid flag combination
echo -e "\n=== Test 19: Another invalid flag (-z) ==="
gcov-dump -z test1.gcda 2>&1
echo "Exit code: $?"

# Test 20: Flag order variation
echo -e "\n=== Test 20: Flag order variation (file before flags) ==="
gcov-dump test1.gcda -l -p 2>&1 | head -5
echo "Exit code: $?"

echo -e "\n=== All tests completed ==="

# Cleanup
cd /
rm -rf "$TEST_DIR"
echo "Cleaned up test directory"
```

This test script comprehensively covers all the cases in the switch statement:

1. **Lines 111-112 (`case 'h'`)**: Covered by tests 1 and 2 (`-h` and `--help`)
2. **Lines 113-115 (`case 'v'`)**: Covered by tests 3 and 4 (`-v` and `--version`)
3. **Lines 116-117 (`case 'l'`)**: Covered by tests 5, 13, 14, 15, and others
4. **Lines 118-119 (`case 'p'`)**: Covered by test 6
5. **Lines 120-121 (`case 'r'`)**: Covered by test 7
6. **Lines 122-123 (`case 's'`)**: Covered by test 8
7. **Lines 124-126 (`default` case)**: Covered by tests 10 and 19 with invalid flags `-x` and `-z`

The script also tests:
- Multiple flag combinations (test 9)
- Different file types (`.gcda` and `.gcno`)
- Multiple input files
- Edge cases like no arguments, non-existent files
- Flag ordering variations

To run this test, ensure:
1. The GCC toolchain is built with coverage instrumentation
2. `gcov-dump` is in your PATH or specify the full path
3. You have write permissions in `/tmp`

The script creates temporary test files, compiles them with coverage flags, runs them to generate `.gcda` files, then exercises `gcov-dump` with all the flag combinations needed to cover the target lines.
