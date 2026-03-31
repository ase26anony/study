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

# Step 1: Generate a simple C program with conditional branches
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
    int num = 5;
    
    if (argc > 1) {
        num = atoi(argv[1]);
    }
    
    printf("Factorial of %d: %d\n", num, factorial(num));
    
    if (num < 10) {
        printf("Fibonacci of %d: %d\n", num, fibonacci(num));
    } else {
        printf("Skipping Fibonacci for large number %d\n", num);
    }
    
    return 0;
}
EOF

# Step 2: Compile with GCOV instrumentation
echo -e "\n${GREEN}Compiling test program with GCOV instrumentation...${NC}"
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog

# Step 3: Generate multiple profile data runs
echo -e "\n${GREEN}Generating profile data with different runs...${NC}"

# Run 1: Default execution
echo "Run 1: Default (num=5)"
./test_prog

# Run 2: Different input
echo "Run 2: With argument (num=3)"
./test_prog 3

# Run 3: Another different input
echo "Run 3: With argument (num=7)"
./test_prog 7

# Create a copy of .gcda files for multiple input testing
cp test.gcda test_run1.gcda
cp test.gcda test_run2.gcda

# Step 4: Test individual flags
echo -e "\n${GREEN}Testing individual flags...${NC}"

# Test -v flag (verbose)
echo "Testing: -v flag"
gcov-tool overlap -v test.gcda 2>&1 | head -20

# Test -f flag (function level)
echo -e "\nTesting: -f flag"
gcov-tool overlap -f test.gcda 2>&1 | head -20

# Test -F flag (fullname)
echo -e "\nTesting: -F flag"
gcov-tool overlap -F test.gcda 2>&1 | head -20

# Test -o flag (object level)
echo -e "\nTesting: -o flag"
gcov-tool overlap -o test.gcda 2>&1 | head -20

# Test -h flag (hot only)
echo -e "\nTesting: -h flag"
gcov-tool overlap -h test.gcda 2>&1 | head -20

# Test -t flag with threshold (requires argument)
echo -e "\nTesting: -t flag with threshold 0.5"
gcov-tool overlap -t 0.5 test.gcda 2>&1 | head -20

echo -e "\nTesting: -t flag with threshold 1.0"
gcov-tool overlap -t 1.0 test.gcda 2>&1 | head -20

echo -e "\nTesting: -t flag with threshold 10.5"
gcov-tool overlap -t 10.5 test.gcda 2>&1 | head -20

# Step 5: Test flag combinations
echo -e "\n${GREEN}Testing flag combinations...${NC}"

# Combination 1: -f -o
echo "Testing: -f -o combination"
gcov-tool overlap -f -o test.gcda 2>&1 | head -20

# Combination 2: -F -h -t 1.0
echo -e "\nTesting: -F -h -t 1.0 combination"
gcov-tool overlap -F -h -t 1.0 test.gcda 2>&1 | head -20

# Combination 3: -v -f -F -o -h -t 5.0
echo -e "\nTesting: -v -f -F -o -h -t 5.0 combination"
gcov-tool overlap -v -f -F -o -h -t 5.0 test.gcda 2>&1 | head -30

# Combination 4: -f -t 0.8 with multiple files
echo -e "\nTesting: -f -t 0.8 with multiple .gcda files"
gcov-tool overlap -f -t 0.8 test_run1.gcda test_run2.gcda 2>&1 | head -20

# Combination 5: -v -F -o with multiple files
echo -e "\nTesting: -v -F -o with multiple .gcda files"
gcov-tool overlap -v -F -o test_run1.gcda test_run2.gcda 2>&1 | head -30

# Step 6: Test invalid flag to trigger usage
echo -e "\n${GREEN}Testing invalid flag to trigger overlap_usage()...${NC}"
echo "Testing: Invalid -Z flag (should show usage)"
gcov-tool overlap -Z test.gcda 2>&1 | head -10

# Step 7: Additional comprehensive tests
echo -e "\n${GREEN}Additional comprehensive tests...${NC}"

# Test with all flags and multiple files
echo "Testing: All flags with multiple files and different thresholds"
for threshold in 0.1 0.5 1.0 5.0 50.0 99.9; do
    echo "  Threshold: $threshold"
    gcov-tool overlap -v -f -F -o -h -t "$threshold" test_run1.gcda test_run2.gcda 2>&1 | tail -5
done

# Test edge cases for -t flag
echo -e "\nTesting edge cases for -t flag:"
echo "  Very small threshold: 0.001"
gcov-tool overlap -t 0.001 test.gcda 2>&1 | tail -3

echo "  Very large threshold: 1000.0"
gcov-tool overlap -t 1000.0 test.gcda 2>&1 | tail -3

echo "  Integer threshold: 2"
gcov-tool overlap -t 2 test.gcda 2>&1 | tail -3

# Step 8: Test with different compilation options
echo -e "\n${GREEN}Testing with differently compiled programs...${NC}"

# Compile with optimization
gcc -fprofile-arcs -ftest-coverage -O2 test.c -o test_prog_opt
./test_prog_opt 4
./test_prog_opt 6

echo "Testing overlap with O2 optimized binary:"
gcov-tool overlap -v -f test.gcda 2>&1 | tail -5

# Compile with LTO (if supported)
if gcc -fprofile-arcs -ftest-coverage -flto -O2 test.c -o test_prog_lto 2>/dev/null; then
    ./test_prog_lto 3
    echo -e "\nTesting overlap with LTO binary:"
    gcov-tool overlap -v -o test.gcda 2>&1 | tail -5
fi

# Step 9: Cleanup (optional)
echo -e "\n${GREEN}Test completed successfully!${NC}"
echo "Generated files in: $TEST_DIR"
echo "To clean up, run: rm -rf $TEST_DIR"

# List generated files for verification
echo -e "\nGenerated files:"
ls -la *.gcda *.gcno test.c test_prog* 2>/dev/null || true

cd -
```

This script provides comprehensive coverage of all the specified requirements:

1. **Individual Flag Testing**: Tests each uncovered flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) individually with valid arguments for `-t`.

2. **Flag Combinations**: Tests various combinations of flags including complex ones like `-v -f -F -o -h -t 5.0`.

3. **Invalid Flag**: Tests `-Z` to trigger the `default` case and `overlap_usage()`.

4. **Multiple Input Files**: Creates multiple `.gcda` files and tests with them.

5. **Valid GCOV Data**: Generates actual GCOV data by compiling and running an instrumented C program.

6. **Different Threshold Values**: Tests various threshold values including edge cases.

7. **Different Compilation Options**: Tests with `-O0`, `-O2`, and `-flto` if supported.

8. **Verbose Output Handling**: Captures and displays output from `-v` flag.

The script is self-contained and creates a temporary directory for all test files. It can be run directly to exercise all the uncovered parsing logic in `gcov-tool.cc`.
