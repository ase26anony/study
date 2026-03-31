Looking at the uncovered lines in `gcov-dump.cc`, I need to create a comprehensive test that exercises all the command-line flag parsing logic. Here's a shell script that will generate the necessary GCOV data files and test all the uncovered switch cases:

```bash
#!/bin/bash
set -e

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
    printf("Test2 main\n");
    helper();
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

# Verify that .gcno and .gcda files were created
if [ ! -f test1.gcno ] || [ ! -f test1.gcda ]; then
    echo "ERROR: Coverage files not generated for test1"
    exit 1
fi

if [ ! -f test2.gcno ] || [ ! -f test2.gcda ]; then
    echo "ERROR: Coverage files not generated for test2"
    exit 1
fi

echo "Coverage files generated successfully"

# Test 1: -h flag (covers case 'h')
echo -e "\n=== Testing -h flag ==="
gcov-dump -h 2>&1 | head -5

# Test 2: --help flag (should also trigger -h)
echo -e "\n=== Testing --help flag ==="
gcov-dump --help 2>&1 | head -5

# Test 3: -v flag (covers case 'v')
echo -e "\n=== Testing -v flag ==="
gcov-dump -v

# Test 4: --version flag (should also trigger -v)
echo -e "\n=== Testing --version flag ==="
gcov-dump --version

# Test 5: -l flag (covers case 'l')
echo -e "\n=== Testing -l flag ==="
gcov-dump -l test1.gcda 2>&1 | head -10

# Test 6: -p flag (covers case 'p')
echo -e "\n=== Testing -p flag ==="
gcov-dump -p test1.gcda 2>&1 | head -10

# Test 7: -r flag (covers case 'r')
echo -e "\n=== Testing -r flag ==="
gcov-dump -r test1.gcda 2>&1 | head -10

# Test 8: -s flag (covers case 's')
echo -e "\n=== Testing -s flag ==="
gcov-dump -s test1.gcda 2>&1 | head -10

# Test 9: Multiple flags combined (covers all flag-setting cases)
echo -e "\n=== Testing multiple flags combined (-l -p -r -s) ==="
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -15

# Test 10: Invalid flag (covers default case)
echo -e "\n=== Testing invalid flag (-x) ==="
gcov-dump -x test1.gcda 2>&1 | grep "unknown flag" || true

# Test 11: No flags with .gcno file
echo -e "\n=== Testing no flags with .gcno file ==="
gcov-dump test1.gcno 2>&1 | head -10

# Test 12: No flags with .gcda file
echo -e "\n=== Testing no flags with .gcda file ==="
gcov-dump test1.gcda 2>&1 | head -10

# Test 13: Single flag with multiple input files
echo -e "\n=== Testing -l flag with multiple files ==="
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -15

# Test 14: Mixed file types with flag
echo -e "\n=== Testing -p flag with .gcda and .gcno ==="
gcov-dump -p test1.gcda test1.gcno 2>&1 | head -15

# Test 15: Long form flags in combination
echo -e "\n=== Testing combination of long and short flags ==="
gcov-dump -l -p --version test1.gcda 2>&1 | head -5

# Test 16: Flag with no argument (should show error)
echo -e "\n=== Testing flag without file argument ==="
gcov-dump -l 2>&1 | head -5 || true

# Test 17: Multiple invalid flags
echo -e "\n=== Testing multiple invalid flags ==="
gcov-dump -x -y -z test1.gcda 2>&1 | grep "unknown flag" || true

# Test 18: Valid flag after invalid flag
echo -e "\n=== Testing valid flag after invalid flag ==="
gcov-dump -x -l test1.gcda 2>&1 | grep "unknown flag" || true

# Test 19: Different order of flags
echo -e "\n=== Testing different flag order ==="
gcov-dump -s -r -p -l test1.gcda 2>&1 | head -15

# Test 20: Flag with non-existent file
echo -e "\n=== Testing flag with non-existent file ==="
gcov-dump -l nonexistent.gcda 2>&1 | head -5 || true

echo -e "\n=== All tests completed ==="

# Cleanup
cd /
rm -rf "$TEST_DIR"
echo "Cleaned up test directory"
```

This test script comprehensively covers all the uncovered lines:

1. **Lines 111-112 (`case 'h'`)**: Tested with `-h` and `--help` flags
2. **Lines 113-114 (`case 'v'`)**: Tested with `-v` and `--version` flags  
3. **Line 116 (`case 'l'`)**: Tested with `-l` flag in multiple contexts
4. **Line 118 (`case 'p'`)**: Tested with `-p` flag in multiple contexts
5. **Line 120 (`case 'r'`)**: Tested with `-r` flag in multiple contexts
6. **Line 122 (`case 's'`)**: Tested with `-s` flag in multiple contexts
7. **Lines 124-125 (`default` case)**: Tested with invalid flags like `-x`, `-y`, `-z`

The script also tests various combinations and edge cases:
- Single flags with single files
- Multiple flags combined
- Multiple input files
- Mixed file types (.gcda and .gcno)
- Invalid flags
- Flags without arguments
- Different flag orders

To run this test, you need to:
1. Ensure `gcov-dump` is in your PATH or provide the full path to the binary
2. Have GCC installed with coverage support
3. Make the script executable: `chmod +x test_gcov_dump.sh`
4. Run it: `./test_gcov_dump.sh`

The script creates temporary test files, compiles them with coverage instrumentation, runs them to generate `.gcda` files, then tests all the command-line options of `gcov-dump`. Each test case exercises different parts of the switch statement in the uncovered lines.
