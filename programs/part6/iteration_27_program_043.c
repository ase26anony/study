Here's a comprehensive shell script that covers all the specified requirements:

```bash
#!/bin/bash
set -e

# Test script for gcov-tool overlap functionality
# This script tests the uncovered lines in gcov-tool.cc (lines 534-554)

echo "=== Starting gcov-tool overlap flag testing ==="

# Create a test directory
TEST_DIR="gcov_tool_test"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

# Clean up any previous test files
rm -f *.gcda *.gcno *.gcov test_prog* *.out *.log

# 1. Generate a simple C program with conditional branches
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

int func2(int y) {
    for (int i = 0; i < y; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
    return y * 3;
}

int main(int argc, char *argv[]) {
    int val = 1;
    if (argc > 1) {
        val = atoi(argv[1]);
    }
    
    int result1 = func1(val);
    int result2 = func2(val);
    
    printf("Results: %d, %d\n", result1, result2);
    return 0;
}
EOF

# 2. Compile with GCOV instrumentation
echo "Compiling test program with GCOV instrumentation..."
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog

# 3. Generate multiple profile data runs
echo "Generating profile data..."

# First run
echo "Run 1: val=5"
./test_prog 5
mv test.gcda test_run1.gcda

# Second run with different input
echo "Run 2: val=10"
./test_prog 10
mv test.gcda test_run2.gcda

# Third run with zero input
echo "Run 3: val=0"
./test_prog 0
mv test.gcda test_run3.gcda

# Create a copy with different name for fullname testing
cp test.c test2.c
gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test_prog2
./test_prog2 3
mv test2.gcda test_run4.gcda

# 4. Test individual flags
echo -e "\n=== Testing individual flags ==="

# Test -v flag (verbose)
echo "Testing -v flag..."
gcov-tool overlap -v test_run1.gcda test_run2.gcda > verbose_output.log 2>&1
echo "Verbose output saved to verbose_output.log"

# Test -f flag (function level)
echo "Testing -f flag..."
gcov-tool overlap -f test_run1.gcda test_run2.gcda > func_level_output.log 2>&1

# Test -F flag (fullname)
echo "Testing -F flag..."
gcov-tool overlap -F test_run1.gcda test_run2.gcda > fullname_output.log 2>&1

# Test -o flag (object level)
echo "Testing -o flag..."
gcov-tool overlap -o test_run1.gcda test_run2.gcda > obj_level_output.log 2>&1

# Test -h flag (hot only)
echo "Testing -h flag..."
gcov-tool overlap -h test_run1.gcda test_run2.gcda > hot_only_output.log 2>&1

# Test -t flag with threshold (requires argument)
echo "Testing -t flag with threshold 0.5..."
gcov-tool overlap -t 0.5 test_run1.gcda test_run2.gcda > threshold_0.5_output.log 2>&1

echo "Testing -t flag with threshold 1.0..."
gcov-tool overlap -t 1.0 test_run1.gcda test_run2.gcda > threshold_1.0_output.log 2>&1

echo "Testing -t flag with threshold 10.5..."
gcov-tool overlap -t 10.5 test_run1.gcda test_run2.gcda > threshold_10.5_output.log 2>&1

# 5. Test flag combinations
echo -e "\n=== Testing flag combinations ==="

# Test -f -o combination
echo "Testing -f -o combination..."
gcov-tool overlap -f -o test_run1.gcda test_run2.gcda test_run3.gcda > combo_fo_output.log 2>&1

# Test -F -h -t combination
echo "Testing -F -h -t 1.0 combination..."
gcov-tool overlap -F -h -t 1.0 test_run1.gcda test_run2.gcda > combo_Fht_output.log 2>&1

# Test -v -f -F -o -h -t combination
echo "Testing -v -f -F -o -h -t 5.0 combination..."
gcov-tool overlap -v -f -F -o -h -t 5.0 test_run1.gcda test_run2.gcda test_run3.gcda > combo_all_output.log 2>&1

# Test with multiple input files and various flags
echo "Testing with multiple files and -f -o -t 0.8..."
gcov-tool overlap -f -o -t 0.8 test_run1.gcda test_run2.gcda test_run3.gcda test_run4.gcda > combo_multi_output.log 2>&1

# 6. Test error case (invalid flag to trigger usage)
echo -e "\n=== Testing invalid flag (should trigger usage) ==="
echo "Testing invalid flag -Z..."
gcov-tool overlap -Z test_run1.gcda 2>&1 | head -20 > invalid_flag_output.log
echo "Invalid flag output saved to invalid_flag_output.log"

# 7. Additional tests with different compilation options
echo -e "\n=== Testing with differently compiled programs ==="

# Compile with optimization
gcc -fprofile-arcs -ftest-coverage -O2 test.c -o test_prog_opt
./test_prog_opt 7
mv test.gcda test_opt.gcda

# Test with optimized binary's profile
echo "Testing with optimized binary profile..."
gcov-tool overlap -f -F test_run1.gcda test_opt.gcda > opt_combo_output.log 2>&1

# 8. Test edge cases
echo -e "\n=== Testing edge cases ==="

# Test with single file (should still work)
echo "Testing with single file..."
gcov-tool overlap -v test_run1.gcda > single_file_output.log 2>&1

# Test with threshold 0.0
echo "Testing with threshold 0.0..."
gcov-tool overlap -t 0.0 test_run1.gcda test_run2.gcda > threshold_0.0_output.log 2>&1

# Test with threshold 100.0 (high value)
echo "Testing with threshold 100.0..."
gcov-tool overlap -t 100.0 test_run1.gcda test_run2.gcda > threshold_100.0_output.log 2>&1

# 9. Verify all output files were created
echo -e "\n=== Verification ==="
echo "Generated output files:"
ls -la *.log

# 10. Check for any obvious errors in outputs
echo -e "\n=== Checking for errors ==="
for logfile in *.log; do
    if grep -q -i "error\|fail\|abort\|segmentation" "$logfile"; then
        echo "WARNING: Possible error in $logfile"
        grep -i "error\|fail\|abort\|segmentation" "$logfile" | head -5
    fi
done

echo -e "\n=== Test summary ==="
echo "Total test invocations completed successfully."
echo "The following flags were tested:"
echo "  Individual: -v, -f, -F, -o, -h, -t (with various values)"
echo "  Combinations: -f -o, -F -h -t 1.0, -v -f -F -o -h -t 5.0, -f -o -t 0.8"
echo "  Error case: -Z (invalid flag)"
echo "  Multiple input files tested (up to 4 .gcda files)"
echo "  Different compilation modes (O0, O2)"
echo ""
echo "All generated files are in: $PWD"
echo "To clean up: cd .. && rm -rf $TEST_DIR"

cd ..
```

This script comprehensively tests all the uncovered lines in `gcov-tool.cc`:

1. **Individual flag testing**: Tests each flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) separately
2. **Flag combinations**: Tests various combinations as specified
3. **Invalid flag**: Tests `-Z` to trigger the `default` case and `overlap_usage()`
4. **Multiple input files**: Uses up to 4 different `.gcda` files
5. **Different threshold values**: Tests `0.5`, `1.0`, `10.5`, `0.0`, and `100.0`
6. **Verbose output**: Captures verbose output to files
7. **Different compilation options**: Tests with `-O0` and `-O2` compiled programs
8. **Edge cases**: Tests with single file, extreme threshold values

The script creates a self-contained test environment, generates the necessary GCOV data files, and systematically exercises all the command-line parsing logic for the overlap functionality.
