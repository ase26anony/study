Here's a comprehensive shell script that systematically tests the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash
# test_gcov_tool.sh - Comprehensive test for gcov-tool overlap command

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== Testing gcov-tool overlap command ===${NC}"

# Clean up any previous test files
cleanup() {
    echo -e "${YELLOW}Cleaning up...${NC}"
    rm -f test.c test_prog test.gcda test.gcno test2.gcda merged.gcda \
          test_output*.txt 2>/dev/null || true
}
cleanup

# Step 1: Create a minimal C program for coverage testing
cat > test.c << 'EOF'
#include <stdio.h>

void function1(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else {
        printf("Non-positive: %d\n", x);
    }
}

void function2(int n) {
    for (int i = 0; i < n; i++) {
        printf("Iteration %d\n", i);
    }
}

int main() {
    function1(5);
    function1(-3);
    function2(3);
    return 0;
}
EOF

echo -e "${GREEN}Created test program${NC}"

# Step 2: Compile with coverage flags
echo -e "${YELLOW}Compiling test program with coverage...${NC}"
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog

if [ ! -f test_prog ]; then
    echo -e "${RED}Compilation failed!${NC}"
    exit 1
fi

echo -e "${GREEN}Compilation successful${NC}"

# Step 3: Run the program to generate initial .gcda file
echo -e "${YELLOW}Running test program to generate coverage data...${NC}"
./test_prog > /dev/null

if [ ! -f test.gcda ]; then
    echo -e "${RED}No .gcda file generated!${NC}"
    exit 1
fi

echo -e "${GREEN}Generated test.gcda${NC}"

# Step 4: Create a second .gcda file with different coverage
# Method 1: Run with different input (modify program slightly)
cat > test2.c << 'EOF'
#include <stdio.h>

void function1(int x) {
    if (x > 10) {  // Different threshold
        printf("Large positive: %d\n", x);
    } else {
        printf("Small or negative: %d\n", x);
    }
}

void function2(int n) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            printf("Even iteration %d\n", i);
        }
    }
}

int main() {
    function1(15);   // Different value
    function1(5);    // Different value
    function2(4);    // Different count
    return 0;
}
EOF

gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test_prog2
./test_prog2 > /dev/null
mv test.gcda test2.gcda 2>/dev/null || true
rm -f test_prog2 test2.c 2>/dev/null

# Method 2: If method 1 fails, create a copy with slight modification
if [ ! -f test2.gcda ]; then
    echo -e "${YELLOW}Creating second .gcda file by copying and modifying...${NC}"
    cp test.gcda test2.gcda
    # Touch the file to make it appear different
    touch test2.gcda
fi

echo -e "${GREEN}Created test2.gcda for comparison${NC}"

# Step 5: Test individual flags (covering switch cases)
echo -e "\n${YELLOW}=== Testing individual flags ===${NC}"

# Test verbose flag (-v) - line 534-536
echo -e "${GREEN}Testing -v flag...${NC}"
gcov-tool overlap -v test.gcda test2.gcda > test_output_v.txt 2>&1
echo "Exit code: $?"

# Test function-level overlap (-f) - line 537-539
echo -e "\n${GREEN}Testing -f flag...${NC}"
gcov-tool overlap -f test.gcda test2.gcda > test_output_f.txt 2>&1
echo "Exit code: $?"

# Test use full pathnames (-F) - line 540-542
echo -e "\n${GREEN}Testing -F flag...${NC}"
gcov-tool overlap -F test.gcda test2.gcda > test_output_F.txt 2>&1
echo "Exit code: $?"

# Test object-level overlap (-o) - line 543-545
echo -e "\n${GREEN}Testing -o flag...${NC}"
gcov-tool overlap -o test.gcda test2.gcda > test_output_o.txt 2>&1
echo "Exit code: $?"

# Test hot only (-h) - line 546-548
echo -e "\n${GREEN}Testing -h flag...${NC}"
gcov-tool overlap -h test.gcda test2.gcda > test_output_h.txt 2>&1
echo "Exit code: $?"

# Test hot threshold with argument (-t) - line 549-551
echo -e "\n${GREEN}Testing -t flag with argument...${NC}"
gcov-tool overlap -t 0.5 test.gcda test2.gcda > test_output_t.txt 2>&1
echo "Exit code: $?"

# Step 6: Test combined flags
echo -e "\n${YELLOW}=== Testing combined flags ===${NC}"

# Test multiple flags together
echo -e "${GREEN}Testing -v -f -F combination...${NC}"
gcov-tool overlap -v -f -F test.gcda test2.gcda > test_output_vfF.txt 2>&1
echo "Exit code: $?"

echo -e "\n${GREEN}Testing -f -o -h combination...${NC}"
gcov-tool overlap -f -o -h test.gcda test2.gcda > test_output_foh.txt 2>&1
echo "Exit code: $?"

echo -e "\n${GREEN}Testing -v -F -o -t 0.7 combination...${NC}"
gcov-tool overlap -v -F -o -t 0.7 test.gcda test2.gcda > test_output_vFot.txt 2>&1
echo "Exit code: $?"

# Test all flags together
echo -e "\n${GREEN}Testing all flags together...${NC}"
gcov-tool overlap -v -f -F -o -h -t 0.3 test.gcda test2.gcda > test_output_all.txt 2>&1
echo "Exit code: $?"

# Step 7: Test edge cases and error handling
echo -e "\n${YELLOW}=== Testing edge cases and error handling ===${NC}"

# Test -t without argument (should trigger error)
echo -e "${GREEN}Testing -t without argument (should fail)...${NC}"
gcov-tool overlap -t test.gcda test2.gcda > test_output_t_noarg.txt 2>&1
echo "Exit code: $?"

# Test -t with non-numeric argument
echo -e "\n${GREEN}Testing -t with non-numeric argument...${NC}"
gcov-tool overlap -t not_a_number test.gcda test2.gcda > test_output_t_nan.txt 2>&1
echo "Exit code: $?"

# Test invalid option (-z) to trigger default case (line 552-554)
echo -e "\n${GREEN}Testing invalid option -z (should trigger default case)...${NC}"
gcov-tool overlap -z test.gcda test2.gcda > test_output_invalid.txt 2>&1
echo "Exit code: $?"

# Test with same file twice
echo -e "\n${GREEN}Testing with same file twice...${NC}"
gcov-tool overlap -v test.gcda test.gcda > test_output_same.txt 2>&1
echo "Exit code: $?"

# Test with non-existent files
echo -e "\n${GREEN}Testing with non-existent files...${NC}"
gcov-tool overlap -v nonexistent1.gcda nonexistent2.gcda > test_output_nonexist.txt 2>&1
echo "Exit code: $?"

# Test with no arguments (should show usage)
echo -e "\n${GREEN}Testing with no arguments...${NC}"
gcov-tool overlap > test_output_noargs.txt 2>&1
echo "Exit code: $?"

# Test with only one file
echo -e "\n${GREEN}Testing with only one file...${NC}"
gcov-tool overlap -v test.gcda > test_output_onefile.txt 2>&1
echo "Exit code: $?"

# Step 8: Test different threshold values
echo -e "\n${YELLOW}=== Testing different threshold values ===${NC}"

for threshold in 0.0 0.1 0.5 0.9 1.0 1.5; do
    echo -e "${GREEN}Testing threshold: $threshold...${NC}"
    gcov-tool overlap -t $threshold test.gcda test2.gcda > /dev/null 2>&1
    echo "  Exit code: $?"
done

# Test negative threshold
echo -e "\n${GREEN}Testing negative threshold...${NC}"
gcov-tool overlap -t -0.5 test.gcda test2.gcda > test_output_neg.txt 2>&1
echo "Exit code: $?"

# Step 9: Test with merged .gcda file (optional)
echo -e "\n${YELLOW}=== Testing with merged coverage data ===${NC}"

# First merge two files
echo -e "${GREEN}Matching profiles first...${NC}"
gcov-tool match test.gcda test2.gcda merged.gcda 2>/dev/null || true

if [ -f merged.gcda ]; then
    echo -e "${GREEN}Testing overlap with merged file...${NC}"
    gcov-tool overlap -v test.gcda merged.gcda > test_output_merged.txt 2>&1
    echo "Exit code: $?"
fi

# Step 10: Summary
echo -e "\n${YELLOW}=== Test Summary ===${NC}"
echo "All tests completed. Check the test_output_*.txt files for detailed output."
echo "Coverage data for gcov-tool should now be available in its .gcda files."

# Cleanup (optional - comment out to keep test files for inspection)
# cleanup

echo -e "${GREEN}Done!${NC}"
```

This script provides comprehensive testing of the uncovered lines:

1. **Creates valid .gcda files** by compiling and running a simple C program with coverage
2. **Tests each individual flag** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) to hit each switch case
3. **Tests flag combinations** to ensure they work together
4. **Triggers the default case** with invalid option `-z`
5. **Tests edge cases**:
   - `-t` without argument
   - `-t` with non-numeric argument
   - Same file twice
   - Non-existent files
   - Various threshold values (including negative)
6. **Tests error conditions** that might exercise additional code paths

To use this script:
1. Ensure `gcov-tool` is built with coverage instrumentation
2. Make the script executable: `chmod +x test_gcov_tool.sh`
3. Run it: `./test_gcov_tool.sh`
4. The coverage data for `gcov-tool.cc` will be generated as `gcov-tool.gcda` (or similar, depending on the build)

The script captures output and exit codes for each test, allowing you to verify that each command-line option is properly processed.
