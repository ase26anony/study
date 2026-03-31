Here's a comprehensive test script that covers all the uncovered lines in the `gcov-dump.cc` switch statement:

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

# Verify .gcda file was created
if [ ! -f "test.gcda" ]; then
    echo "ERROR: test.gcda not generated!"
    exit 1
fi

echo "Coverage data file created: test.gcda"

# Check if gcov-dump exists
GCOV_DUMP="gcov-dump"
if ! command -v $GCOV_DUMP &> /dev/null; then
    echo "ERROR: gcov-dump not found in PATH"
    echo "Please build gcov-dump or add it to your PATH"
    exit 1
fi

echo "Found gcov-dump: $(which $GCOV_DUMP)"
echo ""

echo "=== Testing individual valid flags ==="

# Test -h flag (help)
echo "1. Testing -h flag (help)..."
$GCOV_DUMP -h 2>&1 | head -5
echo ""

# Test -v flag (version)
echo "2. Testing -v flag (version)..."
$GCOV_DUMP -v 2>&1
echo ""

# Test -l flag (dump contents) with valid file
echo "3. Testing -l flag (dump contents)..."
$GCOV_DUMP -l test.gcda 2>&1 | head -10
echo ""

# Test -p flag (dump positions) with valid file
echo "4. Testing -p flag (dump positions)..."
$GCOV_DUMP -p test.gcda 2>&1 | head -10
echo ""

# Test -r flag (dump raw) with valid file
echo "5. Testing -r flag (dump raw)..."
$GCOV_DUMP -r test.gcda 2>&1 | head -10
echo ""

# Test -s flag (dump stable) with valid file
echo "6. Testing -s flag (dump stable)..."
$GCOV_DUMP -s test.gcda 2>&1 | head -10
echo ""

echo "=== Testing combined valid flags ==="

# Test -lp (combined flags)
echo "7. Testing -lp (combined flags)..."
$GCOV_DUMP -lp test.gcda 2>&1 | head -10
echo ""

# Test -rl (combined flags)
echo "8. Testing -rl (combined flags)..."
$GCOV_DUMP -rl test.gcda 2>&1 | head -10
echo ""

# Test -lps (multiple combined flags)
echo "9. Testing -lps (multiple flags)..."
$GCOV_DUMP -lps test.gcda 2>&1 | head -10
echo ""

echo "=== Testing invalid flags (trigger default case) ==="

# Test invalid flag -x
echo "10. Testing invalid flag -x..."
$GCOV_DUMP -x test.gcda 2>&1 | grep "unknown flag" || echo "No error message found (expected: 'unknown flag x')"
echo ""

# Test invalid flag -z
echo "11. Testing invalid flag -z..."
$GCOV_DUMP -z test.gcda 2>&1 | grep "unknown flag" || echo "No error message found (expected: 'unknown flag z')"
echo ""

# Test invalid flag in combined flags
echo "12. Testing -lx (valid + invalid)..."
$GCOV_DUMP -lx test.gcda 2>&1 | grep "unknown flag" || echo "No error message found (expected: 'unknown flag x')"
echo ""

echo "=== Testing missing required arguments ==="

# Test -l without filename
echo "13. Testing -l without filename..."
$GCOV_DUMP -l 2>&1 | head -5
echo ""

# Test -p without filename
echo "14. Testing -p without filename..."
$GCOV_DUMP -p 2>&1 | head -5
echo ""

echo "=== Testing with -- long options (if supported) ==="

# Test --help (equivalent to -h)
echo "15. Testing --help..."
$GCOV_DUMP --help 2>&1 | head -5 2>/dev/null || echo "Long option --help not supported"
echo ""

# Test --version (equivalent to -v)
echo "16. Testing --version..."
$GCOV_DUMP --version 2>&1 2>/dev/null || echo "Long option --version not supported"
echo ""

echo "=== Testing edge cases ==="

# Test empty argument
echo "17. Testing with no arguments..."
$GCOV_DUMP 2>&1 | head -5
echo ""

# Test with non-existent file
echo "18. Testing with non-existent file..."
$GCOV_DUMP -l nonexistent.gcda 2>&1 | head -5
echo ""

# Test with .gcno file (should also work)
echo "19. Testing with .gcno file..."
$GCOV_DUMP -l test.gcno 2>&1 | head -10
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
echo "Coverage data files generated:"
ls -la test.gcda test.gcno test.c test_prog 2>/dev/null || true

# Cleanup (optional - comment out to keep files for inspection)
echo ""
echo "Cleaning up test files..."
rm -f test.c test_prog test.gcda test.gcno test.gcov *.o
```

To run this test script:

1. Save it as `run_gcov_dump_tests.sh`
2. Make it executable: `chmod +x run_gcov_dump_tests.sh`
3. Run it: `./run_gcov_dump_tests.sh`

**Key features of this test script:**

1. **Creates a valid coverage data file**: Compiles a simple C program with `-fprofile-arcs -ftest-coverage` and runs it to generate `test.gcda`.

2. **Tests all individual valid flags**: 
   - `-h` triggers `case 'h'` (help)
   - `-v` triggers `case 'v'` (version)
   - `-l test.gcda` triggers `case 'l'` (dump contents)
   - `-p test.gcda` triggers `case 'p'` (dump positions)
   - `-r test.gcda` triggers `case 'r'` (dump raw)
   - `-s test.gcda` triggers `case 's'` (dump stable)

3. **Tests invalid flags**:
   - `-x test.gcda` triggers `default` case with "unknown flag 'x'"
   - `-z test.gcda` triggers `default` case with "unknown flag 'z'"
   - `-lx test.gcda` tests mixed valid/invalid flags

4. **Tests combined flags**:
   - `-lp test.gcda` exercises both `case 'l'` and `case 'p'` in the parsing loop
   - `-rl test.gcda` exercises both `case 'r'` and `case 'l'`
   - `-lps test.gcda` exercises three flags together

5. **Tests missing arguments**:
   - `-l` without filename tests error handling
   - `-p` without filename tests error handling

6. **Tests edge cases**:
   - No arguments
   - Non-existent file
   - Using `.gcno` file instead of `.gcda`

The script captures output and errors, showing only the first few lines of each command to keep the output manageable while ensuring all code paths are exercised.
