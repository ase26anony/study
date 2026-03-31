Here's a comprehensive shell script that covers all the specified requirements:

```bash
#!/bin/bash
set -e

# Create a test directory
TEST_DIR="gcov_tool_test"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

# Clean up any previous test files
rm -f *.gcda *.gcno *.gcov test_prog* test.c

# Create a simple C program with conditional branches
cat > test.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

int func1(int x) {
    if (x > 0) {
        return x * 2;
    } else {
        return x / 2;
    }
}

void func2(int a, int b) {
    for (int i = 0; i < a; i++) {
        if (b % 2 == 0) {
            printf("Even\n");
        } else {
            printf("Odd\n");
        }
    }
}

int main(int argc, char *argv[]) {
    int val = argc > 1 ? atoi(argv[1]) : 0;
    
    int result = func1(val);
    func2(val, result);
    
    return 0;
}
EOF

# Compile with coverage instrumentation
echo "Compiling test program with coverage instrumentation..."
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog

# Create multiple profile data sets
echo "Generating profile data sets..."

# Run 1: Normal execution
echo "Run 1: val=5"
./test_prog 5

# Copy first gcda file
cp test.gcda test_run1.gcda

# Run 2: Different input
echo "Run 2: val=10"
./test_prog 10

# Copy second gcda file
cp test.gcda test_run2.gcda

# Run 3: Zero input
echo "Run 3: val=0"
./test_prog 0

# Copy third gcda file
cp test.gcda test_run3.gcda

# Run 4: Negative input
echo "Run 4: val=-5"
./test_prog -5

# Copy fourth gcda file
cp test.gcda test_run4.gcda

# Also compile with optimization for different profile patterns
echo "Compiling optimized version..."
gcc -fprofile-arcs -ftest-coverage -O2 test.c -o test_prog_opt

# Generate optimized profile data
echo "Generating optimized profile data..."
./test_prog_opt 3
cp test.gcda test_opt.gcda

# Test gcov-tool with various flag combinations
echo -e "\n=== Testing gcov-tool overlap functionality ===\n"

# Test individual flags
echo "1. Testing individual flags:"

echo "   a) -v flag (verbose)"
gcov-tool overlap -v test.gcda 2>&1 | head -20

echo "   b) -f flag (function level)"
gcov-tool overlap -f test.gcda 2>&1 | head -20

echo "   c) -F flag (fullname)"
gcov-tool overlap -F test.gcda 2>&1 | head -20

echo "   d) -o flag (object level)"
gcov-tool overlap -o test.gcda 2>&1 | head -20

echo "   e) -h flag (hot only)"
gcov-tool overlap -h test.gcda 2>&1 | head -20

echo "   f) -t flag with threshold 0.5"
gcov-tool overlap -t 0.5 test.gcda 2>&1 | head -20

echo "   g) -t flag with threshold 1.0"
gcov-tool overlap -t 1.0 test.gcda 2>&1 | head -20

echo "   h) -t flag with threshold 10.5"
gcov-tool overlap -t 10.5 test.gcda 2>&1 | head -20

# Test flag combinations
echo -e "\n2. Testing flag combinations:"

echo "   a) -f -o combination"
gcov-tool overlap -f -o test.gcda 2>&1 | head -20

echo "   b) -F -h -t 1.0 combination"
gcov-tool overlap -F -h -t 1.0 test.gcda 2>&1 | head -20

echo "   c) -v -f -F -o -h -t 5.0 combination"
gcov-tool overlap -v -f -F -o -h -t 5.0 test.gcda 2>&1 | head -20

echo "   d) -f -t 0.8 -o combination"
gcov-tool overlap -f -t 0.8 -o test.gcda 2>&1 | head -20

# Test with multiple input files
echo -e "\n3. Testing with multiple input files:"

echo "   a) Two input files with -f flag"
gcov-tool overlap -f test_run1.gcda test_run2.gcda 2>&1 | head -20

echo "   b) Three input files with -f -o flags"
gcov-tool overlap -f -o test_run1.gcda test_run2.gcda test_run3.gcda 2>&1 | head -20

echo "   c) Four input files with -v -F -t 0.3"
gcov-tool overlap -v -F -t 0.3 test_run1.gcda test_run2.gcda test_run3.gcda test_run4.gcda 2>&1 | head -20

echo "   d) Mixed normal and optimized profiles"
gcov-tool overlap -f -o test.gcda test_opt.gcda 2>&1 | head -20

# Test error case to trigger usage()
echo -e "\n4. Testing invalid flag to trigger usage():"
echo "   Invalid -Z flag (should show usage)"
gcov-tool overlap -Z test.gcda 2>&1 | head -30 || true

# Test edge cases
echo -e "\n5. Testing edge cases:"

echo "   a) Very low threshold"
gcov-tool overlap -t 0.001 test.gcda 2>&1 | head -20

echo "   b) Very high threshold"
gcov-tool overlap -t 99.9 test.gcda 2>&1 | head -20

echo "   c) All flags with extreme threshold"
gcov-tool overlap -v -f -F -o -h -t 100.0 test.gcda 2>&1 | head -20

# Test with different file orders and combinations
echo -e "\n6. Testing file ordering and combinations:"

echo "   a) Single file with all options"
gcov-tool overlap -v -f -F -o -h -t 0.7 test.gcda > overlap_output.txt 2>&1
echo "      Output saved to overlap_output.txt"

echo "   b) Multiple files in different order"
gcov-tool overlap -f -o test_run4.gcda test_run1.gcda test_run3.gcda test_run2.gcda 2>&1 | head -20

# Create a summary report
echo -e "\n=== Test Summary ==="
echo "Generated files:"
ls -la *.gcda *.gcno 2>/dev/null || echo "No gcda/gcno files found"

echo -e "\nTest completed successfully!"
echo "Coverage should now include the previously uncovered lines in gcov-tool.cc:"
echo "  - Individual flag cases: -v, -f, -F, -o, -h, -t"
echo "  - Flag combinations"
echo "  - Multiple input files"
echo "  - Invalid flag (triggering default case and overlap_usage())"

# Cleanup (optional)
# echo -e "\nCleaning up..."
# cd ..
# rm -rf "$TEST_DIR"
```

This script provides comprehensive coverage of the specified uncovered lines:

1. **Individual flags**: Tests each flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) separately with valid arguments
2. **Flag combinations**: Tests various combinations of flags together
3. **Multiple input files**: Tests with 2-4 different `.gcda` files to stress the overlap logic
4. **Error case**: Includes an invalid `-Z` flag to trigger the `default` case and `overlap_usage()`
5. **Valid GCOV data**: Creates and compiles a C program, runs it multiple times to generate profile data
6. **Different thresholds**: Tests `-t` with various floating-point values (0.5, 1.0, 10.5, 0.001, 99.9, etc.)
7. **Verbose output**: Uses `-v` flag and redirects output to ensure the code path is executed

The script generates multiple profile runs with different inputs to create varied coverage data, then systematically exercises all the command-line parsing logic for the overlap analysis feature.
