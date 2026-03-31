Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that triggers the default case in the switch statement for handling command-line flags. Here's a shell script that generates a simple C program, compiles it with coverage, runs it, and then invokes `gcov-dump` with invalid flags:

```bash
#!/bin/bash

# Test script for gcov-dump to trigger uncovered lines 111-130
# Specifically tests the default case for unknown command-line flags

set -e

# Create a temporary directory for our test
TEST_DIR=$(mktemp -d)
cd "$TEST_DIR"

# Create a minimal C program
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
./test > /dev/null

# Verify that .gcda file was created
if [ ! -f test.gcda ]; then
    echo "ERROR: test.gcda file not created!"
    exit 1
fi

echo "Testing gcov-dump with invalid flags..."

# Test 1: Single invalid flag with valid flags
echo "Test 1: -l -p with invalid -x flag"
gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag" && echo "✓ Unknown flag error detected" || echo "✗ No error for invalid flag"

# Test 2: Multiple invalid flags
echo "Test 2: Multiple invalid flags -y -z"
gcov-dump -y -z test.gcda 2>&1 | grep -q "unknown flag" && echo "✓ Unknown flag error detected" || echo "✗ No error for invalid flag"

# Test 3: Invalid flag alone
echo "Test 3: Invalid flag -? alone"
gcov-dump -? test.gcda 2>&1 | grep -q "unknown flag" && echo "✓ Unknown flag error detected" || echo "✗ No error for invalid flag"

# Test 4: Mix of valid and multiple invalid flags
echo "Test 4: -r -s with invalid -a -b flags"
gcov-dump -r -s -a -b test.gcda 2>&1 | grep -q "unknown flag" && echo "✓ Unknown flag error detected" || echo "✗ No error for invalid flag"

# Test 5: Invalid flag at different positions
echo "Test 5: Invalid flag at beginning -c followed by valid -l"
gcov-dump -c -l test.gcda 2>&1 | grep -q "unknown flag" && echo "✓ Unknown flag error detected" || echo "✗ No error for invalid flag"

echo "Test 6: Valid flag -l followed by invalid -d at end"
gcov-dump -l -d test.gcda 2>&1 | grep -q "unknown flag" && echo "✓ Unknown flag error detected" || echo "✗ No error for invalid flag"

# Test 7: Edge case - uppercase invalid flag (different from lowercase)
echo "Test 7: Uppercase invalid flag -X"
gcov-dump -X test.gcda 2>&1 | grep -q "unknown flag" && echo "✓ Unknown flag error detected" || echo "✗ No error for invalid flag"

# Test 8: Number as invalid flag (not a letter)
echo "Test 8: Number as invalid flag -1"
gcov-dump -1 test.gcda 2>&1 | grep -q "unknown flag" && echo "✓ Unknown flag error detected" || echo "✗ No error for invalid flag"

# Clean up
cd ..
rm -rf "$TEST_DIR"

echo "All tests completed!"
```

**Key aspects of this test:**

1. **Invalid Flags**: Uses various invalid single-character flags (`-x`, `-y`, `-z`, `-?`, `-a`, `-b`, `-c`, `-d`, `-X`, `-1`) that are not in the valid set (`h`, `v`, `l`, `p`, `r`, `s`).

2. **Valid Flag Combinations**: Combines invalid flags with valid ones (`-l -p -x`, `-r -s -a -b`, `-c -l`, `-l -d`) to ensure the parsing loop executes fully.

3. **Input File Requirement**: Creates and compiles a simple C program with coverage flags, runs it to generate `test.gcda`, and uses this as input to `gcov-dump`.

4. **Multiple Invalid Flags**: Tests with multiple invalid flags in single invocations (`-y -z`, `-a -b`).

5. **Edge Cases**: Tests uppercase letters and numbers as invalid flags, which should also trigger the default case.

6. **Error Verification**: Uses `grep` to check for the "unknown flag" error message in stderr, confirming the path was taken.

The test ensures that `gcov-dump` will parse the command-line arguments, encounter the invalid flags, and execute the `default` case in the switch statement, printing the error message via `fprintf(stderr, "unknown flag '%c'\n", opt)`.
