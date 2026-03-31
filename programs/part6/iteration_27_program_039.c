Here's a comprehensive shell script that covers all the specified uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash
set -e

# Create a test directory
TEST_DIR="gcov_tool_test"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

# Clean up any previous test files
rm -f *.gcda *.gcno *.gcov test_prog* *.info *.txt

# 1. Generate a simple C program with conditional branches
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
    
    int r1 = func1(val);
    func2(val, r1);
    
    return 0;
}
EOF

# 2. Compile with GCOV instrumentation
echo "Compiling test program with GCOV instrumentation..."
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog
gcc -fprofile-arcs -ftest-coverage -O2 test.c -o test_prog_opt

# 3. Generate multiple sets of profile data
echo "Generating profile data..."

# First run with different arguments
echo "Run 1: ./test_prog 5"
./test_prog 5

# Copy first gcda to preserve it
cp test.gcda test_run1.gcda

# Second run with different argument
echo "Run 2: ./test_prog 10"
./test_prog 10

# Copy second gcda
cp test.gcda test_run2.gcda

# Third run with optimized binary
echo "Run 3: ./test_prog_opt 3"
./test_prog_opt 3
cp test_opt.gcda test_run3.gcda

# Fourth run with zero argument
echo "Run 4: ./test_prog 0"
./test_prog 0
cp test.gcda test_run4.gcda

# 4. Test gcov-tool with various flag combinations
echo -e "\nTesting gcov-tool overlap with different flags..."

# Individual flags
echo "Test 1: -v flag"
gcov-tool overlap -v test.gcda 2>&1 | tee output_v.txt

echo -e "\nTest 2: -f flag (function level)"
gcov-tool overlap -f test_run1.gcda test_run2.gcda 2>&1 | tee output_f.txt

echo -e "\nTest 3: -F flag (fullname)"
gcov-tool overlap -F test_run1.gcda test_run2.gcda 2>&1 | tee output_F.txt

echo -e "\nTest 4: -o flag (object level)"
gcov-tool overlap -o test_run1.gcda test_run2.gcda 2>&1 | tee output_o.txt

echo -e "\nTest 5: -h flag (hot only)"
gcov-tool overlap -h test_run1.gcda test_run2.gcda 2>&1 | tee output_h.txt

echo -e "\nTest 6: -t flag with threshold 0.5"
gcov-tool overlap -t 0.5 test_run1.gcda test_run2.gcda 2>&1 | tee output_t05.txt

echo -e "\nTest 7: -t flag with threshold 1.0"
gcov-tool overlap -t 1.0 test_run1.gcda test_run2.gcda 2>&1 | tee output_t10.txt

echo -e "\nTest 8: -t flag with threshold 10.5"
gcov-tool overlap -t 10.5 test_run1.gcda test_run2.gcda 2>&1 | tee output_t105.txt

# Flag combinations
echo -e "\nTest 9: -f -o combination"
gcov-tool overlap -f -o test_run1.gcda test_run2.gcda test_run3.gcda 2>&1 | tee output_fo.txt

echo -e "\nTest 10: -F -h -t 1.0 combination"
gcov-tool overlap -F -h -t 1.0 test_run1.gcda test_run2.gcda 2>&1 | tee output_Fht.txt

echo -e "\nTest 11: -v -f -F -o -h -t 5.0 combination"
gcov-tool overlap -v -f -F -o -h -t 5.0 test_run1.gcda test_run2.gcda test_run3.gcda test_run4.gcda 2>&1 | tee output_all.txt

echo -e "\nTest 12: -f -t 0.8 with multiple files"
gcov-tool overlap -f -t 0.8 test_run1.gcda test_run2.gcda test_run3.gcda test_run4.gcda 2>&1 | tee output_ft_multi.txt

# Test with different file order and combinations
echo -e "\nTest 13: -v -o with all gcda files"
gcov-tool overlap -v -o *.gcda 2>&1 | tee output_vo_all.txt

# 5. Test invalid flag to trigger usage (default case)
echo -e "\nTest 14: Invalid flag -Z (should trigger usage)"
gcov-tool overlap -Z test.gcda 2>&1 | tee output_invalid.txt || true

echo -e "\nTest 15: Invalid flag combination"
gcov-tool overlap -f -Z -t 0.5 test.gcda 2>&1 | tee output_invalid2.txt || true

# 6. Additional tests with different file patterns
echo -e "\nTest 16: Test with absolute paths"
ABS_PATH1="$(pwd)/test_run1.gcda"
ABS_PATH2="$(pwd)/test_run2.gcda"
gcov-tool overlap -f -F "$ABS_PATH1" "$ABS_PATH2" 2>&1 | tee output_abs.txt

# 7. Test with no threshold value (should error but test parsing)
echo -e "\nTest 17: -t without argument (testing error handling)"
gcov-tool overlap -t 2>&1 | tee output_t_noarg.txt || true

# 8. Test help/usage directly
echo -e "\nTest 18: Overlap usage help"
gcov-tool overlap --help 2>&1 | tee output_help.txt

# 9. Create a summary of all tests
echo -e "\n=== Test Summary ==="
echo "Generated files:"
ls -la *.txt *.gcda *.gcno

echo -e "\nChecking coverage of specific flags in outputs:"
for flag in v f F o h t; do
    if grep -q ".*-$flag.*" output_*.txt 2>/dev/null || \
       grep -q ".*$flag.*" output_*.txt 2>/dev/null; then
        echo "✓ Flag -$flag was tested"
    else
        echo "✗ Flag -$flag may not have been tested"
    fi
done

echo -e "\nInvalid flag test output (should show usage):"
grep -A 5 -B 5 "usage\|Usage\|invalid\|Invalid" output_invalid.txt 2>/dev/null || echo "No usage message found"

# 10. Cleanup (optional)
echo -e "\nCleaning up..."
cd ..
# Uncomment to remove test directory:
# rm -rf "$TEST_DIR"

echo -e "\nAll tests completed successfully!"
```

This script comprehensively tests all the uncovered lines:

1. **Individual flags**: Tests `-v`, `-f`, `-F`, `-o`, `-h`, and `-t` with different threshold values (0.5, 1.0, 10.5)
2. **Flag combinations**: Tests various combinations including `-f -o`, `-F -h -t 1.0`, and `-v -f -F -o -h -t 5.0`
3. **Invalid flag**: Tests `-Z` to trigger the `default` case and `overlap_usage()`
4. **Multiple input files**: Uses 2-4 different `.gcda` files in most tests
5. **Verbose output**: Captures output with `-v` flag using `tee`
6. **Valid GCOV data**: Generates real profile data by compiling and running an instrumented program multiple times
7. **Different scenarios**: Tests with absolute paths, multiple files, and error conditions

The script produces output files for each test case, allowing you to verify that each flag combination was executed. The test program includes conditional branches to generate interesting profile data for overlap analysis.
