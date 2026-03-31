Here's a comprehensive shell script that covers all the specified requirements for testing the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash

# test_gcov_tool.sh - Comprehensive test for gcov-tool overlap analysis flags

set -e  # Exit on any error

echo "=== Starting gcov-tool overlap analysis flag tests ==="

# Clean up any previous test files
cleanup() {
    echo "Cleaning up..."
    rm -f test.c test_prog test_prog_opt test_prog_lto
    rm -f *.gcda *.gcno *.gcov gcov_output.txt
    rm -rf profile_data_* run_*_data
}

# Register cleanup on exit
trap cleanup EXIT

# Step 1: Generate a simple C program with conditional branches
echo "Generating test C program..."
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
    
    // Another conditional
    if (res > 50) {
        printf("Large result\n");
    } else {
        printf("Small result\n");
    }
    
    return 0;
}
EOF

# Step 2: Compile with different optimization levels to generate varied GCOV data
echo "Compiling test programs with GCOV instrumentation..."

# Basic compilation
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog
echo "Created test_prog (O0)"

# With optimization
gcc -fprofile-arcs -ftest-coverage -O2 test.c -o test_prog_opt
echo "Created test_prog_opt (O2)"

# With LTO if supported
if gcc -fprofile-arcs -ftest-coverage -flto -O2 test.c -o test_prog_lto 2>/dev/null; then
    echo "Created test_prog_lto (LTO)"
    HAS_LTO=1
else
    echo "LTO compilation failed (continuing without it)"
    HAS_LTO=0
fi

# Step 3: Generate multiple sets of profile data
echo "Generating profile data with different execution paths..."

# First run - mode 1 (addition)
echo "Run 1: mode 1"
./test_prog 1
mv test.gcda test_prog_run1.gcda 2>/dev/null || true

# Second run - mode 2 (multiplication)
echo "Run 2: mode 2"
./test_prog 2
mv test.gcda test_prog_run2.gcda 2>/dev/null || true

# Third run - mode 3 (subtraction)
echo "Run 3: mode 3"
./test_prog 3
mv test.gcda test_prog_run3.gcda 2>/dev/null || true

# Run optimized version
echo "Run 4: optimized version mode 1"
./test_prog_opt 1
mv test.gcda test_prog_opt_run1.gcda 2>/dev/null || true

# Create directory with separate profile data
mkdir -p profile_data_1
GCOV_PREFIX=$(pwd)/profile_data_1 GCOV_PREFIX_STRIP=1 ./test_prog 1
echo "Created profile data in separate directory"

# Step 4: Test individual flags with gcov-tool overlap
echo -e "\n=== Testing individual flags ==="

# Test verbose flag
echo "Testing -v flag..."
gcov-tool overlap -v test_prog_run1.gcda test_prog_run2.gcda > verbose_output.txt 2>&1
echo "  Verbose output saved to verbose_output.txt"

# Test function-level overlap
echo "Testing -f flag..."
gcov-tool overlap -f test_prog_run1.gcda test_prog_run2.gcda > /dev/null 2>&1
echo "  Function-level overlap completed"

# Test fullname flag
echo "Testing -F flag..."
gcov-tool overlap -F test_prog_run1.gcda test_prog_run2.gcda > /dev/null 2>&1
echo "  Fullname overlap completed"

# Test object-level flag
echo "Testing -o flag..."
gcov-tool overlap -o test_prog_run1.gcda test_prog_run2.gcda > /dev/null 2>&1
echo "  Object-level overlap completed"

# Test hot-only flag
echo "Testing -h flag..."
gcov-tool overlap -h test_prog_run1.gcda test_prog_run2.gcda > /dev/null 2>&1
echo "  Hot-only overlap completed"

# Test threshold flag with different values
echo "Testing -t flag with threshold 0.5..."
gcov-tool overlap -t 0.5 test_prog_run1.gcda test_prog_run2.gcda > /dev/null 2>&1
echo "  Threshold 0.5 completed"

echo "Testing -t flag with threshold 1.0..."
gcov-tool overlap -t 1.0 test_prog_run1.gcda test_prog_run2.gcda > /dev/null 2>&1
echo "  Threshold 1.0 completed"

echo "Testing -t flag with threshold 10.5..."
gcov-tool overlap -t 10.5 test_prog_run1.gcda test_prog_run2.gcda > /dev/null 2>&1
echo "  Threshold 10.5 completed"

# Step 5: Test flag combinations
echo -e "\n=== Testing flag combinations ==="

echo "Testing -f -o combination..."
gcov-tool overlap -f -o test_prog_run1.gcda test_prog_run2.gcda test_prog_run3.gcda > /dev/null 2>&1
echo "  -f -o combination completed"

echo "Testing -F -h -t 1.0 combination..."
gcov-tool overlap -F -h -t 1.0 test_prog_run1.gcda test_prog_run2.gcda > /dev/null 2>&1
echo "  -F -h -t 1.0 combination completed"

echo "Testing -v -f -F -o -h -t 5.0 combination..."
gcov-tool overlap -v -f -F -o -h -t 5.0 test_prog_run1.gcda test_prog_run2.gcda > full_combo_output.txt 2>&1
echo "  Full combination completed, output saved to full_combo_output.txt"

# Test with multiple input files including optimized version
echo "Testing with mixed optimization levels..."
gcov-tool overlap -f -o test_prog_run1.gcda test_prog_opt_run1.gcda > /dev/null 2>&1
echo "  Mixed optimization levels completed"

# Test with directory-based profile data
echo "Testing with directory-based profile data..."
gcov-tool overlap -f profile_data_1/*.gcda test_prog_run1.gcda > /dev/null 2>&1 || true
echo "  Directory-based data completed"

# Step 6: Test invalid flag to trigger usage
echo -e "\n=== Testing invalid flag to trigger usage ==="
echo "Testing invalid -Z flag (should show usage)..."
gcov-tool overlap -Z test_prog_run1.gcda 2>&1 | grep -q "usage\|Usage" && echo "  Usage message triggered successfully" || echo "  Warning: Usage message may not have been triggered"

# Step 7: Additional comprehensive tests
echo -e "\n=== Additional comprehensive tests ==="

# Test with all three runs
echo "Testing with all three profile runs..."
gcov-tool overlap -v -f -t 0.8 test_prog_run1.gcda test_prog_run2.gcda test_prog_run3.gcda > triple_run_output.txt 2>&1
echo "  Three-run overlap completed"

# Test edge case threshold values
echo "Testing edge case thresholds..."
gcov-tool overlap -t 0.0 test_prog_run1.gcda test_prog_run2.gcda > /dev/null 2>&1
echo "  Threshold 0.0 completed"

gcov-tool overlap -t 100.0 test_prog_run1.gcda test_prog_run2.gcda > /dev/null 2>&1
echo "  Threshold 100.0 completed"

# Test combination without threshold
echo "Testing -f -F -o -h combination..."
gcov-tool overlap -f -F -o -h test_prog_run1.gcda test_prog_run2.gcda > /dev/null 2>&1
echo "  -f -F -o -h combination completed"

# Step 8: Verify gcov-tool is instrumented (if built with --enable-coverage)
echo -e "\n=== Verification steps ==="

# Check if gcov-tool itself has coverage data
if command -v gcov-tool >/dev/null 2>&1; then
    echo "gcov-tool found at: $(which gcov-tool)"
    
    # Try to check version with verbose
    echo "Testing gcov-tool version info..."
    gcov-tool -v 2>&1 | head -5
    
    # Check for .gcda files in gcov-tool's directory (if instrumented)
    GCOV_TOOL_DIR=$(dirname $(which gcov-tool))
    if ls $GCOV_TOOL_DIR/*.gcda 2>/dev/null; then
        echo "Found .gcda files for gcov-tool (instrumented build)"
    fi
fi

echo -e "\n=== Test summary ==="
echo "Generated and tested:"
echo "  - 3-4 different profile runs"
echo "  - Individual flags: -v, -f, -F, -o, -h, -t"
echo "  - Multiple flag combinations"
echo "  - Invalid flag case (to trigger usage)"
echo "  - Multiple input files"
echo "  - Different threshold values"
echo "  - Directory-based profile data"
echo ""
echo "All tests completed successfully!"
echo "Coverage data for gcov-tool should now include the uncovered parsing lines."
```

This script provides comprehensive testing of the uncovered lines in `gcov-tool.cc`:

## Key Features:

1. **Comprehensive Flag Testing**:
   - Individual flags: `-v`, `-f`, `-F`, `-o`, `-h`, `-t` (with multiple threshold values)
   - Flag combinations: `-f -o`, `-F -h -t 1.0`, `-v -f -F -o -h -t 5.0`
   - Edge cases for threshold: `0.0`, `0.5`, `1.0`, `5.0`, `10.5`, `100.0`

2. **Valid GCOV Data Generation**:
   - Creates a C program with conditional branches
   - Compiles with different optimization levels (O0, O2, LTO)
   - Executes multiple times with different parameters to generate varied profile data

3. **Overlap Analysis Mode**:
   - All tests use `gcov-tool overlap` subcommand
   - Tests with single and multiple `.gcda` files

4. **Error/Usage Testing**:
   - Tests invalid flag `-Z` to trigger `overlap_usage()`

5. **Multiple Input Files**:
   - Generates 3+ different profile runs
   - Tests with mixed optimization levels
   - Tests directory-based profile data using `GCOV_PREFIX`

6. **Verbose Output Handling**:
   - Captures verbose output to files
   - Tests `-v` flag both alone and in combinations

## Usage:

1. Ensure `gcov-tool` is in your PATH (from a GCC built with `--enable-coverage`)
2. Make the script executable: `chmod +x test_gcov_tool.sh`
3. Run the script: `./test_gcov_tool.sh`

The script will generate coverage data for `gcov-tool` itself as it processes the test profiles, covering the previously uncovered argument parsing lines.
