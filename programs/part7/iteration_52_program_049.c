Looking at the uncovered lines in `gcov-dump.cc`, I need to create a comprehensive test that exercises all the command-line flag handling logic. Here's a shell script that will generate the necessary GCOV data files and test all the uncovered switch cases:

```bash
#!/bin/bash
# Test script for gcov-dump command-line argument parsing
# This script tests the uncovered lines in gcov-dump.cc (lines 111-130)

set -e  # Exit on any error

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test-XXXXXX)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"
echo "Testing gcov-dump command-line argument parsing..."

# Create a simple C test program
cat > test1.c << 'EOF'
#include <stdio.h>

int main() {
    int x = 5;
    int y = 10;
    int sum = x + y;
    
    if (sum > 10) {
        printf("Sum is greater than 10: %d\n", sum);
    } else {
        printf("Sum is 10 or less: %d\n", sum);
    }
    
    for (int i = 0; i < 3; i++) {
        printf("Iteration %d\n", i);
    }
    
    return 0;
}
EOF

# Create another test program for multiple file testing
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

# Run the programs to generate .gcda files
echo "Running test programs to generate coverage data..."
./test1_prog > /dev/null
./test2_prog > /dev/null

# Verify that GCOV files were created
if [ ! -f "test1.gcda" ] || [ ! -f "test1.gcno" ]; then
    echo "ERROR: GCOV files not created for test1"
    exit 1
fi

if [ ! -f "test2.gcda" ] || [ ! -f "test2.gcno" ]; then
    echo "ERROR: GCOV files not created for test2"
    exit 1
fi

echo "GCOV files created successfully"
echo ""

# Test 1: -h flag (covers case 'h' and print_usage())
echo "=== Test 1: Testing -h flag (help) ==="
gcov-dump -h 2>&1 | head -5
echo "Exit code: $?"
echo ""

# Test 2: --help flag (should also trigger help)
echo "=== Test 2: Testing --help flag ==="
gcov-dump --help 2>&1 | head -5
echo "Exit code: $?"
echo ""

# Test 3: -v flag (covers case 'v' and print_version())
echo "=== Test 3: Testing -v flag (version) ==="
gcov-dump -v 2>&1
echo "Exit code: $?"
echo ""

# Test 4: --version flag (should also trigger version)
echo "=== Test 4: Testing --version flag ==="
gcov-dump --version 2>&1
echo "Exit code: $?"
echo ""

# Test 5: -l flag (covers case 'l', sets flag_dump_contents = 1)
echo "=== Test 5: Testing -l flag (dump contents) ==="
gcov-dump -l test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 6: -p flag (covers case 'p', sets flag_dump_positions = 1)
echo "=== Test 6: Testing -p flag (dump positions) ==="
gcov-dump -p test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 7: -r flag (covers case 'r', sets flag_dump_raw = 1)
echo "=== Test 7: Testing -r flag (dump raw) ==="
gcov-dump -r test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 8: -s flag (covers case 's', sets flag_dump_stable = 1)
echo "=== Test 8: Testing -s flag (dump stable) ==="
gcov-dump -s test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 9: Multiple flags combined (covers multiple cases sequentially)
echo "=== Test 9: Testing multiple flags combined (-l -p -r -s) ==="
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -15
echo "Exit code: $?"
echo ""

# Test 10: Invalid flag (covers default case, prints "unknown flag")
echo "=== Test 10: Testing invalid flag (-x) ==="
gcov-dump -x test1.gcda 2>&1
echo "Exit code: $?"
echo ""

# Test 11: No flags with .gcno file (tests default behavior)
echo "=== Test 11: Testing with .gcno file (no flags) ==="
gcov-dump test1.gcno 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 12: Multiple input files with flag
echo "=== Test 12: Testing multiple input files with -l flag ==="
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -15
echo "Exit code: $?"
echo ""

# Test 13: Mixed file types with flag
echo "=== Test 13: Testing mixed file types (.gcda and .gcno) ==="
gcov-dump -p test1.gcda test1.gcno 2>&1 | head -15
echo "Exit code: $?"
echo ""

# Test 14: Long form of flags (if supported)
echo "=== Test 14: Testing long form flags (if supported) ==="
# Note: gcov-dump might not support long forms, but we try common ones
gcov-dump --long-format test1.gcda 2>&1 | head -5
echo "Exit code: $?"
echo ""

# Test 15: Flag with no argument (should show error or usage)
echo "=== Test 15: Testing flag without file argument ==="
gcov-dump -l 2>&1 | head -5
echo "Exit code: $?"
echo ""

# Test 16: Empty invocation (should show usage or error)
echo "=== Test 16: Testing empty invocation ==="
gcov-dump 2>&1 | head -5
echo "Exit code: $?"
echo ""

# Test 17: Combination of valid and invalid flags
echo "=== Test 17: Testing combination of valid and invalid flags ==="
gcov-dump -l -x -p test1.gcda 2>&1
echo "Exit code: $?"
echo ""

# Test 18: Flag with multiple files including non-existent file
echo "=== Test 18: Testing with non-existent file ==="
gcov-dump -l test1.gcda nonexistent.gcda 2>&1 | head -5
echo "Exit code: $?"
echo ""

# Test 19: Different order of flags
echo "=== Test 19: Testing different flag order ==="
gcov-dump -s -r -p -l test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 20: Flag with .gcda file that has no corresponding .gcno
echo "=== Test 20: Testing orphaned .gcda file ==="
cp test1.gcda orphan.gcda
rm -f orphan.gcno
gcov-dump -l orphan.gcda 2>&1 | head -5
echo "Exit code: $?"
echo ""

echo "=== All tests completed ==="
echo ""

# Cleanup
cd ..
rm -rf "$TEST_DIR"
echo "Cleaned up test directory: $TEST_DIR"
```

This test script comprehensively covers all the uncovered lines:

1. **Lines 111-112 (`case 'h'` and `print_usage()`)** - Tested with `-h` and `--help` flags
2. **Lines 113-115 (`case 'v'` and `print_version()`)** - Tested with `-v` and `--version` flags  
3. **Lines 116-117 (`case 'l'` and `flag_dump_contents = 1`)** - Tested with `-l` flag
4. **Lines 118-119 (`case 'p'` and `flag_dump_positions = 1`)** - Tested with `-p` flag
5. **Lines 120-121 (`case 'r'` and `flag_dump_raw = 1`)** - Tested with `-r` flag
6. **Lines 122-123 (`case 's'` and `flag_dump_stable = 1`)** - Tested with `-s` flag
7. **Lines 124-126 (`default` case with error message)** - Tested with invalid `-x` flag

The script also tests various combinations and edge cases:
- Multiple flags combined in one invocation
- Different types of input files (`.gcda`, `.gcno`)
- Multiple input files
- Invalid/non-existent files
- Empty invocations
- Different flag orders

To run this test, you need to:
1. Ensure `gcov-dump` is in your PATH or provide the full path
2. Have GCC installed with coverage support
3. Make the script executable: `chmod +x test_gcov_dump.sh`
4. Run it: `./test_gcov_dump.sh`

The script creates temporary test files, compiles them with coverage instrumentation, runs them to generate `.gcda` files, then tests all the command-line options on these files. Each test shows the first few lines of output and the exit code for verification.
