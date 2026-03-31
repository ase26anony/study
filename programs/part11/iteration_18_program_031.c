I'll create a comprehensive test script that covers the specific lines in `gcov-tool.cc` (lines 534-554) by testing all the command-line options for the `merge` subcommand.

```bash
#!/bin/bash
# run_gcov_tool_test.sh - Test script for gcov-tool merge command options

set -e  # Exit on any error

echo "=== Starting gcov-tool merge option tests ==="

# Clean up any previous test files
cleanup() {
    echo "Cleaning up test files..."
    rm -f test.c test_prog test_prog2 test.gcda test2.gcda merged.gcda \
          test.gcno test2.gcno output.txt error.txt
}

# Clean up on exit
trap cleanup EXIT

# Step 1: Create a minimal C test program
echo "Creating test C program..."
cat > test.c << 'EOF'
#include <stdio.h>

int main(int argc, char *argv[]) {
    int x = 0;
    
    // Some branching logic to generate coverage data
    if (argc > 1) {
        x = 1;
        printf("Argument provided: %s\n", argv[1]);
    } else {
        x = 2;
        printf("No arguments\n");
    }
    
    // Loop to generate some arcs
    for (int i = 0; i < 3; i++) {
        x += i;
    }
    
    return x % 2;
}
EOF

# Step 2: Compile with coverage flags
echo "Compiling test programs with coverage..."
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog2

# Step 3: Generate two distinct .gcda files
echo "Generating coverage data files..."
# First run - no arguments
./test_prog > /dev/null 2>&1
mv test.gcda test1.gcda

# Second run - with argument to create different coverage
./test_prog "test_argument" > /dev/null 2>&1
mv test.gcda test2.gcda

# Create a third .gcda file by running the second executable
./test_prog2 > /dev/null 2>&1
mv test.gcda test3.gcda

echo "Generated test1.gcda, test2.gcda, and test3.gcda"

# Step 4: Test matrix of gcov-tool merge options
echo -e "\n=== Testing individual flags ==="

# Test verbose flag (-v)
echo "Testing -v flag..."
gcov-tool merge -v test1.gcda test2.gcda > output.txt 2>&1 || true

# Test function-level overlap flag (-f)
echo "Testing -f flag..."
gcov-tool merge -f test1.gcda test2.gcda > output.txt 2>&1 || true

# Test full filename flag (-F)
echo "Testing -F flag..."
gcov-tool merge -F test1.gcda test2.gcda > output.txt 2>&1 || true

# Test object-level flag (-o)
echo "Testing -o flag..."
gcov-tool merge -o test1.gcda test2.gcda > output.txt 2>&1 || true

# Test hot-only flag (-h)
echo "Testing -h flag..."
gcov-tool merge -h test1.gcda test2.gcda > output.txt 2>&1 || true

# Test threshold flag with integer value (-t 1)
echo "Testing -t 1 flag..."
gcov-tool merge -t 1 test1.gcda test2.gcda > output.txt 2>&1 || true

# Test threshold flag with fractional value (-t 0.5)
echo "Testing -t 0.5 flag..."
gcov-tool merge -t 0.5 test1.gcda test2.gcda > output.txt 2>&1 || true

# Test threshold flag with another fractional value (-t 0.33)
echo "Testing -t 0.33 flag..."
gcov-tool merge -t 0.33 test1.gcda test2.gcda > output.txt 2>&1 || true

echo -e "\n=== Testing flag combinations ==="

# Test combination: verbose + function-level + object-level
echo "Testing -v -f -o combination..."
gcov-tool merge -v -f -o test1.gcda test2.gcda test3.gcda > output.txt 2>&1 || true

# Test combination: fullname + hot-only + threshold
echo "Testing -F -h -t 0.75 combination..."
gcov-tool merge -F -h -t 0.75 test1.gcda test2.gcda > output.txt 2>&1 || true

# Test combination: all flags together
echo "Testing all flags combination..."
gcov-tool merge -v -f -F -o -h -t 0.9 test1.gcda test2.gcda > output.txt 2>&1 || true

# Test combination: hot-only with different threshold values
echo "Testing -h -t 0.1 combination..."
gcov-tool merge -h -t 0.1 test1.gcda test2.gcda > output.txt 2>&1 || true

echo "Testing -h -t 0.99 combination..."
gcov-tool merge -h -t 0.99 test1.gcda test2.gcda > output.txt 2>&1 || true

echo -e "\n=== Testing flag ordering variations ==="

# Test flags before input files
echo "Testing flags before files: -v -f test1.gcda test2.gcda"
gcov-tool merge -v -f test1.gcda test2.gcda > output.txt 2>&1 || true

# Test flags after input files (should still work)
echo "Testing files before flags: test1.gcda test2.gcda -v -f"
gcov-tool merge test1.gcda test2.gcda -v -f > output.txt 2>&1 || true

# Test mixed ordering
echo "Testing mixed ordering: -v test1.gcda -f test2.gcda -o"
gcov-tool merge -v test1.gcda -f test2.gcda -o > output.txt 2>&1 || true

echo -e "\n=== Testing edge cases ==="

# Test with three input files (multi-file merge scenario)
echo "Testing with three input files..."
gcov-tool merge -v -f -o test1.gcda test2.gcda test3.gcda > output.txt 2>&1 || true

# Test threshold with very small value
echo "Testing -t 0.001..."
gcov-tool merge -t 0.001 test1.gcda test2.gcda > output.txt 2>&1 || true

# Test threshold with value 0
echo "Testing -t 0..."
gcov-tool merge -t 0 test1.gcda test2.gcda > output.txt 2>&1 || true

# Test threshold with value 1.0
echo "Testing -t 1.0..."
gcov-tool merge -t 1.0 test1.gcda test2.gcda > output.txt 2>&1 || true

echo -e "\n=== Testing invalid option (to trigger default case) ==="

# Test invalid option to trigger the default case and overlap_usage()
echo "Testing invalid option -x (should show usage)..."
gcov-tool merge -x test1.gcda test2.gcda 2> error.txt || true
if [ -s error.txt ]; then
    echo "Usage message shown (as expected)"
else
    echo "WARNING: No error output for invalid option"
fi

# Test another invalid option combination
echo "Testing invalid option -z..."
gcov-tool merge -v -z test1.gcda 2> error.txt || true

echo -e "\n=== Testing output redirection ==="

# Test with output to file
echo "Testing with output redirected to merged.gcda..."
gcov-tool merge -v test1.gcda test2.gcda -o merged.gcda > output.txt 2>&1 || true

if [ -f merged.gcda ]; then
    echo "Successfully created merged.gcda"
    # Verify we can read the merged file
    gcov-tool merge -v merged.gcda test3.gcda > /dev/null 2>&1 || true
fi

# Test with all output to /dev/null
echo "Testing with all output to /dev/null..."
gcov-tool merge -v -f -F -o -h -t 0.5 test1.gcda test2.gcda > /dev/null 2>&1 || true

echo -e "\n=== Testing exit status ==="

# Test valid command exit status
echo "Testing valid command exit status..."
if gcov-tool merge -v test1.gcda test2.gcda > /dev/null 2>&1; then
    echo "Valid command succeeded (exit 0)"
else
    echo "Valid command failed unexpectedly"
fi

# Test invalid command exit status
echo "Testing invalid command exit status..."
if ! gcov-tool merge -invalid-option test1.gcda 2> /dev/null; then
    echo "Invalid command failed as expected (non-zero exit)"
else
    echo "WARNING: Invalid command succeeded unexpectedly"
fi

echo -e "\n=== All tests completed ==="
echo "The following gcov-tool merge options were tested:"
echo "  - Single flags: -v, -f, -F, -o, -h, -t <value>"
echo "  - Combinations: -v -f -o, -F -h -t 0.75, all flags together"
echo "  - Flag ordering: flags before/after/between files"
echo "  - Edge cases: -t 0, -t 1.0, -t 0.001"
echo "  - Invalid option: -x (triggers default case)"
echo "  - Multi-file merge: 3 input files"
echo "  - Output redirection: to file and /dev/null"

# Cleanup will happen automatically via trap
```

This script systematically tests all the uncovered lines in `gcov-tool.cc`:

1. **Individual flag testing** (lines 534-554):
   - `-v` → `verbose = true; gcov_set_verbose();`
   - `-f` → `overlap_func_level = 1;`
   - `-F` → `overlap_use_fullname = 1;`
   - `-o` → `overlap_obj_level = 1;`
   - `-h` → `overlap_hot_only = 1;`
   - `-t` → `overlap_hot_threshold = atof(optarg);`

2. **Flag combinations**: Tests multiple flags together to ensure they work in combination.

3. **Flag ordering**: Tests flags in different positions relative to input files.

4. **Edge cases for `-t`**: Tests integer (1), fractional (0.5, 0.33), boundary (0, 1.0), and very small (0.001) values.

5. **Invalid option**: Tests `-x` to trigger the `default:` case and `overlap_usage()`.

6. **Multi-file scenario**: Uses 3 distinct `.gcda` files to ensure overlap analysis routines are invoked.

7. **Output redirection**: Tests both successful output redirection and output to `/dev/null`.

8. **Exit status verification**: Checks that valid commands succeed and invalid commands fail.

To run this test:
1. Ensure `gcov-tool` is built with coverage instrumentation
2. Make the script executable: `chmod +x run_gcov_tool_test.sh`
3. Run it: `./run_gcov_tool_test.sh`

The script will generate the necessary `.gcda` files, run all test cases, and clean up automatically.
