Looking at the uncovered lines in `gcov-tool.cc`, I need to create a comprehensive test that exercises the overlap analysis feature flags. Here's a shell script that systematically tests all the uncovered command-line options:

```bash
#!/bin/bash
# test_gcov_tool.sh - Comprehensive test for gcov-tool overlap analysis flags

set -e  # Exit on any error

echo "=== Starting gcov-tool overlap flag coverage test ==="

# Create a simple test C program with conditional branches
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
    
    int x = calculate(10, 5, mode);
    printf("Result: %d\n", x);
    
    // Another function call with different parameters
    if (mode % 2 == 0) {
        calculate(3, 7, 2);
    } else {
        calculate(8, 2, 1);
    }
    
    return 0;
}
EOF

echo "Created test.c"

# Compile with coverage instrumentation
echo "Compiling test program with coverage..."
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog

# Create directory for different profile runs
mkdir -p profile_runs

echo -e "\n=== Generating profile data with different executions ==="

# First execution - mode 1
echo "Run 1: mode=1"
./test_prog 1
cp test.gcda profile_runs/test_run1.gcda
rm -f test.gcda

# Second execution - mode 2
echo "Run 2: mode=2"
./test_prog 2
cp test.gcda profile_runs/test_run2.gcda
rm -f test.gcda

# Third execution - mode 3 (different path)
echo "Run 3: mode=3"
./test_prog 3
cp test.gcda profile_runs/test_run3.gcda
rm -f test.gcda

# Create a copy of gcno file in profile_runs for convenience
cp test.gcno profile_runs/

echo -e "\n=== Testing individual overlap flags ==="

# Test verbose flag (-v)
echo "Testing: -v flag"
gcov-tool overlap -v profile_runs/test_run1.gcda profile_runs/test_run2.gcda > verbose_output.txt 2>&1
echo "  Verbose output saved to verbose_output.txt"

# Test function level overlap (-f)
echo "Testing: -f flag"
gcov-tool overlap -f profile_runs/test_run1.gcda profile_runs/test_run2.gcda > func_level_output.txt 2>&1

# Test full filename display (-F)
echo "Testing: -F flag"
gcov-tool overlap -F profile_runs/test_run1.gcda profile_runs/test_run2.gcda > fullname_output.txt 2>&1

# Test object level overlap (-o)
echo "Testing: -o flag"
gcov-tool overlap -o profile_runs/test_run1.gcda profile_runs/test_run2.gcda > obj_level_output.txt 2>&1

# Test hot only (-h)
echo "Testing: -h flag"
gcov-tool overlap -h profile_runs/test_run1.gcda profile_runs/test_run2.gcda > hot_only_output.txt 2>&1

# Test threshold with different values (-t)
echo "Testing: -t 0.5 flag"
gcov-tool overlap -t 0.5 profile_runs/test_run1.gcda profile_runs/test_run2.gcda > threshold_0.5_output.txt 2>&1

echo "Testing: -t 1.0 flag"
gcov-tool overlap -t 1.0 profile_runs/test_run1.gcda profile_runs/test_run2.gcda > threshold_1.0_output.txt 2>&1

echo "Testing: -t 10.5 flag"
gcov-tool overlap -t 10.5 profile_runs/test_run1.gcda profile_runs/test_run2.gcda > threshold_10.5_output.txt 2>&1

echo -e "\n=== Testing flag combinations ==="

# Test combination: -f -o
echo "Testing: -f -o combination"
gcov-tool overlap -f -o profile_runs/test_run1.gcda profile_runs/test_run2.gcda > combo_fo_output.txt 2>&1

# Test combination: -F -h -t 1.0
echo "Testing: -F -h -t 1.0 combination"
gcov-tool overlap -F -h -t 1.0 profile_runs/test_run1.gcda profile_runs/test_run2.gcda > combo_Fht_output.txt 2>&1

# Test combination: -v -f -F -o -h -t 5.0
echo "Testing: -v -f -F -o -h -t 5.0 combination"
gcov-tool overlap -v -f -F -o -h -t 5.0 profile_runs/test_run1.gcda profile_runs/test_run2.gcda > combo_all_output.txt 2>&1

# Test with three input files
echo "Testing with three input files: -f -o"
gcov-tool overlap -f -o profile_runs/test_run1.gcda profile_runs/test_run2.gcda profile_runs/test_run3.gcda > three_files_output.txt 2>&1

echo -e "\n=== Testing error cases ==="

# Test invalid flag to trigger overlap_usage() (default case)
echo "Testing invalid flag -Z (should trigger usage)"
gcov-tool overlap -Z profile_runs/test_run1.gcda 2> invalid_flag_error.txt || true
echo "  Error output saved to invalid_flag_error.txt"

# Test missing argument for -t flag
echo "Testing missing argument for -t flag"
gcov-tool overlap -t profile_runs/test_run1.gcda 2> missing_arg_error.txt || true
echo "  Error output saved to missing_arg_error.txt"

echo -e "\n=== Testing with different compilation options ==="

# Compile with optimization to get different profile patterns
echo "Compiling with optimization -O2"
gcc -fprofile-arcs -ftest-coverage -O2 test.c -o test_prog_opt

# Generate profile with optimized binary
echo "Running optimized binary"
./test_prog_opt 1
gcov-tool overlap -f -F test.gcda profile_runs/test_run1.gcda > optimized_compare_output.txt 2>&1
rm -f test.gcda

echo -e "\n=== Summary ==="
echo "Generated test files:"
ls -la profile_runs/
echo -e "\nGenerated output files:"
ls -la *output*.txt *error*.txt 2>/dev/null || true

echo -e "\n=== Testing completed ==="
echo "The following flags were tested:"
echo "  Individual: -v, -f, -F, -o, -h, -t (0.5, 1.0, 10.5)"
echo "  Combinations: -f -o, -F -h -t 1.0, -v -f -F -o -h -t 5.0"
echo "  Multiple files: 3 .gcda files"
echo "  Error cases: invalid flag -Z, missing -t argument"
echo "  Different compilations: -O0 and -O2"

# Cleanup (optional)
# echo -e "\nCleaning up..."
# rm -f test.c test_prog test_prog_opt *.gcda *.gcno
# rm -rf profile_runs
# rm -f *.txt

echo -e "\nTo clean up generated files, run:"
echo "  rm -f test.c test_prog test_prog_opt *.gcda *.gcno *.txt"
echo "  rm -rf profile_runs"
```

This script provides comprehensive coverage of the uncovered lines:

1. **Individual flag testing**: Each uncovered flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually with valid `.gcda` files.

2. **Flag combinations**: Multiple combinations of flags are tested to ensure the parser handles them correctly together.

3. **Valid GCOV data**: The script creates a C program, compiles it with coverage instrumentation (`-fprofile-arcs -ftest-coverage`), and runs it multiple times to generate distinct `.gcda` files.

4. **Overlap analysis mode**: All invocations use `gcov-tool overlap` as the subcommand before the flags.

5. **Error testing**: The `-Z` flag triggers the `default` case in the switch statement, calling `overlap_usage()`.

6. **Multiple input files**: Tests with 2 and 3 `.gcda` files to stress the overlap merging logic.

7. **Verbose output**: The `-v` flag is tested both individually and in combination, with output captured to files.

8. **Different threshold values**: Tests `-t` with `0.5`, `1.0`, and `10.5` to cover different floating-point inputs.

9. **Different compilation options**: Also tests with an optimized build (`-O2`) to potentially generate different profile patterns.

The script creates a self-contained test environment, generates all necessary profile data, and systematically exercises all the command-line parsing logic for the overlap analysis feature.
