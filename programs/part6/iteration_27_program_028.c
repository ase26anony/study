Here's a comprehensive shell script that covers all the specified requirements for testing the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash

# test_gcov_tool.sh - Comprehensive test for gcov-tool overlap analysis flags

set -e  # Exit on any error

echo "=== Starting gcov-tool overlap analysis flag tests ==="

# Clean up any previous test files
cleanup() {
    echo "Cleaning up..."
    rm -f test.c test_prog* *.gcda *.gcno *.gcov gcov_output.txt 2>/dev/null || true
}
cleanup

# Step 1: Generate a simple C program with conditional branches
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

int func2(int y, int z) {
    int result = 0;
    for (int i = 0; i < y; i++) {
        if (z % 2 == 0) {
            result += i;
        } else {
            result -= i;
        }
    }
    return result;
}

int main(int argc, char *argv[]) {
    int val = (argc > 1) ? atoi(argv[1]) : 0;
    
    int a = func1(val);
    int b = func2(val, a);
    
    printf("Result: %d\n", a + b);
    return 0;
}
EOF

echo "Generated test.c"

# Step 2: Compile with GCOV instrumentation
echo "Compiling test program with GCOV instrumentation..."
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog

# Check if gcov-tool exists
if ! command -v gcov-tool &> /dev/null; then
    echo "Error: gcov-tool not found in PATH"
    echo "Please ensure GCC with gcov-tool is installed"
    exit 1
fi

# Step 3: Generate multiple sets of profile data
echo "Generating profile data..."

# First run - normal execution
echo "Run 1: ./test_prog 5"
./test_prog 5
mv test.gcda test_run1.gcda 2>/dev/null || true

# Second run - different input
echo "Run 2: ./test_prog 10"
./test_prog 10
mv test.gcda test_run2.gcda 2>/dev/null || true

# Third run - zero input
echo "Run 3: ./test_prog 0"
./test_prog 0
mv test.gcda test_run3.gcda 2>/dev/null || true

# Fourth run - negative input
echo "Run 4: ./test_prog -3"
./test_prog -3
mv test.gcda test_run4.gcda 2>/dev/null || true

# Create a copy in different directory for testing multiple paths
mkdir -p subdir
cp test_prog subdir/
(cd subdir && ./test_prog 7)
cp subdir/test.gcda test_run5.gcda 2>/dev/null || true

echo "Generated 5 different .gcda files"

# Step 4: Test individual flags
echo -e "\n=== Testing individual flags ==="

# Test -v flag (verbose)
echo "Testing -v flag..."
gcov-tool overlap -v test_run1.gcda test_run2.gcda > gcov_output.txt 2>&1
echo "  Verbose output generated"

# Test -f flag (function level)
echo "Testing -f flag..."
gcov-tool overlap -f test_run1.gcda test_run2.gcda > gcov_output.txt 2>&1

# Test -F flag (fullname)
echo "Testing -F flag..."
gcov-tool overlap -F test_run1.gcda test_run2.gcda > gcov_output.txt 2>&1

# Test -o flag (object level)
echo "Testing -o flag..."
gcov-tool overlap -o test_run1.gcda test_run2.gcda > gcov_output.txt 2>&1

# Test -h flag (hot only)
echo "Testing -h flag..."
gcov-tool overlap -h test_run1.gcda test_run2.gcda > gcov_output.txt 2>&1

# Test -t flag with threshold (requires argument)
echo "Testing -t flag with threshold 0.5..."
gcov-tool overlap -t 0.5 test_run1.gcda test_run2.gcda > gcov_output.txt 2>&1

echo "Testing -t flag with threshold 1.0..."
gcov-tool overlap -t 1.0 test_run1.gcda test_run2.gcda > gcov_output.txt 2>&1

echo "Testing -t flag with threshold 10.5..."
gcov-tool overlap -t 10.5 test_run1.gcda test_run2.gcda > gcov_output.txt 2>&1

# Step 5: Test flag combinations
echo -e "\n=== Testing flag combinations ==="

# Test -f -o combination
echo "Testing -f -o combination..."
gcov-tool overlap -f -o test_run1.gcda test_run2.gcda test_run3.gcda > gcov_output.txt 2>&1

# Test -F -h -t combination
echo "Testing -F -h -t 1.0 combination..."
gcov-tool overlap -F -h -t 1.0 test_run1.gcda test_run2.gcda > gcov_output.txt 2>&1

# Test -v -f -F -o -h -t combination
echo "Testing -v -f -F -o -h -t 5.0 combination..."
gcov-tool overlap -v -f -F -o -h -t 5.0 test_run1.gcda test_run2.gcda test_run3.gcda > gcov_output.txt 2>&1

# Test with all 5 gcda files
echo "Testing with all 5 gcda files and multiple flags..."
gcov-tool overlap -v -f -t 0.8 test_run1.gcda test_run2.gcda test_run3.gcda test_run4.gcda test_run5.gcda > gcov_output.txt 2>&1

# Step 6: Test invalid flag to trigger usage
echo -e "\n=== Testing invalid flag to trigger overlap_usage() ==="
echo "Testing invalid flag -Z..."
gcov-tool overlap -Z test_run1.gcda 2>&1 | grep -q "Usage:" && echo "  Successfully triggered usage message" || echo "  Usage message not shown"

# Step 7: Additional edge cases
echo -e "\n=== Testing additional edge cases ==="

# Test with threshold 0.0
echo "Testing -t 0.0..."
gcov-tool overlap -t 0.0 test_run1.gcda test_run2.gcda > gcov_output.txt 2>&1

# Test with threshold 100.0
echo "Testing -t 100.0..."
gcov-tool overlap -t 100.0 test_run1.gcda test_run2.gcda > gcov_output.txt 2>&1

# Test -v with multiple combinations
echo "Testing -v with -f -o..."
gcov-tool overlap -v -f -o test_run1.gcda test_run2.gcda > gcov_output.txt 2>&1

# Test with single gcda file (should still work)
echo "Testing with single gcda file..."
gcov-tool overlap -f test_run1.gcda > gcov_output.txt 2>&1

# Step 8: Test with different compilation options
echo -e "\n=== Testing with differently optimized builds ==="

# Compile with optimization
gcc -fprofile-arcs -ftest-coverage -O2 test.c -o test_prog_opt
./test_prog_opt 5
mv test.gcda test_opt.gcda 2>/dev/null || true

# Test overlap between optimized and non-optimized profiles
echo "Testing overlap between O0 and O2 builds..."
gcov-tool overlap -v -f test_run1.gcda test_opt.gcda > gcov_output.txt 2>&1

# Step 9: Verify all required flags were tested
echo -e "\n=== Test Summary ==="
echo "The following flags and combinations were tested:"
echo "  Individual: -v, -f, -F, -o, -h, -t 0.5, -t 1.0, -t 10.5"
echo "  Combinations: -f -o, -F -h -t 1.0, -v -f -F -o -h -t 5.0"
echo "  Multiple files: Up to 5 different .gcda files"
echo "  Invalid flag: -Z (triggered usage)"
echo "  Edge cases: -t 0.0, -t 100.0, single file, optimized build"
echo ""
echo "All uncovered lines in gcov-tool.cc should now be exercised:"
echo "  - case 'v': verbose = true; gcov_set_verbose();"
echo "  - case 'f': overlap_func_level = 1;"
echo "  - case 'F': overlap_use_fullname = 1;"
echo "  - case 'o': overlap_obj_level = 1;"
echo "  - case 'h': overlap_hot_only = 1;"
echo "  - case 't': overlap_hot_threshold = atof(optarg);"
echo "  - default: overlap_usage();"

# Step 10: Cleanup
cleanup
echo -e "\n=== Test completed successfully ==="
```

This script provides comprehensive coverage of all the specified requirements:

1. **Comprehensive Flag Combination**: Tests each individual flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) with various threshold values, and tests multiple combinations of flags together.

2. **Valid GCOV Data Input**: Creates a C program with conditional branches, compiles it with GCOV instrumentation, and executes it multiple times to generate `.gcda` files.

3. **Overlap Analysis Mode**: All invocations use `gcov-tool overlap` as the subcommand before the flags.

4. **Error and Usage Testing**: Includes an invalid flag `-Z` to trigger the `default` case and call `overlap_usage()`.

5. **Multiple Input Files**: Generates 5 different `.gcda` files by running the program with different inputs and in different directories, and tests with multiple files.

6. **Verbose Output Handling**: Tests `-v` flag both individually and in combinations, redirecting output to verify execution.

The script also includes additional edge cases like testing with different optimization levels and various threshold values to ensure robust coverage of the parsing logic.
