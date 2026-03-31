Here's a comprehensive shell script that covers all the specified requirements for testing the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash

# test_gcov_tool.sh - Comprehensive test for gcov-tool overlap functionality

set -e  # Exit on any error

echo "=== Starting gcov-tool overlap flag testing ==="

# Clean up from previous runs
cleanup() {
    echo "Cleaning up..."
    rm -f test.c test_prog test_prog_opt test_prog_lto
    rm -f *.gcda *.gcno *.gcov *.info
    rm -f gcov_output_*.txt
    rm -rf profile_data_*
}

cleanup

# Step 1: Generate a simple C program with conditional branches
cat > test.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

int calculate(int a, int b, int mode) {
    int result = 0;
    
    if (mode == 1) {
        result = a + b;
    } else if (mode == 2) {
        result = a * b;
    } else {
        result = a - b;
    }
    
    for (int i = 0; i < a; i++) {
        result += i;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int mode = 1;
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    int x = 10;
    int y = 5;
    
    int res = calculate(x, y, mode);
    printf("Result: %d (mode: %d)\n", res, mode);
    
    // Another branch
    if (res > 20) {
        printf("Large result\n");
    } else {
        printf("Small result\n");
    }
    
    return 0;
}
EOF

echo "Generated test.c"

# Step 2: Compile with different optimization levels to generate varied profile data
echo "Compiling test programs with GCOV instrumentation..."

# Basic compilation
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog
echo "Compiled test_prog (O0)"

# With optimization
gcc -fprofile-arcs -ftest-coverage -O2 test.c -o test_prog_opt
echo "Compiled test_prog_opt (O2)"

# With LTO if supported
if gcc -fprofile-arcs -ftest-coverage -flto -O2 test.c -o test_prog_lto 2>/dev/null; then
    echo "Compiled test_prog_lto (LTO)"
    HAS_LTO=1
else
    echo "LTO compilation failed (continuing without it)"
    HAS_LTO=0
fi

# Step 3: Generate multiple sets of profile data
echo "Generating profile data..."

# First run - mode 1
echo "Run 1: mode 1"
./test_prog 1
mv test.gcda test_run1.gcda

# Second run - mode 2 (different execution path)
echo "Run 2: mode 2"
./test_prog 2
mv test.gcda test_run2.gcda

# Third run - mode 3 (another path)
echo "Run 3: mode 3"
./test_prog 3
mv test.gcda test_run3.gcda

# Run optimized version for different profile
echo "Run 4: optimized version mode 1"
./test_prog_opt 1
mv test.gcda test_run4.gcda

# Create directories with different .gcda files for multiple input testing
mkdir -p profile_data_1 profile_data_2
cp test_run1.gcda profile_data_1/
cp test_run2.gcda profile_data_1/
cp test_run3.gcda profile_data_2/
cp test_run4.gcda profile_data_2/

# Step 4: Test individual flags
echo -e "\n=== Testing individual flags ==="

# Test verbose flag (-v)
echo "Testing -v flag..."
gcov-tool overlap -v test_run1.gcda test_run2.gcda > gcov_output_verbose.txt 2>&1
echo "  Verbose output saved to gcov_output_verbose.txt"

# Test function level flag (-f)
echo "Testing -f flag..."
gcov-tool overlap -f test_run1.gcda test_run2.gcda > gcov_output_func.txt 2>&1

# Test fullname flag (-F)
echo "Testing -F flag..."
gcov-tool overlap -F test_run1.gcda test_run2.gcda > gcov_output_fullname.txt 2>&1

# Test object level flag (-o)
echo "Testing -o flag..."
gcov-tool overlap -o test_run1.gcda test_run2.gcda > gcov_output_obj.txt 2>&1

# Test hot only flag (-h)
echo "Testing -h flag..."
gcov-tool overlap -h test_run1.gcda test_run2.gcda > gcov_output_hot.txt 2>&1

# Test threshold flag (-t) with different values
echo "Testing -t flag with threshold 0.5..."
gcov-tool overlap -t 0.5 test_run1.gcda test_run2.gcda > gcov_output_thresh05.txt 2>&1

echo "Testing -t flag with threshold 1.0..."
gcov-tool overlap -t 1.0 test_run1.gcda test_run2.gcda > gcov_output_thresh10.txt 2>&1

echo "Testing -t flag with threshold 10.5..."
gcov-tool overlap -t 10.5 test_run1.gcda test_run2.gcda > gcov_output_thresh105.txt 2>&1

# Step 5: Test flag combinations
echo -e "\n=== Testing flag combinations ==="

# Test -f -o combination
echo "Testing -f -o combination..."
gcov-tool overlap -f -o test_run1.gcda test_run2.gcda > gcov_output_fo.txt 2>&1

# Test -F -h -t combination
echo "Testing -F -h -t 1.0 combination..."
gcov-tool overlap -F -h -t 1.0 test_run1.gcda test_run2.gcda > gcov_output_Fht.txt 2>&1

# Test -v -f -F -o -h -t combination
echo "Testing -v -f -F -o -h -t 5.0 combination..."
gcov-tool overlap -v -f -F -o -h -t 5.0 test_run1.gcda test_run2.gcda > gcov_output_allflags.txt 2>&1

# Test with multiple input files from directories
echo "Testing with multiple input directories..."
gcov-tool overlap -f -o profile_data_1/*.gcda profile_data_2/*.gcda > gcov_output_multi.txt 2>&1

# Step 6: Test invalid flag to trigger usage
echo -e "\n=== Testing invalid flag (to trigger usage) ==="
echo "Testing invalid -Z flag (should show usage)..."
gcov-tool overlap -Z test_run1.gcda 2>&1 | head -20 > gcov_output_invalid.txt || true
echo "  Invalid flag output saved to gcov_output_invalid.txt"

# Step 7: Additional comprehensive tests
echo -e "\n=== Additional comprehensive tests ==="

# Test with all .gcda files
echo "Testing with all generated .gcda files..."
gcov-tool overlap -v -f -F *.gcda > gcov_output_allgcda.txt 2>&1

# Test with threshold 0.0 and 100.0 edge cases
echo "Testing edge case thresholds..."
gcov-tool overlap -t 0.0 test_run1.gcda test_run2.gcda > gcov_output_thresh00.txt 2>&1
gcov-tool overlap -t 100.0 test_run1.gcda test_run2.gcda > gcov_output_thresh100.txt 2>&1

# Test combination with just -v and -h
echo "Testing -v -h combination..."
gcov-tool overlap -v -h test_run1.gcda test_run2.gcda > gcov_output_vh.txt 2>&1

# Test with single file (should still work)
echo "Testing with single input file..."
gcov-tool overlap -f -o test_run1.gcda > gcov_output_single.txt 2>&1

# Step 8: Verify outputs were created
echo -e "\n=== Verification ==="
echo "Generated output files:"
ls -la gcov_output_*.txt

echo -e "\nSample of verbose output:"
head -10 gcov_output_verbose.txt

echo -e "\nSample of invalid flag output:"
head -5 gcov_output_invalid.txt

# Step 9: Summary
echo -e "\n=== Test Summary ==="
echo "Tested all individual flags: -v, -f, -F, -o, -h, -t"
echo "Tested flag combinations"
echo "Tested with multiple input files"
echo "Tested invalid flag to trigger usage"
echo "All tests completed successfully!"

# Optional: Keep files for inspection
echo -e "\nGenerated files kept for inspection:"
echo "  test.c - Source file"
echo "  test_prog* - Compiled binaries"
echo "  *.gcda, *.gcno - Profile data files"
echo "  gcov_output_*.txt - gcov-tool outputs"
echo "  profile_data_*/ - Directories with organized .gcda files"

echo -e "\nTo clean up all generated files, run:"
echo "  rm -f test.c test_prog* *.gcda *.gcno gcov_output_*.txt"
echo "  rm -rf profile_data_*"
```

This script provides comprehensive coverage of the uncovered lines by:

1. **Generating valid GCOV data**: Creates a C program with conditional branches, compiles it with GCOV instrumentation, and runs it multiple times to generate `.gcda` files.

2. **Testing all individual flags**:
   - `-v` (verbose)
   - `-f` (function level overlap)
   - `-F` (use fullname)
   - `-o` (object level)
   - `-h` (hot only)
   - `-t` with various thresholds (0.5, 1.0, 10.5)

3. **Testing flag combinations**:
   - `-f -o`
   - `-F -h -t 1.0`
   - `-v -f -F -o -h -t 5.0`
   - And several other combinations

4. **Triggering the default/usage case**: Includes an invalid `-Z` flag to trigger `overlap_usage()`.

5. **Testing with multiple input files**: Creates multiple `.gcda` files and organizes them in directories to test with multiple inputs.

6. **Edge cases**: Tests threshold values of 0.0 and 100.0, single file input, and all generated files together.

The script captures all outputs to files for verification and provides a clean summary of what was tested. Each `gcov-tool` invocation uses the `overlap` subcommand as required, followed by the appropriate flags and input files.
