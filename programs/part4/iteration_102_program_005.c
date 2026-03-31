Here's a comprehensive test script that covers all the uncovered lines in the `gcov-dump` command-line argument parsing logic:

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

# Run the test program to generate .gcda file
echo "Running test program to generate coverage data..."
./test_prog > /dev/null

# Verify coverage files were generated
if [ ! -f test.gcda ]; then
    echo "ERROR: test.gcda not generated!"
    exit 1
fi

echo "Coverage data generated: test.gcda"

# Check if gcov-dump exists
GCOV_DUMP="gcov-dump"
if ! command -v $GCOV_DUMP &> /dev/null; then
    echo "ERROR: gcov-dump not found in PATH"
    echo "Please ensure gcov-dump is built and available"
    exit 1
fi

echo "Using gcov-dump: $(which $GCOV_DUMP)"
echo ""

echo "=== Testing individual valid flags ==="

# Test -h flag (help)
echo "1. Testing -h flag (help):"
$GCOV_DUMP -h 2>&1 | head -5
echo ""

# Test -v flag (version)
echo "2. Testing -v flag (version):"
$GCOV_DUMP -v 2>&1
echo ""

# Test -l flag (dump contents) with valid file
echo "3. Testing -l flag (dump contents):"
$GCOV_DUMP -l test.gcda 2>&1 | head -10
echo ""

# Test -p flag (dump positions) with valid file
echo "4. Testing -p flag (dump positions):"
$GCOV_DUMP -p test.gcda 2>&1 | head -10
echo ""

# Test -r flag (dump raw) with valid file
echo "5. Testing -r flag (dump raw):"
$GCOV_DUMP -r test.gcda 2>&1 | head -10
echo ""

# Test -s flag (dump stable) with valid file
echo "6. Testing -s flag (dump stable):"
$GCOV_DUMP -s test.gcda 2>&1 | head -10
echo ""

echo "=== Testing combined flags ==="

# Test -lp combination (dump contents and positions)
echo "7. Testing -lp combination:"
$GCOV_DUMP -lp test.gcda 2>&1 | head -10
echo ""

# Test -rl combination (dump raw and contents)
echo "8. Testing -rl combination:"
$GCOV_DUMP -rl test.gcda 2>&1 | head -10
echo ""

# Test -spr combination (multiple flags)
echo "9. Testing -spr combination:"
$GCOV_DUMP -spr test.gcda 2>&1 | head -10
echo ""

echo "=== Testing invalid flags ==="

# Test invalid single-character flag
echo "10. Testing invalid flag -x:"
$GCOV_DUMP -x test.gcda 2>&1 | grep "unknown flag"
echo ""

# Test another invalid flag
echo "11. Testing invalid flag -z:"
$GCOV_DUMP -z test.gcda 2>&1 | grep "unknown flag"
echo ""

# Test invalid flag in combination
echo "12. Testing invalid flag in combination -lx:"
$GCOV_DUMP -lx test.gcda 2>&1 | grep "unknown flag"
echo ""

echo "=== Testing edge cases ==="

# Test flag without required filename
echo "13. Testing -l flag without filename (should error):"
$GCOV_DUMP -l 2>&1 | head -5
echo ""

# Test multiple files with flag
echo "14. Testing with multiple files:"
$GCOV_DUMP -l test.gcda test.gcno 2>&1 | head -10
echo ""

# Test with .gcno file (coverage notes)
echo "15. Testing with .gcno file:"
$GCOV_DUMP -l test.gcno 2>&1 | head -10
echo ""

# Test with non-existent file
echo "16. Testing with non-existent file:"
$GCOV_DUMP -l nonexistent.gcda 2>&1 | head -5
echo ""

echo "=== Testing long options (if supported) ==="

# Test --help (if supported)
echo "17. Testing --help:"
$GCOV_DUMP --help 2>&1 | head -5
echo ""

# Test --version (if supported)
echo "18. Testing --version:"
$GCOV_DUMP --version 2>&1 | head -5
echo ""

echo "=== Testing no arguments ==="

# Test with no arguments
echo "19. Testing with no arguments:"
$GCOV_DUMP 2>&1 | head -5
echo ""

echo "=== Summary ==="
echo "All tests completed successfully!"
echo "The following switch cases were exercised:"
echo "  - case 'h': help flag"
echo "  - case 'v': version flag"
echo "  - case 'l': dump contents flag"
echo "  - case 'p': dump positions flag"
echo "  - case 'r': dump raw flag"
echo "  - case 's': dump stable flag"
echo "  - default: invalid flags (-x, -z)"
echo ""
echo "Additionally tested:"
echo "  - Combined flags (-lp, -rl, -spr)"
echo "  - Missing filename error case"
echo "  - Multiple file arguments"
echo "  - Different file types (.gcda, .gcno)"
echo "  - Non-existent file handling"

# Cleanup (optional - comment out to keep files for inspection)
echo ""
echo "=== Cleaning up ==="
rm -f test.c test_prog test.gcda test.gcno test.o
echo "Test files cleaned up"
```

To run this test script:

1. Save it as `run_gcov_dump_tests.sh`
2. Make it executable: `chmod +x run_gcov_dump_tests.sh`
3. Ensure `gcov-dump` is in your PATH (or modify the `GCOV_DUMP` variable to point to your binary)
4. Run it: `./run_gcov_dump_tests.sh`

The script will:
1. Create a minimal C program and compile it with coverage instrumentation
2. Run the program to generate `.gcda` coverage data files
3. Test each individual flag (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`)
4. Test combined flags (`-lp`, `-rl`, etc.)
5. Test invalid flags to trigger the `default` case
6. Test edge cases (missing files, multiple files, etc.)
7. Clean up generated files

This comprehensive test ensures that all the uncovered lines in the switch statement are executed, including:
- Each `case` for valid flags
- The `default` case for invalid flags
- The flag parsing loop for combined flags
- Error paths for missing arguments

The output will show the results of each test, and you can verify that the coverage data now shows these lines as covered.
