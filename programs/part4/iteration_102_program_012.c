Here's a comprehensive test script that covers all the specified requirements for the uncovered lines in `gcov-dump.cc`:

```bash
#!/bin/bash
# run_gcov_dump_tests.sh - Test script to cover gcov-dump command-line parsing

set -e  # Exit on any error

echo "=== Setting up test environment ==="

# Create a minimal C test program
cat > test.c << 'EOF'
#include <stdio.h>

int main() {
    int i, sum = 0;
    
    // Some basic operations to generate coverage data
    for (i = 0; i < 10; i++) {
        sum += i;
    }
    
    if (sum > 0) {
        printf("Sum: %d\n", sum);
    } else {
        printf("No sum\n");
    }
    
    return 0;
}
EOF

# Compile test program with coverage instrumentation
echo "Compiling test program with coverage..."
gcc -fprofile-arcs -ftest-coverage -O0 -o test_prog test.c

# Run test program to generate .gcda file
echo "Running test program to generate coverage data..."
./test_prog > /dev/null

# Verify coverage files were generated
if [ ! -f test.gcda ] || [ ! -f test.gcno ]; then
    echo "ERROR: Coverage files not generated!"
    exit 1
fi

echo "Coverage files generated: test.gcda, test.gcno"

# Check if gcov-dump exists
GCOV_DUMP="gcov-dump"
if ! command -v $GCOV_DUMP &> /dev/null; then
    echo "ERROR: gcov-dump not found in PATH"
    echo "Please build gcov-dump from GCC source or ensure it's in your PATH"
    exit 1
fi

echo "Found gcov-dump: $(which $GCOV_DUMP)"
echo ""

echo "=== Testing valid single-character flags ==="

# Test -h flag (help)
echo "1. Testing -h flag (help):"
$GCOV_DUMP -h 2>&1 | head -5
echo ""

# Test -v flag (version)
echo "2. Testing -v flag (version):"
$GCOV_DUMP -v 2>&1
echo ""

# Test -l flag (dump contents) with file
echo "3. Testing -l flag (dump contents):"
$GCOV_DUMP -l test.gcda 2>&1 | head -10
echo ""

# Test -p flag (dump positions) with file
echo "4. Testing -p flag (dump positions):"
$GCOV_DUMP -p test.gcda 2>&1 | head -10
echo ""

# Test -r flag (dump raw) with file
echo "5. Testing -r flag (dump raw):"
$GCOV_DUMP -r test.gcda 2>&1 | head -10
echo ""

# Test -s flag (dump stable) with file
echo "6. Testing -s flag (dump stable):"
$GCOV_DUMP -s test.gcda 2>&1 | head -10
echo ""

echo "=== Testing flag combinations ==="

# Test -lp combination (both flags)
echo "7. Testing -lp flag combination:"
$GCOV_DUMP -lp test.gcda 2>&1 | head -10
echo ""

# Test -rl combination (both flags)
echo "8. Testing -rl flag combination:"
$GCOV_DUMP -rl test.gcda 2>&1 | head -10
echo ""

# Test -lps combination (three flags)
echo "9. Testing -lps flag combination:"
$GCOV_DUMP -lps test.gcda 2>&1 | head -10
echo ""

echo "=== Testing invalid flags ==="

# Test invalid single-character flag
echo "10. Testing invalid flag -x:"
$GCOV_DUMP -x test.gcda 2>&1 | grep -E "unknown flag|error" || true
echo ""

# Test another invalid flag
echo "11. Testing invalid flag -z:"
$GCOV_DUMP -z test.gcda 2>&1 | grep -E "unknown flag|error" || true
echo ""

# Test invalid flag in combination
echo "12. Testing invalid flag in combination -lx:"
$GCOV_DUMP -lx test.gcda 2>&1 | grep -E "unknown flag|error" || true
echo ""

echo "=== Testing edge cases ==="

# Test missing filename for flags that require it
echo "13. Testing -l flag without filename (should error):"
$GCOV_DUMP -l 2>&1 | grep -E "error|usage|missing" || true
echo ""

# Test empty arguments
echo "14. Testing with no arguments:"
$GCOV_DUMP 2>&1 | head -5
echo ""

# Test with --help (long option if supported)
echo "15. Testing --help (long option):"
$GCOV_DUMP --help 2>&1 | head -5
echo ""

# Test with --version (long option if supported)
echo "16. Testing --version (long option):"
$GCOV_DUMP --version 2>&1 | head -5
echo ""

# Test with multiple files
echo "17. Testing with multiple files:"
$GCOV_DUMP -l test.gcda test.gcno 2>&1 | head -10
echo ""

# Test with non-existent file
echo "18. Testing with non-existent file:"
$GCOV_DUMP -l nonexistent.gcda 2>&1 | grep -E "error|cannot open" || true
echo ""

echo "=== Testing with different file types ==="

# Test with .gcno file
echo "19. Testing with .gcno file:"
$GCOV_DUMP -l test.gcno 2>&1 | head -10
echo ""

# Test with both .gcda and .gcno
echo "20. Testing with both file types:"
$GCOV_DUMP -l test.gcda test.gcno 2>&1 | tail -5
echo ""

echo "=== Summary ==="
echo "All tests completed successfully!"
echo "The following command-line options were tested:"
echo "  - Valid single flags: -h, -v, -l, -p, -r, -s"
echo "  - Flag combinations: -lp, -rl, -lps"
echo "  - Invalid flags: -x, -z, -lx"
echo "  - Edge cases: missing filename, no args, long options"
echo "  - Different file types: .gcda, .gcno, both"

# Cleanup (optional - comment out to keep files for inspection)
echo ""
echo "=== Cleaning up ==="
rm -f test.c test_prog test.gcda test.gcno test.o
echo "Test files cleaned up"
```

To use this test script:

1. Save it as `run_gcov_dump_tests.sh`
2. Make it executable: `chmod +x run_gcov_dump_tests.sh`
3. Ensure `gcov-dump` is in your PATH (or modify the `GCOV_DUMP` variable to point to your binary)
4. Run it: `./run_gcov_dump_tests.sh`

This script comprehensively tests:

1. **All valid single-character flags** (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) - covering each case in the switch statement
2. **Invalid flags** (`-x`, `-z`) - triggering the default case
3. **Flag combinations** (`-lp`, `-rl`, `-lps`) - testing the option parsing loop
4. **Edge cases** - missing files, no arguments, long options
5. **Different file types** - both `.gcda` and `.gcno` files

The script generates a minimal C program, compiles it with coverage, runs it to produce `.gcda` files, and then uses those files to test `gcov-dump` with various command-line arguments. Each test case redirects output appropriately and shows only relevant portions to keep the output manageable.

This should provide 100% coverage for the specified switch statement lines in `gcov-dump.cc`.
