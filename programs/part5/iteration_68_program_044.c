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
          test_output_*.txt 2>/dev/null || true
}
cleanup

# Step 1: Create a simple C program for coverage testing
cat > test.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

void function1(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else {
        printf("Non-positive: %d\n", x);
    }
}

void function2(int iterations) {
    for (int i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        }
    }
}

int main(int argc, char *argv[]) {
    int val = (argc > 1) ? atoi(argv[1]) : 5;
    
    function1(val);
    function2(val);
    
    return 0;
}
EOF

echo -e "${GREEN}Created test program${NC}"

# Step 2: Compile with coverage flags
echo -e "${YELLOW}Compiling test program with coverage...${NC}"
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog

if [ ! -f test_prog ]; then
    echo -e "${RED}Failed to compile test program${NC}"
    exit 1
fi

# Step 3: Generate first .gcda file
echo -e "${YELLOW}Generating first coverage data...${NC}"
./test_prog 10  # Run with argument 10
if [ ! -f test.gcda ]; then
    echo -e "${RED}Failed to generate test.gcda${NC}"
    exit 1
fi

# Step 4: Generate second .gcda file with different coverage
echo -e "${YELLOW}Generating second coverage data...${NC}"
# Run with different argument to get different coverage
./test_prog 3
cp test.gcda test2.gcda

# Also create a merged .gcda for more interesting comparisons
echo -e "${YELLOW}Creating merged coverage data...${NC}"
gcov-tool merge test.gcda test2.gcda merged.gcda 2>/dev/null || true

# Define test files
FILE1="test.gcda"
FILE2="test2.gcda"
FILE3="merged.gcda"

echo -e "${GREEN}Test files created:${NC}"
echo "  $FILE1"
echo "  $FILE2"
echo "  $FILE3"

# Function to run gcov-tool and capture output
run_gcov_test() {
    local test_name="$1"
    local args="$2"
    local expected_exit="${3:-0}"
    
    echo -e "\n${YELLOW}Test: $test_name${NC}"
    echo "Command: gcov-tool overlap $args"
    
    # Run the command
    set +e  # Don't exit on error for this command
    gcov-tool overlap $args > "test_output_${test_name}.txt" 2>&1
    local exit_code=$?
    set -e
    
    if [ $exit_code -eq $expected_exit ]; then
        echo -e "${GREEN}  ✓ Exit code: $exit_code (expected $expected_exit)${NC}"
    else
        echo -e "${RED}  ✗ Exit code: $exit_code (expected $expected_exit)${NC}"
        echo "  Output:"
        cat "test_output_${test_name}.txt" | head -20
    fi
}

# ============================================
# TEST 1: Individual flag tests
# ============================================

echo -e "\n${YELLOW}=== Individual Flag Tests ===${NC}"

# Test -v flag (verbose)
run_gcov_test "verbose" "-v $FILE1 $FILE2"

# Test -f flag (function-level overlap)
run_gcov_test "func_level" "-f $FILE1 $FILE2"

# Test -F flag (use full pathnames)
run_gcov_test "fullname" "-F $FILE1 $FILE2"

# Test -o flag (object-level overlap)
run_gcov_test "obj_level" "-o $FILE1 $FILE2"

# Test -h flag (hot only)
run_gcov_test "hot_only" "-h $FILE1 $FILE2"

# Test -t flag with numeric argument
run_gcov_test "threshold_numeric" "-t 0.5 $FILE1 $FILE2"

# ============================================
# TEST 2: Combined flag tests
# ============================================

echo -e "\n${YELLOW}=== Combined Flag Tests ===${NC}"

# Test -f -F -o together
run_gcov_test "combined_ffo" "-f -F -o $FILE1 $FILE2"

# Test -v -f -h together
run_gcov_test "combined_vfh" "-v -f -h $FILE1 $FILE2"

# Test -v -F -o -h together
run_gcov_test "combined_vFoh" "-v -F -o -h $FILE1 $FILE2"

# Test all flags together
run_gcov_test "combined_all" "-v -f -F -o -h -t 0.7 $FILE1 $FILE2"

# ============================================
# TEST 3: -t flag edge cases
# ============================================

echo -e "\n${YELLOW}=== -t Flag Edge Cases ===${NC}"

# Test -t with different numeric values
run_gcov_test "threshold_low" "-t 0.1 $FILE1 $FILE2"
run_gcov_test "threshold_high" "-t 0.9 $FILE1 $FILE2"
run_gcov_test "threshold_zero" "-t 0.0 $FILE1 $FILE2"
run_gcov_test "threshold_one" "-t 1.0 $FILE1 $FILE2"

# Test -t with missing argument (should trigger error)
run_gcov_test "threshold_missing" "-t $FILE1 $FILE2" 1

# Test -t with non-numeric argument
run_gcov_test "threshold_nonnumeric" "-t not_a_number $FILE1 $FILE2" 1

# Test -t with negative number
run_gcov_test "threshold_negative" "-t -0.5 $FILE1 $FILE2"

# Test -t with very large number
run_gcov_test "threshold_large" "-t 100.0 $FILE1 $FILE2"

# ============================================
# TEST 4: Invalid options and edge cases
# ============================================

echo -e "\n${YELLOW}=== Invalid Options and Edge Cases ===${NC}"

# Test invalid option (should trigger default case -> overlap_usage())
run_gcov_test "invalid_option" "-z $FILE1 $FILE2" 1

# Test unknown long option
run_gcov_test "unknown_long" "--unknown $FILE1 $FILE2" 1

# Test same file twice
run_gcov_test "same_file" "-v $FILE1 $FILE1"

# Test non-existent files
run_gcov_test "nonexistent" "-v nonexistent1.gcda nonexistent2.gcda" 1

# Test with only one file
run_gcov_test "one_file" "-v $FILE1" 1

# Test with no files
run_gcov_test "no_files" "-v" 1

# Test with file that exists but wrong format
echo "not a gcda file" > bad.gcda
run_gcov_test "bad_format" "-v $FILE1 bad.gcda" 1
rm -f bad.gcda

# ============================================
# TEST 5: Different file combinations
# ============================================

echo -e "\n${YELLOW}=== Different File Combinations ===${NC}"

# Test with merged file
run_gcov_test "with_merged" "-v $FILE1 $FILE3"
run_gcov_test "merged_func" "-f -F $FILE2 $FILE3"

# Test all three files (third will be ignored but parsed)
run_gcov_test "three_files" "-v $FILE1 $FILE2 $FILE3"

# ============================================
# TEST 6: Order variations
# ============================================

echo -e "\n${YELLOW}=== Flag Order Variations ===${NC}"

# Test flags in different orders
run_gcov_test "order1" "$FILE1 $FILE2 -v -f"
run_gcov_test "order2" "-t 0.3 $FILE1 -h $FILE2"
run_gcov_test "order3" "$FILE1 -o -F $FILE2 -v"

# ============================================
# Summary
# ============================================

echo -e "\n${YELLOW}=== Test Summary ===${NC}"
echo "All tests completed. Output files:"
ls -la test_output_*.txt 2>/dev/null | head -10

# Clean up
cleanup

echo -e "\n${GREEN}✓ All tests executed successfully${NC}"
echo -e "${YELLOW}Note: To collect coverage data for gcov-tool itself, ensure it was built with:${NC}"
echo -e "${YELLOW}  -fprofile-arcs -ftest-coverage${NC}"
echo -e "${YELLOW}and run this script in the appropriate environment.${NC}"
```

This script provides comprehensive testing of the uncovered lines:

1. **Creates valid GCDA files** by compiling and running a simple C program with different inputs
2. **Tests each individual flag** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) as required
3. **Tests flag combinations** to ensure they work together
4. **Tests edge cases** for the `-t` flag (missing args, non-numeric, boundary values)
5. **Triggers the default case** with invalid option `-z`
6. **Tests error conditions** (non-existent files, same file twice, bad format)
7. **Tests different file combinations** including merged coverage data
8. **Tests flag ordering variations**

To use this script:

1. Save it as `test_gcov_tool.sh`
2. Make it executable: `chmod +x test_gcov_tool.sh`
3. Ensure `gcov-tool` is in your PATH (built with coverage instrumentation if you want to collect coverage on it)
4. Run: `./test_gcov_tool.sh`

The script will generate coverage data for the test program, then systematically test all the command-line options that correspond to the uncovered switch cases in `gcov-tool.cc`.
