Looking at the uncovered lines in `gcov-tool.cc`, I need to create a comprehensive test that exercises the command-line argument parsing for the `overlap` command. Here's a shell script that systematically tests all the uncovered switch cases:

```bash
#!/bin/bash
# test_gcov_tool_overlap.sh - Comprehensive test for gcov-tool overlap command

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== Starting gcov-tool overlap command tests ===${NC}"

# Clean up any previous test files
cleanup() {
    echo -e "${YELLOW}Cleaning up...${NC}"
    rm -f test_prog test.c test.gcda test.gcno test2.gcda merged.gcda
    rm -f *.gcov
}

trap cleanup EXIT

# Step 1: Create a simple C program for coverage testing
cat > test.c << 'EOF'
#include <stdio.h>

void function1() {
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
}

void function2(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else if (x < 0) {
        printf("Negative: %d\n", x);
    } else {
        printf("Zero\n");
    }
}

int main() {
    printf("Running test program...\n");
    function1();
    function2(5);
    function2(-3);
    function2(0);
    return 0;
}
EOF

echo -e "${GREEN}Created test.c${NC}"

# Step 2: Compile with coverage flags
echo -e "${YELLOW}Compiling test program with coverage...${NC}"
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog

if [ ! -f test_prog ]; then
    echo -e "${RED}Failed to compile test program${NC}"
    exit 1
fi

echo -e "${GREEN}Compiled test_prog successfully${NC}"

# Step 3: Run the program to generate initial .gcda file
echo -e "${YELLOW}Running test program to generate coverage data...${NC}"
./test_prog > /dev/null

if [ ! -f test.gcda ]; then
    echo -e "${RED}Failed to generate test.gcda${NC}"
    exit 1
fi

echo -e "${GREEN}Generated test.gcda${NC}"

# Step 4: Create a second .gcda file with different coverage
# We'll run the program with different behavior by modifying environment
echo -e "${YELLOW}Creating second coverage data file...${NC}"

# Method 1: Copy and modify the .gcda file slightly
cp test.gcda test2.gcda

# Method 2: Alternatively, we could merge with itself to create variation
# This creates a different coverage profile
echo -e "${YELLOW}Creating merged.gcda for more variation...${NC}"
if command -v gcov-tool &> /dev/null; then
    gcov-tool merge test.gcda test.gcda merged.gcda 2>/dev/null || true
    if [ -f merged.gcda ]; then
        # Use merged.gcda as our second file
        cp merged.gcda test2.gcda
    fi
fi

if [ ! -f test2.gcda ]; then
    echo -e "${RED}Failed to create second .gcda file${NC}"
    exit 1
fi

echo -e "${GREEN}Created test2.gcda${NC}"

# Step 5: Test individual flag cases from the uncovered switch block
echo -e "\n${YELLOW}=== Testing individual flag cases ===${NC}"

# Test case 'v': verbose mode
echo -e "${GREEN}Testing -v flag (verbose)...${NC}"
gcov-tool overlap -v test.gcda test2.gcda 2>&1 | head -20

# Test case 'f': function-level overlap
echo -e "\n${GREEN}Testing -f flag (function-level overlap)...${NC}"
gcov-tool overlap -f test.gcda test2.gcda 2>&1 | head -20

# Test case 'F': use full pathnames
echo -e "\n${GREEN}Testing -F flag (use full pathnames)...${NC}"
gcov-tool overlap -F test.gcda test2.gcda 2>&1 | head -20

# Test case 'o': object-level overlap
echo -e "\n${GREEN}Testing -o flag (object-level overlap)...${NC}"
gcov-tool overlap -o test.gcda test2.gcda 2>&1 | head -20

# Test case 'h': hot only
echo -e "\n${GREEN}Testing -h flag (hot only)...${NC}"
gcov-tool overlap -h test.gcda test2.gcda 2>&1 | head -20

# Test case 't': hot threshold with numeric argument
echo -e "\n${GREEN}Testing -t flag with numeric argument (0.5)...${NC}"
gcov-tool overlap -t 0.5 test.gcda test2.gcda 2>&1 | head -20

# Test case 't': hot threshold with different numeric argument
echo -e "\n${GREEN}Testing -t flag with different numeric argument (0.75)...${NC}"
gcov-tool overlap -t 0.75 test.gcda test2.gcda 2>&1 | head -20

# Step 6: Test combined flag cases
echo -e "\n${YELLOW}=== Testing combined flag cases ===${NC}"

# Test multiple flags together
echo -e "${GREEN}Testing -v -f -F flags combined...${NC}"
gcov-tool overlap -v -f -F test.gcda test2.gcda 2>&1 | head -20

echo -e "\n${GREEN}Testing -f -o -h flags combined...${NC}"
gcov-tool overlap -f -o -h test.gcda test2.gcda 2>&1 | head -20

echo -e "\n${GREEN}Testing -v -F -o -t 0.3 flags combined...${NC}"
gcov-tool overlap -v -F -o -t 0.3 test.gcda test2.gcda 2>&1 | head -20

# Test all flags together
echo -e "\n${GREEN}Testing all flags combined (-v -f -F -o -h -t 0.6)...${NC}"
gcov-tool overlap -v -f -F -o -h -t 0.6 test.gcda test2.gcda 2>&1 | head -20

# Step 7: Test edge cases and error handling
echo -e "\n${YELLOW}=== Testing edge cases and error handling ===${NC}"

# Test default case: invalid option (should trigger overlap_usage())
echo -e "${GREEN}Testing invalid option -z (should trigger usage)...${NC}"
gcov-tool overlap -z test.gcda test2.gcda 2>&1 | head -10 || true

# Test -t flag without argument (edge case)
echo -e "\n${GREEN}Testing -t flag without argument (edge case)...${NC}"
gcov-tool overlap -t test.gcda test2.gcda 2>&1 | head -10 || true

# Test -t flag with non-numeric argument
echo -e "\n${GREEN}Testing -t flag with non-numeric argument...${NC}"
gcov-tool overlap -t not_a_number test.gcda test2.gcda 2>&1 | head -10 || true

# Test with same file twice
echo -e "\n${GREEN}Testing with same input file twice...${NC}"
gcov-tool overlap -v test.gcda test.gcda 2>&1 | head -10 || true

# Test with non-existent files
echo -e "\n${GREEN}Testing with non-existent files...${NC}"
gcov-tool overlap -v nonexistent1.gcda nonexistent2.gcda 2>&1 | head -10 || true

# Test with no arguments (should show usage)
echo -e "\n${GREEN}Testing with no arguments...${NC}"
gcov-tool overlap 2>&1 | head -10 || true

# Test with only one file argument
echo -e "\n${GREEN}Testing with only one file argument...${NC}"
gcov-tool overlap -v test.gcda 2>&1 | head -10 || true

# Test with too many arguments
echo -e "\n${GREEN}Testing with too many arguments...${NC}"
gcov-tool overlap -v test.gcda test2.gcda extra_arg.gcda 2>&1 | head -10 || true

# Step 8: Test different order of arguments
echo -e "\n${YELLOW}=== Testing different argument orders ===${NC}"

echo -e "${GREEN}Testing flags after filenames...${NC}"
gcov-tool overlap test.gcda test2.gcda -v 2>&1 | head -10 || true

echo -e "\n${GREEN}Testing flags interspersed with filenames...${NC}"
gcov-tool overlap -v test.gcda -f test2.gcda 2>&1 | head -10 || true

# Step 9: Test with different threshold values (boundary conditions)
echo -e "\n${YELLOW}=== Testing threshold boundary conditions ===${NC}"

echo -e "${GREEN}Testing -t 0.0 (minimum)...${NC}"
gcov-tool overlap -t 0.0 test.gcda test2.gcda 2>&1 | head -5 || true

echo -e "\n${GREEN}Testing -t 1.0 (maximum)...${NC}"
gcov-tool overlap -t 1.0 test.gcda test2.gcda 2>&1 | head -5 || true

echo -e "\n${GREEN}Testing -t 1.5 (above maximum)...${NC}"
gcov-tool overlap -t 1.5 test.gcda test2.gcda 2>&1 | head -5 || true

echo -e "\n${GREEN}Testing -t -0.1 (negative)...${NC}"
gcov-tool overlap -t -0.1 test.gcda test2.gcda 2>&1 | head -5 || true

# Step 10: Test with empty .gcda files
echo -e "\n${YELLOW}=== Testing with special .gcda files ===${NC}"

# Create an empty .gcda file
touch empty.gcda
echo -e "${GREEN}Testing with empty .gcda file...${NC}"
gcov-tool overlap -v empty.gcda test.gcda 2>&1 | head -10 || true

# Create a corrupted .gcda file
echo "not a valid gcda file" > corrupt.gcda
echo -e "\n${GREEN}Testing with corrupted .gcda file...${NC}"
gcov-tool overlap -v corrupt.gcda test.gcda 2>&1 | head -10 || true

echo -e "\n${YELLOW}=== All tests completed ===${NC}"
echo -e "${GREEN}Check the coverage report to verify all switch cases were hit.${NC}"
```

This comprehensive test script:

1. **Creates valid GCDA files**: Compiles and runs a simple C program to generate `.gcda` files
2. **Tests individual flags**: Exercises each switch case (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`)
3. **Tests flag combinations**: Tests multiple flags together to ensure they work in combination
4. **Triggers the default case**: Uses `-z` (invalid option) to trigger `overlap_usage()`
5. **Tests edge cases**:
   - `-t` without argument
   - `-t` with non-numeric argument
   - Same file twice
   - Non-existent files
   - Boundary threshold values
   - Empty/corrupted files
6. **Tests argument ordering**: Different placements of flags and filenames

To use this script:

1. Ensure `gcov-tool` is built with coverage instrumentation
2. Make the script executable: `chmod +x test_gcov_tool_overlap.sh`
3. Run it: `./test_gcov_tool_overlap.sh`
4. Check the coverage report for `gcov-tool.cc` to verify lines 534-554 are covered

The script captures output with `head` to limit verbosity and uses `|| true` to continue on errors, ensuring all test cases are attempted regardless of previous failures.
