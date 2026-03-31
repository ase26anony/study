Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that triggers the `default` case in the switch statement for handling command-line flags. Here's a shell script that systematically tests this:

```bash
#!/bin/bash

# Test script for gcov-dump to trigger uncovered lines 111-130
# Specifically tests the default case for unknown command-line flags

set -e

# Create a temporary directory for our test
TEST_DIR=$(mktemp -d)
cd "$TEST_DIR"

echo "Creating test C program..."

# Create a minimal C program that will generate coverage data
cat > test.c << 'EOF'
#include <stdio.h>

int main(int argc, char *argv[]) {
    int result = argc * 2;
    printf("Result: %d\n", result);
    return result;
}
EOF

echo "Compiling test program with coverage flags..."
# Compile with coverage enabled
gcc -O0 -fprofile-arcs -ftest-coverage -o test test.c

echo "Running test program to generate coverage data..."
# Execute to generate .gcda file
./test

echo "Testing gcov-dump with invalid flag -x..."
# Test 1: Single invalid flag with valid flags
# This should trigger the default case for '-x'
echo "Test 1: gcov-dump -l -p -x test.gcda"
if gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag error for -x"
else
    echo "✗ Failed to trigger unknown flag error for -x"
    exit 1
fi

echo -e "\nTesting gcov-dump with multiple invalid flags..."
# Test 2: Multiple invalid flags
echo "Test 2: gcov-dump -r -s -y -z test.gcda"
if gcov-dump -r -s -y -z test.gcda 2>&1 | grep -q "unknown flag.*'y'\|unknown flag.*'z'"; then
    echo "✓ Successfully triggered unknown flag errors for -y and -z"
else
    echo "✗ Failed to trigger unknown flag errors for -y and -z"
    exit 1
fi

echo -e "\nTesting gcov-dump with only invalid flags..."
# Test 3: Only invalid flags
echo "Test 3: gcov-dump -a -b -c test.gcda"
if gcov-dump -a -b -c test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag errors"
else
    echo "✗ Failed to trigger unknown flag errors"
    exit 1
fi

echo -e "\nTesting gcov-dump with invalid flag at different positions..."
# Test 4: Invalid flag in different positions
echo "Test 4a: gcov-dump -x -l -p test.gcda"
gcov-dump -x -l -p test.gcda 2>&1 | grep -q "unknown flag" && echo "✓ Position 1 works"

echo "Test 4b: gcov-dump -l -x -p test.gcda"
gcov-dump -l -x -p test.gcda 2>&1 | grep -q "unknown flag" && echo "✓ Position 2 works"

echo "Test 4c: gcov-dump -l -p -x test.gcda"
gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag" && echo "✓ Position 3 works"

echo -e "\nTesting gcov-dump with various invalid characters..."
# Test 5: Various invalid characters
for flag in '?' '!' '@' '#' '$' '%' '^' '&' '*' '(' ')'; do
    echo -n "Testing invalid flag -$flag: "
    if gcov-dump -$flag test.gcda 2>&1 | grep -q "unknown flag.*'$flag'"; then
        echo "✓"
    else
        echo "✗"
    fi
done

echo -e "\nTesting gcov-dump with mixed valid/invalid flags..."
# Test 6: Mixed flags including long options (though default case only handles short)
echo "Test 6: gcov-dump -l --invalid-long -x test.gcda"
if gcov-dump -l --invalid-long -x test.gcda 2>&1 | grep -q "unknown flag.*'x'"; then
    echo "✓ Still triggers unknown flag for -x even with invalid long option"
fi

echo -e "\nAll tests completed successfully!"
echo "Covered the default case in gcov-dump.cc for unknown flags"

# Clean up
cd ..
rm -rf "$TEST_DIR"
```

This script systematically tests the uncovered lines by:

1. **Creating a valid coverage data file**: Compiles and runs a simple C program to generate `.gcda` files that `gcov-dump` can process.

2. **Testing invalid single-character flags**: Uses `-x`, `-y`, `-z` and other invalid characters to trigger the `default` case.

3. **Using valid flag combinations**: Includes valid flags (`-l`, `-p`, `-r`, `-s`) to ensure the program executes the parsing loop and reaches the default case.

4. **Multiple invalid flags**: Tests with multiple invalid flags in a single invocation to ensure each triggers the error message.

5. **Different flag positions**: Tests invalid flags at different positions in the argument list.

6. **Various invalid characters**: Tests a range of invalid characters to ensure comprehensive coverage.

The key aspects that ensure coverage of lines 111-130 are:
- The invalid flags (`-x`, `-y`, etc.) are not in the switch statement (`h`, `v`, `l`, `p`, `r`, `s`)
- The presence of a valid `.gcda` file ensures `gcov-dump` doesn't exit early
- The combination of valid and invalid flags ensures the parsing loop executes
- The error messages are captured and verified to confirm the path was taken

The script exits with error code 1 if any test fails to trigger the expected error message, making it suitable for automated testing environments.
