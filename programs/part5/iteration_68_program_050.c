I'll create a comprehensive test script that exercises all the uncovered lines in the `gcov-tool.cc` overlap command parsing. Here's the complete solution:

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
    rm -f test.c test_prog test.gcda test.gcno test2.gcda merged.gcda \
          test_output.txt error_output.txt
}

trap cleanup EXIT

# Step 1: Create a simple C program for coverage testing
echo -e "${GREEN}Creating test program...${NC}"
cat > test.c << 'EOF'
#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int main(int argc, char *argv[]) {
    int i, result;
    
    // Call factorial with different values
    for (i = 0; i < 5; i++) {
        result = factorial(i);
        printf("factorial(%d) = %d\n", i, result);
    }
    
    // Call fibonacci with different values
    for (i = 0; i < 5; i++) {
        result = fibonacci(i);
        printf("fibonacci(%d) = %d\n", i, result);
    }
    
    // Conditional execution based on argument
    if (argc > 1) {
        printf("Extra argument provided: %s\n", argv[1]);
        return 1;
    }
    
    return 0;
}
EOF

# Step 2: Compile with coverage flags
echo -e "${GREEN}Compiling test program with coverage...${NC}"
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog

# Step 3: Run the program to generate initial .gcda file
echo -e "${GREEN}Running test program (first run)...${NC}"
./test_prog > /dev/null

# Step 4: Create a second .gcda file with different coverage
echo -e "${GREEN}Creating second .gcda file with different coverage...${NC}"
# Run with argument to take different code path
./test_prog "test_argument" > /dev/null 2>&1 || true

# Copy and rename to create second file
cp test.gcda test2.gcda

# Step 5: Also create a merged .gcda for more interesting comparisons
echo -e "${GREEN}Creating merged .gcda file...${NC}"
if command -v gcov-tool &> /dev/null; then
    gcov-tool merge test.gcda test2.gcda merged.gcda 2>/dev/null || true
fi

# Function to run gcov-tool and capture output
run_gcov_tool() {
    local description="$1"
    shift
    
    echo -e "\n${YELLOW}Test: $description${NC}"
    echo "Command: gcov-tool overlap $*"
    
    if command -v gcov-tool &> /dev/null; then
        # Run the command, capturing stdout and stderr separately
        gcov-tool overlap "$@" > test_output.txt 2> error_output.txt
        local exit_code=$?
        
        echo "Exit code: $exit_code"
        if [ -s test_output.txt ]; then
            echo "Output:"
            cat test_output.txt
        fi
        if [ -s error_output.txt ]; then
            echo -e "${RED}Errors:${NC}"
            cat error_output.txt
        fi
    else
        echo -e "${RED}Error: gcov-tool not found in PATH${NC}"
        echo "Skipping test..."
    fi
}

# Step 6: Test individual flags (Target: lines 534-554)

# Test 1: -v flag (verbose)
run_gcov_tool "Individual flag: -v (verbose)" -v test.gcda test2.gcda

# Test 2: -f flag (function-level overlap)
run_gcov_tool "Individual flag: -f (function-level)" -f test.gcda test2.gcda

# Test 3: -F flag (use full pathnames)
run_gcov_tool "Individual flag: -F (full pathnames)" -F test.gcda test2.gcda

# Test 4: -o flag (object-level overlap)
run_gcov_tool "Individual flag: -o (object-level)" -o test.gcda test2.gcda

# Test 5: -h flag (hot only)
run_gcov_tool "Individual flag: -h (hot only)" -h test.gcda test2.gcda

# Test 6: -t flag with valid argument (hot threshold)
run_gcov_tool "Individual flag: -t with valid argument" -t 0.5 test.gcda test2.gcda

# Step 7: Test combined flags
run_gcov_tool "Combined flags: -v -f -F" -v -f -F test.gcda test2.gcda

run_gcov_tool "Combined flags: -o -h -t 0.7" -o -h -t 0.7 test.gcda test2.gcda

run_gcov_tool "All flags combined" -v -f -F -o -h -t 0.3 test.gcda test2.gcda

# Step 8: Test edge cases and error conditions

# Test 9: -t flag without argument (should trigger error)
run_gcov_tool "Edge case: -t without argument (should fail)" -t test.gcda test2.gcda

# Test 10: -t flag with non-numeric argument
run_gcov_tool "Edge case: -t with non-numeric argument" -t "not_a_number" test.gcda test2.gcda

# Test 11: Invalid flag (should trigger default case -> overlap_usage())
run_gcov_tool "Invalid flag: -z (should show usage)" -z test.gcda test2.gcda

# Test 12: Same file twice
run_gcov_tool "Same file twice" -v test.gcda test.gcda

# Test 13: Non-existent files
run_gcov_tool "Non-existent files" -v nonexistent1.gcda nonexistent2.gcda

# Test 14: Mixed valid and invalid flags
run_gcov_tool "Mixed valid and invalid flags" -v -z -f test.gcda test2.gcda

# Test 15: Different threshold values (boundary testing)
run_gcov_tool "Threshold boundary: 0.0" -t 0.0 test.gcda test2.gcda
run_gcov_tool "Threshold boundary: 1.0" -t 1.0 test.gcda test2.gcda
run_gcov_tool "Threshold boundary: negative" -t -0.5 test.gcda test2.gcda
run_gcov_tool "Threshold boundary: > 1.0" -t 1.5 test.gcda test2.gcda

# Test 16: With merged file (different coverage profile)
if [ -f merged.gcda ]; then
    run_gcov_tool "With merged .gcda file" -v -f merged.gcda test.gcda
fi

# Test 17: Multiple -t flags (last one should win)
run_gcov_tool "Multiple -t flags" -t 0.1 -t 0.9 test.gcda test2.gcda

# Test 18: Flags in different order
run_gcov_tool "Flags in different order" test.gcda -v -f test2.gcda

# Test 19: Very long threshold value
run_gcov_tool "Very long threshold value" -t 0.123456789 test.gcda test2.gcda

# Test 20: Empty threshold argument
run_gcov_tool "Empty threshold argument" -t "" test.gcda test2.gcda

echo -e "\n${GREEN}=== All tests completed ===${NC}"
echo -e "${YELLOW}Note: To collect coverage data for gcov-tool itself, ensure it was built with:${NC}"
echo -e "${YELLOW}  -fprofile-arcs -ftest-coverage${NC}"
echo -e "${YELLOW}and run this script in the appropriate environment.${NC}"
```

This script provides a comprehensive test suite that:

1. **Creates valid GCDA files**: Compiles and runs a test program to generate `.gcda` files with actual coverage data.

2. **Tests all individual flags**: Each of the uncovered switch cases (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually.

3. **Tests flag combinations**: Multiple flags are combined to ensure they work together correctly.

4. **Tests edge cases**:
   - `-t` without argument (triggers error handling)
   - `-t` with non-numeric argument
   - Invalid flag `-z` (triggers the `default` case and `overlap_usage()`)
   - Same file twice
   - Non-existent files
   - Boundary threshold values (0.0, 1.0, negative, >1.0)

5. **Provides proper execution flow**: The script follows the specified flow of compile-run-generate-test.

To use this script:

1. Save it as `test_gcov_tool_overlap.sh`
2. Make it executable: `chmod +x test_gcov_tool_overlap.sh`
3. Ensure `gcov-tool` is in your PATH (built with coverage instrumentation if you want to collect coverage on `gcov-tool.cc`)
4. Run it: `./test_gcov_tool_overlap.sh`

The script will generate coverage data for the test program, then systematically test all the command-line parsing logic in the overlap command of `gcov-tool`. Each test case is clearly labeled and the output shows what's being tested.
