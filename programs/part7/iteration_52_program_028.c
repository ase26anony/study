Looking at the uncovered lines in `gcov-dump.cc`, I need to create a comprehensive test that exercises all the command-line flag parsing logic. Here's a shell script that will cover all the uncovered lines:

```bash
#!/bin/bash
# Test script for gcov-dump command-line argument parsing
# This script covers lines 111-130 in gcov-dump.cc

set -e  # Exit on any error

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

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
echo "Compiling test programs with coverage..."
gcc -fprofile-arcs -ftest-coverage -O0 test1.c -o test1_prog
gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test2_prog

# Run the programs to generate .gcda files
echo "Running test programs to generate coverage data..."
./test1_prog > /dev/null
./test2_prog > /dev/null

# Verify that coverage files were created
if [ ! -f test1.gcda ] || [ ! -f test1.gcno ]; then
    echo "ERROR: test1 coverage files not created"
    exit 1
fi

if [ ! -f test2.gcda ] || [ ! -f test2.gcno ]; then
    echo "ERROR: test2 coverage files not created"
    exit 1
fi

echo "Coverage files created successfully"

# Test 1: -h flag (covers case 'h' and print_usage())
echo -e "\n=== Test 1: -h flag (help) ==="
gcov-dump -h 2>&1 | head -5

# Test 2: --help flag (should also trigger help)
echo -e "\n=== Test 2: --help flag ==="
gcov-dump --help 2>&1 | head -5

# Test 3: -v flag (covers case 'v' and print_version())
echo -e "\n=== Test 3: -v flag (version) ==="
gcov-dump -v 2>&1

# Test 4: --version flag (should also trigger version)
echo -e "\n=== Test 4: --version flag ==="
gcov-dump --version 2>&1

# Test 5: -l flag (covers case 'l', sets flag_dump_contents = 1)
echo -e "\n=== Test 5: -l flag (dump contents) ==="
gcov-dump -l test1.gcda 2>&1 | head -10

# Test 6: -p flag (covers case 'p', sets flag_dump_positions = 1)
echo -e "\n=== Test 6: -p flag (dump positions) ==="
gcov-dump -p test1.gcda 2>&1 | head -10

# Test 7: -r flag (covers case 'r', sets flag_dump_raw = 1)
echo -e "\n=== Test 7: -r flag (dump raw) ==="
gcov-dump -r test1.gcda 2>&1 | head -10

# Test 8: -s flag (covers case 's', sets flag_dump_stable = 1)
echo -e "\n=== Test 8: -s flag (dump stable) ==="
gcov-dump -s test1.gcda 2>&1 | head -10

# Test 9: Combination of all flags (exercises multiple cases sequentially)
echo -e "\n=== Test 9: Combination of all flags (-l -p -r -s) ==="
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -10

# Test 10: Invalid flag (covers default case, prints "unknown flag")
echo -e "\n=== Test 10: Invalid flag (-x) ==="
gcov-dump -x test1.gcda 2>&1 || true  # Continue even if command fails

# Test 11: No flags with .gcno file (tests default behavior)
echo -e "\n=== Test 11: No flags with .gcno file ==="
gcov-dump test1.gcno 2>&1 | head -10

# Test 12: Multiple input files with flag
echo -e "\n=== Test 12: Multiple input files with -l flag ==="
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -10

# Test 13: Multiple input files including .gcno
echo -e "\n=== Test 13: Multiple files (.gcda and .gcno) with -p flag ==="
gcov-dump -p test1.gcda test1.gcno 2>&1 | head -10

# Test 14: Long form flags (if supported)
echo -e "\n=== Test 14: Testing long form flags ==="
# Note: gcov-dump might not support long forms, but we try common ones
gcov-dump --help 2>&1 | head -5 || true
gcov-dump --version 2>&1 || true

# Test 15: Flag with no argument (should show error or usage)
echo -e "\n=== Test 15: Flag with no filename ==="
gcov-dump -l 2>&1 || true

# Test 16: Multiple flags combined in single argument
echo -e "\n=== Test 16: Combined flags (-lprs) ==="
gcov-dump -lprs test1.gcda 2>&1 | head -10 || true

# Test 17: Different order of flags
echo -e "\n=== Test 17: Different flag order (-s -r -p -l) ==="
gcov-dump -s -r -p -l test1.gcda 2>&1 | head -10

# Test 18: Flag with non-existent file
echo -e "\n=== Test 18: Flag with non-existent file ==="
gcov-dump -l nonexistent.gcda 2>&1 || true

# Test 19: Empty .gcda file (edge case)
echo -e "\n=== Test 19: Empty file ==="
touch empty.gcda
gcov-dump -l empty.gcda 2>&1 || true

# Test 20: Corrupted .gcda file (edge case)
echo -e "\n=== Test 20: Corrupted file ==="
echo "not a valid gcda file" > corrupted.gcda
gcov-dump -l corrupted.gcda 2>&1 || true

echo -e "\n=== All tests completed ==="

# Cleanup
cd ..
rm -rf "$TEST_DIR"
echo "Cleaned up test directory"
```

This test script comprehensively covers all the uncovered lines:

1. **Lines 111-112 (`case 'h'` and `print_usage()`)**:
   - Covered by tests 1 and 2 (`-h` and `--help` flags)

2. **Lines 113-114 (`case 'v'` and `print_version()`)**:
   - Covered by tests 3 and 4 (`-v` and `--version` flags)

3. **Line 116 (`case 'l'` and `flag_dump_contents = 1`)**:
   - Covered by tests 5, 9, 12, 15, 16, 17

4. **Line 118 (`case 'p'` and `flag_dump_positions = 1`)**:
   - Covered by tests 6, 9, 13, 16, 17

5. **Line 120 (`case 'r'` and `flag_dump_raw = 1`)**:
   - Covered by tests 7, 9, 16, 17

6. **Line 122 (`case 's'` and `flag_dump_stable = 1`)**:
   - Covered by tests 8, 9, 16, 17

7. **Lines 124-125 (`default` case and error message)**:
   - Covered by test 10 (invalid `-x` flag)

The script also tests various edge cases and combinations:
- Multiple input files (tests 12, 13)
- Different file types (`.gcda` and `.gcno`) - test 11
- Invalid files (tests 18, 19, 20)
- Flag combinations in different orders (tests 9, 16, 17)
- No flags (test 11)

To run this test, you need to:
1. Ensure `gcov-dump` is in your PATH or provide the full path
2. Have GCC installed with coverage support
3. Make the script executable: `chmod +x test_gcov_dump.sh`
4. Run it: `./test_gcov_dump.sh`

The script creates a temporary directory, generates test coverage files, runs all the test cases, and cleans up after itself. Each test case exercises different parts of the command-line parsing logic in `gcov-dump.cc`.
