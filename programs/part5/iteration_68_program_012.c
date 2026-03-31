Here's a comprehensive shell script that tests the uncovered lines in `gcov-tool.cc` by systematically exercising the overlap command with various flag combinations:

```bash
#!/bin/bash
# test_gcov_tool.sh - Comprehensive test for gcov-tool overlap command parsing

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== Testing gcov-tool overlap command parsing ===${NC}"

# Clean up from previous runs
cleanup() {
    echo -e "\n${YELLOW}Cleaning up...${NC}"
    rm -f test.c test_prog test.gcda test.gcno test2.gcda merged.gcda
    rm -f *.gcov
}
trap cleanup EXIT

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

void func2(int n) {
    for (int i = 0; i < n; i++) {
        printf("Iteration %d\n", i);
    }
}

int main(int argc, char *argv[]) {
    int val = (argc > 1) ? atoi(argv[1]) : 5;
    
    func1(val);
    func2(val);
    
    return 0;
}
EOF

echo -e "${GREEN}Created test program${NC}"

# Step 2: Compile with coverage instrumentation
echo -e "${YELLOW}Compiling test program with coverage...${NC}"
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog

# Step 3: Generate first .gcda file
echo -e "${YELLOW}Generating first coverage data file...${NC}"
./test_prog 3
mv test.gcda test1.gcda

# Step 4: Generate second .gcda file with different coverage
echo -e "${YELLOW}Generating second coverage data file...${NC}"
./test_prog 7
mv test.gcda test2.gcda

# Step 5: Create a third .gcda by merging (for more interesting comparisons)
echo -e "${YELLOW}Creating merged coverage file...${NC}"
gcov-tool merge test1.gcda test2.gcda merged.gcda 2>/dev/null || true

echo -e "\n${GREEN}=== Starting gcov-tool overlap tests ===${NC}"

# Function to run gcov-tool and report
run_gcov_tool() {
    local description="$1"
    local cmd="$2"
    
    echo -e "\n${YELLOW}Test: $description${NC}"
    echo "Command: $cmd"
    
    # Run the command, capture output and exit code
    output=$(eval "$cmd" 2>&1)
    exit_code=$?
    
    if [ $exit_code -eq 0 ]; then
        echo -e "${GREEN}✓ Command succeeded${NC}"
        # Show first few lines of output if verbose
        if [ -n "$output" ]; then
            echo "Output (first 5 lines):"
            echo "$output" | head -5
        fi
    else
        echo -e "${RED}✗ Command failed with exit code $exit_code${NC}"
        if [ -n "$output" ]; then
            echo "Error output:"
            echo "$output" | head -10
        fi
    fi
}

# Test 1: Basic overlap with verbose flag (-v)
run_gcov_tool "Basic overlap with verbose flag" \
    "gcov-tool overlap -v test1.gcda test2.gcda"

# Test 2: Function-level overlap (-f)
run_gcov_tool "Function-level overlap" \
    "gcov-tool overlap -f test1.gcda test2.gcda"

# Test 3: Use full pathnames (-F)
run_gcov_tool "Overlap with full pathnames" \
    "gcov-tool overlap -F test1.gcda test2.gcda"

# Test 4: Object-level overlap (-o)
run_gcov_tool "Object-level overlap" \
    "gcov-tool overlap -o test1.gcda test2.gcda"

# Test 5: Hot only overlap (-h)
run_gcov_tool "Hot only overlap" \
    "gcov-tool overlap -h test1.gcda test2.gcda"

# Test 6: Hot threshold with floating point argument (-t)
run_gcov_tool "Hot threshold with 0.5" \
    "gcov-tool overlap -t 0.5 test1.gcda test2.gcda"

# Test 7: Hot threshold with different value (-t)
run_gcov_tool "Hot threshold with 0.1" \
    "gcov-tool overlap -t 0.1 test1.gcda test2.gcda"

# Test 8: Hot threshold with high value (-t)
run_gcov_tool "Hot threshold with 0.9" \
    "gcov-tool overlap -t 0.9 test1.gcda test2.gcda"

# Test 9: Combined flags (-f -F -o)
run_gcov_tool "Combined flags: function, fullname, object" \
    "gcov-tool overlap -f -F -o test1.gcda test2.gcda"

# Test 10: Combined with verbose and hot threshold
run_gcov_tool "Combined: verbose, hot threshold, hot only" \
    "gcov-tool overlap -v -t 0.3 -h test1.gcda test2.gcda"

# Test 11: All flags combined
run_gcov_tool "All flags combined" \
    "gcov-tool overlap -v -f -F -o -h -t 0.7 test1.gcda test2.gcda"

# Test 12: Same file twice (edge case)
run_gcov_tool "Same file twice" \
    "gcov-tool overlap -v test1.gcda test1.gcda"

# Test 13: Different combination with merged file
run_gcov_tool "With merged file" \
    "gcov-tool overlap -f -F merged.gcda test1.gcda"

# Test 14: Invalid option (should trigger default case -> overlap_usage())
echo -e "\n${YELLOW}Test: Invalid option (should show usage)${NC}"
echo "Command: gcov-tool overlap -z test1.gcda test2.gcda"
gcov-tool overlap -z test1.gcda test2.gcda 2>&1 | head -5

# Test 15: -t flag without argument (edge case)
echo -e "\n${YELLOW}Test: -t without argument (should error)${NC}"
echo "Command: gcov-tool overlap -t test1.gcda test2.gcda"
gcov-tool overlap -t test1.gcda test2.gcda 2>&1 | head -5

# Test 16: -t with non-numeric argument (edge case)
echo -e "\n${YELLOW}Test: -t with non-numeric argument${NC}"
echo "Command: gcov-tool overlap -t not_a_number test1.gcda test2.gcda"
gcov-tool overlap -t not_a_number test1.gcda test2.gcda 2>&1 | head -5

# Test 17: Non-existent files (error handling)
echo -e "\n${YELLOW}Test: Non-existent input files${NC}"
echo "Command: gcov-tool overlap -v nonexistent1.gcda nonexistent2.gcda"
gcov-tool overlap -v nonexistent1.gcda nonexistent2.gcda 2>&1 | head -5

# Test 18: Missing required arguments
echo -e "\n${YELLOW}Test: Missing required arguments${NC}"
echo "Command: gcov-tool overlap -v"
gcov-tool overlap -v 2>&1 | head -5

# Test 19: Only one file provided
echo -e "\n${YELLOW}Test: Only one file provided${NC}"
echo "Command: gcov-tool overlap -f test1.gcda"
gcov-tool overlap -f test1.gcda 2>&1 | head -5

echo -e "\n${GREEN}=== All tests completed ===${NC}"
echo -e "${YELLOW}Note: To collect coverage data for gcov-tool itself, ensure it was built with:${NC}"
echo -e "${YELLOW}  -fprofile-arcs -ftest-coverage${NC}"
echo -e "${YELLOW}and run this script in the appropriate environment.${NC}"
```

This script systematically tests all the uncovered lines:

1. **Creates valid GCDA files** by compiling and running a simple C program with different inputs
2. **Tests each individual flag** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`)
3. **Tests flag combinations** to ensure they work together
4. **Tests edge cases**:
   - `-t` without argument (line 534-554 handles `-t` with argument)
   - `-t` with non-numeric argument
   - Same file twice
   - Non-existent files
   - Invalid option (`-z`) to trigger the `default` case and `overlap_usage()`
5. **Uses different threshold values** for `-t` flag

To use this script:

1. Save it as `test_gcov_tool.sh`
2. Make it executable: `chmod +x test_gcov_tool.sh`
3. Ensure `gcov-tool` is in your PATH (preferably built with coverage instrumentation)
4. Run it: `./test_gcov_tool.sh`

The script will generate coverage data for the test program, then systematically invoke `gcov-tool overlap` with all the flag combinations that should trigger the uncovered switch cases in lines 534-554 of `gcov-tool.cc`.
