Here's a shell script that specifically targets the uncovered lines in `gcov-dump.cc` by generating coverage data and then invoking `gcov-dump` with invalid command-line flags:

```bash
#!/bin/bash

# Test script for gcov-dump to trigger uncovered lines 111-130
# Specifically tests the default case for unknown command-line flags

set -e

# Create a temporary directory for our test
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test-XXXXXX)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"

# 1. Create a minimal C program
cat > test.c << 'EOF'
#include <stdio.h>

int main(int argc, char *argv[]) {
    int result = argc * 2;
    printf("Result: %d\n", result);
    return result;
}
EOF

echo "Created test.c"

# 2. Compile with coverage enabled
echo "Compiling with coverage flags..."
gcc -O0 -fprofile-arcs -ftest-coverage -o test test.c

# 3. Execute the program to generate .gcda file
echo "Running program to generate coverage data..."
./test > /dev/null

# Verify that .gcda file was created
if [ ! -f test.gcda ]; then
    echo "ERROR: test.gcda not generated!"
    exit 1
fi

echo "Generated test.gcda file"

# 4. Test 1: Single invalid flag with valid flags
echo -e "\n=== Test 1: Valid flags with one invalid flag (-x) ==="
echo "Command: gcov-dump -l -p -x test.gcda"
if ! gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
    # Continue anyway to test other cases
fi

# 5. Test 2: Multiple invalid flags
echo -e "\n=== Test 2: Multiple invalid flags (-y -z) with valid flags ==="
echo "Command: gcov-dump -r -s -y -z test.gcda"
if ! gcov-dump -r -s -y -z test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 6. Test 3: Only invalid flags
echo -e "\n=== Test 3: Only invalid flags (-a -b -c) ==="
echo "Command: gcov-dump -a -b -c test.gcda"
if ! gcov-dump -a -b -c test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 7. Test 4: Invalid flag at different positions
echo -e "\n=== Test 4: Invalid flag in middle of arguments ==="
echo "Command: gcov-dump -l -? -p test.gcda"
if ! gcov-dump -l -? -p test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 8. Test 5: Test with .gcno file as well (different file type)
echo -e "\n=== Test 5: Testing with .gcno file ==="
echo "Command: gcov-dump -v -x test.gcno"
if ! gcov-dump -v -x test.gcno 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 9. Test 6: Edge case - invalid flag that's a number
echo -e "\n=== Test 6: Invalid numeric flag ==="
echo "Command: gcov-dump -1 test.gcda"
if ! gcov-dump -1 test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 10. Test 7: Mixed valid and invalid flags with file at beginning
echo -e "\n=== Test 7: File argument at beginning ==="
echo "Command: gcov-dump test.gcda -l -x -p"
if ! gcov-dump test.gcda -l -x -p 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

echo -e "\n=== All tests completed ==="
echo "The following invalid flags were tested: x, y, z, a, b, c, ?, 1"
echo "Each should have triggered the 'default' case in the switch statement."

# Cleanup
cd /
rm -rf "$TEST_DIR"
echo "Cleaned up test directory"
```

**Key aspects of this test script:**

1. **Creates valid coverage data**: Compiles and runs a simple C program to generate `.gcda` and `.gcno` files that `gcov-dump` can parse.

2. **Tests multiple invalid flag scenarios**:
   - Test 1: `-l -p -x` - Valid flags followed by invalid `-x`
   - Test 2: `-r -s -y -z` - Valid flags with multiple invalid flags
   - Test 3: `-a -b -c` - Only invalid flags
   - Test 4: `-l -? -p` - Invalid flag `-?` sandwiched between valid flags
   - Test 5: Tests with `.gcno` file instead of `.gcda`
   - Test 6: `-1` - Invalid numeric flag
   - Test 7: `test.gcda -l -x -p` - File argument at beginning

3. **Ensures the parser reaches the default case**: By including valid flags (`-l`, `-p`, `-r`, `-s`, `-v`), the program will enter the option parsing loop and process all arguments.

4. **Uses single-character invalid flags**: All invalid flags are single characters following a single dash, which matches the `case` statement pattern in the uncovered code.

5. **Verifies error messages**: The script checks for the "unknown flag" error message to confirm the path was taken.

**To run this test:**
1. Save the script as `test_gcov_dump.sh`
2. Make it executable: `chmod +x test_gcov_dump.sh`
3. Ensure `gcov-dump` is in your PATH (it's typically in the GCC build directory)
4. Run: `./test_gcov_dump.sh`

The script will create a temporary directory, generate coverage data, and systematically test various invalid flag combinations that should trigger the `default` case in the switch statement at lines 111-130 of `gcov-dump.cc`.
