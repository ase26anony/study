Looking at the uncovered lines in `gcov-dump.cc`, I need to create a comprehensive test that exercises all command-line parsing logic. Here's a shell script that systematically tests each case:

```bash
#!/bin/bash
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo "=== Testing gcov-dump command-line parsing ==="

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

# Create a simple test program
cat > test.c << 'EOF'
#include <stdio.h>

int add(int a, int b) {
    return a + b;
}

int main() {
    printf("Result: %d\n", add(2, 3));
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
echo "Test 1: -h flag (help)"
if gcov-dump -h 2>&1 | grep -q "Usage:"; then
    echo -e "${GREEN}✓ -h flag works${NC}"
else
    echo -e "${RED}✗ -h flag failed${NC}"
fi

# Test 2: -v (version)
echo -e "\nTest 2: -v flag (version)"
if gcov-dump -v 2>&1 | grep -q "gcov-dump"; then
    echo -e "${GREEN}✓ -v flag works${NC}"
else
    echo -e "${RED}✗ -v flag failed${NC}"
fi

# Test 3: -l flag (dump contents)
echo -e "\nTest 3: -l flag (dump contents)"
if gcov-dump -l test.gcno 2>&1 | head -20 | grep -q "magic\|version\|stamp"; then
    echo -e "${GREEN}✓ -l flag works${NC}"
else
    echo -e "${RED}✗ -l flag failed${NC}"
fi

# Test 4: -p flag (dump positions)
echo -e "\nTest 4: -p flag (dump positions)"
if gcov-dump -p test.gcno 2>&1 | head -20 | grep -q "positions\|blocks"; then
    echo -e "${GREEN}✓ -p flag works${NC}"
else
    echo -e "${RED}✗ -p flag failed${NC}"
fi

# Test 5: -r flag (dump raw)
echo -e "\nTest 5: -r flag (dump raw)"
if gcov-dump -r test.gcda 2>&1 | head -20 | grep -q "counts\|summary"; then
    echo -e "${GREEN}✓ -r flag works${NC}"
else
    echo -e "${RED}✗ -r flag failed${NC}"
fi

# Test 6: -s flag (dump stable)
echo -e "\nTest 6: -s flag (dump stable)"
if gcov-dump -s test.gcda 2>&1 | head -20 | grep -q "stamp\|checksum"; then
    echo -e "${GREEN}✓ -s flag works${NC}"
else
    echo -e "${RED}✗ -s flag failed${NC}"
fi

# Test 7: Combined flags
echo -e "\nTest 7: Combined flags (-l -p -r -s)"
if gcov-dump -l -p -r -s test.gcda 2>&1 | head -30 | grep -q "magic\|positions\|counts\|stamp"; then
    echo -e "${GREEN}✓ Combined flags work${NC}"
else
    echo -e "${RED}✗ Combined flags failed${NC}"
fi

# Test 8: Flags with -- separator
echo -e "\nTest 8: Flags with -- separator"
if gcov-dump -l -p -- test.gcno 2>&1 | head -20 | grep -q "magic\|positions"; then
    echo -e "${GREEN}✓ -- separator works${NC}"
else
    echo -e "${RED}✗ -- separator failed${NC}"
fi

echo -e "\n${GREEN}=== Testing invalid flags ===${NC}"

# Test 9: Invalid flag -x
echo -e "\nTest 9: Invalid flag -x"
if gcov-dump -x 2>&1 | grep -q "unknown flag 'x'"; then
    echo -e "${GREEN}✓ Invalid flag -x correctly rejected${NC}"
else
    echo -e "${RED}✗ Invalid flag -x not properly handled${NC}"
fi

# Test 10: Invalid flag -z
echo -e "\nTest 10: Invalid flag -z"
if gcov-dump -z 2>&1 | grep -q "unknown flag 'z'"; then
    echo -e "${GREEN}✓ Invalid flag -z correctly rejected${NC}"
else
    echo -e "${RED}✗ Invalid flag -z not properly handled${NC}"
fi

# Test 11: Invalid flag -9 (non-alphabetic)
echo -e "\nTest 11: Invalid flag -9 (non-alphabetic)"
if gcov-dump -9 2>&1 | grep -q "unknown flag '9'"; then
    echo -e "${GREEN}✓ Invalid flag -9 correctly rejected${NC}"
else
    echo -e "${RED}✗ Invalid flag -9 not properly handled${NC}"
fi

# Test 12: Valid flag with invalid flag (-l -x)
echo -e "\nTest 12: Valid flag with invalid flag (-l -x)"
if gcov-dump -l -x test.gcno 2>&1 | grep -q "unknown flag 'x'"; then
    echo -e "${GREEN}✓ Mixed valid/invalid flags correctly handled${NC}"
else
    echo -e "${RED}✗ Mixed valid/invalid flags not properly handled${NC}"
fi

# Test 13: Invalid flag with file argument
echo -e "\nTest 13: Invalid flag with file argument (-y test.gcno)"
if gcov-dump -y test.gcno 2>&1 | grep -q "unknown flag 'y'"; then
    echo -e "${GREEN}✓ Invalid flag with file correctly rejected${NC}"
else
    echo -e "${RED}✗ Invalid flag with file not properly handled${NC}"
fi

# Test 14: No arguments
echo -e "\nTest 14: No arguments"
if ! gcov-dump 2>&1 >/dev/null; then
    echo -e "${GREEN}✓ No arguments correctly handled${NC}"
else
    echo -e "${RED}✗ No arguments not properly handled${NC}"
fi

# Test 15: Flags only, no filename
echo -e "\nTest 15: Flags only, no filename (-l -p)"
if ! gcov-dump -l -p 2>&1 >/dev/null; then
    echo -e "${GREEN}✓ Flags without filename correctly handled${NC}"
else
    echo -e "${RED}✗ Flags without filename not properly handled${NC}"
fi

# Test 16: Non-existent file
echo -e "\nTest 16: Non-existent file"
if ! gcov-dump -l nonexistent.gcda 2>&1 >/dev/null; then
    echo -e "${GREEN}✓ Non-existent file correctly handled${NC}"
else
    echo -e "${RED}✗ Non-existent file not properly handled${NC}"
fi

# Test 17: Multiple files with flags
echo -e "\nTest 17: Multiple files"
if gcov-dump -l test.gcno test.gcda 2>&1 | head -40 | grep -q "magic"; then
    echo -e "${GREEN}✓ Multiple files work${NC}"
else
    echo -e "${RED}✗ Multiple files failed${NC}"
fi

# Test 18: Special character flag (simulate invalid option)
echo -e "\nTest 18: Special character flag -?"
if gcov-dump -\? 2>&1 | grep -q "unknown flag"; then
    echo -e "${GREEN}✓ Special character flag correctly rejected${NC}"
else
    echo -e "${RED}✗ Special character flag not properly handled${NC}"
fi

# Test 19: Long option (if supported, should trigger error if not implemented)
echo -e "\nTest 19: Long option --help (if unsupported)"
if gcov-dump --help 2>&1 | grep -q "unknown flag\|Usage:"; then
    echo -e "${GREEN}✓ Long option handled${NC}"
else
    echo -e "${RED}✗ Long option not properly handled${NC}"
fi

echo -e "\n${GREEN}=== Creating edge case coverage files ===${NC}"

# Create a more complex test program for varied coverage data
cat > test_complex.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

void func1(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else {
        printf("Non-positive: %d\n", x);
    }
}

int func2(int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += i;
        if (i % 2 == 0) {
            sum += 1;
        }
    }
    return sum;
}

int main(int argc, char **argv) {
    func1(argc);
    int result = func2(argc * 2);
    printf("Result: %d\n", result);
    
    // Try to create some branching
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == 't') {
            printf("Starts with t: %s\n", argv[i]);
        }
    }
    
    return 0;
}
EOF

# Compile with optimization and coverage
echo "Compiling complex test with optimization..."
gcc -fprofile-arcs -ftest-coverage -O3 -funroll-loops test_complex.c -o test_complex

# Run multiple times with different arguments
echo "Running complex test with different arguments..."
./test_complex > /dev/null
./test_complex arg1 arg2 > /dev/null
./test_complex test another > /dev/null

# Test 20: Dump optimized coverage data
echo -e "\nTest 20: Dump optimized coverage data"
if gcov-dump -l -p -r test_complex.gcda 2>&1 | head -50 | grep -q "magic\|positions\|counts"; then
    echo -e "${GREEN}✓ Optimized coverage data dump works${NC}"
else
    echo -e "${RED}✗ Optimized coverage data dump failed${NC}"
fi

# Cleanup
echo -e "\n=== Cleaning up ==="
cd ..
rm -rf "$TEST_DIR"
echo "Temporary directory removed: $TEST_DIR"

echo -e "\n${GREEN}=== All tests completed ===${NC}"
echo "The following switch cases should now be covered:"
echo "  case 'h': print_usage()"
echo "  case 'v': print_version()"
echo "  case 'l': flag_dump_contents = 1"
echo "  case 'p': flag_dump_positions = 1"
echo "  case 'r': flag_dump_raw = 1"
echo "  case 's': flag_dump_stable = 1"
echo "  default: fprintf(stderr, \"unknown flag\")"
echo ""
echo "Plus various edge cases in argument parsing."
```

This test script systematically covers:

1. **All valid flags**: `-h`, `-v`, `-l`, `-p`, `-r`, `-s` individually and in combination
2. **Invalid flags**: `-x`, `-z`, `-9`, `-?`, `-y` to trigger the default case
3. **Edge cases**: 
   - No arguments
   - Flags without filename
   - Non-existent file
   - Multiple files
   - `--` separator
   - Mixed valid/invalid flags
4. **Different coverage data**: Simple and optimized compilation to test parsing of various .gcno/.gcda formats
5. **Error handling**: Verifies error messages for invalid flags

The script creates actual coverage data files by compiling and running test programs, then invokes `gcov-dump` with various argument combinations. Each test checks for expected output or error messages to ensure the specific code paths are executed.
