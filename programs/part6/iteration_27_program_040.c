Here's a comprehensive shell script that covers all the specified requirements for testing the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash

# test_gcov_tool.sh - Comprehensive test for gcov-tool overlap analysis flags

set -e  # Exit on any error

echo "=== Starting gcov-tool overlap analysis flag coverage test ==="

# Clean up from previous runs
cleanup() {
    echo "Cleaning up..."
    rm -f test.c test_prog* *.gcda *.gcno *.gcov gcov_output*.txt
    rm -rf profile_data_* subdir
}

cleanup

# Step 1: Generate a simple C program with conditional branches
echo "Generating test C program..."
cat > test.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

int calculate(int a, int b, int mode) {
    int result = 0;
    
    if (mode == 0) {
        result = a + b;
    } else if (mode == 1) {
        result = a - b;
    } else {
        result = a * b;
    }
    
    for (int i = 0; i < a; i++) {
        result += i;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    int x = 10;
    int y = 5;
    
    int res = calculate(x, y, mode);
    printf("Result: %d (mode: %d)\n", res, mode);
    
    // Another conditional
    if (res > 20) {
        printf("Large result\n");
    } else {
        printf("Small result\n");
    }
    
    return 0;
}
EOF

# Step 2: Compile with GCOV instrumentation
echo "Compiling test program with GCOV instrumentation..."
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog
gcc -fprofile-arcs -ftest-coverage -O2 test.c -o test_prog_opt

# Step 3: Generate multiple sets of profile data
echo "Generating profile data with different execution paths..."

# First run - mode 0
echo "Run 1: mode 0"
./test_prog 0
mv test.gcda test1.gcda

# Second run - mode 1 (different path)
echo "Run 2: mode 1"
./test_prog 1
mv test.gcda test2.gcda

# Third run - mode 2 (another different path)
echo "Run 3: mode 2"
./test_prog 2
mv test.gcda test3.gcda

# Create a subdirectory with different profile data
mkdir -p subdir
echo "Run 4: mode 0 in subdirectory"
cd subdir
cp ../test_prog .
./test_prog 0
cd ..

# Optimized version run
echo "Run 5: optimized version mode 1"
./test_prog_opt 1
mv test.gcda test_opt.gcda

# Step 4: Test individual flags
echo -e "\n=== Testing individual flags ==="

echo "Test 1: Verbose flag (-v)"
gcov-tool overlap -v test1.gcda test2.gcda > gcov_output1.txt 2>&1

echo "Test 2: Function level flag (-f)"
gcov-tool overlap -f test1.gcda test2.gcda > gcov_output2.txt 2>&1

echo "Test 3: Fullname flag (-F)"
gcov-tool overlap -F test1.gcda test2.gcda > gcov_output3.txt 2>&1

echo "Test 4: Object level flag (-o)"
gcov-tool overlap -o test1.gcda test2.gcda > gcov_output4.txt 2>&1

echo "Test 5: Hot only flag (-h)"
gcov-tool overlap -h test1.gcda test2.gcda > gcov_output5.txt 2>&1

echo "Test 6: Threshold flag (-t 0.5)"
gcov-tool overlap -t 0.5 test1.gcda test2.gcda > gcov_output6.txt 2>&1

echo "Test 7: Different threshold (-t 1.0)"
gcov-tool overlap -t 1.0 test1.gcda test2.gcda > gcov_output7.txt 2>&1

echo "Test 8: High threshold (-t 10.5)"
gcov-tool overlap -t 10.5 test1.gcda test2.gcda > gcov_output8.txt 2>&1

# Step 5: Test flag combinations
echo -e "\n=== Testing flag combinations ==="

echo "Test 9: -f -o combination"
gcov-tool overlap -f -o test1.gcda test2.gcda test3.gcda > gcov_output9.txt 2>&1

echo "Test 10: -F -h -t combination"
gcov-tool overlap -F -h -t 1.0 test1.gcda test2.gcda > gcov_output10.txt 2>&1

echo "Test 11: -v -f -F combination"
gcov-tool overlap -v -f -F test1.gcda test2.gcda > gcov_output11.txt 2>&1

echo "Test 12: -f -o -h combination"
gcov-tool overlap -f -o -h test1.gcda test2.gcda > gcov_output12.txt 2>&1

echo "Test 13: -v -f -F -o -h -t combination (all flags)"
gcov-tool overlap -v -f -F -o -h -t 5.0 test1.gcda test2.gcda test3.gcda > gcov_output13.txt 2>&1

echo "Test 14: -t with decimal threshold"
gcov-tool overlap -t 0.75 test1.gcda test2.gcda > gcov_output14.txt 2>&1

# Step 6: Test with multiple input files
echo -e "\n=== Testing with multiple input files ==="

echo "Test 15: Multiple .gcda files with -f -o"
gcov-tool overlap -f -o test1.gcda test2.gcda test3.gcda test_opt.gcda > gcov_output15.txt 2>&1

echo "Test 16: With subdirectory .gcda file"
gcov-tool overlap -v -f subdir/test.gcda test1.gcda > gcov_output16.txt 2>&1

# Step 7: Test error case (to trigger overlap_usage)
echo -e "\n=== Testing error case to trigger usage ==="

echo "Test 17: Invalid flag (-Z) - should show usage"
gcov-tool overlap -Z test1.gcda 2> gcov_error.txt || true

echo "Test 18: Missing argument for -t"
gcov-tool overlap -t 2> gcov_error2.txt || true

# Step 8: Test with different file orders and combinations
echo -e "\n=== Testing various file combinations ==="

echo "Test 19: Single file with -v -f"
gcov-tool overlap -v -f test1.gcda > gcov_output19.txt 2>&1

echo "Test 20: Three files with -F -o -t 0.3"
gcov-tool overlap -F -o -t 0.3 test1.gcda test2.gcda test3.gcda > gcov_output20.txt 2>&1

# Step 9: Verify outputs were generated
echo -e "\n=== Verifying test outputs ==="
for i in {1..20}; do
    if [ -f "gcov_output${i}.txt" ]; then
        lines=$(wc -l < "gcov_output${i}.txt")
        echo "Output $i: $lines lines"
    fi
done

# Step 10: Display sample output for verification
echo -e "\n=== Sample output from test 13 (all flags) ==="
head -20 gcov_output13.txt

echo -e "\n=== Sample error output (invalid flag) ==="
cat gcov_error.txt

echo -e "\n=== Test summary ==="
echo "Generated and tested:"
echo "- 3 different .gcda files from normal runs"
echo "- 1 .gcda file from optimized build"
echo "- 1 .gcda file in subdirectory"
echo "- 20 different gcov-tool invocations"
echo "- All target flags tested individually and in combination"
echo "- Invalid flag test to trigger usage function"

# Optional: Clean up
# cleanup

echo -e "\n=== Test completed successfully ==="
```

This script provides comprehensive coverage of the specified requirements:

1. **Individual flag testing**: Tests each uncovered flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) individually with different threshold values.

2. **Flag combinations**: Tests various combinations including `-f -o`, `-F -h -t 1.0`, and `-v -f -F -o -h -t 5.0`.

3. **Error case**: Includes an invalid flag `-Z` to trigger the `default` case and `overlap_usage()`.

4. **Multiple input files**: Creates and uses multiple `.gcda` files from different program executions with different parameters.

5. **Valid GCOV data**: Generates a C program, compiles it with GCOV instrumentation, and executes it multiple times to produce valid profile data.

6. **Verbose output handling**: Captures output from `-v` flag invocations to files.

7. **Different compilation options**: Tests with both `-O0` and `-O2` optimized builds.

The script creates a test program with conditional branches to ensure varied execution paths, generates multiple profile data files, and systematically tests all the command-line argument parsing logic for the overlap analysis feature.
