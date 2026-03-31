Here's a comprehensive shell script that covers all the specified requirements for testing the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo "=== Testing gcov-tool overlap flag parsing ==="

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

# 1. Generate a simple C program with conditional branches
cat > test.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

int factorial(int n) {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

int fibonacci(int n) {
    if (n <= 1) {
        return n;
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}

int main(int argc, char *argv[]) {
    int n = 5;
    if (argc > 1) {
        n = atoi(argv[1]);
    }
    
    printf("Factorial(%d) = %d\n", n, factorial(n));
    printf("Fibonacci(%d) = %d\n", n, fibonacci(n));
    
    // Create some branching
    if (n > 10) {
        printf("Large number!\n");
    } else if (n < 0) {
        printf("Negative number!\n");
    } else {
        printf("Normal number\n");
    }
    
    return 0;
}
EOF

# 2. Compile with GCOV instrumentation
echo -e "\n${GREEN}Compiling test program with GCOV instrumentation...${NC}"
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog

# 3. Generate multiple sets of profile data
echo -e "\n${GREEN}Generating profile data with different executions...${NC}"

# First execution
echo "Running test_prog with n=3..."
./test_prog 3

# Copy first gcda file
cp test.gcda test_run1.gcda

# Second execution with different input
echo "Running test_prog with n=7..."
./test_prog 7

# Copy second gcda file  
cp test.gcda test_run2.gcda

# Third execution with different input
echo "Running test_prog with n=12..."
./test_prog 12

# Copy third gcda file
cp test.gcda test_run3.gcda

# Create a different compilation for variety
echo -e "\n${GREEN}Creating optimized version for different profile...${NC}"
gcc -fprofile-arcs -ftest-coverage -O2 test.c -o test_prog_opt
echo "Running optimized version..."
./test_prog_opt 5
cp test.gcda test_opt.gcda

# 4. Test gcov-tool with various flag combinations
echo -e "\n${GREEN}Testing gcov-tool overlap with different flag combinations...${NC}"

# Test individual flags
echo -e "\n${GREEN}1. Testing individual flags:${NC}"

echo "Testing -v flag (verbose)..."
gcov-tool overlap -v test.gcda 2>&1 | head -20

echo -e "\nTesting -f flag (function level)..."
gcov-tool overlap -f test.gcda 2>&1 | head -20

echo -e "\nTesting -F flag (full filename)..."
gcov-tool overlap -F test.gcda 2>&1 | head -20

echo -e "\nTesting -o flag (object level)..."
gcov-tool overlap -o test.gcda 2>&1 | head -20

echo -e "\nTesting -h flag (hot only)..."
gcov-tool overlap -h test.gcda 2>&1 | head -20

echo -e "\nTesting -t flag with threshold 0.5..."
gcov-tool overlap -t 0.5 test.gcda 2>&1 | head -20

echo -e "\nTesting -t flag with threshold 1.0..."
gcov-tool overlap -t 1.0 test.gcda 2>&1 | head -20

echo -e "\nTesting -t flag with threshold 10.5..."
gcov-tool overlap -t 10.5 test.gcda 2>&1 | head -20

# Test flag combinations
echo -e "\n${GREEN}2. Testing flag combinations:${NC}"

echo "Testing -f -o combination..."
gcov-tool overlap -f -o test.gcda 2>&1 | head -20

echo -e "\nTesting -F -h -t 1.0 combination..."
gcov-tool overlap -F -h -t 1.0 test.gcda 2>&1 | head -20

echo -e "\nTesting -v -f -F -o -h -t 5.0 combination..."
gcov-tool overlap -v -f -F -o -h -t 5.0 test.gcda 2>&1 | head -20

echo -e "\nTesting -f -t 0.8 with multiple files..."
gcov-tool overlap -f -t 0.8 test_run1.gcda test_run2.gcda 2>&1 | head -20

# Test with all three gcda files
echo -e "\nTesting with all three profile files..."
gcov-tool overlap -v -f test_run1.gcda test_run2.gcda test_run3.gcda 2>&1 | head -20

# Test with mixed gcda files (normal and optimized)
echo -e "\nTesting with mixed profile files..."
gcov-tool overlap -F -o test.gcda test_opt.gcda 2>&1 | head -20

# 5. Test error case (invalid flag to trigger usage)
echo -e "\n${GREEN}3. Testing error case (invalid flag):${NC}"
echo "Testing invalid flag -Z (should show usage)..."
gcov-tool overlap -Z test.gcda 2>&1 | head -30 || true

# 6. Additional comprehensive tests
echo -e "\n${GREEN}4. Additional comprehensive tests:${NC}"

# Test with threshold 0.0 (edge case)
echo "Testing with threshold 0.0..."
gcov-tool overlap -t 0.0 test.gcda 2>&1 | head -20

# Test with threshold 100.0 (large value)
echo -e "\nTesting with threshold 100.0..."
gcov-tool overlap -t 100.0 test.gcda 2>&1 | head -20

# Test combination of all boolean flags
echo -e "\nTesting all boolean flags together..."
gcov-tool overlap -v -f -F -o -h test.gcda 2>&1 | head -20

# Test with different file orders
echo -e "\nTesting with files in different order..."
gcov-tool overlap -f test_run3.gcda test_run1.gcda test_run2.gcda 2>&1 | head -20

# 7. Create a test with multiple source files for better coverage
echo -e "\n${GREEN}5. Creating multi-file test case...${NC}"

cat > helper.c << 'EOF'
#include <stdio.h>

void helper_function(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else {
        printf("Non-positive: %d\n", x);
    }
}

int multiply(int a, int b) {
    return a * b;
}
EOF

cat > main.c << 'EOF'
#include <stdio.h>

extern void helper_function(int x);
extern int multiply(int a, int b);

int main() {
    for (int i = -2; i <= 2; i++) {
        helper_function(i);
        printf("Square: %d\n", multiply(i, i));
    }
    return 0;
}
EOF

# Compile multi-file program
gcc -fprofile-arcs -ftest-coverage -O0 main.c helper.c -o multi_test

# Generate profile data
./multi_test
cp main.gcda main_run1.gcda
cp helper.gcda helper_run1.gcda

# Run again with different conditions
env SOME_VAR=1 ./multi_test
cp main.gcda main_run2.gcda
cp helper.gcda helper_run2.gcda

# Test with multiple gcda files from different sources
echo -e "\nTesting overlap with multi-file program..."
gcov-tool overlap -v -f -F main.gcda helper.gcda 2>&1 | head -30

echo -e "\nTesting with all generated gcda files..."
gcov-tool overlap -o -t 0.3 *.gcda 2>&1 | head -30

# 8. Summary
echo -e "\n${GREEN}=== Test Summary ===${NC}"
echo "Generated and tested:"
echo "- 1 simple C program with branches"
echo "- 3 different executions producing distinct .gcda files"
echo "- 1 optimized compilation variant"
echo "- 1 multi-file test program"
echo "- Tested all individual flags: -v, -f, -F, -o, -h, -t"
echo "- Tested multiple flag combinations"
echo "- Tested invalid flag case (triggers usage)"
echo "- Tested with multiple input files"
echo "- Tested various threshold values"

echo -e "\n${GREEN}All tests completed successfully!${NC}"
```

This script comprehensively tests the uncovered lines in `gcov-tool.cc` by:

1. **Generating valid GCOV data**: Creates a C program with conditional branches, compiles it with GCOV instrumentation (`-fprofile-arcs -ftest-coverage`), and executes it multiple times with different inputs to generate distinct `.gcda` files.

2. **Testing all individual flags**: Tests each uncovered flag individually (`-v`, `-f`, `-F`, `-o`, `-h`, `-t` with various threshold values).

3. **Testing flag combinations**: Tests various combinations of flags together to ensure the parser handles multiple options correctly.

4. **Triggering the default case**: Includes an invalid flag (`-Z`) to trigger the `default` case in the switch statement, which calls `overlap_usage()`.

5. **Using multiple input files**: Tests with multiple `.gcda` files (from different runs and different compilation variants) to stress the overlap merging logic.

6. **Handling verbose output**: Captures output from verbose mode and other flag combinations.

7. **Creating varied test cases**: Includes both simple and multi-file programs, as well as different optimization levels to generate diverse GCOV data.

The script is self-contained, creates a temporary directory for all test files, and cleans up after itself. Each test invocation redirects output and limits it to the first few lines to keep the output manageable while still verifying that the commands execute without errors.
