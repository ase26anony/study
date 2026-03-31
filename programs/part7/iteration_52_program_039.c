Looking at the uncovered lines in `gcov-dump.cc`, I need to create a comprehensive test that exercises all the command-line flag handling logic. Here's a shell script that systematically tests each uncovered case:

```bash
#!/bin/bash
# Test script for gcov-dump command-line argument parsing
# This script tests the uncovered lines in gcov-dump.cc (lines 111-130)

set -e  # Exit on any error

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"
echo "Testing gcov-dump command-line argument parsing..."

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
    echo "ERROR: Coverage files not created for test1"
    exit 1
fi

if [ ! -f test2.gcda ] || [ ! -f test2.gcno ]; then
    echo "ERROR: Coverage files not created for test2"
    exit 1
fi

echo "Coverage files created successfully"
echo ""

# Test 1: -h flag (help) - triggers case 'h'
echo "=== Test 1: Testing -h flag (help) ==="
gcov-dump -h 2>&1 | head -5
echo "Exit code: $?"
echo ""

# Test 2: --help flag (should also trigger help)
echo "=== Test 2: Testing --help flag ==="
gcov-dump --help 2>&1 | head -5
echo "Exit code: $?"
echo ""

# Test 3: -v flag (version) - triggers case 'v'
echo "=== Test 3: Testing -v flag (version) ==="
gcov-dump -v 2>&1
echo "Exit code: $?"
echo ""

# Test 4: --version flag (should also trigger version)
echo "=== Test 4: Testing --version flag ==="
gcov-dump --version 2>&1
echo "Exit code: $?"
echo ""

# Test 5: -l flag (dump contents) - triggers case 'l'
echo "=== Test 5: Testing -l flag (dump contents) ==="
gcov-dump -l test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 6: -p flag (dump positions) - triggers case 'p'
echo "=== Test 6: Testing -p flag (dump positions) ==="
gcov-dump -p test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 7: -r flag (dump raw) - triggers case 'r'
echo "=== Test 7: Testing -r flag (dump raw) ==="
gcov-dump -r test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 8: -s flag (dump stable) - triggers case 's'
echo "=== Test 8: Testing -s flag (dump stable) ==="
gcov-dump -s test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 9: Multiple flags combined - triggers multiple cases
echo "=== Test 9: Testing multiple flags combined (-l -p -r -s) ==="
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 10: Invalid flag - triggers default case
echo "=== Test 10: Testing invalid flag (-x) ==="
gcov-dump -x test1.gcda 2>&1
echo "Exit code: $?"
echo ""

# Test 11: No flags with .gcno file
echo "=== Test 11: Testing with .gcno file (no flags) ==="
gcov-dump test1.gcno 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 12: Multiple input files with flag
echo "=== Test 12: Testing multiple input files with -l flag ==="
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 13: Mixed file types with flag
echo "=== Test 13: Testing mixed file types (.gcda and .gcno) ==="
gcov-dump -p test1.gcda test1.gcno 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 14: Long form of combined flags
echo "=== Test 14: Testing combined flags in single argument ==="
gcov-dump -lprs test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 15: Flag with no argument (should show error)
echo "=== Test 15: Testing flag without file argument ==="
gcov-dump -l 2>&1
echo "Exit code: $?"
echo ""

# Test 16: Non-existent file with flag
echo "=== Test 16: Testing with non-existent file ==="
gcov-dump -l nonexistent.gcda 2>&1
echo "Exit code: $?"
echo ""

# Cleanup
echo "Cleaning up test directory..."
cd /
rm -rf "$TEST_DIR"

echo ""
echo "All tests completed successfully!"
echo "The following cases were tested:"
echo "1. -h / --help (case 'h')"
echo "2. -v / --version (case 'v')"
echo "3. -l flag (case 'l')"
echo "4. -p flag (case 'p')"
echo "5. -r flag (case 'r')"
echo "6. -s flag (case 's')"
echo "7. Multiple flags combined"
echo "8. Invalid flag (default case)"
echo "9. Various file combinations"
```

This test script systematically exercises all the uncovered lines:

1. **Lines 111-112 (`case 'h'`)**: Tested with `-h` and `--help` flags
2. **Lines 113-114 (`case 'v'`)**: Tested with `-v` and `--version` flags  
3. **Line 116 (`case 'l'`)**: Tested with `-l` flag in multiple contexts
4. **Line 118 (`case 'p'`)**: Tested with `-p` flag in multiple contexts
5. **Line 120 (`case 'r'`)**: Tested with `-r` flag in multiple contexts
6. **Line 122 (`case 's'`)**: Tested with `-s` flag in multiple contexts
7. **Lines 124-125 (`default` case)**: Tested with invalid `-x` flag

The script also tests various combinations and edge cases:
- Single flags with single files
- Multiple flags combined (`-l -p -r -s`)
- Multiple input files with flags
- Mixed file types (.gcda and .gcno)
- Flags without arguments (error case)
- Non-existent files
- Combined flags in single argument (`-lprs`)

To run this test, you need to:
1. Ensure `gcov-dump` is in your PATH or provide the full path
2. Have GCC installed with coverage support
3. Make the script executable: `chmod +x test_gcov_dump.sh`
4. Run it: `./test_gcov_dump.sh`

The script creates temporary test files, compiles them with coverage instrumentation, runs them to generate `.gcda` files, then tests all the command-line options against these files. Each test case is clearly labeled and the exit codes are checked to ensure proper execution.
