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

# Verify coverage files were created
if [ ! -f test.gcda ] || [ ! -f test.gcno ]; then
    echo "ERROR: Coverage files not generated!"
    exit 1
fi

echo "Coverage files created: test.gcda, test.gcno"

# Check if gcov-dump exists
GCOV_DUMP="gcov-dump"
if ! command -v $GCOV_DUMP &> /dev/null; then
    echo "ERROR: gcov-dump not found in PATH"
    echo "Please build gcov-dump from GCC source or install it"
    exit 1
fi

echo "Found gcov-dump: $(which $GCOV_DUMP)"
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

echo "=== Testing flag combinations ==="

# Test -lp combination (both flags set)
echo "7. Testing -lp flag combination:"
$GCOV_DUMP -lp test.gcda 2>&1 | head -10
echo ""

# Test -rl combination (both flags set)
echo "8. Testing -rl flag combination:"
$GCOV_DUMP -rl test.gcda 2>&1 | head -10
echo ""

# Test -lps combination (three flags)
echo "9. Testing -lps flag combination:"
$GCOV_DUMP -lps test.gcda 2>&1 | head -10
echo ""

echo "=== Testing invalid flags ==="

# Test invalid single-character flag
echo "10. Testing invalid flag -x (should trigger default case):"
$GCOV_DUMP -x test.gcda 2>&1 | grep "unknown flag"
echo ""

# Test another invalid flag
echo "11. Testing invalid flag -z:"
$GCOV_DUMP -z test.gcda 2>&1 | grep "unknown flag"
echo ""

# Test invalid flag in combination
echo "12. Testing -lx (valid + invalid combination):"
$GCOV_DUMP -lx test.gcda 2>&1 | grep "unknown flag"
echo ""

echo "=== Testing edge cases ==="

# Test missing filename for flags that need it
echo "13. Testing -l without filename (should show error):"
$GCOV_DUMP -l 2>&1 | head -3
echo ""

# Test -p without filename
echo "14. Testing -p without filename:"
$GCOV_DUMP -p 2>&1 | head -3
echo ""

# Test empty arguments
echo "15. Testing with no arguments (should show usage or error):"
$GCOV_DUMP 2>&1 | head -3
echo ""

# Test with just a filename (no flags)
echo "16. Testing with just filename (no flags):"
$GCOV_DUMP test.gcda 2>&1 | head -5
echo ""

# Test long options if supported (--help, --version)
echo "17. Testing long options if supported:"
$GCOV_DUMP --help 2>&1 | head -5
echo ""

$GCOV_DUMP --version 2>&1 | head -2
echo ""

echo "=== Testing with different file types ==="

# Test with .gcno file (notes file)
echo "18. Testing with .gcno file:"
$GCOV_DUMP -l test.gcno 2>&1 | head -10
echo ""

# Test with non-existent file
echo "19. Testing with non-existent file:"
$GCOV_DUMP -l nonexistent.gcda 2>&1 | head -3
echo ""

echo "=== Testing output redirection ==="

# Test with output redirection
echo "20. Testing with stdout/stderr redirection:"
$GCOV_DUMP -h > help_output.txt 2>&1
echo "Help output saved to help_output.txt ($(wc -l < help_output.txt) lines)"

$GCOV_DUMP -x test.gcda 2> error_output.txt
echo "Error output saved to error_output.txt:"
cat error_output.txt
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
echo "Flag combinations tested: -lp, -rl, -lps"
echo "Edge cases tested: missing filename, no arguments, different file types"

# Cleanup
echo ""
echo "=== Cleaning up ==="
rm -f test.c test_prog test.gcda test.gcno test.gcov
rm -f help_output.txt error_output.txt
echo "Test files cleaned up"
```

To run this test script:

1. Save it as `run_gcov_dump_tests.sh`
2. Make it executable: `chmod +x run_gcov_dump_tests.sh`
3. Run it: `./run_gcov_dump_tests.sh`

**Key features of this test script:**

1. **Comprehensive coverage**: Tests all single-character flags (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) individually
2. **Invalid flag handling**: Tests `-x` and `-z` to trigger the `default` case
3. **Flag combinations**: Tests `-lp`, `-rl`, `-lps` to ensure the option parsing loop processes each character
4. **Missing arguments**: Tests `-l` and `-p` without filenames
5. **Different file types**: Tests with both `.gcda` and `.gcno` files
6. **Error cases**: Tests with non-existent files
7. **Output redirection**: Captures output for verification
8. **Long options**: Tests `--help` and `--version` if supported

**Expected coverage results:**
- Lines 111-130 in `gcov-dump.cc` should be fully covered
- The `switch` statement cases for each flag will be executed
- The `default` case will be triggered by invalid flags
- The option parsing loop will handle multiple flags in a single argument

**Note**: If `gcov-dump` is not in your PATH, modify the `GCOV_DUMP` variable at the top of the script to point to the correct location.
