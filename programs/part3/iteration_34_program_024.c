Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that triggers the default case when an invalid single-character flag is provided. Here's a comprehensive shell script that covers all the requirements:

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
if [ ! -f test.gcda ]; then
    echo "ERROR: test.gcda file not created!"
    exit 1
fi

echo "Testing gcov-dump with invalid flags..."

# Test 1: Single invalid flag with valid flags
echo "Test 1: -l -p -x (x is invalid)"
if ! gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found!"
    exit 1
fi

# Test 2: Multiple invalid flags
echo "Test 2: -r -s -y -z (y and z are invalid)"
if ! gcov-dump -r -s -y -z test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found!"
    exit 1
fi

# Test 3: Only invalid flags
echo "Test 3: -a -b -c (all invalid)"
if ! gcov-dump -a -b -c test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found!"
    exit 1
fi

# Test 4: Invalid flag at different positions
echo "Test 4: -x test.gcda -l (invalid flag first)"
if ! gcov-dump -x test.gcda -l 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found!"
    exit 1
fi

echo "Test 5: -l test.gcda -y (invalid flag last)"
if ! gcov-dump -l test.gcda -y 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found!"
    exit 1
fi

# Test 6: Mixed case invalid flags (uppercase)
echo "Test 6: -L -P -X (uppercase invalid flags)"
if ! gcov-dump -L -P -X test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found!"
    exit 1
fi

# Test 7: Special character invalid flag
echo "Test 7: -? (special character invalid flag)"
if ! gcov-dump -\? test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "ERROR: Expected 'unknown flag' error not found!"
    exit 1
fi

# Test 8: Long option (should not trigger the default case but included for completeness)
echo "Test 8: --invalid-long-option"
gcov-dump --invalid-long-option test.gcda 2>&1 || true

echo "All tests completed successfully!"
echo "The default case in gcov-dump.cc should now be covered for invalid flags."

# Cleanup
rm -f test test.c test.gcda test.gcno
```

This script systematically tests the uncovered lines by:

1. **Creating a valid coverage file**: Compiles and runs a simple C program to generate `.gcda` files that `gcov-dump` can process.

2. **Testing invalid flags in various combinations**:
   - Test 1: Mixes valid flags (`-l`, `-p`) with an invalid flag (`-x`)
   - Test 2: Uses multiple invalid flags (`-y`, `-z`) with valid flags
   - Test 3: Uses only invalid flags (`-a`, `-b`, `-c`)
   - Tests 4-5: Places invalid flags at different positions in the argument list
   - Test 6: Tests uppercase invalid flags (not in the switch statement)
   - Test 7: Tests special character `-?` as an invalid flag

3. **Ensuring the default case is triggered**: Each test uses `grep` to verify that the "unknown flag" error message is printed, confirming that the `default` case in the switch statement was executed.

4. **Including a long option test**: While not targeting the uncovered lines directly, it's included for completeness to show how long options behave differently.

The script ensures that `gcov-dump` parses the command-line arguments and reaches the `default` case for each invalid single-character flag, covering lines 111-130 in `gcov-dump.cc`.
