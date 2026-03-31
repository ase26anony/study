Here's a shell script that generates a test program, compiles it with coverage, and invokes `gcov-dump` with invalid flags to trigger the uncovered lines:

```bash
#!/bin/bash

# Test script for gcov-dump uncovered lines (111-130)
# Specifically tests the default case for unknown command-line flags

set -e

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test-XXXXXX)
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    cd /
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

# 1. Create a minimal C program
cat > test.c << 'EOF'
#include <stdio.h>

int main(int argc, char *argv[]) {
    int result = argc * 2;
    printf("Result: %d\n", result);
    return result;
}
EOF

# 2. Compile with coverage enabled
echo "Compiling test program with coverage flags..."
gcc -O0 -fprofile-arcs -ftest-coverage -o test test.c

# 3. Execute to generate .gcda file
echo "Running test program to generate coverage data..."
./test > /dev/null

# Verify .gcda file was created
if [ ! -f test.gcda ]; then
    echo "ERROR: test.gcda not generated!"
    exit 1
fi

echo "Generated test.gcda successfully."

# 4. Test 1: Single invalid flag with valid flags
echo -e "\n=== Test 1: Valid flags with one invalid flag ==="
echo "Command: gcov-dump -l -p -x test.gcda"
if ! gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
    # Continue anyway to try other tests
fi

# 5. Test 2: Multiple invalid flags
echo -e "\n=== Test 2: Multiple invalid flags ==="
echo "Command: gcov-dump -r -s -y -z test.gcda"
if ! gcov-dump -r -s -y -z test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 6. Test 3: Only invalid flags
echo -e "\n=== Test 3: Only invalid flags ==="
echo "Command: gcov-dump -a -b -c test.gcda"
if ! gcov-dump -a -b -c test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 7. Test 4: Invalid flag at different positions
echo -e "\n=== Test 4: Invalid flag at beginning ==="
echo "Command: gcov-dump -? -l -p test.gcda"
if ! gcov-dump -? -l -p test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

echo -e "\n=== Test 5: Invalid flag at end ==="
echo "Command: gcov-dump -l -p -# test.gcda"
if ! gcov-dump -l -p -# test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
fi

# 8. Test 6: Mixed valid/invalid with .gcno file (different input type)
echo -e "\n=== Test 6: Testing with .gcno file ==="
if [ -f test.gcno ]; then
    echo "Command: gcov-dump -v -x test.gcno"
    if ! gcov-dump -v -x test.gcno 2>&1 | grep -q "unknown flag"; then
        echo "ERROR: Expected 'unknown flag' error message not found!"
    fi
else
    echo "Note: test.gcno not found (might be in build directory)"
fi

echo -e "\n=== All tests completed ==="
echo "Check coverage reports to verify lines 111-130 in gcov-dump.cc were executed."
```

**Key aspects of this test script:**

1. **Invalid Flags**: Uses `-x`, `-y`, `-z`, `-?`, `-#`, `-a`, `-b`, `-c` which are not in the valid set (`h`, `v`, `l`, `p`, `r`, `s`).

2. **Flag Combinations**: Tests various combinations:
   - Valid + invalid (`-l -p -x`)
   - Multiple invalid (`-y -z`)
   - Invalid at different positions (`-? -l -p`, `-l -p -#`)

3. **Required Input File**: Creates `test.gcda` by compiling and running a simple C program with coverage flags.

4. **Multiple Invalid Flags**: Tests with multiple invalid flags in single invocations.

5. **Execution Flow**: The script ensures `gcov-dump` parses the flags by providing a valid input file, so it reaches the switch statement and hits the `default` case for invalid flags.

6. **Error Verification**: Checks stderr output for "unknown flag" message to confirm the path was taken.

**To run this test:**

1. Save the script as `test_gcov_dump.sh`
2. Make it executable: `chmod +x test_gcov_dump.sh`
3. Run it: `./test_gcov_dump.sh`

**Expected Output:**
The script will show various "unknown flag" error messages from `gcov-dump`, confirming that the `default` case in the switch statement (lines 111-130) was executed for each invalid flag.
