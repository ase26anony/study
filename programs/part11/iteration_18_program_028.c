I'll create a comprehensive test script that covers the specific lines in `gcov-tool.cc` (lines 534-554) by testing all the command-line options for the `merge` subcommand.

```bash
#!/bin/bash
# run_gcov_tool_test.sh - Test script for gcov-tool merge command options

set -e  # Exit on error

echo "=== Starting gcov-tool merge option tests ==="

# Clean up any existing test files
cleanup() {
    echo "Cleaning up test files..."
    rm -f test.c test_prog test_prog2 test.gcda test2.gcda test.gcno test2.gcno \
          merge_output.txt error_output.txt
}

trap cleanup EXIT

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
        printf("No arguments\n");
    }
    
    for (int i = 0; i < 3; i++) {
        x += i;
    }
    
    return x % 2;
}
EOF

# Step 2: Compile with coverage flags
echo "Compiling test programs with coverage flags..."
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog2

# Step 3: Generate .gcda files with different executions
echo "Generating coverage data files..."
# First execution - no arguments
./test_prog
mv test.gcda test1.gcda

# Second execution - with argument
./test_prog "test_argument"
mv test.gcda test2.gcda

# Third execution - different program name
./test_prog2 "different"
mv test.gcda test3.gcda

# Make copies for testing
cp test1.gcda test.gcda
cp test2.gcda test2.gcda
cp test3.gcda test3.gcda

echo "Generated test.gcda, test2.gcda, and test3.gcda"

# Step 4: Test matrix of command-line options
echo -e "\n=== Testing individual flags ==="

# Test verbose flag (-v)
echo "Testing -v flag..."
gcov-tool merge -v test.gcda test2.gcda > merge_output.txt 2>&1 || true

# Test function-level overlap flag (-f)
echo "Testing -f flag..."
gcov-tool merge -f test.gcda test2.gcda > merge_output.txt 2>&1 || true

# Test full filename flag (-F)
echo "Testing -F flag..."
gcov-tool merge -F test.gcda test2.gcda > merge_output.txt 2>&1 || true

# Test object-level flag (-o)
echo "Testing -o flag..."
gcov-tool merge -o test.gcda test2.gcda > merge_output.txt 2>&1 || true

# Test hot-only flag (-h)
echo "Testing -h flag..."
gcov-tool merge -h test.gcda test2.gcda > merge_output.txt 2>&1 || true

# Test threshold flag with integer value (-t 1)
echo "Testing -t 1 flag..."
gcov-tool merge -t 1 test.gcda test2.gcda > merge_output.txt 2>&1 || true

# Test threshold flag with fractional value (-t 0.5)
echo "Testing -t 0.5 flag..."
gcov-tool merge -t 0.5 test.gcda test2.gcda > merge_output.txt 2>&1 || true

# Test threshold flag with another fractional value (-t 0.33)
echo "Testing -t 0.33 flag..."
gcov-tool merge -t 0.33 test.gcda test2.gcda > merge_output.txt 2>&1 || true

echo -e "\n=== Testing flag combinations ==="

# Test combination: verbose + function-level + object-level
echo "Testing -v -f -o combination..."
gcov-tool merge -v -f -o test.gcda test2.gcda test3.gcda > merge_output.txt 2>&1 || true

# Test combination: fullname + hot-only + threshold
echo "Testing -F -h -t 0.75 combination..."
gcov-tool merge -F -h -t 0.75 test.gcda test2.gcda > merge_output.txt 2>&1 || true

# Test combination: all flags together
echo "Testing all flags combination..."
gcov-tool merge -v -f -F -o -h -t 0.8 test.gcda test2.gcda test3.gcda > merge_output.txt 2>&1 || true

# Test different flag orders
echo "Testing different flag orders (flags before files)..."
gcov-tool merge -v -f -o test.gcda test2.gcda > merge_output.txt 2>&1 || true

echo "Testing different flag orders (flags after files)..."
gcov-tool merge test.gcda -v -f -o test2.gcda > merge_output.txt 2>&1 || true

echo "Testing mixed flag order..."
gcov-tool merge -v test.gcda -f test2.gcda -o > merge_output.txt 2>&1 || true

# Test with single input file (should still parse options)
echo "Testing with single input file..."
gcov-tool merge -v -f test.gcda > merge_output.txt 2>&1 || true

# Test with three input files to ensure proper merge logic
echo "Testing with three input files and -f -o flags..."
gcov-tool merge -f -o test.gcda test2.gcda test3.gcda > merge_output.txt 2>&1 || true

echo -e "\n=== Testing edge cases ==="

# Test threshold with very small value
echo "Testing -t 0.001..."
gcov-tool merge -t 0.001 test.gcda test2.gcda > merge_output.txt 2>&1 || true

# Test threshold with value 0
echo "Testing -t 0..."
gcov-tool merge -t 0 test.gcda test2.gcda > merge_output.txt 2>&1 || true

# Test threshold with value 1.0
echo "Testing -t 1.0..."
gcov-tool merge -t 1.0 test.gcda test2.gcda > merge_output.txt 2>&1 || true

# Test hot-only without threshold (default threshold should be used)
echo "Testing -h without explicit -t..."
gcov-tool merge -h test.gcda test2.gcda > merge_output.txt 2>&1 || true

# Test hot-only with threshold
echo "Testing -h with -t 0.9..."
gcov-tool merge -h -t 0.9 test.gcda test2.gcda > merge_output.txt 2>&1 || true

echo -e "\n=== Testing error conditions ==="

# Test invalid option to trigger default case and overlap_usage()
echo "Testing invalid option -x (should trigger usage)..."
gcov-tool merge -x test.gcda test2.gcda 2> error_output.txt || true
if [ -s error_output.txt ]; then
    echo "Usage message printed (expected)"
fi

# Test invalid threshold value (non-numeric)
echo "Testing invalid threshold value..."
gcov-tool merge -t invalid test.gcda test2.gcda 2> error_output.txt || true

# Test missing threshold argument
echo "Testing missing threshold argument..."
gcov-tool merge -t test.gcda test2.gcda 2> error_output.txt || true

# Test with non-existent input file
echo "Testing with non-existent file..."
gcov-tool merge -v nonexistent.gcda 2> error_output.txt || true

# Test with no input files (should fail before reaching our target code)
echo "Testing with no input files..."
gcov-tool merge -v 2> error_output.txt || true

echo -e "\n=== Testing output redirection ==="

# Test with output to file
echo "Testing with output redirected to file..."
gcov-tool merge -v -f test.gcda test2.gcda > merge_output.txt 2>&1
if [ -s merge_output.txt ]; then
    echo "Output written to file successfully"
fi

# Test with stderr redirected
echo "Testing with stderr redirected..."
gcov-tool merge -v test.gcda test2.gcda 2> error_output.txt > merge_output.txt || true

# Test silent operation (all output to /dev/null)
echo "Testing silent operation..."
gcov-tool merge -v -f -o test.gcda test2.gcda > /dev/null 2>&1 || true

echo -e "\n=== Multi-file merge scenarios ==="

# Create a more complex scenario with multiple distinct .gcda files
echo "Creating additional test files for complex merge..."

# Create another test program with different structure
cat > test2.c << 'EOF'
#include <stdio.h>

void function1() {
    printf("Function1\n");
}

void function2(int n) {
    for(int i = 0; i < n; i++) {
        printf("Iteration %d\n", i);
    }
}

int main() {
    function1();
    function2(2);
    return 0;
}
EOF

# Compile and run to generate different coverage patterns
gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test2_prog
./test2_prog
mv test2.gcda test4.gcda

# Test with mixed .gcda files from different sources
echo "Testing merge with heterogeneous .gcda files..."
gcov-tool merge -v -f -o test.gcda test4.gcda > merge_output.txt 2>&1 || true

# Test with object-level and function-level flags together
echo "Testing -f and -o together with multiple files..."
gcov-tool merge -f -o test.gcda test2.gcda test3.gcda test4.gcda > merge_output.txt 2>&1 || true

# Test full filename with multiple files
echo "Testing -F with multiple files..."
gcov-tool merge -F test.gcda test2.gcda test3.gcda test4.gcda > merge_output.txt 2>&1 || true

echo -e "\n=== Final comprehensive test ==="

# One final test hitting all code paths
echo "Running final comprehensive test..."
gcov-tool merge -v -f -F -o -h -t 0.5 test.gcda test2.gcda test3.gcda test4.gcda > /dev/null 2>&1 || true

echo -e "\n=== All tests completed ==="
echo "The following command-line options have been tested:"
echo "  -v (verbose)"
echo "  -f (function-level overlap)"
echo "  -F (full filename)"
echo "  -o (object-level overlap)"
echo "  -h (hot-only)"
echo "  -t <value> (threshold with various values)"
echo "  Invalid option -x (triggers default case)"
echo ""
echo "All flag combinations and edge cases have been exercised."
```

This script provides comprehensive coverage of the target lines in `gcov-tool.cc`:

1. **Individual flag testing**: Each flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually
2. **Flag combinations**: Various combinations of flags are tested together
3. **Different flag orders**: Tests flags before, after, and mixed with input files
4. **Threshold edge cases**: Tests integer (1), fractional (0.5, 0.33), and boundary (0, 1.0, 0.001) values
5. **Error conditions**: Tests invalid option `-x` to trigger the `default:` case and `overlap_usage()`
6. **Multi-file scenarios**: Tests with 2, 3, and 4 input files to exercise merge logic
7. **Output redirection**: Tests output to files, `/dev/null`, and separate stdout/stderr
8. **Hot-only with threshold**: Tests interaction between `-h` and `-t` flags

To execute this test:
1. Ensure `gcov-tool` is built with coverage instrumentation
2. Make the script executable: `chmod +x run_gcov_tool_test.sh`
3. Run the script: `./run_gcov_tool_test.sh`

The script will generate the necessary `.gcda` files and systematically test all the command-line options that trigger the switch-case logic in lines 534-554 of `gcov-tool.cc`.
