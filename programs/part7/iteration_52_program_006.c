I'll create a comprehensive test script to cover the uncovered lines in `gcov-dump.cc`. This script will generate GCOV data files and test all the command-line flags.

```bash
#!/bin/bash
# Test script for gcov-dump to cover lines 111-130 in gcov-dump.cc
# This script tests the command-line argument parsing logic

set -e  # Exit on any error

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
echo "Test directory: $TEST_DIR"
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    cd /
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

# Create a simple C source file to generate GCOV data
cat > test1.c << 'EOF'
#include <stdio.h>

int main() {
    int x = 5;
    int y = 10;
    
    if (x < y) {
        printf("x is less than y\n");
    } else {
        printf("x is not less than y\n");
    }
    
    for (int i = 0; i < 3; i++) {
        printf("Loop iteration %d\n", i);
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

# Verify that GCOV files were created
echo "Checking for generated GCOV files..."
ls -la *.gcno *.gcda

# Test 1: Help flag (-h)
echo -e "\n=== Test 1: Help flag (-h) ==="
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
gcov-dump -l test1.gcda 2>&1 | head -20
echo "Exit code: $?"

# Test 6: Dump positions flag (-p)
echo -e "\n=== Test 6: Dump positions flag (-p) ==="
gcov-dump -p test1.gcda 2>&1 | head -20
echo "Exit code: $?"

# Test 7: Dump raw flag (-r)
echo -e "\n=== Test 7: Dump raw flag (-r) ==="
gcov-dump -r test1.gcda 2>&1 | head -20
echo "Exit code: $?"

# Test 8: Dump stable flag (-s)
echo -e "\n=== Test 8: Dump stable flag (-s) ==="
gcov-dump -s test1.gcda 2>&1 | head -20
echo "Exit code: $?"

# Test 9: Multiple flags combined
echo -e "\n=== Test 9: Multiple flags combined (-l -p -r -s) ==="
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -30
echo "Exit code: $?"

# Test 10: Invalid flag (triggers default case)
echo -e "\n=== Test 10: Invalid flag (-x) triggers default case ==="
gcov-dump -x test1.gcda 2>&1
echo "Exit code: $?"

# Test 11: No flags (default behavior)
echo -e "\n=== Test 11: No flags (default behavior) ==="
gcov-dump test1.gcda 2>&1 | head -20
echo "Exit code: $?"

# Test 12: Process .gcno file
echo -e "\n=== Test 12: Process .gcno file ==="
gcov-dump -l test1.gcno 2>&1 | head -20
echo "Exit code: $?"

# Test 13: Multiple input files
echo -e "\n=== Test 13: Multiple input files ==="
gcov-dump -l test1.gcda test1.gcno 2>&1 | head -30
echo "Exit code: $?"

# Test 14: Different combinations with multiple files
echo -e "\n=== Test 14: Multiple files with -p flag ==="
gcov-dump -p test1.gcda test2.gcda 2>&1 | head -30
echo "Exit code: $?"

# Test 15: Test with both .gcda and .gcno from different programs
echo -e "\n=== Test 15: Mixed files from different programs ==="
gcov-dump -r test1.gcda test2.gcno 2>&1 | head -30
echo "Exit code: $?"

# Test 16: Long form flags with multiple files
echo -e "\n=== Test 16: All files with -s flag ==="
gcov-dump -s *.gcda *.gcno 2>&1 | head -30
echo "Exit code: $?"

# Test 17: Another invalid flag combination
echo -e "\n=== Test 17: Another invalid flag (-z) ==="
gcov-dump -z test1.gcda 2>&1
echo "Exit code: $?"

# Test 18: Valid flag after invalid file (should still process flags)
echo -e "\n=== Test 18: Valid flag with non-existent file ==="
gcov-dump -l nonexistent.gcda 2>&1
echo "Exit code: $?"

# Test 19: Empty arguments (should show usage)
echo -e "\n=== Test 19: No arguments ==="
gcov-dump 2>&1 | head -10
echo "Exit code: $?"

# Test 20: Flag with no file argument
echo -e "\n=== Test 20: Flag with no file ==="
gcov-dump -l 2>&1 | head -10
echo "Exit code: $?"

echo -e "\n=== All tests completed ==="
echo "Summary of flag coverage:"
echo "  -h / --help    : print_usage() called"
echo "  -v / --version : print_version() called"
echo "  -l             : flag_dump_contents = 1"
echo "  -p             : flag_dump_positions = 1"
echo "  -r             : flag_dump_raw = 1"
echo "  -s             : flag_dump_stable = 1"
echo "  invalid flag   : default case triggered"
echo "  multiple flags : all flag-setting logic executed"
echo "  various files  : .gcda, .gcno, multiple files"
```

This comprehensive test script will:

1. **Generate valid GCOV data files** by compiling and running two simple C programs with coverage instrumentation
2. **Test each individual flag** (`-l`, `-p`, `-r`, `-s`) to ensure the corresponding case statements are executed
3. **Test help and version flags** (`-h`, `--help`, `-v`, `--version`) to cover those cases
4. **Test flag combinations** (`-l -p -r -s`) to ensure multiple flags can be set in one invocation
5. **Test invalid flags** (`-x`, `-z`) to trigger the `default` case
6. **Test various file combinations**:
   - Single `.gcda` file
   - Single `.gcno` file  
   - Multiple `.gcda` files
   - Mixed `.gcda` and `.gcno` files
   - All files with wildcards
7. **Test edge cases**:
   - No arguments
   - Flags without files
   - Non-existent files with valid flags

The script captures output and exit codes to verify that each invocation works as expected. Each test case exercises different paths through the switch statement in lines 111-130 of `gcov-dump.cc`, ensuring complete coverage of the uncovered lines.
