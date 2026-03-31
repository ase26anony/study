Here's a comprehensive shell script that covers all the specified requirements for testing the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo "=== GCOV-TOOL Overlap Flag Coverage Test ==="

# Create a test directory
TEST_DIR="gcov_tool_test_$(date +%s)"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

# Create a simple C program with conditional branches
cat > test.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

int func1(int x) {
    if (x > 0) {
        return x * 2;
    } else {
        return x - 1;
    }
}

int func2(int a, int b) {
    for (int i = 0; i < a; i++) {
        b += i;
    }
    return b;
}

int main(int argc, char *argv[]) {
    int val = (argc > 1) ? atoi(argv[1]) : 0;
    
    int r1 = func1(val);
    int r2 = func2(val, 10);
    
    printf("Result: %d\n", r1 + r2);
    return 0;
}
EOF

echo "Created test.c"

# Check if gcov-tool exists
if ! command -v gcov-tool &> /dev/null; then
    echo -e "${RED}Error: gcov-tool not found in PATH${NC}"
    echo "Please ensure GCC with gcov-tool is installed"
    exit 1
fi

# Check if we have a suitable GCC
if ! command -v gcc &> /dev/null; then
    echo -e "${RED}Error: gcc not found${NC}"
    exit 1
fi

# Compile the test program with coverage instrumentation
echo "Compiling test program with coverage instrumentation..."
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog

# Create multiple .gcda files with different execution paths
echo "Generating profile data..."

# Run 1: Normal execution
echo "  Run 1: val=5"
./test_prog 5

# Copy .gcda files for first profile set
mkdir -p profile_set1
cp *.gcda profile_set1/ 2>/dev/null || true

# Remove .gcda files for fresh run
rm -f *.gcda

# Run 2: Different input
echo "  Run 2: val=0"
./test_prog 0

# Copy .gcda files for second profile set
mkdir -p profile_set2
cp *.gcda profile_set2/ 2>/dev/null || true

# Run 3: Another different input
echo "  Run 3: val=-3"
./test_prog -3

# Copy .gcda files for third profile set
mkdir -p profile_set3
cp *.gcda profile_set3/ 2>/dev/null || true

# Also compile with optimization for different profile patterns
echo "Compiling optimized version..."
gcc -fprofile-arcs -ftest-coverage -O2 test.c -o test_prog_opt

# Run optimized version
echo "  Run optimized: val=10"
./test_prog_opt 10

# Copy .gcda files for optimized profile set
mkdir -p profile_set_opt
cp *.gcda profile_set_opt/ 2>/dev/null || true

echo -e "\n=== Testing individual flags ==="

# Test verbose flag (-v)
echo "Testing: -v flag"
gcov-tool overlap -v test.gcda 2>&1 | head -20

# Test function level flag (-f)
echo -e "\nTesting: -f flag"
gcov-tool overlap -f test.gcda 2>&1 | head -20

# Test fullname flag (-F)
echo -e "\nTesting: -F flag"
gcov-tool overlap -F test.gcda 2>&1 | head -20

# Test object level flag (-o)
echo -e "\nTesting: -o flag"
gcov-tool overlap -o test.gcda 2>&1 | head -20

# Test hot only flag (-h)
echo -e "\nTesting: -h flag"
gcov-tool overlap -h test.gcda 2>&1 | head -20

# Test threshold flag with argument (-t)
echo -e "\nTesting: -t 0.5 flag"
gcov-tool overlap -t 0.5 test.gcda 2>&1 | head -20

echo -e "\nTesting: -t 1.0 flag"
gcov-tool overlap -t 1.0 test.gcda 2>&1 | head -20

echo -e "\nTesting: -t 10.5 flag"
gcov-tool overlap -t 10.5 test.gcda 2>&1 | head -20

echo -e "\n=== Testing flag combinations ==="

# Test combination 1: -f -o
echo "Testing: -f -o combination"
gcov-tool overlap -f -o test.gcda 2>&1 | head -20

# Test combination 2: -F -h -t 1.0
echo -e "\nTesting: -F -h -t 1.0 combination"
gcov-tool overlap -F -h -t 1.0 test.gcda 2>&1 | head -20

# Test combination 3: -v -f -F -o -h -t 5.0
echo -e "\nTesting: -v -f -F -o -h -t 5.0 combination"
gcov-tool overlap -v -f -F -o -h -t 5.0 test.gcda 2>&1 | head -20

# Test combination 4: -f -t 0.8 with multiple files
echo -e "\nTesting: -f -t 0.8 with multiple .gcda files"
gcov-tool overlap -f -t 0.8 profile_set1/*.gcda profile_set2/*.gcda 2>&1 | head -20

echo -e "\n=== Testing with multiple input files ==="

# Test with all profile sets
echo "Testing with all profile sets..."
gcov-tool overlap -f -o profile_set1/*.gcda profile_set2/*.gcda profile_set3/*.gcda 2>&1 | head -30

# Test with optimized profile
echo -e "\nTesting with optimized profile..."
gcov-tool overlap -F -h profile_set_opt/*.gcda 2>&1 | head -20

echo -e "\n=== Testing error cases ==="

# Test invalid flag to trigger overlap_usage() (default case)
echo "Testing invalid flag -Z (should trigger usage)"
gcov-tool overlap -Z test.gcda 2>&1 | head -30

# Test missing argument for -t flag
echo -e "\nTesting missing argument for -t flag"
gcov-tool overlap -t test.gcda 2>&1 | head -20

# Test with non-existent file
echo -e "\nTesting with non-existent file"
gcov-tool overlap -f nonexistent.gcda 2>&1 | head -10

echo -e "\n=== Testing verbose output capture ==="

# Capture verbose output to file
echo "Capturing verbose output to verbose_output.txt"
gcov-tool overlap -v -f -t 0.5 test.gcda 2> verbose_output.txt
echo "Verbose output size: $(wc -l < verbose_output.txt) lines"

# Test verbose with multiple files
gcov-tool overlap -v -F -o profile_set1/*.gcda profile_set2/*.gcda 2> verbose_multi.txt
echo "Verbose multi-file output size: $(wc -l < verbose_multi.txt) lines"

echo -e "\n=== Additional comprehensive tests ==="

# Test all flags together with different threshold values
for threshold in 0.1 0.5 1.0 2.5 10.0 50.0 99.9; do
    echo "Testing with threshold=$threshold"
    gcov-tool overlap -v -f -F -o -h -t $threshold test.gcda 2>&1 | tail -5
done

# Test with .gcno files as well (some overlap modes might use them)
echo -e "\nTesting with .gcno files"
gcov-tool overlap -f test.gcno test.gcda 2>&1 | head -20

echo -e "\n=== Summary ==="
echo "Generated and tested:"
echo "  - Test program with coverage instrumentation"
echo "  - 4 sets of profile data (.gcda files)"
echo "  - All individual flags: -v, -f, -F, -o, -h, -t"
echo "  - Multiple flag combinations"
echo "  - Multiple input file scenarios"
echo "  - Error case with invalid flag (-Z)"
echo "  - Verbose output capture"

# List generated files
echo -e "\nGenerated files:"
find . -type f -name "*.gcda" -o -name "*.gcno" -o -name "*.txt" | sort

# Cleanup (optional - comment out for debugging)
echo -e "\nCleaning up..."
cd ..
rm -rf "$TEST_DIR"

echo -e "${GREEN}Test completed successfully!${NC}"
```

This script provides comprehensive coverage of the target lines in `gcov-tool.cc`:

1. **Individual Flag Testing**: Tests each uncovered flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) separately
2. **Flag Combinations**: Tests various combinations of flags as specified
3. **Multiple Input Files**: Creates 4 different profile sets and tests with multiple `.gcda` files
4. **Error Case**: Tests invalid flag `-Z` to trigger the `default` case and `overlap_usage()`
5. **Valid GCOV Data**: Generates actual `.gcda` files by compiling and running an instrumented C program
6. **Overlap Mode**: All invocations use `gcov-tool overlap` subcommand
7. **Verbose Output**: Captures verbose output to files to ensure the `-v` flag path is executed
8. **Threshold Values**: Tests multiple floating-point values for `-t` flag

The script creates a self-contained test environment, generates the necessary GCOV data, systematically tests all flag combinations, and cleans up after itself.
