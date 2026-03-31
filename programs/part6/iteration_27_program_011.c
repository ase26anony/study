Here's a comprehensive shell script that covers all the specified requirements for testing the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash

# test_gcov_tool.sh - Comprehensive test for gcov-tool overlap flags

set -e  # Exit on any error

echo "=== Starting gcov-tool overlap flag coverage test ==="

# Clean up from previous runs
cleanup() {
    echo "Cleaning up..."
    rm -f test.c test_prog test_prog_opt test_prog_lto
    rm -f *.gcda *.gcno *.gcov gcov_output.txt
    rm -rf profile_data_* gcov_data_*
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
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    int x = 10;
    int y = 5;
    
    int res = calculate(x, y, mode);
    printf("Result: %d (mode: %d)\n", res, mode);
    
    return 0;
}
EOF

echo "Generated test.c"

# Step 2: Compile with different optimization levels to get varied profile data
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
mv test_prog.gcda test_prog_run1.gcda
cp test_prog.gcno test_prog_run1.gcno

# Second run - mode 2 (different execution path)
echo "Run 2: mode 2"
./test_prog 2
mv test_prog.gcda test_prog_run2.gcda

# Third run - mode 0 (another different path)
echo "Run 3: mode 0"
./test_prog 0
mv test_prog.gcda test_prog_run3.gcda

# Run optimized version for different profile patterns
echo "Run 4: optimized version mode 1"
./test_prog_opt 1
mv test_prog_opt.gcda test_prog_opt_run1.gcda
cp test_prog_opt.gcno test_prog_opt_run1.gcno

echo "Run 5: optimized version mode 3"
./test_prog_opt 3
mv test_prog_opt.gcda test_prog_opt_run2.gcda

# Create directory with multiple gcda files for batch processing
mkdir -p gcov_data_multi
cp *.gcda gcov_data_multi/
cp *.gcno gcov_data_multi/

# Step 4: Test individual flags
echo -e "\n=== Testing individual flags ==="

# Test verbose flag (-v)
echo "Testing -v flag..."
gcov-tool overlap -v gcov_data_multi/*.gcda > gcov_output.txt 2>&1
echo "  -v test completed"

# Test function level flag (-f)
echo "Testing -f flag..."
gcov-tool overlap -f gcov_data_multi/*.gcda > gcov_output.txt 2>&1

# Test fullname flag (-F)
echo "Testing -F flag..."
gcov-tool overlap -F gcov_data_multi/*.gcda > gcov_output.txt 2>&1

# Test object level flag (-o)
echo "Testing -o flag..."
gcov-tool overlap -o gcov_data_multi/*.gcda > gcov_output.txt 2>&1

# Test hot only flag (-h)
echo "Testing -h flag..."
gcov-tool overlap -h gcov_data_multi/*.gcda > gcov_output.txt 2>&1

# Test threshold flag (-t) with different values
echo "Testing -t flag with 0.5..."
gcov-tool overlap -t 0.5 gcov_data_multi/*.gcda > gcov_output.txt 2>&1

echo "Testing -t flag with 1.0..."
gcov-tool overlap -t 1.0 gcov_data_multi/*.gcda > gcov_output.txt 2>&1

echo "Testing -t flag with 10.5..."
gcov-tool overlap -t 10.5 gcov_data_multi/*.gcda > gcov_output.txt 2>&1

# Step 5: Test flag combinations
echo -e "\n=== Testing flag combinations ==="

# Combination 1: -f -o
echo "Testing -f -o combination..."
gcov-tool overlap -f -o gcov_data_multi/*.gcda > gcov_output.txt 2>&1

# Combination 2: -F -h -t 1.0
echo "Testing -F -h -t 1.0 combination..."
gcov-tool overlap -F -h -t 1.0 gcov_data_multi/*.gcda > gcov_output.txt 2>&1

# Combination 3: -v -f -F -o -h -t 5.0 (all flags)
echo "Testing all flags combination..."
gcov-tool overlap -v -f -F -o -h -t 5.0 gcov_data_multi/*.gcda > gcov_output.txt 2>&1

# Combination 4: -v -f -t 0.8 with specific files
echo "Testing -v -f -t 0.8 with specific files..."
gcov-tool overlap -v -f -t 0.8 test_prog_run1.gcda test_prog_run2.gcda > gcov_output.txt 2>&1

# Combination 5: -F -o -t 0.3 with mixed files
echo "Testing -F -o -t 0.3 with mixed files..."
gcov-tool overlap -F -o -t 0.3 test_prog_run1.gcda test_prog_opt_run1.gcda > gcov_output.txt 2>&1

# Step 6: Test invalid flag to trigger usage
echo -e "\n=== Testing invalid flag (to trigger usage) ==="
echo "Testing invalid -Z flag (should show usage)..."
gcov-tool overlap -Z gcov_data_multi/*.gcda 2>&1 | head -20

# Step 7: Test with different numbers of input files
echo -e "\n=== Testing with varying numbers of input files ==="

# Single file
echo "Testing with single file..."
gcov-tool overlap -f test_prog_run1.gcda > gcov_output.txt 2>&1

# Two files
echo "Testing with two files..."
gcov-tool overlap -f -o test_prog_run1.gcda test_prog_run2.gcda > gcov_output.txt 2>&1

# Three files
echo "Testing with three files..."
gcov-tool overlap -v -f -t 0.7 test_prog_run1.gcda test_prog_run2.gcda test_prog_run3.gcda > gcov_output.txt 2>&1

# Multiple files from directory
echo "Testing with all files from directory..."
gcov-tool overlap -F -h gcov_data_multi/*.gcda > gcov_output.txt 2>&1

# Step 8: Test edge cases for threshold
echo -e "\n=== Testing threshold edge cases ==="

echo "Testing with threshold 0.0..."
gcov-tool overlap -t 0.0 gcov_data_multi/*.gcda > gcov_output.txt 2>&1

echo "Testing with threshold 100.0..."
gcov-tool overlap -t 100.0 gcov_data_multi/*.gcda > gcov_output.txt 2>&1

echo "Testing with threshold 0.001..."
gcov-tool overlap -t 0.001 gcov_data_multi/*.gcda > gcov_output.txt 2>&1

# Step 9: Test with different file orders and combinations
echo -e "\n=== Testing file ordering and combinations ==="

# Different order of flags
echo "Testing flag order variation 1..."
gcov-tool overlap -t 2.5 -f -o gcov_data_multi/*.gcda > gcov_output.txt 2>&1

echo "Testing flag order variation 2..."
gcov-tool overlap -h -F -v -t 3.0 gcov_data_multi/*.gcda > gcov_output.txt 2>&1

# Mixed gcda files from different compilations
echo "Testing with mixed compilation profiles..."
gcov-tool overlap -f -o test_prog_run1.gcda test_prog_opt_run1.gcda > gcov_output.txt 2>&1

# Step 10: Verify coverage by checking if commands executed
echo -e "\n=== Test Summary ==="
echo "All gcov-tool overlap flag tests completed successfully!"
echo ""
echo "Flags tested individually:"
echo "  -v (verbose)"
echo "  -f (function level)"
echo "  -F (fullname)"
echo "  -o (object level)"
echo "  -h (hot only)"
echo "  -t (threshold with values: 0.0, 0.5, 1.0, 10.5, 100.0, 0.001)"
echo ""
echo "Flag combinations tested:"
echo "  -f -o"
echo "  -F -h -t 1.0"
echo "  -v -f -F -o -h -t 5.0 (all flags)"
echo "  -v -f -t 0.8"
echo "  -F -o -t 0.3"
echo ""
echo "Additional tests:"
echo "  Invalid flag (-Z) to trigger usage"
echo "  Varying numbers of input files (1, 2, 3, many)"
echo "  Different flag orders"
echo "  Mixed profile files from different compilations"
echo ""
echo "Generated files:"
ls -la *.gcda *.gcno 2>/dev/null | wc -l | xargs echo "  Number of gcda/gcno files:"
echo "  Directory with multiple files: gcov_data_multi/"

# Optional: Keep generated files for inspection
# cleanup

echo -e "\n=== Test completed successfully! ==="
```

This script comprehensively tests all the uncovered lines by:

1. **Generating valid GCOV data**: Creates a C program with conditional logic, compiles it with GCOV instrumentation at different optimization levels, and runs it multiple times with different inputs to generate varied `.gcda` files.

2. **Testing all individual flags**: Each uncovered flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually with valid GCOV data.

3. **Testing flag combinations**: Multiple combinations of flags are tested together to ensure the parser handles them correctly.

4. **Triggering the usage function**: Includes a test with invalid flag `-Z` to exercise the `default` case and `overlap_usage()`.

5. **Using multiple input files**: Tests with 1, 2, 3, and many `.gcda` files to stress the overlap merging logic.

6. **Testing threshold variations**: Uses different floating-point values for `-t` flag including edge cases.

7. **Handling verbose output**: The `-v` flag is tested both individually and in combinations, with output redirected.

The script is self-contained and will clean up after itself (commented out at the end, but can be enabled). It provides clear output showing what's being tested at each step.
