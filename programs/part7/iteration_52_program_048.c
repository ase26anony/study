I'll create a comprehensive test script to cover the uncovered lines in `gcov-dump.cc`. This script will generate GCOV data files and test all the command-line options.

```bash
#!/bin/bash
# Test script for gcov-dump to cover lines 111-130 in gcov-dump.cc
# This script tests the command-line argument parsing logic

set -e

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"

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

echo "Compiling test programs with coverage instrumentation..."

# Compile test programs with coverage flags
gcc -fprofile-arcs -ftest-coverage -O0 test1.c -o test1_prog
gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test2_prog

echo "Running test programs to generate .gcda files..."

# Run the programs to generate coverage data
./test1_prog
./test2_prog

echo "Generated coverage files:"
ls -la *.gcno *.gcda

echo ""
echo "=== Testing gcov-dump command-line options ==="
echo ""

# Test 1: Help option (-h)
echo "Test 1: Testing -h flag (should trigger case 'h')"
gcov-dump -h 2>&1 | head -5
echo "Exit code: $?"
echo ""

# Test 2: Help option (--help)
echo "Test 2: Testing --help flag"
gcov-dump --help 2>&1 | head -5
echo "Exit code: $?"
echo ""

# Test 3: Version option (-v)
echo "Test 3: Testing -v flag (should trigger case 'v')"
gcov-dump -v 2>&1
echo "Exit code: $?"
echo ""

# Test 4: Version option (--version)
echo "Test 4: Testing --version flag"
gcov-dump --version 2>&1
echo "Exit code: $?"
echo ""

# Test 5: Dump contents flag (-l)
echo "Test 5: Testing -l flag (should trigger case 'l')"
gcov-dump -l test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 6: Dump positions flag (-p)
echo "Test 6: Testing -p flag (should trigger case 'p')"
gcov-dump -p test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 7: Dump raw flag (-r)
echo "Test 7: Testing -r flag (should trigger case 'r')"
gcov-dump -r test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 8: Dump stable flag (-s)
echo "Test 8: Testing -s flag (should trigger case 's')"
gcov-dump -s test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 9: Multiple flags combined
echo "Test 9: Testing multiple flags combined (-l -p -r -s)"
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -15
echo "Exit code: $?"
echo ""

# Test 10: Invalid flag (should trigger default case)
echo "Test 10: Testing invalid flag -x (should trigger default case)"
gcov-dump -x test1.gcda 2>&1
echo "Exit code: $?"
echo ""

# Test 11: Test with .gcno file (no flags)
echo "Test 11: Testing with .gcno file only (no flags)"
gcov-dump test1.gcno 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 12: Test with multiple input files
echo "Test 12: Testing with multiple input files (-l flag)"
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -15
echo "Exit code: $?"
echo ""

# Test 13: Test with both .gcda and .gcno files
echo "Test 13: Testing with both .gcda and .gcno files"
gcov-dump -l test1.gcda test1.gcno 2>&1 | head -15
echo "Exit code: $?"
echo ""

# Test 14: Test with different flag order
echo "Test 14: Testing with different flag order (-s -r -p)"
gcov-dump -s -r -p test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 15: Test with flag and no filename (should show error)
echo "Test 15: Testing -l flag without filename"
gcov-dump -l 2>&1
echo "Exit code: $?"
echo ""

# Test 16: Test with non-existent file
echo "Test 16: Testing with non-existent file"
gcov-dump -l nonexistent.gcda 2>&1
echo "Exit code: $?"
echo ""

# Test 17: Test with empty flags (just --)
echo "Test 17: Testing with -- separator"
gcov-dump -- -l 2>&1 | head -5
echo "Exit code: $?"
echo ""

# Test 18: Test with long invalid flag
echo "Test 18: Testing with long invalid flag"
gcov-dump --invalid-flag 2>&1
echo "Exit code: $?"
echo ""

# Test 19: Test all short flags in different combinations
echo "Test 19: Testing all flag combinations systematically"

# Array of flags to test
flags=("-l" "-p" "-r" "-s" "-l -p" "-l -r" "-l -s" "-p -r" "-p -s" "-r -s" \
       "-l -p -r" "-l -p -s" "-l -r -s" "-p -r -s")

for flag_combo in "${flags[@]}"; do
    echo "Testing: gcov-dump $flag_combo test1.gcda"
    gcov-dump $flag_combo test1.gcda >/dev/null 2>&1
    echo "  Exit code: $?"
done
echo ""

# Cleanup
echo "Cleaning up test directory..."
cd /
rm -rf "$TEST_DIR"

echo ""
echo "=== All tests completed ==="
echo "The following cases in the switch statement should now be covered:"
echo "  case 'h': print_usage()"
echo "  case 'v': print_version()"
echo "  case 'l': flag_dump_contents = 1"
echo "  case 'p': flag_dump_positions = 1"
echo "  case 'r': flag_dump_raw = 1"
echo "  case 's': flag_dump_stable = 1"
echo "  default: fprintf(stderr, \"unknown flag\")"
```

This comprehensive test script will:

1. **Create test C programs** that generate GCOV data files (`.gcno` and `.gcda`)
2. **Test each individual flag** (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) to trigger the corresponding switch cases
3. **Test flag combinations** to ensure sequential execution of flag-setting logic
4. **Test the default case** with an invalid flag (`-x`)
5. **Test with different file types** (`.gcda`, `.gcno`, multiple files)
6. **Test edge cases** (no filename, non-existent file, `--` separator)
7. **Systematically test all flag combinations** to ensure complete coverage

To run this test, you need to:
1. Ensure `gcov-dump` is in your PATH or provide the full path to the binary
2. Have GCC installed with coverage support
3. Make the script executable: `chmod +x test_gcov_dump.sh`
4. Run it: `./test_gcov_dump.sh`

The script will create a temporary directory, generate all necessary test files, run all the test cases, and clean up afterwards. Each test case exercises different parts of the command-line parsing logic in `gcov-dump.cc`.
