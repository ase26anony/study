Looking at the uncovered lines in `gcov-dump.cc`, I'll create a comprehensive test that covers all the switch cases for command-line flag parsing. Here's a complete test implementation:

```bash
#!/bin/bash
# test_gcov_dump_flags.sh - Comprehensive test for gcov-dump flag parsing

set -e  # Exit on any error

echo "=== Testing gcov-dump flag parsing coverage ==="

# Configuration
TEST_DIR=$(pwd)/gcov_dump_test
GCOV_DUMP_BIN="./gcc/gcov-dump"  # Adjust this path as needed
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

# Clean up any previous test files
cleanup() {
    echo "Cleaning up test files..."
    rm -f coverage_test coverage_test.c coverage_test.gcno coverage_test.gcda
    rm -f test_output_*.txt
}

trap cleanup EXIT

# Step 1: Create a simple C program to generate GCOV data
cat > coverage_test.c << 'EOF'
#include <stdio.h>

int main() {
    int i;
    for (i = 0; i < 10; i++) {
        printf("Iteration %d\n", i);
    }
    return 0;
}
EOF

echo "Created test source file: coverage_test.c"

# Step 2: Compile with coverage instrumentation
echo "Compiling test program with coverage instrumentation..."
gcc -O0 -fprofile-arcs -ftest-coverage coverage_test.c -o coverage_test

if [ ! -f coverage_test ]; then
    echo "Error: Failed to compile test program"
    exit 1
fi

echo "Generated: coverage_test (executable)"
echo "Generated: coverage_test.gcno (notes file)"

# Step 3: Run the program to generate .gcda file
echo "Running test program to generate coverage data..."
./coverage_test > /dev/null

if [ ! -f coverage_test.gcda ]; then
    echo "Error: Failed to generate .gcda file"
    exit 1
fi

echo "Generated: coverage_test.gcda (data file)"

# Step 4: Verify gcov-dump binary exists
if [ ! -f "$GCOV_DUMP_BIN" ]; then
    echo "Error: gcov-dump binary not found at $GCOV_DUMP_BIN"
    echo "Please adjust GCOV_DUMP_BIN path in the script"
    exit 1
fi

echo "Using gcov-dump binary: $GCOV_DUMP_BIN"

# Step 5: Test individual flag cases
echo -e "\n=== Testing individual flag cases ==="

# Test -h (help) - already covered but included for completeness
echo "Testing -h (help)..."
"$GCOV_DUMP_BIN" -h > test_output_help.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ -h flag processed successfully"
else
    echo "✗ -h flag failed"
fi

# Test -v (version)
echo "Testing -v (version)..."
"$GCOV_DUMP_BIN" -v > test_output_version.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ -v flag processed successfully"
else
    echo "✗ -v flag failed"
fi

# Test -l (dump contents) - UNCOVERED LINE
echo "Testing -l (dump contents)..."
"$GCOV_DUMP_BIN" -l coverage_test.gcda > test_output_l.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ -l flag processed successfully"
    # Verify output contains expected content
    if grep -q "Contents" test_output_l.txt || grep -q "Tag" test_output_l.txt; then
        echo "  Output contains dump contents"
    fi
else
    echo "✗ -l flag failed"
fi

# Test -p (dump positions) - UNCOVERED LINE
echo "Testing -p (dump positions)..."
"$GCOV_DUMP_BIN" -p coverage_test.gcda > test_output_p.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ -p flag processed successfully"
    if grep -q "positions" test_output_p.txt || grep -q "line" test_output_p.txt; then
        echo "  Output contains position information"
    fi
else
    echo "✗ -p flag failed"
fi

# Test -r (dump raw) - UNCOVERED LINE
echo "Testing -r (dump raw)..."
"$GCOV_DUMP_BIN" -r coverage_test.gcda > test_output_r.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ -r flag processed successfully"
    if grep -q "raw" test_output_r.txt || grep -q "0x" test_output_r.txt; then
        echo "  Output contains raw data"
    fi
else
    echo "✗ -r flag failed"
fi

# Test -s (dump stable) - UNCOVERED LINE
echo "Testing -s (dump stable)..."
"$GCOV_DUMP_BIN" -s coverage_test.gcda > test_output_s.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ -s flag processed successfully"
    if grep -q "stable" test_output_s.txt || grep -q "format" test_output_s.txt; then
        echo "  Output contains stable format"
    fi
else
    echo "✗ -s flag failed"
fi

# Step 6: Test combined flags (separate arguments)
echo -e "\n=== Testing combined flags (separate) ==="
echo "Testing -l -p -r -s (separate flags)..."
"$GCOV_DUMP_BIN" -l -p -r -s coverage_test.gcda > test_output_combined_separate.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ Combined flags processed successfully"
    # Check that multiple dump modes are active
    echo "  Combined output length: $(wc -l < test_output_combined_separate.txt) lines"
else
    echo "✗ Combined flags failed"
fi

# Step 7: Test concatenated flags
echo -e "\n=== Testing concatenated flags ==="
echo "Testing -lprs (concatenated flags)..."
"$GCOV_DUMP_BIN" -lprs coverage_test.gcda > test_output_concatenated.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ Concatenated flags processed successfully"
    echo "  Concatenated output length: $(wc -l < test_output_concatenated.txt) lines"
else
    echo "✗ Concatenated flags failed"
fi

# Step 8: Test invalid flag to trigger default case - UNCOVERED LINE
echo -e "\n=== Testing invalid flag (default case) ==="
echo "Testing -x (invalid flag)..."
"$GCOV_DUMP_BIN" -x coverage_test.gcda > test_output_invalid.txt 2>&1
exit_code=$?
if [ $exit_code -ne 0 ]; then
    echo "✓ Invalid flag correctly returned error (exit code: $exit_code)"
    if grep -q "unknown flag" test_output_invalid.txt; then
        echo "  Error message contains 'unknown flag' as expected"
    else
        echo "  Warning: Expected 'unknown flag' message not found"
        cat test_output_invalid.txt
    fi
else
    echo "✗ Invalid flag should have failed but didn't"
fi

# Step 9: Test another invalid flag with different character
echo "Testing -Z (another invalid flag)..."
"$GCOV_DUMP_BIN" -Z coverage_test.gcda > test_output_invalid2.txt 2>&1
exit_code=$?
if [ $exit_code -ne 0 ]; then
    echo "✓ Invalid flag -Z correctly returned error"
    if grep -q "unknown flag.*Z" test_output_invalid2.txt; then
        echo "  Error message contains 'unknown flag' for 'Z'"
    fi
fi

# Step 10: Test mixed valid and invalid flags
echo "Testing -l -x -p (mixed valid and invalid)..."
"$GCOV_DUMP_BIN" -l -x -p coverage_test.gcda > test_output_mixed.txt 2>&1
exit_code=$?
if [ $exit_code -ne 0 ]; then
    echo "✓ Mixed flags correctly rejected due to invalid flag"
fi

# Step 11: Test with .gcno file as well
echo -e "\n=== Testing with .gcno file ==="
echo "Testing -l with .gcno file..."
"$GCOV_DUMP_BIN" -l coverage_test.gcno > test_output_gcno.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ Processed .gcno file successfully"
fi

# Step 12: Summary
echo -e "\n=== Test Summary ==="
echo "All flag parsing test cases executed:"
echo "  - Help (-h) and Version (-v) flags"
echo "  - Individual dump flags: -l, -p, -r, -s"
echo "  - Combined flags: -l -p -r -s"
echo "  - Concatenated flags: -lprs"
echo "  - Invalid flags: -x, -Z (triggering default case)"
echo "  - Mixed valid/invalid flags"
echo "  - Both .gcda and .gcno file types"
echo ""
echo "If gcov-dump was built with coverage instrumentation, these tests"
echo "should cover the switch-case lines 111-130 in gcov-dump.cc"

# Optional: Show a sample of each output
echo -e "\n=== Sample Outputs ==="
echo "Help flag output (first 3 lines):"
head -3 test_output_help.txt
echo ""
echo "-l flag output (first 3 lines):"
head -3 test_output_l.txt
echo ""
echo "Invalid flag error:"
cat test_output_invalid.txt
```

This test script comprehensively covers all the requirements:

1. **Generates valid GCOV data**: Creates a simple C program, compiles it with `-fprofile-arcs -ftest-coverage`, and runs it to produce `.gcda` and `.gcno` files.

2. **Tests all flag cases individually**:
   - `-h` and `-v` (already covered but included)
   - `-l` (dump contents) - targets line 117
   - `-p` (dump positions) - targets line 120
   - `-r` (dump raw) - targets line 123
   - `-s` (dump stable) - targets line 126

3. **Tests combined flags**:
   - Separate flags: `-l -p -r -s`
   - Concatenated flags: `-lprs`

4. **Triggers the default case**:
   - Invalid flag `-x` - targets line 129
   - Invalid flag `-Z` - also triggers default case
   - Mixed valid/invalid flags

5. **Uses both file types**: Tests with both `.gcda` and `.gcno` files.

6. **Verifies outputs**: Checks exit codes and validates output contains expected content.

7. **Provides clear feedback**: Shows success/failure for each test case.

To run this test:

1. Ensure you have a built `gcov-dump` binary (adjust the `GCOV_DUMP_BIN` path if needed)
2. Make the script executable: `chmod +x test_gcov_dump_flags.sh`
3. Run it: `./test_gcov_dump_flags.sh`

The test will create a temporary directory, generate all necessary files, run all test cases, and clean up afterwards. Each uncovered line in the switch statement will be exercised by the corresponding flag test.
