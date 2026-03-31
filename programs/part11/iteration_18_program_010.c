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
          test_output.txt test_output2.txt merge_output.txt 2>/dev/null || true
}

# Clean up at start and on exit
cleanup
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
        printf("No arguments provided\n");
    }
    
    // Loop to generate some arcs
    for (int i = 0; i < 3; i++) {
        x += i;
    }
    
    return x % 2;
}
EOF

# Step 2: Compile with coverage flags
echo "Compiling test program with coverage flags..."
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog2  # Second executable

# Step 3: Run the program to generate .gcda files
echo "Generating coverage data files..."
# First run - no arguments
./test_prog > test_output.txt
# Move the gcda file
mv test.gcda test1.gcda 2>/dev/null || true

# Second run - with argument
./test_prog "test_argument" > test_output2.txt
# Move the gcda file  
mv test.gcda test2.gcda 2>/dev/null || true

# Third run with second executable to get different coverage
./test_prog2 "different" > /dev/null
mv test.gcda test3.gcda 2>/dev/null || true

# Verify we have gcda files
if [[ ! -f test1.gcda ]] || [[ ! -f test2.gcda ]] || [[ ! -f test3.gcda ]]; then
    echo "ERROR: Failed to generate .gcda files"
    exit 1
fi

echo "Generated test1.gcda, test2.gcda, and test3.gcda"

# Step 4: Test matrix of gcov-tool merge options
echo ""
echo "=== Testing gcov-tool merge options ==="

# Helper function to run gcov-tool and check exit status
run_gcov_merge() {
    local description="$1"
    shift
    echo "Test: $description"
    echo "Command: gcov-tool merge $@"
    
    # Run the command, redirect output
    if gcov-tool merge "$@" > merge_output.txt 2>&1; then
        echo "  ✓ Success"
    else
        echo "  ✗ Failed with exit code $?"
        # Show error output for debugging
        cat merge_output.txt
    fi
    echo ""
}

# Test individual flags (lines 534-554)
echo "--- Testing individual flags ---"

# -v flag: verbose mode (line 535-538)
run_gcov_merge "Verbose flag (-v)" -v test1.gcda test2.gcda

# -f flag: function-level overlap (line 540-541)
run_gcov_merge "Function-level overlap flag (-f)" -f test1.gcda test2.gcda

# -F flag: use full filename (line 543-544)
run_gcov_merge "Full filename flag (-F)" -F test1.gcda test2.gcda

# -o flag: object-level overlap (line 546-547)
run_gcov_merge "Object-level overlap flag (-o)" -o test1.gcda test2.gcda

# -h flag: hot-only reporting (line 549-550)
run_gcov_merge "Hot-only flag (-h)" -h test1.gcda test2.gcda

# -t flag: hot threshold with integer value (line 552-553)
run_gcov_merge "Hot threshold flag with integer (-t 1)" -t 1 test1.gcda test2.gcda

# -t flag: hot threshold with fractional value
run_gcov_merge "Hot threshold flag with fraction (-t 0.33)" -t 0.33 test1.gcda test2.gcda

# -t flag: hot threshold with another fractional value
run_gcov_merge "Hot threshold flag with fraction (-t 0.75)" -t 0.75 test1.gcda test2.gcda

echo "--- Testing flag combinations ---"

# Test various combinations of flags
run_gcov_merge "Verbose + function-level (-v -f)" -v -f test1.gcda test2.gcda
run_gcov_merge "Verbose + object-level (-v -o)" -v -o test1.gcda test2.gcda
run_gcov_merge "Function + fullname (-f -F)" -f -F test1.gcda test2.gcda
run_gcov_merge "Object + fullname (-o -F)" -o -F test1.gcda test2.gcda
run_gcov_merge "Hot-only + threshold (-h -t 0.5)" -h -t 0.5 test1.gcda test2.gcda
run_gcov_merge "Verbose + function + object (-v -f -o)" -v -f -o test1.gcda test2.gcda
run_gcov_merge "All flags except threshold (-v -f -F -o -h)" -v -f -F -o -h test1.gcda test2.gcda
run_gcov_merge "All flags with threshold (-v -f -F -o -h -t 0.8)" -v -f -F -o -h -t 0.8 test1.gcda test2.gcda

echo "--- Testing flag order independence ---"

# Test flags in different orders
run_gcov_merge "Flags before files (-v -f test1.gcda)" -v -f test1.gcda test2.gcda
run_gcov_merge "Flags between files (test1.gcda -v -f test2.gcda)" test1.gcda -v -f test2.gcda
run_gcov_merge "Flags after files (test1.gcda test2.gcda -v -f)" test1.gcda test2.gcda -v -f
run_gcov_merge "Mixed order (-f test1.gcda -v test2.gcda -o)" -f test1.gcda -v test2.gcda -o

echo "--- Testing with three input files ---"

# Test with three files to exercise merge logic more thoroughly
run_gcov_merge "Three files with verbose (-v)" -v test1.gcda test2.gcda test3.gcda
run_gcov_merge "Three files with function-level (-f)" -f test1.gcda test2.gcda test3.gcda
run_gcov_merge "Three files with all flags (-v -f -F -o -h -t 0.6)" -v -f -F -o -h -t 0.6 test1.gcda test2.gcda test3.gcda

echo "--- Testing edge cases ---"

# Test with threshold value 0 (edge case)
run_gcov_merge "Threshold zero (-t 0)" -t 0 test1.gcda test2.gcda

# Test with threshold value 1.0 (edge case)
run_gcov_merge "Threshold one (-t 1.0)" -t 1.0 test1.gcda test2.gcda

# Test with very small threshold
run_gcov_merge "Very small threshold (-t 0.001)" -t 0.001 test1.gcda test2.gcda

# Test with threshold > 1 (might be invalid but should be parsed)
run_gcov_merge "Threshold > 1 (-t 1.5)" -t 1.5 test1.gcda test2.gcda

echo "--- Testing invalid option (to trigger default case) ---"

# Test invalid option to trigger default case and overlap_usage() (line 554)
echo "Test: Invalid option (-x) - should show usage"
echo "Command: gcov-tool merge -x test1.gcda test2.gcda"
if gcov-tool merge -x test1.gcda test2.gcda 2>&1 | grep -q "usage\|Usage\|OPTION"; then
    echo "  ✓ Successfully triggered usage message"
else
    echo "  ✗ Did not show expected usage message"
    gcov-tool merge -x test1.gcda test2.gcda 2>&1 | head -20
fi
echo ""

# Test invalid option in combination with valid ones
echo "Test: Valid and invalid option mix (-v -x -f)"
echo "Command: gcov-tool merge -v -x -f test1.gcda test2.gcda"
if gcov-tool merge -v -x -f test1.gcda test2.gcda 2>&1 | grep -q "usage\|Usage\|OPTION"; then
    echo "  ✓ Successfully triggered usage message"
else
    echo "  ✗ Did not show expected usage message"
fi
echo ""

echo "=== Testing complete ==="
echo ""
echo "Summary:"
echo "- Tested all individual flags: -v, -f, -F, -o, -h, -t <value>"
echo "- Tested various flag combinations"
echo "- Tested flag position independence"
echo "- Tested with multiple input files"
echo "- Tested edge cases for threshold values"
echo "- Tested invalid option to trigger default case"
echo ""
echo "All tests executed. Check coverage data to verify lines 534-554 were hit."
```

This script provides comprehensive testing of the uncovered lines in `gcov-tool.cc`:

1. **Command-Line Argument Matrix**: Tests each flag individually (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) and in various combinations.

2. **Valid Input Data Files**: Creates a C program, compiles it with coverage flags, runs it multiple times to generate three distinct `.gcda` files.

3. **Edge Case and Error Testing**:
   - Tests `-t` with integer (1), fractional (0.33, 0.75), edge (0, 1.0, 0.001, 1.5) values
   - Tests `-h` combined with `-t`
   - Tests flags in different positions (before, between, after files)
   - Tests invalid option `-x` to trigger the `default:` case

4. **Output Redirection**: All successful commands redirect output to `merge_output.txt` to test code paths after flag processing.

5. **Multi-File Merge**: Tests with 2 and 3 input files to exercise merge logic influenced by the flags.

6. **Automation and Cleanup**: Script is self-contained, cleans up at start and exit.

To run this test:
1. Ensure `gcov-tool` is built with coverage instrumentation
2. Make the script executable: `chmod +x run_gcov_tool_test.sh`
3. Run it: `./run_gcov_tool_test.sh`

The script will systematically test all code paths through the switch-case block at lines 534-554 of `gcov-tool.cc`.
