Looking at the uncovered lines in `gcov-dump.cc`, I need to create a comprehensive test that exercises all the command-line flag handling logic. Here's a shell script that systematically tests each uncovered case:

```bash
#!/bin/bash
# Test script for gcov-dump command-line argument parsing
# This script tests the uncovered switch cases in gcov-dump.cc lines 111-130

set -e  # Exit on any error

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"

# Create a simple C source file for generating GCOV data
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

# Create another test file for testing multiple input files
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

# Compile both test programs with coverage flags
gcc -fprofile-arcs -ftest-coverage -O0 test1.c -o test1_prog
gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test2_prog

echo "Running test programs to generate .gcda files..."

# Run the programs to generate coverage data
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

echo "Testing gcov-dump with various command-line flags..."
echo "======================================================"

# Test 1: Help flag (case 'h')
echo -e "\n1. Testing -h flag (should show usage):"
gcov-dump -h 2>&1 | head -5

echo -e "\n2. Testing --help flag (should also show usage):"
gcov-dump --help 2>&1 | head -5

# Test 2: Version flag (case 'v')
echo -e "\n3. Testing -v flag (should show version):"
gcov-dump -v 2>&1

echo -e "\n4. Testing --version flag (should also show version):"
gcov-dump --version 2>&1

# Test 3: -l flag (case 'l') - dump contents
echo -e "\n5. Testing -l flag (dump contents):"
gcov-dump -l test1.gcda 2>&1 | head -10

# Test 4: -p flag (case 'p') - dump positions
echo -e "\n6. Testing -p flag (dump positions):"
gcov-dump -p test1.gcda 2>&1 | head -10

# Test 5: -r flag (case 'r') - dump raw
echo -e "\n7. Testing -r flag (dump raw):"
gcov-dump -r test1.gcda 2>&1 | head -10

# Test 6: -s flag (case 's') - dump stable
echo -e "\n8. Testing -s flag (dump stable):"
gcov-dump -s test1.gcda 2>&1 | head -10

# Test 7: Multiple flags combined
echo -e "\n9. Testing multiple flags combined (-l -p -r -s):"
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -15

# Test 8: Different order of flags
echo -e "\n10. Testing flags in different order (-s -r -p -l):"
gcov-dump -s -r -p -l test1.gcda 2>&1 | head -15

# Test 9: No flags (default behavior)
echo -e "\n11. Testing with no flags (default dump):"
gcov-dump test1.gcda 2>&1 | head -10

# Test 10: .gcno file instead of .gcda
echo -e "\n12. Testing with .gcno file:"
gcov-dump test1.gcno 2>&1 | head -10

# Test 11: Multiple input files
echo -e "\n13. Testing with multiple input files:"
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -15

# Test 12: Mix of .gcda and .gcno files
echo -e "\n14. Testing with mix of .gcda and .gcno files:"
gcov-dump -p test1.gcda test1.gcno 2>&1 | head -15

# Test 13: Invalid flag (should trigger default case)
echo -e "\n15. Testing invalid flag -x (should show error):"
gcov-dump -x test1.gcda 2>&1 || true  # Don't fail on expected error

# Test 14: Another invalid flag
echo -e "\n16. Testing invalid flag -z (should show error):"
gcov-dump -z test1.gcda 2>&1 || true

# Test 15: Valid flag with invalid file
echo -e "\n17. Testing valid flag with non-existent file:"
gcov-dump -l nonexistent.gcda 2>&1 || true

# Test 16: Flag with no argument (where argument is expected)
echo -e "\n18. Testing -l flag without file argument:"
gcov-dump -l 2>&1 || true

# Test 17: Long form of flags (if supported)
echo -e "\n19. Testing --long options (if supported):"
gcov-dump --help --version 2>&1 | head -5

# Test 18: Repeated flags
echo -e "\n20. Testing repeated flags:"
gcov-dump -l -l -l test1.gcda 2>&1 | head -10

# Test 19: Flag with file that has spaces in name (edge case)
echo -e "\n21. Testing with file containing spaces:"
cp test1.gcda "test with spaces.gcda"
gcov-dump -l "test with spaces.gcda" 2>&1 | head -5
rm -f "test with spaces.gcda"

# Test 20: Empty .gcda file
echo -e "\n22. Testing with empty .gcda file:"
touch empty.gcda
gcov-dump -l empty.gcda 2>&1 || true
rm -f empty.gcda

echo -e "\n======================================================"
echo "All tests completed successfully!"

# Cleanup
cd ..
rm -rf "$TEST_DIR"

echo "Test directory cleaned up: $TEST_DIR"
```

This test script comprehensively covers all the uncovered lines:

1. **`case 'h'`**: Tested with `-h` and `--help` flags
2. **`case 'v'`**: Tested with `-v` and `--version` flags  
3. **`case 'l'`**: Tested with `-l` flag alone and in combinations
4. **`case 'p'`**: Tested with `-p` flag alone and in combinations
5. **`case 'r'`**: Tested with `-r` flag alone and in combinations
6. **`case 's'`**: Tested with `-s` flag alone and in combinations
7. **`default` case**: Tested with invalid flags `-x` and `-z`

The script also tests various edge cases:
- Multiple flags combined in different orders
- No flags (default behavior)
- Different file types (.gcda vs .gcno)
- Multiple input files
- Invalid files
- Files with special characters
- Empty files

To run this test, you need to:
1. Ensure `gcov-dump` is in your PATH or provide the full path
2. Have GCC installed with coverage support
3. Make the script executable: `chmod +x test_gcov_dump.sh`
4. Run it: `./test_gcov_dump.sh`

The script creates a temporary directory, generates test GCOV files, runs all the test cases, and cleans up after itself. Each test case exercises different code paths in the switch statement to ensure all uncovered lines are hit.
