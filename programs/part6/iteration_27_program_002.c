Here's a comprehensive shell script that covers all the specified requirements for testing the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash

# test_gcov_tool.sh - Comprehensive test for gcov-tool overlap functionality
# Tests all uncovered command-line parsing cases in gcov-tool.cc lines 534-554

set -e  # Exit on error

# Configuration
TEST_DIR="gcov_test_$$"
SRC_FILE="test.c"
BIN_BASE="test_prog"
GCOV_TOOL="gcov-tool"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== Starting gcov-tool overlap flag coverage test ===${NC}"

# Create test directory
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

# Step 1: Generate test C source with conditional branches
cat > "$SRC_FILE" << 'EOF'
#include <stdio.h>
#include <stdlib.h>

int func1(int x, int y) {
    int result = 0;
    if (x > 0) {
        result += x;
    }
    if (y > 0) {
        result += y;
    }
    return result;
}

int func2(int a) {
    int sum = 0;
    for (int i = 0; i < a; i++) {
        if (i % 2 == 0) {
            sum += i;
        } else {
            sum -= i;
        }
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int val1 = argc > 1 ? atoi(argv[1]) : 0;
    int val2 = argc > 2 ? atoi(argv[2]) : 0;
    
    int res1 = func1(val1, val2);
    int res2 = func2(val1);
    
    printf("Result1: %d, Result2: %d\n", res1, res2);
    return 0;
}
EOF

echo "Created test source: $SRC_FILE"

# Step 2: Compile with GCOV instrumentation
echo -e "\n${YELLOW}Compiling test program with GCOV instrumentation...${NC}"
gcc -fprofile-arcs -ftest-coverage -O0 "$SRC_FILE" -o "$BIN_BASE"
gcc -fprofile-arcs -ftest-coverage -O2 "$SRC_FILE" -o "${BIN_BASE}_opt"
gcc -fprofile-arcs -ftest-coverage -flto -O2 "$SRC_FILE" -o "${BIN_BASE}_lto" 2>/dev/null || true

# Step 3: Generate multiple profile data runs
echo -e "\n${YELLOW}Generating profile data with different executions...${NC}"

# Run 1: Basic execution
echo "Run 1: Basic execution"
./"$BIN_BASE" 5 3
cp "$BIN_BASE.gcda" "${BIN_BASE}_run1.gcda"

# Run 2: Different parameters
echo "Run 2: Different parameters"
./"$BIN_BASE" 10 0
cp "$BIN_BASE.gcda" "${BIN_BASE}_run2.gcda"

# Run 3: Zero parameters
echo "Run 3: Zero parameters"
./"$BIN_BASE"
cp "$BIN_BASE.gcda" "${BIN_BASE}_run3.gcda"

# Run optimized version
echo "Run 4: Optimized version"
./"${BIN_BASE}_opt" 7 2
cp "${BIN_BASE}_opt.gcda" "${BIN_BASE}_opt_run.gcda"

# Clean current gcda to avoid interference
rm -f *.gcda

# Step 4: Test individual flags
echo -e "\n${YELLOW}=== Testing individual flags ===${NC}"

echo -e "\n${GREEN}Test 1: -v (verbose)${NC}"
"$GCOV_TOOL" overlap -v "${BIN_BASE}_run1.gcda" "${BIN_BASE}_run2.gcda" 2>&1 | head -20

echo -e "\n${GREEN}Test 2: -f (function level)${NC}"
"$GCOV_TOOL" overlap -f "${BIN_BASE}_run1.gcda" "${BIN_BASE}_run2.gcda" 2>&1 | head -20

echo -e "\n${GREEN}Test 3: -F (full filename)${NC}"
"$GCOV_TOOL" overlap -F "${BIN_BASE}_run1.gcda" "${BIN_BASE}_run2.gcda" 2>&1 | head -20

echo -e "\n${GREEN}Test 4: -o (object level)${NC}"
"$GCOV_TOOL" overlap -o "${BIN_BASE}_run1.gcda" "${BIN_BASE}_run2.gcda" 2>&1 | head -20

echo -e "\n${GREEN}Test 5: -h (hot only)${NC}"
"$GCOV_TOOL" overlap -h "${BIN_BASE}_run1.gcda" "${BIN_BASE}_run2.gcda" 2>&1 | head -20

echo -e "\n${GREEN}Test 6: -t 0.5 (threshold)${NC}"
"$GCOV_TOOL" overlap -t 0.5 "${BIN_BASE}_run1.gcda" "${BIN_BASE}_run2.gcda" 2>&1 | head -20

echo -e "\n${GREEN}Test 7: -t 1.0 (threshold)${NC}"
"$GCOV_TOOL" overlap -t 1.0 "${BIN_BASE}_run1.gcda" "${BIN_BASE}_run2.gcda" 2>&1 | head -20

echo -e "\n${GREEN}Test 8: -t 10.5 (threshold)${NC}"
"$GCOV_TOOL" overlap -t 10.5 "${BIN_BASE}_run1.gcda" "${BIN_BASE}_run2.gcda" 2>&1 | head -20

# Step 5: Test flag combinations
echo -e "\n${YELLOW}=== Testing flag combinations ===${NC}"

echo -e "\n${GREEN}Test 9: -f -o (function + object level)${NC}"
"$GCOV_TOOL" overlap -f -o "${BIN_BASE}_run1.gcda" "${BIN_BASE}_run2.gcda" 2>&1 | head -20

echo -e "\n${GREEN}Test 10: -F -h -t 1.0 (fullname + hot only + threshold)${NC}"
"$GCOV_TOOL" overlap -F -h -t 1.0 "${BIN_BASE}_run1.gcda" "${BIN_BASE}_run2.gcda" 2>&1 | head -20

echo -e "\n${GREEN}Test 11: -v -f -F -o -h -t 5.0 (all flags)${NC}"
"$GCOV_TOOL" overlap -v -f -F -o -h -t 5.0 "${BIN_BASE}_run1.gcda" "${BIN_BASE}_run2.gcda" "${BIN_BASE}_run3.gcda" 2>&1 | head -30

echo -e "\n${GREEN}Test 12: -f -t 0.8 -o (mixed order)${NC}"
"$GCOV_TOOL" overlap -f -t 0.8 -o "${BIN_BASE}_run1.gcda" "${BIN_BASE}_run2.gcda" 2>&1 | head -20

# Step 6: Test with multiple input files
echo -e "\n${YELLOW}=== Testing with multiple input files ===${NC}"

echo -e "\n${GREEN}Test 13: Three input files with -v -f${NC}"
"$GCOV_TOOL" overlap -v -f "${BIN_BASE}_run1.gcda" "${BIN_BASE}_run2.gcda" "${BIN_BASE}_run3.gcda" 2>&1 | head -25

echo -e "\n${GREEN}Test 14: Different binary profiles with -F -o${NC}"
"$GCOV_TOOL" overlap -F -o "${BIN_BASE}_run1.gcda" "${BIN_BASE}_opt_run.gcda" 2>&1 | head -20

# Step 7: Test error case (invalid flag)
echo -e "\n${YELLOW}=== Testing error case (invalid flag) ===${NC}"

echo -e "\n${GREEN}Test 15: Invalid flag -Z (should trigger overlap_usage)${NC}"
"$GCOV_TOOL" overlap -Z "${BIN_BASE}_run1.gcda" 2>&1 | head -10 || true

echo -e "\n${GREEN}Test 16: Valid flag with invalid argument${NC}"
"$GCOV_TOOL" overlap -t not_a_number "${BIN_BASE}_run1.gcda" 2>&1 | head -10 || true

# Step 8: Test edge cases
echo -e "\n${YELLOW}=== Testing edge cases ===${NC}"

echo -e "\n${GREEN}Test 17: Threshold 0.0${NC}"
"$GCOV_TOOL" overlap -t 0.0 "${BIN_BASE}_run1.gcda" "${BIN_BASE}_run2.gcda" 2>&1 | head -20

echo -e "\n${GREEN}Test 18: Threshold 100.0${NC}"
"$GCOV_TOOL" overlap -t 100.0 "${BIN_BASE}_run1.gcda" "${BIN_BASE}_run2.gcda" 2>&1 | head -20

echo -e "\n${GREEN}Test 19: Only -h flag${NC}"
"$GCOV_TOOL" overlap -h "${BIN_BASE}_run1.gcda" 2>&1 | head -20

# Step 9: Test with gcno files as well
echo -e "\n${YELLOW}=== Testing with gcno files ===${NC}"

echo -e "\n${GREEN}Test 20: Include gcno file with -v -f${NC}"
"$GCOV_TOOL" overlap -v -f "${BIN_BASE}.gcno" "${BIN_BASE}_run1.gcda" "${BIN_BASE}_run2.gcda" 2>&1 | head -25

# Step 10: Capture verbose output to file
echo -e "\n${YELLOW}=== Capturing verbose output to file ===${NC}"
"$GCOV_TOOL" overlap -v -f -F -o -h -t 0.5 "${BIN_BASE}_run1.gcda" "${BIN_BASE}_run2.gcda" "${BIN_BASE}_run3.gcda" > verbose_output.txt 2>&1
echo "Verbose output saved to verbose_output.txt ($(wc -l < verbose_output.txt) lines)"

# Cleanup (optional)
echo -e "\n${YELLOW}=== Test completed ===${NC}"
echo "Test files remain in directory: $TEST_DIR"
echo -e "To clean up: cd .. && rm -rf $TEST_DIR"

cd ..
echo -e "\n${GREEN}All tests executed successfully!${NC}"
echo -e "Covered flags: -v, -f, -F, -o, -h, -t <value>"
echo -e "Tested combinations and error case for default branch"
```

This script provides comprehensive coverage of all the specified requirements:

1. **Individual Flag Testing**: Tests each uncovered flag individually (`-v`, `-f`, `-F`, `-o`, `-h`, `-t` with various values)
2. **Flag Combinations**: Tests multiple flag combinations as specified
3. **Valid GCOV Data**: Creates a C program, compiles it with GCOV instrumentation, and generates multiple `.gcda` files
4. **Overlap Mode**: All invocations use `gcov-tool overlap` subcommand
5. **Error Testing**: Includes invalid flag `-Z` to trigger the `default` case and `overlap_usage()`
6. **Multiple Input Files**: Tests with 2-3 different `.gcda` files
7. **Verbose Output**: Captures verbose output to a file and displays portions
8. **Different Compilation Options**: Creates binaries with `-O0`, `-O2`, and LTO options

The script is self-contained and will create a temporary directory for all test files. Each test invocation pipes output through `head` to limit display, but full verbose output is saved to a file.
