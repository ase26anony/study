Here's a comprehensive shell script that tests the uncovered lines in `gcov-tool.cc` by systematically exercising the overlap command with various flag combinations:

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

# Clean up from previous runs
cleanup() {
    echo -e "${YELLOW}Cleaning up...${NC}"
    rm -f test_prog test.c test.gcda test.gcno test2.gcda merged.gcda \
          test_output_*.txt error_output_*.txt
}

cleanup

# Step 1: Create a simple C program for coverage testing
cat > test.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

void func1(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else {
        printf("Non-positive: %d\n", x);
    }
}

void func2(int iterations) {
    for (int i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
}

int main(int argc, char *argv[]) {
    int val = (argc > 1) ? atoi(argv[1]) : 5;
    
    func1(val);
    func2(val);
    
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

# Step 3: Run program with different inputs to generate varied coverage data
echo -e "${YELLOW}Generating coverage data...${NC}"

# First run with input 3
./test_prog 3 > /dev/null 2>&1
if [ -f test.gcda ]; then
    echo -e "${GREEN}Generated test.gcda (first run)${NC}"
    cp test.gcda test.gcda.orig
else
    echo -e "${RED}Failed to generate test.gcda${NC}"
    exit 1
fi

# Second run with input 10 to get different coverage
./test_prog 10 > /dev/null 2>&1
if [ -f test.gcda ]; then
    echo -e "${GREEN}Generated test.gcda (second run)${NC}"
    cp test.gcda test2.gcda
else
    echo -e "${RED}Failed to generate second gcda file${NC}"
    exit 1
fi

# Restore first gcda for testing
cp test.gcda.orig test.gcda

echo -e "${GREEN}Created two different .gcda files for comparison${NC}"

# Step 4: Test individual flags (Requirement 3.1)
echo -e "\n${YELLOW}=== Testing individual flags ===${NC}"

# Test -v (verbose) - hits line 534-536
echo -e "${GREEN}Testing -v flag...${NC}"
gcov-tool overlap -v test.gcda test2.gcda > test_output_v.txt 2>&1
echo "Exit code: $?"

# Test -f (function-level overlap) - hits line 538
echo -e "${GREEN}Testing -f flag...${NC}"
gcov-tool overlap -f test.gcda test2.gcda > test_output_f.txt 2>&1
echo "Exit code: $?"

# Test -F (use full pathnames) - hits line 540
echo -e "${GREEN}Testing -F flag...${NC}"
gcov-tool overlap -F test.gcda test2.gcda > test_output_F.txt 2>&1
echo "Exit code: $?"

# Test -o (object-level overlap) - hits line 542
echo -e "${GREEN}Testing -o flag...${NC}"
gcov-tool overlap -o test.gcda test2.gcda > test_output_o.txt 2>&1
echo "Exit code: $?"

# Test -h (hot only) - hits line 544
echo -e "${GREEN}Testing -h flag...${NC}"
gcov-tool overlap -h test.gcda test2.gcda > test_output_h.txt 2>&1
echo "Exit code: $?"

# Test -t with argument (hot threshold) - hits line 546
echo -e "${GREEN}Testing -t 0.5 flag...${NC}"
gcov-tool overlap -t 0.5 test.gcda test2.gcda > test_output_t1.txt 2>&1
echo "Exit code: $?"

# Step 5: Test combined flags (Requirement 3.2)
echo -e "\n${YELLOW}=== Testing combined flags ===${NC}"

# Test -f -F -o combination
echo -e "${GREEN}Testing -f -F -o combination...${NC}"
gcov-tool overlap -f -F -o test.gcda test2.gcda > test_output_ffo.txt 2>&1
echo "Exit code: $?"

# Test -v -h -t combination
echo -e "${GREEN}Testing -v -h -t 0.7 combination...${NC}"
gcov-tool overlap -v -h -t 0.7 test.gcda test2.gcda > test_output_vht.txt 2>&1
echo "Exit code: $?"

# Test all flags together
echo -e "${GREEN}Testing all flags together...${NC}"
gcov-tool overlap -v -f -F -o -h -t 0.3 test.gcda test2.gcda > test_output_all.txt 2>&1
echo "Exit code: $?"

# Step 6: Test edge cases and error handling (Requirement 4)
echo -e "\n${YELLOW}=== Testing edge cases and error handling ===${NC}"

# Test -t without argument (should trigger error)
echo -e "${GREEN}Testing -t without argument...${NC}"
gcov-tool overlap -t test.gcda test2.gcda > error_output_t_noarg.txt 2>&1 || true
echo "Exit code: $?"

# Test -t with non-numeric argument
echo -e "${GREEN}Testing -t with non-numeric argument...${NC}"
gcov-tool overlap -t not_a_number test.gcda test2.gcda > error_output_t_nan.txt 2>&1 || true
echo "Exit code: $?"

# Test with same input file twice
echo -e "${GREEN}Testing with same input file twice...${NC}"
gcov-tool overlap -v test.gcda test.gcda > test_output_same.txt 2>&1
echo "Exit code: $?"

# Test with non-existent files
echo -e "${GREEN}Testing with non-existent files...${NC}"
gcov-tool overlap -v nonexistent1.gcda nonexistent2.gcda > error_output_nonexist.txt 2>&1 || true
echo "Exit code: $?"

# Test invalid option (should trigger default case and overlap_usage) - hits line 548-550
echo -e "${GREEN}Testing invalid option -z (should trigger default case)...${NC}"
gcov-tool overlap -z test.gcda test2.gcda > error_output_invalid.txt 2>&1 || true
echo "Exit code: $?"

# Test another invalid option
echo -e "${GREEN}Testing invalid option --invalid...${NC}"
gcov-tool overlap --invalid test.gcda test2.gcda > error_output_invalid2.txt 2>&1 || true
echo "Exit code: $?"

# Test boundary values for -t
echo -e "${GREEN}Testing -t with boundary values...${NC}"
for threshold in 0.0 0.1 0.5 0.9 1.0 1.5; do
    echo -n "  Testing -t $threshold: "
    gcov-tool overlap -t $threshold test.gcda test2.gcda > /dev/null 2>&1 && echo "OK" || echo "Failed (exit: $?)"
done

# Test negative threshold
echo -e "${GREEN}Testing -t with negative value...${NC}"
gcov-tool overlap -t -0.5 test.gcda test2.gcda > test_output_t_neg.txt 2>&1 || true
echo "Exit code: $?"

# Step 7: Test with merged gcda file (alternative second file)
echo -e "\n${YELLOW}=== Testing with merged gcda file ===${NC}"

# Create a merged gcda file using gcov-tool merge
echo -e "${GREEN}Creating merged gcda file...${NC}"
gcov-tool merge test.gcda test2.gcda merged.gcda > /dev/null 2>&1

if [ -f merged.gcda ]; then
    echo -e "${GREEN}Testing overlap with merged file...${NC}"
    gcov-tool overlap -v -f -F test.gcda merged.gcda > test_output_merged.txt 2>&1
    echo "Exit code: $?"
fi

# Step 8: Summary
echo -e "\n${YELLOW}=== Test Summary ===${NC}"
echo "Individual flag tests completed: -v, -f, -F, -o, -h, -t"
echo "Combined flag tests completed: -f -F -o, -v -h -t, all flags"
echo "Edge case tests completed: missing argument, non-numeric, same file, non-existent files"
echo "Invalid option test completed (should trigger default case)"

# List generated test files
echo -e "\n${YELLOW}Generated test output files:${NC}"
ls -la test_output_*.txt error_output_*.txt 2>/dev/null || echo "No output files found"

# Show sample outputs
echo -e "\n${YELLOW}Sample output from verbose mode:${NC}"
head -5 test_output_v.txt 2>/dev/null || echo "No verbose output"

echo -e "\n${YELLOW}Sample error from invalid option:${NC}"
head -5 error_output_invalid.txt 2>/dev/null || echo "No error output"

# Cleanup
cleanup

echo -e "\n${GREEN}=== All tests completed successfully ===${NC}"
```

This script systematically tests all the uncovered lines in the switch statement:

1. **Creates valid GCDA files**: Compiles and runs a simple C program twice with different inputs to generate two different `.gcda` files.

2. **Tests individual flags**: Each switch case (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually.

3. **Tests combined flags**: Multiple flags are combined to ensure they work together.

4. **Tests edge cases**:
   - `-t` without argument (triggers getopt error handling)
   - `-t` with non-numeric argument
   - Same input file twice
   - Non-existent files
   - Invalid option `-z` (triggers the `default` case calling `overlap_usage()`)

5. **Tests boundary conditions**: Various threshold values including 0.0, 1.0, and negative values.

To use this script:
1. Ensure `gcov-tool` is built with coverage instrumentation (`-fprofile-arcs -ftest-coverage`)
2. Make the script executable: `chmod +x test_gcov_tool.sh`
3. Run it: `./test_gcov_tool.sh`

The script will generate coverage data for `gcov-tool` itself when run in an environment where the instrumented `gcov-tool` binary is used. Each test case exercises different paths through the switch statement, ensuring the uncovered lines are executed.
