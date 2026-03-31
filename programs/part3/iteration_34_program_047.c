Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that triggers the default case when an invalid single-character flag is provided. Here's a comprehensive shell script that meets all the requirements:

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
echo "=== Test 1: -l -p -x (x is invalid) ==="
gcov-dump -l -p -x test.gcda 2>&1 || true

echo -e "\n=== Test 2: Multiple invalid flags ==="
# Test 2: Multiple invalid flags
gcov-dump -r -s -y -z test.gcda 2>&1 || true

echo -e "\n=== Test 3: Only invalid flags ==="
# Test 3: Only invalid flags
gcov-dump -x -y -z test.gcda 2>&1 || true

echo -e "\n=== Test 4: Invalid flag at different positions ==="
# Test 4: Invalid flag at beginning, middle, and end
gcov-dump -x -l -p test.gcda 2>&1 || true
gcov-dump -l -x -p test.gcda 2>&1 || true
gcov-dump -l -p -x test.gcda 2>&1 || true

echo -e "\n=== Test 5: Various invalid characters ==="
# Test 5: Test various invalid characters
gcov-dump -? test.gcda 2>&1 || true
gcov-dump -! test.gcda 2>&1 || true
gcov-dump -@ test.gcda 2>&1 || true
gcov-dump -1 test.gcda 2>&1 || true
gcov-dump -a test.gcda 2>&1 || true  # 'a' is not a valid flag
gcov-dump -b test.gcda 2>&1 || true  # 'b' is not a valid flag
gcov-dump -c test.gcda 2>&1 || true  # 'c' is not a valid flag

echo -e "\n=== Test 6: Mixed valid and multiple invalid flags ==="
# Test 6: Complex combination
gcov-dump -l -p -r -s -x -y -z test.gcda 2>&1 || true

# Cleanup
echo -e "\nCleaning up..."
rm -f test test.c test.gcda test.gcno test.o

echo "Test completed successfully!"
```

This script comprehensively tests the uncovered lines by:

1. **Creating a valid coverage file**: Compiles and runs a simple C program to generate `.gcda` files that `gcov-dump` can process.

2. **Testing invalid flags**: Uses various invalid single-character flags (`-x`, `-y`, `-z`, `-?`, `-!`, `-@`, `-1`, `-a`, `-b`, `-c`) that will trigger the default case in the switch statement.

3. **Combining valid and invalid flags**: Tests combinations like `-l -p -x` where `-l` and `-p` are valid flags and `-x` is invalid, ensuring the parser reaches the invalid flag.

4. **Multiple invalid flags**: Tests with multiple invalid flags in a single invocation to trigger the error message multiple times.

5. **Different flag positions**: Tests invalid flags at the beginning, middle, and end of the argument list.

6. **Mixed syntax**: While focusing on single-character flags, the script tests various invalid characters to ensure comprehensive coverage.

The script uses `2>&1` to capture stderr output (where the "unknown flag" error message is printed) and `|| true` to prevent the script from exiting when `gcov-dump` returns a non-zero exit code (which it should when encountering invalid flags).

This test will exercise the exact uncovered lines 111-130 in `gcov-dump.cc`, particularly the `default` case that prints "unknown flag `%c'\n" for invalid single-character flags.
