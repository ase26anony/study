Looking at the uncovered lines in `gcov-tool.cc`, I'll create a comprehensive test script that exercises all the command-line options for the `overlap` command. Here's a shell script that systematically tests each case:

```bash
#!/bin/bash
# test_gcov_tool_overlap.sh
# Comprehensive test for gcov-tool overlap command options

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== Starting gcov-tool overlap command tests ===${NC}"

# Clean up from previous runs
cleanup() {
    echo -e "${YELLOW}Cleaning up...${NC}"
    rm -f test_prog test.c test.gcda test.gcno test2.gcda merged.gcda
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

void function2() {
    int x = 5;
    while (x > 0) {
        printf("Countdown: %d\n", x);
        x--;
    }
}

int main(int argc, char *argv[]) {
    function1();
    if (argc > 1) {
        function2();
    }
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

echo -e "${GREEN}Compiled test_prog${NC}"

# Step 3: Generate first .gcda file
echo -e "${YELLOW}Generating first coverage data...${NC}"
./test_prog > /dev/null

if [ ! -f test.gcda ]; then
    echo -e "${RED}Failed to generate test.gcda${NC}"
    exit 1
fi

echo -e "${GREEN}Generated test.gcda${NC}"

# Step 4: Generate second .gcda file with different coverage
echo -e "${YELLOW}Generating second coverage data...${NC}"
./test_prog different_argument > /dev/null

# Create a second .gcda file by copying and modifying
cp test.gcda test2.gcda

# Alternative: Use gcov-tool merge to create a different profile
echo -e "${YELLOW}Creating merged profile for comparison...${NC}"
if command -v gcov-tool &> /dev/null; then
    # Merge with itself to create a slightly different profile
    gcov-tool merge test.gcda test.gcda merged.gcda 2>/dev/null || true
    if [ -f merged.gcda ]; then
        cp merged.gcda test2.gcda
    fi
fi

if [ ! -f test2.gcda ]; then
    echo -e "${RED}Failed to create second .gcda file${NC}"
    exit 1
fi

echo -e "${GREEN}Generated test2.gcda${NC}"

# Step 5: Test individual flags
echo -e "\n${YELLOW}=== Testing individual flags ===${NC}"

# Test verbose flag (-v)
echo -e "${GREEN}Testing -v flag...${NC}"
gcov-tool overlap -v test.gcda test2.gcda 2>&1 | head -20

# Test function-level overlap (-f)
echo -e "\n${GREEN}Testing -f flag...${NC}"
gcov-tool overlap -f test.gcda test2.gcda 2>&1 | head -20

# Test use full pathnames (-F)
echo -e "\n${GREEN}Testing -F flag...${NC}"
gcov-tool overlap -F test.gcda test2.gcda 2>&1 | head -20

# Test object-level overlap (-o)
echo -e "\n${GREEN}Testing -o flag...${NC}"
gcov-tool overlap -o test.gcda test2.gcda 2>&1 | head -20

# Test hot only (-h)
echo -e "\n${GREEN}Testing -h flag...${NC}"
gcov-tool overlap -h test.gcda test2.gcda 2>&1 | head -20

# Test hot threshold with valid float (-t)
echo -e "\n${GREEN}Testing -t flag with valid float...${NC}"
gcov-tool overlap -t 0.5 test.gcda test2.gcda 2>&1 | head -20

# Test hot threshold with different float values
echo -e "\n${GREEN}Testing -t flag with 0.0...${NC}"
gcov-tool overlap -t 0.0 test.gcda test2.gcda 2>&1 | head -20

echo -e "\n${GREEN}Testing -t flag with 1.0...${NC}"
gcov-tool overlap -t 1.0 test.gcda test2.gcda 2>&1 | head -20

echo -e "\n${GREEN}Testing -t flag with 0.75...${NC}"
gcov-tool overlap -t 0.75 test.gcda test2.gcda 2>&1 | head -20

# Step 6: Test combined flags
echo -e "\n${YELLOW}=== Testing combined flags ===${NC}"

echo -e "${GREEN}Testing -v -f -F combination...${NC}"
gcov-tool overlap -v -f -F test.gcda test2.gcda 2>&1 | head -20

echo -e "\n${GREEN}Testing -f -o -h combination...${NC}"
gcov-tool overlap -f -o -h test.gcda test2.gcda 2>&1 | head -20

echo -e "\n${GREEN}Testing -v -F -o -h combination...${NC}"
gcov-tool overlap -v -F -o -h test.gcda test2.gcda 2>&1 | head -20

echo -e "\n${GREEN}Testing -f -F -o -t 0.3 combination...${NC}"
gcov-tool overlap -f -F -o -t 0.3 test.gcda test2.gcda 2>&1 | head -20

echo -e "\n${GREEN}Testing all flags combined...${NC}"
gcov-tool overlap -v -f -F -o -h -t 0.25 test.gcda test2.gcda 2>&1 | head -20

# Step 7: Test edge cases and error conditions
echo -e "\n${YELLOW}=== Testing edge cases and error conditions ===${NC}"

# Test -t without argument (should trigger error)
echo -e "${GREEN}Testing -t without argument (should fail)...${NC}"
gcov-tool overlap -t test.gcda test2.gcda 2>&1 | head -10 || true

# Test -t with non-numeric argument
echo -e "\n${GREEN}Testing -t with non-numeric argument...${NC}"
gcov-tool overlap -t not_a_number test.gcda test2.gcda 2>&1 | head -10 || true

# Test -t with negative number
echo -e "\n${GREEN}Testing -t with negative number...${NC}"
gcov-tool overlap -t -0.5 test.gcda test2.gcda 2>&1 | head -10 || true

# Test -t with very large number
echo -e "\n${GREEN}Testing -t with large number...${NC}"
gcov-tool overlap -t 999.999 test.gcda test2.gcda 2>&1 | head -10 || true

# Test same file twice
echo -e "\n${GREEN}Testing with same file twice...${NC}"
gcov-tool overlap -v test.gcda test.gcda 2>&1 | head -10 || true

# Test with non-existent files
echo -e "\n${GREEN}Testing with non-existent files...${NC}"
gcov-tool overlap -v nonexistent1.gcda nonexistent2.gcda 2>&1 | head -10 || true

# Test with one valid and one invalid file
echo -e "\n${GREEN}Testing with mixed valid/invalid files...${NC}"
gcov-tool overlap -v test.gcda nonexistent.gcda 2>&1 | head -10 || true

# Step 8: Test invalid option to trigger default case (overlap_usage)
echo -e "\n${YELLOW}=== Testing invalid option to trigger default case ===${NC}"

echo -e "${GREEN}Testing invalid option -z...${NC}"
gcov-tool overlap -z test.gcda test2.gcda 2>&1 | head -10 || true

echo -e "\n${GREEN}Testing invalid option --invalid...${NC}"
gcov-tool overlap --invalid test.gcda test2.gcda 2>&1 | head -10 || true

echo -e "\n${GREEN}Testing combination with invalid option...${NC}"
gcov-tool overlap -v -z -f test.gcda test2.gcda 2>&1 | head -10 || true

# Step 9: Test with different argument orders
echo -e "\n${YELLOW}=== Testing different argument orders ===${NC}"

echo -e "${GREEN}Testing flags after filenames...${NC}"
gcov-tool overlap test.gcda test2.gcda -v 2>&1 | head -10 || true

echo -e "\n${GREEN}Testing mixed flag order...${NC}"
gcov-tool overlap -f test.gcda -F test2.gcda -o 2>&1 | head -10 || true

echo -e "\n${YELLOW}=== All tests completed ===${NC}"

# Optional: Create a summary of what was tested
cat > test_summary.txt << 'EOF'
Test coverage for gcov-tool overlap command options:

Individual flags tested:
- -v (verbose)
- -f (function-level overlap)
- -F (use full pathnames)
- -o (object-level overlap)
- -h (hot only)
- -t (hot threshold with various float values)

Combinations tested:
- -v -f -F
- -f -o -h
- -v -F -o -h
- -f -F -o -t 0.3
- All flags combined

Edge cases tested:
- -t without argument
- -t with non-numeric argument
- -t with negative number
- -t with large number
- Same file twice
- Non-existent files
- Mixed valid/invalid files

Invalid options tested (to trigger default case):
- -z
- --invalid
- Combination with invalid option

Argument order variations tested.
EOF

echo -e "${GREEN}Test summary written to test_summary.txt${NC}"
```

This script provides comprehensive testing of the uncovered lines in `gcov-tool.cc`:

1. **Creates valid GCDA files**: Compiles a simple C program with coverage, runs it with different arguments to generate two `.gcda` files with potentially different coverage profiles.

2. **Tests all individual flags**: Each of the switch cases (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually.

3. **Tests flag combinations**: Multiple flags are combined to ensure they work together correctly.

4. **Tests edge cases for `-t`**: Includes tests for missing arguments, non-numeric arguments, negative numbers, and large numbers.

5. **Triggers the default case**: Tests invalid options (`-z`, `--invalid`) to trigger the `overlap_usage()` call.

6. **Tests error conditions**: Tests with same file, non-existent files, and mixed valid/invalid files.

7. **Tests argument order variations**: Ensures the parser handles different argument orders correctly.

To run this test with coverage collection on `gcov-tool` itself, you would need to:
1. Build `gcov-tool` with coverage flags: `g++ -fprofile-arcs -ftest-coverage -O0 gcov-tool.cc -o gcov-tool`
2. Ensure this instrumented `gcov-tool` is in your PATH
3. Run the test script: `bash test_gcov_tool_overlap.sh`
4. The coverage data for `gcov-tool` will be generated in `.gcda` files that can be processed with `gcov` or `lcov`.
