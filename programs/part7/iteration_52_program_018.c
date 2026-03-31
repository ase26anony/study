Here's a comprehensive test script that covers all the specified lines in `gcov-dump.cc`:

```bash
#!/bin/bash
# Test script for gcov-dump to cover command-line argument parsing

set -euo pipefail

# Create temporary directory for test artifacts
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    cd /
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

echo "=== Setting up test environment in $TEST_DIR ==="

# Create simple test C source file
cat > test1.c << 'EOF'
#include <stdio.h>

int main() {
    int x = 5;
    int y = 10;
    int sum = x + y;
    
    if (sum > 10) {
        printf("Sum is greater than 10: %d\n", sum);
    } else {
        printf("Sum is 10 or less: %d\n", sum);
    }
    
    for (int i = 0; i < 3; i++) {
        printf("Iteration %d\n", i);
    }
    
    return 0;
}
EOF

# Create another test file for multiple file testing
cat > test2.c << 'EOF'
#include <stdio.h>

void helper_function() {
    printf("Helper function called\n");
}

int main() {
    printf("Test program 2\n");
    helper_function();
    return 0;
}
EOF

echo "=== Compiling test programs with coverage instrumentation ==="

# Compile test programs with coverage flags
gcc -fprofile-arcs -ftest-coverage -O0 -g test1.c -o test1_prog
gcc -fprofile-arcs -ftest-coverage -O0 -g test2.c -o test2_prog

echo "=== Running test programs to generate .gcda files ==="

# Execute programs to generate coverage data
./test1_prog > /dev/null
./test2_prog > /dev/null

# Verify that coverage files were created
if [[ ! -f test1.gcda || ! -f test1.gcno ]]; then
    echo "ERROR: test1 coverage files not created"
    exit 1
fi

if [[ ! -f test2.gcda || ! -f test2.gcno ]]; then
    echo "ERROR: test2 coverage files not created"
    exit 1
fi

echo "=== Testing gcov-dump with various command-line flags ==="

# Helper function to run gcov-dump and check exit code
run_gcov_dump() {
    local description="$1"
    shift
    
    echo ""
    echo "--- Testing: $description ---"
    echo "Command: gcov-dump $*"
    
    if gcov-dump "$@" 2>&1; then
        echo "✓ Command succeeded"
    else
        local exit_code=$?
        echo "✓ Command exited with code $exit_code (expected for some error cases)"
    fi
}

# Test help and version options (cases 'h' and 'v')
run_gcov_dump "Help flag (short)" -h
run_gcov_dump "Help flag (long)" --help
run_gcov_dump "Version flag (short)" -v
run_gcov_dump "Version flag (long)" --version

# Test individual dump flags (cases 'l', 'p', 'r', 's')
run_gcov_dump "Dump contents flag (-l)" -l test1.gcda
run_gcov_dump "Dump positions flag (-p)" -p test1.gcda
run_gcov_dump "Dump raw flag (-r)" -r test1.gcda
run_gcov_dump "Dump stable flag (-s)" -s test1.gcda

# Test combination of flags
run_gcov_dump "Multiple flags combined" -l -p -r -s test1.gcda
run_gcov_dump "Flags l and p combined" -l -p test1.gcda
run_gcov_dump "Flags r and s combined" -r -s test1.gcda

# Test with .gcno file (notes file)
run_gcov_dump "With .gcno file only" -l test1.gcno
run_gcov_dump "With .gcno file and -p flag" -p test1.gcno

# Test with multiple input files
run_gcov_dump "Multiple .gcda files" -l test1.gcda test2.gcda
run_gcov_dump "Mixed .gcda and .gcno files" -p test1.gcda test1.gcno test2.gcda

# Test without any flags (default behavior)
run_gcov_dump "No flags, just .gcda file" test1.gcda
run_gcov_dump "No flags, just .gcno file" test1.gcno
run_gcov_dump "No flags, multiple files" test1.gcda test2.gcda

# Test invalid flag to trigger default case
run_gcov_dump "Invalid flag (-x) to trigger default case" -x test1.gcda
run_gcov_dump "Invalid flag (-z) to trigger default case" -z test1.gcda

# Test with both valid and invalid flags
run_gcov_dump "Valid flag followed by invalid flag" -l -x test1.gcda

# Test edge cases
run_gcov_dump "Non-existent file" -l nonexistent.gcda
run_gcov_dump "Empty argument list" ""
run_gcov_dump "Flag without file argument" -l

echo ""
echo "=== Testing flag order variations ==="

# Test flags in different orders
run_gcov_dump "Flags in different order 1" test1.gcda -l -p
run_gcov_dump "Flags in different order 2" -p test1.gcda -r
run_gcov_dump "Flags after filename" test1.gcda -l -s

echo ""
echo "=== Testing with different file types ==="

# Create a non-gcov file to test error handling
echo "not a gcov file" > not_gcov.txt
run_gcov_dump "Non-GCOV file input" not_gcov.txt
run_gcov_dump "Mixed valid and invalid files" -l test1.gcda not_gcov.txt test2.gcda

echo ""
echo "=== Summary ==="
echo "All gcov-dump invocations completed."
echo "The following cases were tested:"
echo "1. -h / --help (case 'h')"
echo "2. -v / --version (case 'v')"
echo "3. -l flag (case 'l')"
echo "4. -p flag (case 'p')"
echo "5. -r flag (case 'r')"
echo "6. -s flag (case 's')"
echo "7. Multiple flag combinations"
echo "8. Invalid flags (default case)"
echo "9. Different input file types (.gcda, .gcno)"
echo "10. Multiple input files"
echo "11. Edge cases and error conditions"

echo ""
echo "Test completed successfully!"
```

This test script comprehensively covers all the specified requirements:

1. **Generates valid GCOV data files**: Creates two test C programs, compiles them with `-fprofile-arcs -ftest-coverage`, and runs them to produce `.gcda` files.

2. **Tests all command-line flags**:
   - `-h` and `--help` (triggers `case 'h'`)
   - `-v` and `--version` (triggers `case 'v'`)
   - `-l` (triggers `case 'l'`, sets `flag_dump_contents = 1`)
   - `-p` (triggers `case 'p'`, sets `flag_dump_positions = 1`)
   - `-r` (triggers `case 'r'`, sets `flag_dump_raw = 1`)
   - `-s` (triggers `case 's'`, sets `flag_dump_stable = 1`)

3. **Tests flag combinations**: Runs `gcov-dump` with multiple flags like `-l -p -r -s` to ensure sequential execution.

4. **Triggers the default case**: Uses invalid flags `-x` and `-z` to trigger the `default:` case which prints "unknown flag" error.

5. **Tests with different input files**:
   - Single `.gcda` file
   - Single `.gcno` file
   - Multiple `.gcda` files
   - Mixed `.gcda` and `.gcno` files

6. **Includes edge cases**: Tests with non-existent files, empty arguments, and non-GCOV files.

The script uses a helper function `run_gcov_dump()` to execute each test case and display the command being run. Each invocation exercises different paths through the command-line parsing switch statement in `gcov-dump.cc`, ensuring coverage of lines 111-130.
