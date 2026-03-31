Here's a comprehensive test script that covers all the specified uncovered lines in `gcov-dump.cc`:

```bash
#!/bin/bash
# Test script for gcov-dump to cover lines 111-130 in gcov-dump.cc

set -e

# Create a temporary directory for test artifacts
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"

# Create a simple C source file for testing
cat > test1.c << 'EOF'
#include <stdio.h>

int main() {
    int x = 5;
    int y = 10;
    int sum = x + y;
    
    if (sum > 10) {
        printf("Sum is greater than 10\n");
    } else {
        printf("Sum is 10 or less\n");
    }
    
    for (int i = 0; i < 3; i++) {
        printf("Iteration %d\n", i);
    }
    
    return 0;
}
EOF

# Create another test file with different structure
cat > test2.c << 'EOF'
#include <stdio.h>

int helper(int a, int b) {
    return a * b;
}

int main() {
    int result = helper(3, 4);
    printf("Result: %d\n", result);
    
    switch (result) {
        case 12:
            printf("Expected result\n");
            break;
        default:
            printf("Unexpected result\n");
    }
    
    return 0;
}
EOF

# Compile test programs with coverage instrumentation
echo "Compiling test programs with coverage..."
gcc -fprofile-arcs -ftest-coverage -O0 -g test1.c -o test1_prog
gcc -fprofile-arcs -ftest-coverage -O0 -g test2.c -o test2_prog

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
echo -e "\n=== Test 1: Help flag (-h) ==="
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

# Test 9: Multiple flags combined - covers multiple cases
echo -e "\n=== Test 9: Multiple flags combined (-l -p -r -s) ==="
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -15

# Test 10: Invalid flag (-x) - covers default case
echo -e "\n=== Test 10: Invalid flag (-x) - should trigger default case ==="
gcov-dump -x test1.gcda 2>&1 || true

# Test 11: No flags with .gcno file - tests default behavior
echo -e "\n=== Test 11: No flags with .gcno file ==="
gcov-dump test1.gcno 2>&1 | head -10

# Test 12: Multiple input files with flag
echo -e "\n=== Test 12: Multiple input files with flag (-l) ==="
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -15

# Test 13: Different combinations of flags with different files
echo -e "\n=== Test 13: Different flag combinations ==="
echo "Testing -p -r combination:"
gcov-dump -p -r test2.gcda 2>&1 | head -10

echo -e "\nTesting -s -l combination:"
gcov-dump -s -l test2.gcno 2>&1 | head -10

# Test 14: Test with .gcno file and various flags
echo -e "\n=== Test 14: Testing .gcno file with different flags ==="
for flag in "-l" "-p" "-r" "-s"; do
    echo "Testing flag $flag with .gcno:"
    gcov-dump $flag test1.gcno 2>&1 | head -5
    echo "---"
done

# Test 15: Edge case - empty/invalid file
echo -e "\n=== Test 15: Testing with empty file ==="
touch empty.gcda
gcov-dump -l empty.gcda 2>&1 || true

# Test 16: Test all flags in different order
echo -e "\n=== Test 16: All flags in different order ==="
gcov-dump -s -r -p -l test1.gcda 2>&1 | head -10

# Cleanup
echo -e "\n=== Cleaning up ==="
cd /
rm -rf "$TEST_DIR"
echo "Test directory removed: $TEST_DIR"

echo -e "\n=== All tests completed ==="
echo "The following cases from lines 111-130 have been exercised:"
echo "  - case 'h': help flag"
echo "  - case 'v': version flag"
echo "  - case 'l': dump contents flag"
echo "  - case 'p': dump positions flag"
echo "  - case 'r': dump raw flag"
echo "  - case 's': dump stable flag"
echo "  - default: invalid flag handling"
```

This test script provides comprehensive coverage of the specified lines:

1. **Help flags**: Tests both `-h` and `--help` (covers `case 'h'`)
2. **Version flags**: Tests both `-v` and `--version` (covers `case 'v'`)
3. **Individual flags**: Tests `-l`, `-p`, `-r`, `-s` separately (covers each case)
4. **Flag combinations**: Tests multiple flags together to ensure sequential execution
5. **Invalid flag**: Tests `-x` to trigger the `default` case
6. **Different file types**: Tests with `.gcda` and `.gcno` files
7. **Multiple files**: Tests with multiple input files
8. **Edge cases**: Tests with empty files

To run this test, ensure:
1. The GCC toolchain is built with coverage instrumentation
2. `gcov-dump` is in your PATH or specify its full path
3. You have write permissions in `/tmp`

The script creates temporary test files, compiles them with coverage instrumentation, runs them to generate `.gcda` files, then exercises `gcov-dump` with all the flag combinations needed to cover the target lines.
