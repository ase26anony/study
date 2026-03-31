Here's a comprehensive shell script that covers all the specified requirements for testing the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash

# test_gcov_tool.sh - Comprehensive test for gcov-tool overlap analysis flags

set -e  # Exit on any error

echo "=== Starting gcov-tool overlap analysis flag coverage test ==="

# Clean up any previous test files
cleanup() {
    echo "Cleaning up..."
    rm -f test.c test_prog test_prog_opt test_prog_lto
    rm -f *.gcda *.gcno *.gcov gcov_output*.txt
    rm -rf profile_data_* test_dir_*
}

trap cleanup EXIT

# Step 1: Generate a simple C program with conditional branches
echo "Generating test C program..."
cat > test.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

int function1(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
        return x * 2;
    } else if (x < 0) {
        printf("Negative: %d\n", x);
        return x * 3;
    } else {
        printf("Zero\n");
        return 0;
    }
}

void function2(int a, int b) {
    for (int i = 0; i < a; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
    
    while (b-- > 0) {
        printf("Countdown: %d\n", b);
    }
}

int main(int argc, char *argv[]) {
    int val = 0;
    if (argc > 1) {
        val = atoi(argv[1]);
    }
    
    int result = function1(val);
    function2(val, result % 3);
    
    return 0;
}
EOF

# Step 2: Compile with different optimization levels to generate varied .gcno files
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
echo "Generating profile data..."

# Create directories for different profile runs
mkdir -p profile_data_1 profile_data_2 profile_data_3

# Run 1: Normal execution
echo "Run 1: Normal execution"
./test_prog 5
mv *.gcda profile_data_1/ 2>/dev/null || true

# Run 2: Different input
echo "Run 2: Different input"
./test_prog -3
mv *.gcda profile_data_2/ 2>/dev/null || true

# Run 3: Zero input
echo "Run 3: Zero input"
./test_prog 0
mv *.gcda profile_data_3/ 2>/dev/null || true

# Run optimized version for different profile patterns
echo "Run 4: Optimized version"
./test_prog_opt 2
mv *.gcda profile_data_1/ 2>/dev/null || true

# Copy .gcno files to each directory
cp test.gcno profile_data_1/
cp test.gcno profile_data_2/
cp test.gcno profile_data_3/

# Step 4: Test individual flags
echo -e "\n=== Testing individual flags ==="

# Test -v flag (verbose)
echo "Testing -v flag..."
gcov-tool overlap -v profile_data_1/test.gcda profile_data_2/test.gcda > gcov_output_verbose.txt 2>&1
echo "  Verbose output saved to gcov_output_verbose.txt"

# Test -f flag (function level)
echo "Testing -f flag..."
gcov-tool overlap -f profile_data_1/test.gcda profile_data_2/test.gcda > gcov_output_func.txt 2>&1

# Test -F flag (full name)
echo "Testing -F flag..."
gcov-tool overlap -F profile_data_1/test.gcda profile_data_2/test.gcda > gcov_output_fullname.txt 2>&1

# Test -o flag (object level)
echo "Testing -o flag..."
gcov-tool overlap -o profile_data_1/test.gcda profile_data_2/test.gcda > gcov_output_obj.txt 2>&1

# Test -h flag (hot only)
echo "Testing -h flag..."
gcov-tool overlap -h profile_data_1/test.gcda profile_data_2/test.gcda > gcov_output_hot.txt 2>&1

# Test -t flag with threshold (requires argument)
echo "Testing -t flag with threshold 0.5..."
gcov-tool overlap -t 0.5 profile_data_1/test.gcda profile_data_2/test.gcda > gcov_output_threshold_0.5.txt 2>&1

echo "Testing -t flag with threshold 1.0..."
gcov-tool overlap -t 1.0 profile_data_1/test.gcda profile_data_2/test.gcda > gcov_output_threshold_1.0.txt 2>&1

echo "Testing -t flag with threshold 10.5..."
gcov-tool overlap -t 10.5 profile_data_1/test.gcda profile_data_2/test.gcda > gcov_output_threshold_10.5.txt 2>&1

# Step 5: Test flag combinations
echo -e "\n=== Testing flag combinations ==="

# Test -f -o combination
echo "Testing -f -o combination..."
gcov-tool overlap -f -o profile_data_1/test.gcda profile_data_2/test.gcda profile_data_3/test.gcda > gcov_output_fo.txt 2>&1

# Test -F -h -t combination
echo "Testing -F -h -t 1.0 combination..."
gcov-tool overlap -F -h -t 1.0 profile_data_1/test.gcda profile_data_2/test.gcda > gcov_output_Fht.txt 2>&1

# Test -v -f -F -o -h -t combination (all flags)
echo "Testing all flags combination (-v -f -F -o -h -t 5.0)..."
gcov-tool overlap -v -f -F -o -h -t 5.0 profile_data_1/test.gcda profile_data_2/test.gcda > gcov_output_all_flags.txt 2>&1

# Test with multiple input files (3 different profiles)
echo "Testing with 3 input files and -f -o flags..."
gcov-tool overlap -f -o profile_data_1/test.gcda profile_data_2/test.gcda profile_data_3/test.gcda > gcov_output_multi.txt 2>&1

# Step 6: Test error case (invalid flag to trigger usage)
echo -e "\n=== Testing error case (invalid flag) ==="
echo "Testing invalid flag -Z (should trigger usage)..."
gcov-tool overlap -Z profile_data_1/test.gcda 2>&1 | head -20 > gcov_output_invalid.txt || true
echo "  Invalid flag output saved to gcov_output_invalid.txt"

# Step 7: Test with different .gcno structures
echo -e "\n=== Testing with different .gcno structures ==="

# Test with optimized version's .gcda
if [ -f test_prog_opt.gcda ]; then
    echo "Testing with optimized program's profile..."
    gcov-tool overlap -f -o test.gcda test_prog_opt.gcda > gcov_output_opt.txt 2>&1
fi

# Test with absolute paths
echo "Testing with absolute paths..."
ABS_PATH1="$(pwd)/profile_data_1/test.gcda"
ABS_PATH2="$(pwd)/profile_data_2/test.gcda"
gcov-tool overlap -F "$ABS_PATH1" "$ABS_PATH2" > gcov_output_abs.txt 2>&1

# Step 8: Test edge cases
echo -e "\n=== Testing edge cases ==="

# Test with threshold 0.0
echo "Testing with threshold 0.0..."
gcov-tool overlap -t 0.0 profile_data_1/test.gcda profile_data_2/test.gcda > gcov_output_threshold_0.0.txt 2>&1

# Test with threshold 100.0
echo "Testing with threshold 100.0..."
gcov-tool overlap -t 100.0 profile_data_1/test.gcda profile_data_2/test.gcda > gcov_output_threshold_100.0.txt 2>&1

# Test -h without other flags
echo "Testing -h flag alone..."
gcov-tool overlap -h profile_data_1/test.gcda profile_data_2/test.gcda > gcov_output_h_only.txt 2>&1

# Test -v with -t combination
echo "Testing -v with -t 0.8..."
gcov-tool overlap -v -t 0.8 profile_data_1/test.gcda profile_data_2/test.gcda > gcov_output_vt.txt 2>&1

# Step 9: Verify all output files were created
echo -e "\n=== Verification ==="
echo "Generated output files:"
ls -la gcov_output_*.txt

echo -e "\n=== Test Summary ==="
echo "Tested all individual flags: -v, -f, -F, -o, -h, -t"
echo "Tested flag combinations: -f -o, -F -h -t, all flags together"
echo "Tested error case: invalid -Z flag"
echo "Tested with multiple input files (3 different profiles)"
echo "Tested with absolute paths"
echo "Tested various threshold values: 0.0, 0.5, 1.0, 5.0, 10.5, 100.0"

echo -e "\n=== gcov-tool overlap analysis flag coverage test completed ==="
```

This script provides comprehensive coverage of the uncovered lines by:

1. **Generating valid GCOV data**: Creates a C program with conditional branches, compiles it with GCOV instrumentation, and runs it multiple times to generate `.gcda` files.

2. **Testing all individual flags**: 
   - `-v` (verbose)
   - `-f` (function level)
   - `-F` (full name)
   - `-o` (object level)
   - `-h` (hot only)
   - `-t` with various thresholds (0.5, 1.0, 10.5)

3. **Testing flag combinations**:
   - `-f -o`
   - `-F -h -t 1.0`
   - All flags together: `-v -f -F -o -h -t 5.0`

4. **Triggering the default case**: Includes an invalid flag `-Z` to trigger `overlap_usage()`.

5. **Using multiple input files**: Tests with 3 different `.gcda` files from different runs.

6. **Testing edge cases**: Includes threshold values of 0.0 and 100.0, and tests with absolute paths.

7. **Handling verbose output**: Captures verbose output to files to ensure the code path is executed.

The script is self-contained, cleans up after itself, and provides clear output about what's being tested. Each test invocation uses the `overlap` subcommand followed by the flags being tested, exactly as required by the parsing logic in the uncovered lines.
