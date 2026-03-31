I'll create a comprehensive test script to cover the uncovered lines in `gcov-dump.cc`. This script will generate GCOV data files and test all the command-line flags mentioned.

```bash
#!/bin/bash

# Test script for gcov-dump to cover the uncovered lines in gcov-dump.cc
# Lines 111-130: case handlers for command-line flags

set -e

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test-XXXXXX)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"

# Create a simple C source file to generate coverage data
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
echo "Compiling test programs with coverage flags..."
gcc -fprofile-arcs -ftest-coverage -O0 -g test1.c -o test1_prog
gcc -fprofile-arcs -ftest-coverage -O0 -g test2.c -o test2_prog

# Run the programs to generate .gcda files
echo "Running test programs to generate coverage data..."
./test1_prog > /dev/null
./test2_prog > /dev/null

# Verify that .gcno and .gcda files were created
echo "Checking for generated coverage files..."
ls -la *.gcno *.gcda

# Test 1: Help flag (-h)
echo -e "\n=== Test 1: Help flag (-h) ==="
gcov-dump -h 2>&1 | head -5

# Test 2: Help flag (--help)
echo -e "\n=== Test 2: Help flag (--help) ==="
gcov-dump --help 2>&1 | head -5

# Test 3: Version flag (-v)
echo -e "\n=== Test 3: Version flag (-v) ==="
gcov-dump -v 2>&1

# Test 4: Version flag (--version)
echo -e "\n=== Test 4: Version flag (--version) ==="
gcov-dump --version 2>&1

# Test 5: Dump contents flag (-l)
echo -e "\n=== Test 5: Dump contents flag (-l) ==="
gcov-dump -l test1.gcda 2>&1 | head -20

# Test 6: Dump positions flag (-p)
echo -e "\n=== Test 6: Dump positions flag (-p) ==="
gcov-dump -p test1.gcda 2>&1 | head -20

# Test 7: Dump raw flag (-r)
echo -e "\n=== Test 7: Dump raw flag (-r) ==="
gcov-dump -r test1.gcda 2>&1 | head -20

# Test 8: Dump stable flag (-s)
echo -e "\n=== Test 8: Dump stable flag (-s) ==="
gcov-dump -s test1.gcda 2>&1 | head -20

# Test 9: Multiple flags combined
echo -e "\n=== Test 9: Multiple flags combined (-l -p -r -s) ==="
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -30

# Test 10: Test with .gcno file (no flags)
echo -e "\n=== Test 10: .gcno file with no flags ==="
gcov-dump test1.gcno 2>&1 | head -20

# Test 11: Test with .gcno file with flags
echo -e "\n=== Test 11: .gcno file with -l flag ==="
gcov-dump -l test1.gcno 2>&1 | head -20

# Test 12: Multiple input files
echo -e "\n=== Test 12: Multiple input files ==="
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -30

# Test 13: Invalid flag (to trigger default case)
echo -e "\n=== Test 13: Invalid flag (-x) to trigger default case ==="
gcov-dump -x test1.gcda 2>&1 || true

# Test 14: Another invalid flag
echo -e "\n=== Test 14: Another invalid flag (-z) ==="
gcov-dump -z test1.gcda 2>&1 || true

# Test 15: Valid flag with invalid file
echo -e "\n=== Test 15: Valid flag with non-existent file ==="
gcov-dump -l nonexistent.gcda 2>&1 || true

# Test 16: No input files with flags
echo -e "\n=== Test 16: Flags without input files ==="
gcov-dump -l 2>&1 || true

# Test 17: All flags in different order
echo -e "\n=== Test 17: All flags in different order ==="
gcov-dump -s -r -p -l test1.gcda 2>&1 | head -30

# Test 18: Short and long options mixed
echo -e "\n=== Test 18: Mixed short and long options ==="
gcov-dump -l --version test1.gcda 2>&1 | head -10

# Test 19: Flag with equals sign (if supported)
echo -e "\n=== Test 19: Testing flag parsing edge cases ==="
gcov-dump -l=test1.gcda 2>&1 || true

# Test 20: Repeated flags
echo -e "\n=== Test 20: Repeated flags ==="
gcov-dump -l -l -l test1.gcda 2>&1 | head -20

# Test 21: Flag in the middle of files
echo -e "\n=== Test 21: Flag between files ==="
gcov-dump test1.gcda -l test2.gcda 2>&1 | head -20

# Test 22: Empty arguments
echo -e "\n=== Test 22: Empty arguments ==="
gcov-dump "" 2>&1 || true

# Test 23: Very long command line
echo -e "\n=== Test 23: Many files and flags ==="
gcov-dump -l -p test1.gcda test1.gcno test2.gcda test2.gcno 2>&1 | head -40

# Test 24: Create a corrupted .gcda file to test error handling
echo -e "\n=== Test 24: Corrupted .gcda file ==="
echo "corrupted data" > corrupted.gcda
gcov-dump -l corrupted.gcda 2>&1 || true

# Test 25: Test with directory as input
echo -e "\n=== Test 25: Directory as input ==="
mkdir testdir
gcov-dump -l testdir 2>&1 || true

# Summary
echo -e "\n=== Test Summary ==="
echo "All tests completed. The following cases were covered:"
echo "1. -h / --help (print_usage)"
echo "2. -v / --version (print_version)"
echo "3. -l (flag_dump_contents = 1)"
echo "4. -p (flag_dump_positions = 1)"
echo "5. -r (flag_dump_raw = 1)"
echo "6. -s (flag_dump_stable = 1)"
echo "7. Multiple flags combined"
echo "8. Invalid flags (default case)"
echo "9. Various edge cases and error conditions"

# Cleanup
echo -e "\nCleaning up test directory..."
cd /
rm -rf "$TEST_DIR"

echo "Test completed successfully!"
```

This comprehensive test script covers:

1. **All individual flags**: `-h`, `--help`, `-v`, `--version`, `-l`, `-p`, `-r`, `-s`
2. **Flag combinations**: Multiple flags together in different orders
3. **Different input types**: `.gcda` files, `.gcno` files, multiple files
4. **Error cases**: Invalid flags (triggers the `default` case), non-existent files, corrupted files
5. **Edge cases**: Empty arguments, directories as input, flags between files

The script:
- Creates temporary test C programs
- Compiles them with coverage flags (`-fprofile-arcs -ftest-coverage`)
- Runs them to generate `.gcda` files
- Tests `gcov-dump` with various flag combinations
- Handles errors gracefully (using `|| true` where errors are expected)
- Cleans up after itself

To run this test, save it as `test_gcov_dump.sh`, make it executable (`chmod +x test_gcov_dump.sh`), and execute it. The `gcov-dump` binary must be in your PATH or you can modify the script to use the full path to your `gcov-dump` executable.
