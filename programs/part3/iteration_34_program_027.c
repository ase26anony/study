Here's a shell script that specifically targets the uncovered lines in `gcov-dump.cc` by generating coverage data and then invoking `gcov-dump` with invalid flags to trigger the `default` case:

```bash
#!/bin/bash
set -e

# Create a minimal C program for coverage testing
cat > test.c << 'EOF'
#include <stdio.h>

int main(int argc, char *argv[]) {
    int result = argc * 2;
    printf("Result: %d\n", result);
    return result;
}
EOF

# Compile with coverage flags
echo "Compiling test program with coverage..."
gcc -O0 -fprofile-arcs -ftest-coverage -o test test.c

# Run the program to generate .gcda file
echo "Running test program to generate coverage data..."
./test

# Verify .gcda file was created
if [ ! -f "test.gcda" ]; then
    echo "ERROR: test.gcda file not created!"
    exit 1
fi

echo "Testing gcov-dump with invalid flags..."

# Test 1: Single invalid flag with valid flags
echo "=== Test 1: -l -p with invalid -x ==="
gcov-dump -l -p -x test.gcda 2>&1 || true

echo -e "\n=== Test 2: Multiple invalid flags ==="
# Test 2: Multiple invalid flags
gcov-dump -r -s -y -z test.gcda 2>&1 || true

echo -e "\n=== Test 3: Only invalid flags ==="
# Test 3: Only invalid flags
gcov-dump -a -b -c test.gcda 2>&1 || true

echo -e "\n=== Test 4: Invalid flag at different positions ==="
# Test 4: Invalid flag at beginning, middle, and end
gcov-dump -q -l -p test.gcda 2>&1 || true
gcov-dump -l -w -p test.gcda 2>&1 || true
gcov-dump -l -p -? test.gcda 2>&1 || true

echo -e "\n=== Test 5: Mixed valid/invalid with file at different positions ==="
# Test 5: Test with file argument in different positions
gcov-dump test.gcda -l -m 2>&1 || true
gcov-dump -l test.gcda -n 2>&1 || true

# Also test with .gcno file
echo -e "\n=== Test 6: Testing with .gcno file ==="
if [ -f "test.gcno" ]; then
    gcov-dump -l -p -@ test.gcno 2>&1 || true
fi

echo -e "\n=== Test 7: Edge cases ==="
# Test 7: Edge cases - non-alphabetic invalid flags
gcov-dump -1 -2 test.gcda 2>&1 || true
gcov-dump -[ -] test.gcda 2>&1 || true

# Cleanup (optional)
# echo "Cleaning up..."
# rm -f test test.c test.gcno test.gcda test.c.gcov

echo "Test completed. Check output above for 'unknown flag' messages."
```

This script systematically tests the uncovered lines by:

1. **Creating a valid coverage data file**: Compiles and runs a simple C program to generate `.gcda` and `.gcno` files.

2. **Testing various invalid flag scenarios**:
   - Test 1: Mixes valid flags (`-l`, `-p`) with an invalid flag (`-x`)
   - Test 2: Multiple invalid flags (`-y`, `-z`) with valid flags
   - Test 3: Only invalid flags (`-a`, `-b`, `-c`)
   - Test 4: Invalid flags in different positions
   - Test 5: File argument in different positions with invalid flags
   - Test 6: Tests with `.gcno` file (alternative input)
   - Test 7: Non-alphabetic invalid flags

3. **Ensuring the default case is triggered**: Each invalid single-character flag (like `-x`, `-y`, `-z`, `-?`, `-@`, `-1`, etc.) should trigger the `default` case in the switch statement, printing "unknown flag" messages.

The script uses `2>&1` to capture stderr output (where the error messages are printed) and `|| true` to continue even if `gcov-dump` exits with an error code (which it likely will when encountering invalid flags).

To run this test:
1. Save the script as `test_gcov_dump.sh`
2. Make it executable: `chmod +x test_gcov_dump.sh`
3. Run it: `./test_gcov_dump.sh`

The output should show multiple "unknown flag" error messages, confirming that the uncovered lines (111-130, particularly the `default` case) are being executed.
