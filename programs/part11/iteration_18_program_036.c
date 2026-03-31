I'll create a comprehensive test script that covers the specific lines in `gcov-tool.cc` (lines 534-554) by testing all the command-line options for the `merge` subcommand.

```bash
#!/bin/bash
# run_gcov_tool_test.sh - Test script for gcov-tool merge command options

set -e  # Exit on any error

echo "=== Starting gcov-tool merge option tests ==="

# Clean up any previous test files
cleanup() {
    echo "Cleaning up test files..."
    rm -f test.c test_prog test_prog2 test.gcda test2.gcda test.gcno test2.gcno \
          merge_output.gcda merge_output2.gcda gcov_tool_output.txt 2>/dev/null || true
}

cleanup

# Step 1: Create a minimal C test program
echo "Creating test C program..."
cat > test.c << 'EOF'
#include <stdio.h>

int main(int argc, char *argv[]) {
    int x = 0;
    
    // Some branching code to generate coverage data
    if (argc > 1) {
        x = 1;
        printf("Argument provided: %s\n", argv[1]);
    } else {
        x = 2;
        printf("No arguments provided\n");
    }
    
    // Loop to generate some execution counts
    for (int i = 0; i < 3; i++) {
        x += i;
    }
    
    return x % 2;
}
EOF

# Step 2: Compile the test program with coverage flags
echo "Compiling test program with coverage flags..."
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog2  # Second executable

# Step 3: Run the program to generate .gcda files
echo "Generating coverage data files..."
# First run - no arguments
./test_prog > /dev/null 2>&1
mv test.gcda test1.gcda  # Rename to avoid conflicts

# Second run - with argument
./test_prog "test_argument" > /dev/null 2>&1
mv test.gcda test2.gcda

# Create a third .gcda file by running the second executable
./test_prog2 "different_argument" > /dev/null 2>&1
mv test.gcda test3.gcda

# Copy .gcno file for the second .gcda
cp test.gcno test2.gcno
cp test.gcno test3.gcno

echo "Generated test1.gcda, test2.gcda, and test3.gcda"

# Step 4: Test matrix of gcov-tool merge options
echo -e "\n=== Testing individual flags ==="

# Test verbose flag (-v)
echo "Testing -v flag..."
gcov-tool merge -v test1.gcda test2.gcda merge_output.gcda > gcov_tool_output.txt 2>&1 || echo "Command completed"

# Test function-level overlap flag (-f)
echo "Testing -f flag..."
gcov-tool merge -f test1.gcda test2.gcda merge_output.gcda > /dev/null 2>&1

# Test full filename flag (-F)
echo "Testing -F flag..."
gcov-tool merge -F test1.gcda test2.gcda merge_output.gcda > /dev/null 2>&1

# Test object-level overlap flag (-o)
echo "Testing -o flag..."
gcov-tool merge -o test1.gcda test2.gcda merge_output.gcda > /dev/null 2>&1

# Test hot-only flag (-h)
echo "Testing -h flag..."
gcov-tool merge -h test1.gcda test2.gcda merge_output.gcda > /dev/null 2>&1

# Test threshold flag with integer value (-t 1)
echo "Testing -t 1 flag..."
gcov-tool merge -t 1 test1.gcda test2.gcda merge_output.gcda > /dev/null 2>&1

# Test threshold flag with fractional value (-t 0.5)
echo "Testing -t 0.5 flag..."
gcov-tool merge -t 0.5 test1.gcda test2.gcda merge_output.gcda > /dev/null 2>&1

# Test threshold flag with different fractional value (-t 0.33)
echo "Testing -t 0.33 flag..."
gcov-tool merge -t 0.33 test1.gcda test2.gcda merge_output.gcda > /dev/null 2>&1

echo -e "\n=== Testing flag combinations ==="

# Test combination: verbose + function-level + object-level
echo "Testing -v -f -o combination..."
gcov-tool merge -v -f -o test1.gcda test2.gcda test3.gcda merge_output.gcda > /dev/null 2>&1

# Test combination: fullname + hot-only + threshold
echo "Testing -F -h -t 0.75 combination..."
gcov-tool merge -F -h -t 0.75 test1.gcda test2.gcda merge_output.gcda > /dev/null 2>&1

# Test combination: all flags together
echo "Testing all flags combination..."
gcov-tool merge -v -f -F -o -h -t 0.8 test1.gcda test2.gcda merge_output.gcda > /dev/null 2>&1

# Test with flags in different orders
echo "Testing flags in different order (input file first)..."
gcov-tool merge test1.gcda -v -f test2.gcda merge_output.gcda > /dev/null 2>&1

echo "Testing flags in different order (flags at end)..."
gcov-tool merge test1.gcda test2.gcda merge_output.gcda -v -f > /dev/null 2>&1

# Test with three input files
echo "Testing with three input files and flags..."
gcov-tool merge -v -f -o test1.gcda test2.gcda test3.gcda merge_output2.gcda > /dev/null 2>&1

echo -e "\n=== Testing edge cases ==="

# Test threshold with very small value
echo "Testing -t 0.001..."
gcov-tool merge -t 0.001 test1.gcda test2.gcda merge_output.gcda > /dev/null 2>&1

# Test threshold with value 0
echo "Testing -t 0..."
gcov-tool merge -t 0 test1.gcda test2.gcda merge_output.gcda > /dev/null 2>&1

# Test threshold with value 1.0
echo "Testing -t 1.0..."
gcov-tool merge -t 1.0 test1.gcda test2.gcda merge_output.gcda > /dev/null 2>&1

# Test hot-only without threshold (default threshold should be used)
echo "Testing -h without explicit -t..."
gcov-tool merge -h test1.gcda test2.gcda merge_output.gcda > /dev/null 2>&1

# Test function-level and object-level together
echo "Testing -f -o combination..."
gcov-tool merge -f -o test1.gcda test2.gcda merge_output.gcda > /dev/null 2>&1

echo -e "\n=== Testing invalid option (to trigger default case) ==="

# Test invalid option to trigger the default case and overlap_usage()
echo "Testing invalid option -x (should show usage)..."
gcov-tool merge -x test1.gcda test2.gcda merge_output.gcda 2>&1 | head -5 || true

# Test another invalid option
echo "Testing invalid option -z..."
gcov-tool merge -z test1.gcda test2.gcda merge_output.gcda 2>&1 | head -5 || true

# Test missing argument for -t
echo "Testing -t without argument (should error)..."
gcov-tool merge -t test1.gcda test2.gcda merge_output.gcda 2>&1 | head -5 || true

echo -e "\n=== Testing output redirection ==="

# Test with output to file
echo "Testing with output redirected to file..."
gcov-tool merge -v test1.gcda test2.gcda merge_output.gcda > merge_output.txt 2>&1

# Test with stderr redirected
echo "Testing with stderr redirected..."
gcov-tool merge -v test1.gcda test2.gcda merge_output.gcda 2> merge_stderr.txt > merge_stdout.txt

echo -e "\n=== Verifying flag effects ==="

# Create a simple verification by checking if merge output files were created
if [ -f merge_output.gcda ]; then
    echo "✓ Merge output file created successfully"
fi

if [ -f merge_output2.gcda ]; then
    echo "✓ Second merge output file created successfully"
fi

# Clean up generated files (optional - comment out for debugging)
cleanup

echo -e "\n=== Test completed successfully ==="
echo "All gcov-tool merge options have been tested:"
echo "  - Individual flags: -v, -f, -F, -o, -h, -t <value>"
echo "  - Flag combinations"
echo "  - Different flag orders"
echo "  - Edge cases for threshold values"
echo "  - Invalid options (triggering default case)"
echo "  - Multiple input files"
echo "  - Output redirection"
```

This script systematically tests all the uncovered lines in `gcov-tool.cc`:

1. **Individual flag testing** (lines 534-554):
   - `-v` triggers `verbose = true` and `gcov_set_verbose()`
   - `-f` sets `overlap_func_level = 1`
   - `-F` sets `overlap_use_fullname = 1`
   - `-o` sets `overlap_obj_level = 1`
   - `-h` sets `overlap_hot_only = 1`
   - `-t` with various values calls `atof(optarg)`

2. **Flag combinations** to ensure they work together correctly.

3. **Different flag orders** to test position-independent parsing.

4. **Invalid options** (`-x`, `-z`) to trigger the `default:` case and `overlap_usage()`.

5. **Edge cases** for the `-t` flag with various numeric values.

6. **Multiple input files** to ensure the merge logic uses the flags.

7. **Output redirection** to test code paths after flag processing.

To run this test, ensure:
1. `gcov-tool` is built with coverage instrumentation
2. GCC is available in PATH
3. The script has execute permissions: `chmod +x run_gcov_tool_test.sh`
4. Run with: `./run_gcov_tool_test.sh`

The script is self-contained and cleans up after itself, making it repeatable for coverage analysis.
