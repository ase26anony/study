Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that triggers the default case when an invalid single-character flag is provided. Here's a shell script that implements all the requirements:

```bash
#!/bin/bash

# Test script for gcov-dump to trigger uncovered lines 111-130
# Specifically tests the default case for unknown command-line flags

set -e

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d)
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

# 3. Execute the program to generate .gcda file
echo "Running test program to generate coverage data..."
./test > /dev/null

# Verify that .gcda file was created
if [ ! -f test.gcda ]; then
    echo "ERROR: test.gcda file not created!"
    exit 1
fi

echo "Generated test.gcda file successfully."

# 4. Test 1: Single invalid flag with valid flags
echo -e "\n=== Test 1: Valid flags with one invalid flag ==="
echo "Running: gcov-dump -l -p -x test.gcda"
if ! gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
    exit 1
else
    echo "SUCCESS: 'unknown flag' error message detected for -x"
fi

# 5. Test 2: Multiple invalid flags
echo -e "\n=== Test 2: Multiple invalid flags ==="
echo "Running: gcov-dump -r -s -y -z test.gcda"
if ! gcov-dump -r -s -y -z test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
    exit 1
else
    echo "SUCCESS: 'unknown flag' error message detected for -y and -z"
fi

# 6. Test 3: Only invalid flags
echo -e "\n=== Test 3: Only invalid flags ==="
echo "Running: gcov-dump -a -b -c test.gcda"
if ! gcov-dump -a -b -c test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
    exit 1
else
    echo "SUCCESS: 'unknown flag' error message detected for -a, -b, -c"
fi

# 7. Test 4: Invalid flag at different positions
echo -e "\n=== Test 4: Invalid flag at different positions ==="
echo "Running: gcov-dump -? test.gcda"
if ! gcov-dump -? test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
    exit 1
else
    echo "SUCCESS: 'unknown flag' error message detected for -?"
fi

# 8. Test 5: Mixed valid and invalid flags in different order
echo -e "\n=== Test 5: Mixed flags in different order ==="
echo "Running: gcov-dump -x -l -y -p -z test.gcda"
if ! gcov-dump -x -l -y -p -z test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error message not found!"
    exit 1
else
    echo "SUCCESS: 'unknown flag' error message detected for -x, -y, -z"
fi

# 9. Optional: Test with .gcno file as well
echo -e "\n=== Test 6: Testing with .gcno file ==="
if [ -f test.gcno ]; then
    echo "Running: gcov-dump -l -q test.gcno"
    if ! gcov-dump -l -q test.gcno 2>&1 | grep -q "unknown flag"; then
        echo "ERROR: Expected 'unknown flag' error message not found!"
        exit 1
    else
        echo "SUCCESS: 'unknown flag' error message detected for -q with .gcno file"
    fi
fi

echo -e "\n=== All tests completed successfully ==="
echo "The uncovered lines in gcov-dump.cc (111-130) should now be covered."
```

This script implements all the requirements:

1. **Invalid Command-Line Flags**: Uses `-x`, `-y`, `-z`, `-a`, `-b`, `-c`, `-?` which are not in the valid set (`h`, `v`, `l`, `p`, `r`, `s`).

2. **Valid Flag Combinations**: Combines valid flags (`-l`, `-p`, `-r`, `-s`) with invalid ones to ensure the parser reaches the default case.

3. **Input GCOV File Requirement**: Creates a simple C program, compiles it with `-fprofile-arcs -ftest-coverage`, runs it to generate `.gcda` file, and passes it to `gcov-dump`.

4. **Multiple Invalid Flags**: Tests with multiple invalid flags in single invocations (e.g., `-y -z`, `-a -b -c`).

5. **Mixed Flag Syntax**: Tests invalid flags at different positions and in different combinations with valid flags.

6. **Execution Flow**: The script creates the test environment, runs the program, and invokes `gcov-dump` with various flag combinations that should trigger the default case.

The script also includes error checking to verify that:
- The `.gcda` file is created
- The "unknown flag" error message is printed (confirming the path was taken)
- Tests work with both `.gcda` and `.gcno` files

Each test case is designed to ensure the `default` case in the switch statement is executed, covering lines 111-130 in `gcov-dump.cc`.
