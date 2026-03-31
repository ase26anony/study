Looking at the uncovered lines in `gcov-dump.cc`, I need to create tests that exercise all branches of the command-line argument parsing switch statement. Here's a comprehensive test script:

```bash
#!/bin/bash
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo "=== Testing gcov-dump command-line argument parsing ==="

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d)
cd "$TEST_DIR"

# Create a simple test C source file
cat > test.c << 'EOF'
#include <stdio.h>

int add(int a, int b) {
    return a + b;
}

int main() {
    int result = add(2, 3);
    printf("Result: %d\n", result);
    return 0;
}
EOF

# Compile with coverage instrumentation
echo "Compiling test program with coverage..."
gcc -fprofile-arcs -ftest-coverage -O0 -g test.c -o test

# Run the program to generate .gcda file
echo "Running test program to generate coverage data..."
./test > /dev/null

# Verify coverage files were created
if [[ ! -f test.gcno ]] || [[ ! -f test.gcda ]]; then
    echo -e "${RED}Error: Coverage files not created${NC}"
    exit 1
fi

echo -e "\n${GREEN}=== Testing valid flags ===${NC}"

# Test 1: -h (help)
echo "Test 1: Testing -h flag (help)"
if gcov-dump -h 2>&1 | grep -q "Usage:"; then
    echo -e "${GREEN}✓ -h flag works${NC}"
else
    echo -e "${RED}✗ -h flag failed${NC}"
fi

# Test 2: -v (version)
echo -e "\nTest 2: Testing -v flag (version)"
if gcov-dump -v 2>&1 | grep -q "gcov-dump"; then
    echo -e "${GREEN}✓ -v flag works${NC}"
else
    echo -e "${RED}✗ -v flag failed${NC}"
fi

# Test 3: -l (dump contents)
echo -e "\nTest 3: Testing -l flag (dump contents)"
if gcov-dump -l test.gcno 2>&1 | head -20 | grep -q "magic:"; then
    echo -e "${GREEN}✓ -l flag works${NC}"
else
    echo -e "${RED}✗ -l flag failed${NC}"
fi

# Test 4: -p (dump positions)
echo -e "\nTest 4: Testing -p flag (dump positions)"
if gcov-dump -p test.gcno 2>&1 | head -20 | grep -q "version:"; then
    echo -e "${GREEN}✓ -p flag works${NC}"
else
    echo -e "${RED}✗ -p flag failed${NC}"
fi

# Test 5: -r (dump raw)
echo -e "\nTest 5: Testing -r flag (dump raw)"
if gcov-dump -r test.gcno 2>&1 | head -20 | grep -q "stamp:"; then
    echo -e "${GREEN}✓ -r flag works${NC}"
else
    echo -e "${RED}✗ -r flag failed${NC}"
fi

# Test 6: -s (dump stable)
echo -e "\nTest 6: Testing -s flag (dump stable)"
if gcov-dump -s test.gcno 2>&1 | head -20 | grep -q "version:"; then
    echo -e "${GREEN}✓ -s flag works${NC}"
else
    echo -e "${RED}✗ -s flag failed${NC}"
fi

# Test 7: Combined flags
echo -e "\nTest 7: Testing combined flags (-l -p -r -s)"
if gcov-dump -l -p -r -s test.gcno 2>&1 | head -20 | grep -q "magic:"; then
    echo -e "${GREEN}✓ Combined flags work${NC}"
else
    echo -e "${RED}✗ Combined flags failed${NC}"
fi

# Test 8: Test with .gcda file
echo -e "\nTest 8: Testing with .gcda file"
if gcov-dump -l test.gcda 2>&1 | head -20 | grep -q "magic:"; then
    echo -e "${GREEN}✓ .gcda file works${NC}"
else
    echo -e "${RED}✗ .gcda file failed${NC}"
fi

echo -e "\n${GREEN}=== Testing invalid flags ===${NC}"

# Test 9: Invalid flag -x
echo -e "\nTest 9: Testing invalid flag -x"
if gcov-dump -x 2>&1 | grep -q "unknown flag 'x'"; then
    echo -e "${GREEN}✓ Invalid flag -x correctly detected${NC}"
else
    echo -e "${RED}✗ Invalid flag -x not detected${NC}"
fi

# Test 10: Invalid flag -z
echo -e "\nTest 10: Testing invalid flag -z"
if gcov-dump -z 2>&1 | grep -q "unknown flag 'z'"; then
    echo -e "${GREEN}✓ Invalid flag -z correctly detected${NC}"
else
    echo -e "${RED}✗ Invalid flag -z not detected${NC}"
fi

# Test 11: Invalid flag -9 (non-alphabetic)
echo -e "\nTest 11: Testing invalid flag -9 (non-alphabetic)"
if gcov-dump -9 2>&1 | grep -q "unknown flag '9'"; then
    echo -e "${GREEN}✓ Invalid flag -9 correctly detected${NC}"
else
    echo -e "${RED}✗ Invalid flag -9 not detected${NC}"
fi

# Test 12: Valid flag with invalid flag
echo -e "\nTest 12: Testing valid flag -l with invalid flag -x"
if gcov-dump -l -x test.gcno 2>&1 | grep -q "unknown flag 'x'"; then
    echo -e "${GREEN}✓ Mixed valid/invalid flags correctly detected${NC}"
else
    echo -e "${RED}✗ Mixed valid/invalid flags not detected${NC}"
fi

# Test 13: Invalid flag with argument separator
echo -e "\nTest 13: Testing invalid flag with -- separator"
if gcov-dump -x -- test.gcno 2>&1 | grep -q "unknown flag 'x'"; then
    echo -e "${GREEN}✓ Invalid flag with -- separator detected${NC}"
else
    echo -e "${RED}✗ Invalid flag with -- separator not detected${NC}"
fi

echo -e "\n${GREEN}=== Testing edge cases ===${NC}"

# Test 14: No arguments
echo -e "\nTest 14: Testing no arguments"
if ! gcov-dump 2>&1 | grep -q "Usage:"; then
    echo -e "${GREEN}✓ No arguments shows usage or error${NC}"
else
    echo -e "${RED}✗ No arguments behavior unexpected${NC}"
fi

# Test 15: Flags only, no filename
echo -e "\nTest 15: Testing flags only (no filename)"
if gcov-dump -l 2>&1 | grep -q "no input files"; then
    echo -e "${GREEN}✓ Flags only correctly handled${NC}"
else
    echo -e "${RED}✗ Flags only behavior unexpected${NC}"
fi

# Test 16: Non-existent file
echo -e "\nTest 16: Testing with non-existent file"
if gcov-dump -l nonexistent.gcno 2>&1 | grep -q "cannot open"; then
    echo -e "${GREEN}✓ Non-existent file correctly handled${NC}"
else
    echo -e "${RED}✗ Non-existent file behavior unexpected${NC}"
fi

# Test 17: Multiple files
echo -e "\nTest 17: Testing with multiple files"
if gcov-dump -l test.gcno test.gcda 2>&1 | grep -q "magic:"; then
    echo -e "${GREEN}✓ Multiple files work${NC}"
else
    echo -e "${RED}✗ Multiple files failed${NC}"
fi

# Test 18: Different flag combinations
echo -e "\nTest 18: Testing various flag combinations"
echo "Testing -l -p combination..."
if gcov-dump -l -p test.gcno 2>&1 | head -30 | grep -q "magic:"; then
    echo -e "${GREEN}✓ -l -p combination works${NC}"
else
    echo -e "${RED}✗ -l -p combination failed${NC}"
fi

echo -e "\nTesting -r -s combination..."
if gcov-dump -r -s test.gcno 2>&1 | head -30 | grep -q "stamp:"; then
    echo -e "${GREEN}✓ -r -s combination works${NC}"
else
    echo -e "${RED}✗ -r -s combination failed${NC}"
fi

# Cleanup
echo -e "\n${GREEN}=== Cleaning up ===${NC}"
cd ..
rm -rf "$TEST_DIR"
echo "Temporary directory removed: $TEST_DIR"

echo -e "\n${GREEN}=== All tests completed ===${NC}"
echo "The following switch cases should now be covered:"
echo "  - 'h': print_usage()"
echo "  - 'v': print_version()"
echo "  - 'l': flag_dump_contents = 1"
echo "  - 'p': flag_dump_positions = 1"
echo "  - 'r': flag_dump_raw = 1"
echo "  - 's': flag_dump_stable = 1"
echo "  - default: fprintf(stderr, \"unknown flag\")"
```

This test script comprehensively covers all the requirements:

1. **Valid flag invocations**: Tests each of `-h`, `-v`, `-l`, `-p`, `-r`, `-s` individually and in combination.

2. **Invalid flag invocations**: Tests `-x`, `-z`, `-9` (non-alphabetic), and mixed valid/invalid flags.

3. **File handling**: Creates a minimal C program, compiles it with coverage flags (`-fprofile-arcs -ftest-coverage`), runs it to generate `.gcda`, and tests with both `.gcno` and `.gcda` files.

4. **Edge cases**: Tests no arguments, flags only, non-existent files, multiple files, and the `--` separator.

5. **Verification**: Each test checks for expected output patterns to ensure the code paths are actually executed.

To run this test, save it as `test_gcov_dump.sh`, make it executable (`chmod +x test_gcov_dump.sh`), and execute it. The script will create a temporary directory, generate all necessary test files, run all test cases, and clean up afterwards.

This should provide 100% coverage for the switch statement in lines 111-130 of `gcov-dump.cc`.
