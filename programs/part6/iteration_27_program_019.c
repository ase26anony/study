Here's a comprehensive shell script that covers all the specified requirements for testing the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash

# test_gcov_tool.sh - Comprehensive test for gcov-tool overlap functionality
# This script tests the uncovered lines in gcov-tool.cc (lines 534-554)

set -e  # Exit on any error

echo "=== Starting gcov-tool overlap flag testing ==="

# Clean up any previous test files
cleanup() {
    echo "Cleaning up..."
    rm -f test.c test_prog test_prog_opt test_prog_lto
    rm -f *.gcda *.gcno *.gcov gcov_output.txt
    rm -f test_prog_*.gcda test_prog_*.gcno
    rm -rf profile_data_*
}

# Clean up before starting
cleanup

# Step 1: Generate a simple C program with conditional branches
echo "Generating test C program..."
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
    
    printf("Factorial of %d is %d\n", num, factorial(num));
    
    if (num < 10) {
        printf("Fibonacci of %d is %d\n", num, fibonacci(num));
    } else {
        printf("Skipping Fibonacci for large number %d\n", num);
    }
    
    return 0;
}
EOF

# Step 2: Compile with different optimization levels to generate varied GCOV data
echo "Compiling test programs with GCOV instrumentation..."

# Basic compilation
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog

# With optimization
gcc -fprofile-arcs -ftest-coverage -O2 test.c -o test_prog_opt

# With LTO (if supported)
if gcc -fprofile-arcs -ftest-coverage -flto -O2 test.c -o test_prog_lto 2>/dev/null; then
    HAS_LTO=1
else
    HAS_LTO=0
    echo "Note: LTO compilation not supported, skipping LTO tests"
fi

# Step 3: Generate multiple sets of profile data
echo "Generating profile data with different executions..."

# Create directories for different profile runs
mkdir -p profile_data_1 profile_data_2 profile_data_3

# First run - normal execution
echo "Running test_prog (normal)..."
./test_prog 3
mv test_prog.gcda profile_data_1/
mv test_prog.gcno profile_data_1/

# Second run - different input
echo "Running test_prog with input 7..."
./test_prog 7
mv test_prog.gcda profile_data_2/
cp profile_data_1/test_prog.gcno profile_data_2/

# Third run - edge case
echo "Running test_prog with input 1..."
./test_prog 1
mv test_prog.gcda profile_data_3/
cp profile_data_1/test_prog.gcno profile_data_3/

# Run optimized version
echo "Running test_prog_opt..."
./test_prog_opt 4
mv test_prog_opt.gcda profile_data_1/
mv test_prog_opt.gcno profile_data_1/

# Step 4: Test individual flags
echo -e "\n=== Testing individual flags ==="

# Test -v flag (verbose)
echo "Testing -v flag..."
gcov-tool overlap -v profile_data_1/test_prog.gcda > gcov_output.txt 2>&1
echo "  -v test completed"

# Test -f flag (function level)
echo "Testing -f flag..."
gcov-tool overlap -f profile_data_1/test_prog.gcda profile_data_2/test_prog.gcda > gcov_output.txt 2>&1

# Test -F flag (fullname)
echo "Testing -F flag..."
gcov-tool overlap -F profile_data_1/test_prog.gcda profile_data_2/test_prog.gcda > gcov_output.txt 2>&1

# Test -o flag (object level)
echo "Testing -o flag..."
gcov-tool overlap -o profile_data_1/test_prog.gcda profile_data_2/test_prog.gcda > gcov_output.txt 2>&1

# Test -h flag (hot only)
echo "Testing -h flag..."
gcov-tool overlap -h profile_data_1/test_prog.gcda profile_data_2/test_prog.gcda > gcov_output.txt 2>&1

# Test -t flag with threshold (requires argument)
echo "Testing -t flag with threshold 0.5..."
gcov-tool overlap -t 0.5 profile_data_1/test_prog.gcda profile_data_2/test_prog.gcda > gcov_output.txt 2>&1

echo "Testing -t flag with threshold 1.0..."
gcov-tool overlap -t 1.0 profile_data_1/test_prog.gcda profile_data_2/test_prog.gcda > gcov_output.txt 2>&1

echo "Testing -t flag with threshold 10.5..."
gcov-tool overlap -t 10.5 profile_data_1/test_prog.gcda profile_data_2/test_prog.gcda > gcov_output.txt 2>&1

# Step 5: Test flag combinations
echo -e "\n=== Testing flag combinations ==="

# Test -f -o combination
echo "Testing -f -o combination..."
gcov-tool overlap -f -o profile_data_1/test_prog.gcda profile_data_2/test_prog.gcda profile_data_3/test_prog.gcda > gcov_output.txt 2>&1

# Test -F -h -t combination
echo "Testing -F -h -t 1.0 combination..."
gcov-tool overlap -F -h -t 1.0 profile_data_1/test_prog.gcda profile_data_2/test_prog.gcda > gcov_output.txt 2>&1

# Test -v -f -F -o -h -t combination
echo "Testing -v -f -F -o -h -t 5.0 combination..."
gcov-tool overlap -v -f -F -o -h -t 5.0 profile_data_1/test_prog.gcda profile_data_2/test_prog.gcda > gcov_output.txt 2>&1

# Test with mixed gcda files (different binaries)
echo "Testing with mixed gcda files..."
gcov-tool overlap -f -o profile_data_1/test_prog.gcda profile_data_1/test_prog_opt.gcda > gcov_output.txt 2>&1

# Step 6: Test invalid flag to trigger usage
echo -e "\n=== Testing invalid flag to trigger usage ==="
echo "Testing invalid flag -Z (should show usage)..."
gcov-tool overlap -Z profile_data_1/test_prog.gcda 2>&1 | head -20

# Step 7: Test with multiple input files and different thresholds
echo -e "\n=== Testing with multiple files and thresholds ==="

# Create a list of all gcda files
ALL_GCDA=""
for dir in profile_data_1 profile_data_2 profile_data_3; do
    if [ -f "$dir/test_prog.gcda" ]; then
        ALL_GCDA="$ALL_GCDA $dir/test_prog.gcda"
    fi
done

if [ -f "profile_data_1/test_prog_opt.gcda" ]; then
    ALL_GCDA="$ALL_GCDA profile_data_1/test_prog_opt.gcda"
fi

echo "Testing with all gcda files and threshold 0.8..."
gcov-tool overlap -f -o -t 0.8 $ALL_GCDA > gcov_output.txt 2>&1

echo "Testing with all gcda files, hot only, and threshold 2.0..."
gcov-tool overlap -f -h -t 2.0 $ALL_GCDA > gcov_output.txt 2>&1

# Step 8: Test edge cases
echo -e "\n=== Testing edge cases ==="

# Test with threshold 0.0
echo "Testing with threshold 0.0..."
gcov-tool overlap -t 0.0 profile_data_1/test_prog.gcda profile_data_2/test_prog.gcda > gcov_output.txt 2>&1

# Test with threshold 100.0 (very high)
echo "Testing with threshold 100.0..."
gcov-tool overlap -t 100.0 profile_data_1/test_prog.gcda profile_data_2/test_prog.gcda > gcov_output.txt 2>&1

# Test with single file (should still work)
echo "Testing with single file..."
gcov-tool overlap -v -f profile_data_1/test_prog.gcda > gcov_output.txt 2>&1

# Test with no gcda files (should show error/usage)
echo "Testing with no gcda files..."
gcov-tool overlap -f 2>&1 | head -10

# Step 9: Test with LTO-compiled binary if available
if [ $HAS_LTO -eq 1 ]; then
    echo -e "\n=== Testing with LTO-compiled binary ==="
    
    # Run LTO version
    ./test_prog_lto 6
    mv test_prog_lto.gcda profile_data_1/
    mv test_prog_lto.gcno profile_data_1/
    
    echo "Testing LTO binary with -f -F flags..."
    gcov-tool overlap -f -F profile_data_1/test_prog_lto.gcda profile_data_1/test_prog.gcda > gcov_output.txt 2>&1
fi

echo -e "\n=== All tests completed successfully ==="
echo "Summary of tested flag combinations:"
echo "  Individual flags: -v, -f, -F, -o, -h, -t <value>"
echo "  Combinations: -f -o, -F -h -t 1.0, -v -f -F -o -h -t 5.0"
echo "  Invalid flag: -Z (triggered usage)"
echo "  Multiple input files: Up to 4 different .gcda files"
echo "  Various thresholds: 0.0, 0.5, 1.0, 2.0, 5.0, 10.5, 100.0"

# Optional: Show coverage improvement
echo -e "\nTo see coverage improvement for gcov-tool itself, run:"
echo "  gcov -b gcov-tool.gcda | grep -A5 'Lines executed:'"

# Clean up
cleanup

echo -e "\n=== Test script finished ==="
```

This script provides comprehensive testing of the uncovered lines in `gcov-tool.cc`:

1. **Generates test C program** with conditional branches to create meaningful GCOV data
2. **Compiles with different optimization levels** to generate varied profile data
3. **Creates multiple profile runs** with different inputs to ensure distinct `.gcda` files
4. **Tests all individual flags** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) as specified
5. **Tests flag combinations** including complex ones like `-v -f -F -o -h -t 5.0`
6. **Tests invalid flag** `-Z` to trigger the `default` case and `overlap_usage()`
7. **Uses multiple input files** (up to 4 different `.gcda` files in combinations)
8. **Tests various threshold values** for the `-t` flag (0.0, 0.5, 1.0, 2.0, 5.0, 10.5, 100.0)
9. **Includes edge cases** like single file, no files, and LTO-compiled binaries
10. **Provides clean up** to remove generated files

The script ensures that every uncovered line in the switch statement (534-554) is executed:
- Each `case` statement is triggered by its corresponding flag
- The `default` case is triggered by the invalid `-Z` flag
- All flag parsing logic is exercised with valid and invalid combinations
