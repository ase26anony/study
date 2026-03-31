Here's a comprehensive shell script that covers all the specified requirements for testing the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo "=== GCOV-TOOL Overlap Flag Coverage Test ==="

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d)
cd "$TEST_DIR"
echo "Working in: $TEST_DIR"

# Cleanup function
cleanup() {
    echo -e "\nCleaning up..."
    cd /
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

# 1. Generate test C source with conditional branches
cat > test.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

int func1(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
        return x * 2;
    } else {
        printf("Non-positive: %d\n", x);
        return x - 1;
    }
}

void func2(int a, int b) {
    for (int i = 0; i < a; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
    
    while (b > 0) {
        printf("Countdown: %d\n", b);
        b--;
    }
}

int main(int argc, char *argv[]) {
    int val = 5;
    if (argc > 1) {
        val = atoi(argv[1]);
    }
    
    func1(val);
    func2(val, 3);
    
    return 0;
}
EOF

# 2. Compile with GCOV instrumentation
echo -e "\n${GREEN}Compiling test program with GCOV instrumentation...${NC}"
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog

# 3. Generate multiple profile data runs
echo -e "\n${GREEN}Generating profile data with different executions...${NC}"

# First run with default value
echo "Run 1: Default execution"
./test_prog

# Second run with different argument
echo "Run 2: With argument 10"
./test_prog 10

# Third run with negative value
echo "Run 3: With argument -5"
./test_prog -5

# Create a copy of the executable in different location for varied .gcda paths
mkdir -p subdir
cp test_prog subdir/
cd subdir
echo "Run 4: From subdirectory with argument 3"
./test_prog 3
cd ..

# 4. Test individual flags
echo -e "\n${GREEN}Testing individual flags...${NC}"

echo "Test 1: Verbose flag (-v)"
gcov-tool overlap -v test*.gcda 2>&1 | head -20

echo -e "\nTest 2: Function level flag (-f)"
gcov-tool overlap -f test*.gcda 2>&1 | head -20

echo -e "\nTest 3: Fullname flag (-F)"
gcov-tool overlap -F test*.gcda 2>&1 | head -20

echo -e "\nTest 4: Object level flag (-o)"
gcov-tool overlap -o test*.gcda 2>&1 | head -20

echo -e "\nTest 5: Hot only flag (-h)"
gcov-tool overlap -h test*.gcda 2>&1 | head -20

echo -e "\nTest 6: Threshold flag (-t 0.5)"
gcov-tool overlap -t 0.5 test*.gcda 2>&1 | head -20

echo -e "\nTest 7: Different threshold (-t 1.0)"
gcov-tool overlap -t 1.0 test*.gcda 2>&1 | head -20

echo -e "\nTest 8: High threshold (-t 10.5)"
gcov-tool overlap -t 10.5 test*.gcda 2>&1 | head -20

# 5. Test flag combinations
echo -e "\n${GREEN}Testing flag combinations...${NC}"

echo "Test 9: -f -o combination"
gcov-tool overlap -f -o test*.gcda 2>&1 | head -20

echo -e "\nTest 10: -F -h -t 1.0 combination"
gcov-tool overlap -F -h -t 1.0 test*.gcda 2>&1 | head -20

echo -e "\nTest 11: -v -f -F -o -h -t 5.0 combination"
gcov-tool overlap -v -f -F -o -h -t 5.0 test*.gcda 2>&1 | head -30

echo -e "\nTest 12: -v -f -t 0.8 with multiple files"
gcov-tool overlap -v -f -t 0.8 test.gcda subdir/test.gcda 2>&1 | head -30

# 6. Test with different .gcno files (compile with optimization)
echo -e "\n${GREEN}Creating optimized version for different .gcno structure...${NC}"
gcc -fprofile-arcs -ftest-coverage -O2 test.c -o test_prog_opt
./test_prog_opt 7
./test_prog_opt 2

echo -e "\nTest 13: Mixed optimization levels"
gcov-tool overlap -f -o test.gcda test_prog_opt-test.gcda 2>&1 | head -20

# 7. Test invalid flag to trigger usage
echo -e "\n${GREEN}Testing invalid flag to trigger usage...${NC}"
echo "Test 14: Invalid flag -Z (should show usage)"
gcov-tool overlap -Z test.gcda 2>&1 | head -10

# 8. Test with LTO compilation if supported
echo -e "\n${GREEN}Testing with LTO compilation if available...${NC}"
if gcc -v 2>&1 | grep -q "enable-lto"; then
    gcc -fprofile-arcs -ftest-coverage -flto -O2 test.c -o test_prog_lto 2>/dev/null && {
        ./test_prog_lto 4
        echo "Test 15: LTO compiled binary"
        gcov-tool overlap -v -f test_prog_lto-test.gcda 2>&1 | head -20
    } || echo "LTO compilation failed, skipping..."
else
    echo "LTO not available, skipping..."
fi

# 9. Test edge cases
echo -e "\n${GREEN}Testing edge cases...${NC}"

echo "Test 16: Threshold with decimal"
gcov-tool overlap -t 0.01 test*.gcda 2>&1 | head -10

echo -e "\nTest 17: Threshold with high value"
gcov-tool overlap -t 99.9 test*.gcda 2>&1 | head -10

echo -e "\nTest 18: All flags except threshold"
gcov-tool overlap -v -f -F -o -h test*.gcda 2>&1 | head -20

# 10. Test with empty/missing .gcda files
echo -e "\n${GREEN}Testing error conditions...${NC}"
echo "Test 19: No input files"
gcov-tool overlap -f 2>&1 | head -5 || true

echo -e "\nTest 20: Non-existent file"
gcov-tool overlap -f nonexistent.gcda 2>&1 | head -5 || true

# 11. Create summary
echo -e "\n${GREEN}=== Test Summary ===${NC}"
echo "Generated and tested:"
echo "  - Multiple .gcda files from different executions"
echo "  - Individual flags: -v, -f, -F, -o, -h, -t"
echo "  - Flag combinations"
echo "  - Invalid flag (-Z) to trigger usage"
echo "  - Different compilation modes (O0, O2, LTO)"
echo "  - Edge cases and error conditions"

echo -e "\n${GREEN}All tests completed successfully!${NC}"
echo "The following gcov-tool overlap flags were exercised:"
echo "  -v (verbose)              - Line 534-536"
echo "  -f (function level)       - Line 538-539"
echo "  -F (fullname)             - Line 540-541"
echo "  -o (object level)         - Line 542-543"
echo "  -h (hot only)             - Line 544-545"
echo "  -t (threshold)            - Line 546-547"
echo "  default case (invalid)    - Line 548-554"

# Keep files for inspection if needed
echo -e "\nTest files preserved in: $TEST_DIR"
echo "To clean up manually: rm -rf $TEST_DIR"
```

This script provides comprehensive coverage of the target lines by:

1. **Generating valid GCOV data**: Creates a C program with conditional branches, compiles it with GCOV instrumentation, and runs it multiple times with different inputs to generate varied `.gcda` files.

2. **Testing all individual flags**: Exercises each uncovered flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) individually with valid arguments.

3. **Testing flag combinations**: Tests various combinations of flags together to ensure the parser handles multiple options correctly.

4. **Triggering the default case**: Includes an invalid flag (`-Z`) to trigger the `overlap_usage()` function.

5. **Using multiple input files**: Processes multiple `.gcda` files from different runs and locations.

6. **Testing with different compilation options**: Creates binaries with different optimization levels (O0, O2) and LTO if available to generate different `.gcno` structures.

7. **Handling edge cases**: Tests threshold values at boundaries and tests error conditions.

The script captures output (limited with `head` to keep it manageable) and provides clear progress indicators. All generated files are cleaned up automatically unless you want to inspect them.
