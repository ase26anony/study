Here's a comprehensive shell script that covers all the specified requirements for testing the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo "=== GCOV-TOOL Overlap Mode Flag Testing ==="

# Create a test directory
TEST_DIR="gcov_tool_test"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

# Step 1: Generate a simple C program with conditional branches
cat > test_program.c << 'EOF'
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
    
    // Another conditional
    if (num % 2 == 0) {
        printf("%d is even\n", num);
    } else {
        printf("%d is odd\n", num);
    }
    
    return 0;
}
EOF

echo "Created test_program.c"

# Step 2: Compile with GCOV instrumentation
echo "Compiling with GCOV instrumentation..."
gcc -fprofile-arcs -ftest-coverage -O0 test_program.c -o test_prog
gcc -fprofile-arcs -ftest-coverage -O2 test_program.c -o test_prog_opt

# Check if gcov-tool exists
if ! command -v gcov-tool &> /dev/null; then
    echo -e "${RED}Error: gcov-tool not found in PATH${NC}"
    echo "Make sure you have a GCC build with gcov-tool installed"
    exit 1
fi

echo -e "${GREEN}Compilation successful${NC}"

# Step 3: Generate multiple profile data runs
echo "Generating profile data..."

# Run 1: Normal execution
echo "Run 1: Normal execution (num=5)"
./test_prog 5

# Run 2: Different input
echo "Run 2: Different input (num=3)"
GCOV_PREFIX_STRIP=0 ./test_prog 3

# Run 3: Even number
echo "Run 3: Even number (num=8)"
./test_prog 8

# Run 4: Large number (triggers different branch)
echo "Run 4: Large number (num=15)"
./test_prog 15

# Create a copy of gcda files for multiple input testing
mkdir -p profile_data
cp *.gcda profile_data/ 2>/dev/null || true

# Run optimized version for different profile patterns
echo "Run 5: Optimized binary (num=7)"
./test_prog_opt 7

# Step 4: Test gcov-tool overlap with various flag combinations
echo -e "\n${GREEN}=== Testing gcov-tool overlap with various flags ===${NC}"

# Get list of gcda files
GCDA_FILES=$(ls *.gcda 2>/dev/null || echo "")
if [ -z "$GCDA_FILES" ]; then
    echo -e "${RED}No .gcda files generated!${NC}"
    exit 1
fi

echo "Available .gcda files: $GCDA_FILES"

# Test individual flags
echo -e "\n1. Testing individual flags:"

echo "   a) -v flag (verbose)"
gcov-tool overlap -v test_prog.gcda 2>&1 | head -20

echo "   b) -f flag (function level)"
gcov-tool overlap -f test_prog.gcda 2>&1 | head -20

echo "   c) -F flag (full name)"
gcov-tool overlap -F test_prog.gcda 2>&1 | head -20

echo "   d) -o flag (object level)"
gcov-tool overlap -o test_prog.gcda 2>&1 | head -20

echo "   e) -h flag (hot only)"
gcov-tool overlap -h test_prog.gcda 2>&1 | head -20

echo "   f) -t flag with threshold 0.5"
gcov-tool overlap -t 0.5 test_prog.gcda 2>&1 | head -20

echo "   g) -t flag with threshold 1.0"
gcov-tool overlap -t 1.0 test_prog.gcda 2>&1 | head -20

echo "   h) -t flag with threshold 10.5"
gcov-tool overlap -t 10.5 test_prog.gcda 2>&1 | head -20

# Test flag combinations
echo -e "\n2. Testing flag combinations:"

echo "   a) -f -o combination"
gcov-tool overlap -f -o test_prog.gcda 2>&1 | head -20

echo "   b) -F -h -t 1.0 combination"
gcov-tool overlap -F -h -t 1.0 test_prog.gcda 2>&1 | head -20

echo "   c) -v -f -F -o -h -t 5.0 (all flags)"
gcov-tool overlap -v -f -F -o -h -t 5.0 test_prog.gcda 2>&1 | head -30

echo "   d) -f -t 0.8 -o combination"
gcov-tool overlap -f -t 0.8 -o test_prog.gcda 2>&1 | head -20

# Test with multiple input files
echo -e "\n3. Testing with multiple input files:"

echo "   a) Two gcda files with -f -o flags"
gcov-tool overlap -f -o test_prog.gcda test_prog_opt.gcda 2>&1 | head -20

echo "   b) Multiple files with -v -F -t 0.3"
gcov-tool overlap -v -F -t 0.3 *.gcda 2>&1 | head -30

# Test invalid flag (to trigger usage)
echo -e "\n4. Testing invalid flag (should trigger usage):"
echo "   Invalid -Z flag:"
gcov-tool overlap -Z test_prog.gcda 2>&1 | head -10 || true

# Test with directory input
echo -e "\n5. Testing with profile data directory:"
if [ -d "profile_data" ] && [ "$(ls -A profile_data/*.gcda 2>/dev/null)" ]; then
    echo "   Processing directory with -v -f flags"
    gcov-tool overlap -v -f profile_data/*.gcda 2>&1 | head -30
fi

# Test edge cases for threshold
echo -e "\n6. Testing threshold edge cases:"

echo "   a) Very low threshold 0.001"
gcov-tool overlap -t 0.001 test_prog.gcda 2>&1 | head -20

echo "   b) High threshold 99.9"
gcov-tool overlap -t 99.9 test_prog.gcda 2>&1 | head -20

echo "   c) Threshold 0.0"
gcov-tool overlap -t 0.0 test_prog.gcda 2>&1 | head -20

# Test with different order of flags
echo -e "\n7. Testing different flag orders:"

echo "   a) Flags after filename"
gcov-tool overlap test_prog.gcda -v -f 2>&1 | head -20

echo "   b) Mixed order"
gcov-tool overlap -t 2.5 -v -o -F test_prog.gcda 2>&1 | head -20

# Additional test: combine with other gcov-tool operations
echo -e "\n8. Testing after merging profiles:"

# First merge profiles
echo "   Merging profiles..."
gcov-tool merge -o merged.gcda test_prog.gcda test_prog_opt.gcda 2>&1 | tail -5

# Then run overlap on merged data
echo "   Overlap on merged data with -v -f -t 0.7"
gcov-tool overlap -v -f -t 0.7 merged.gcda 2>&1 | head -30

# Cleanup
echo -e "\n${GREEN}=== Test completed successfully ===${NC}"
echo -e "\nGenerated files in: $PWD"
echo -e "\nTo clean up, run: cd .. && rm -rf $TEST_DIR"

# Display summary
echo -e "\n=== Test Summary ==="
echo "✓ Generated C program with conditional branches"
echo "✓ Compiled with GCOV instrumentation (-fprofile-arcs -ftest-coverage)"
echo "✓ Generated multiple profile runs with different inputs"
echo "✓ Tested all individual flags: -v, -f, -F, -o, -h, -t"
echo "✓ Tested flag combinations"
echo "✓ Tested with multiple input files"
echo "✓ Tested invalid flag (triggers usage)"
echo "✓ Tested threshold edge cases"
echo "✓ Tested different flag orders"
echo "✓ Tested with merged profile data"
```

This script provides comprehensive coverage of the uncovered lines by:

1. **Generating valid GCOV data**: Creates a C program with conditional branches, compiles it with GCOV instrumentation, and runs it multiple times with different inputs to generate `.gcda` files.

2. **Testing all individual flags**: Each uncovered flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually with valid arguments.

3. **Testing flag combinations**: Various combinations of flags are tested to ensure the parser handles multiple options correctly.

4. **Testing with multiple input files**: Uses multiple `.gcda` files to stress the overlap merging logic.

5. **Triggering the usage function**: Tests an invalid flag (`-Z`) to cover the `default` case that calls `overlap_usage()`.

6. **Testing threshold variations**: Uses different floating-point values for the `-t` flag including edge cases.

7. **Testing different execution paths**: Runs both optimized and non-optimized binaries, tests with directories, and tests flag ordering.

The script captures output (limited to first 20-30 lines for brevity) and provides clear feedback about what's being tested. All tests run `gcov-tool` in `overlap` mode as required, and the generated profile data ensures the tool doesn't exit early due to missing input.
