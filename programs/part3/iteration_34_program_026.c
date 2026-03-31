Here's a shell script that generates a test to cover the uncovered lines in `gcov-dump.cc`:

```bash
#!/bin/bash

# Test script for gcov-dump to trigger uncovered lines (111-130)
# Specifically tests the default case for unknown command-line flags

set -e

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test-XXXXXX)
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

echo "1. Compiling test program with coverage flags..."
gcc -O0 -fprofile-arcs -ftest-coverage -o test test.c

echo "2. Running the program to generate .gcda file..."
./test

echo "3. Testing gcov-dump with valid and invalid flags (single invalid flag)..."
# This should trigger the default case for '-x'
echo "Command: gcov-dump -l -p -x test.gcda"
if gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag error for '-x'"
else
    echo "✗ Failed to trigger unknown flag error"
    # Show the output for debugging
    gcov-dump -l -p -x test.gcda 2>&1
fi

echo ""
echo "4. Testing gcov-dump with multiple invalid flags..."
# This should trigger the default case for '-y' and '-z'
echo "Command: gcov-dump -r -s -y -z test.gcda"
if gcov-dump -r -s -y -z test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag error for '-y' and '-z'"
else
    echo "✗ Failed to trigger unknown flag error"
    # Show the output for debugging
    gcov-dump -r -s -y -z test.gcda 2>&1
fi

echo ""
echo "5. Testing gcov-dump with only invalid flags..."
# This should trigger the default case for '-a', '-b', '-c'
echo "Command: gcov-dump -a -b -c test.gcda"
if gcov-dump -a -b -c test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag error for '-a', '-b', '-c'"
else
    echo "✗ Failed to trigger unknown flag error"
    # Show the output for debugging
    gcov-dump -a -b -c test.gcda 2>&1
fi

echo ""
echo "6. Testing gcov-dump with invalid flag at different positions..."
# Test invalid flag at beginning
echo "Command: gcov-dump -? -l -p test.gcda"
if gcov-dump -? -l -p test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag error for '-?' at beginning"
else
    echo "✗ Failed to trigger unknown flag error"
fi

# Test invalid flag in middle
echo "Command: gcov-dump -l -@ -p test.gcda"
if gcov-dump -l -@ -p test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag error for '-@' in middle"
else
    echo "✗ Failed to trigger unknown flag error"
fi

# Test invalid flag at end
echo "Command: gcov-dump -l -p -# test.gcda"
if gcov-dump -l -p -# test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag error for '-#' at end"
else
    echo "✗ Failed to trigger unknown flag error"
fi

echo ""
echo "7. Testing with .gcno file as well..."
# Also test with .gcno file
echo "Command: gcov-dump -l -q test.gcno"
if gcov-dump -l -q test.gcno 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag error with .gcno file"
else
    echo "✗ Failed to trigger unknown flag error"
fi

echo ""
echo "8. Testing mixed valid/invalid flags to ensure parsing continues..."
echo "Command: gcov-dump -v -h -! -l -$ test.gcda"
output=$(gcov-dump -v -h -! -l -$ test.gcda 2>&1)
echo "$output" | grep "unknown flag" || true

echo ""
echo "9. Verifying valid flags still work when used correctly..."
echo "Command: gcov-dump -l -p -r -s test.gcda"
# This should work without errors
if gcov-dump -l -p -r -s test.gcda >/dev/null 2>&1; then
    echo "✓ Valid flags work correctly"
else
    echo "✗ Valid flags failed unexpectedly"
fi

echo ""
echo "Test completed. Cleaning up..."
cd ..
rm -rf "$TEST_DIR"

echo ""
echo "Summary: The test script has executed multiple invocations of gcov-dump"
echo "with various invalid single-character flags to trigger the default case"
echo "in the switch statement (lines 111-130 of gcov-dump.cc)."
echo "Each invalid flag should have printed: 'unknown flag \`X\''"
```

This script:

1. **Creates a minimal C program** and compiles it with coverage flags to generate `.gcno` and `.gcda` files
2. **Tests various invalid flag combinations**:
   - Single invalid flag with valid flags (`-l -p -x`)
   - Multiple invalid flags (`-r -s -y -z`)
   - Only invalid flags (`-a -b -c`)
   - Invalid flags at different positions (beginning, middle, end)
   - Special characters as invalid flags (`-?`, `-@`, `-#`, `-$`, `-!`)
3. **Tests with both `.gcda` and `.gcno` files**
4. **Verifies valid flags still work** when used without invalid flags
5. **Captures stderr output** to confirm the "unknown flag" error message is printed

The script ensures that `gcov-dump` parses the command-line arguments and reaches the `default` case in the switch statement for each invalid single-character flag, covering lines 111-130.
