Looking at the uncovered lines in `gcov-dump.cc`, I need to create a test that triggers the default case for invalid command-line flags. Here's a comprehensive shell script that meets all the requirements:

```bash
#!/bin/bash

# Test script for gcov-dump to trigger uncovered lines 111-130
# Specifically tests the default case for unknown command-line flags

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

echo "1. Compiling test program with coverage flags..."
gcc -O0 -fprofile-arcs -ftest-coverage -o test test.c

echo "2. Running test program to generate coverage data..."
./test

echo "3. Testing gcov-dump with valid and invalid flags (single invalid flag)..."
# This should trigger the default case for '-x'
echo "Command: gcov-dump -l -p -x test.gcda"
if gcov-dump -l -p -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Successfully triggered unknown flag error for '-x'"
else
    echo "✗ Failed to trigger unknown flag error for '-x'"
    exit 1
fi

echo ""
echo "4. Testing gcov-dump with multiple invalid flags..."
# This should trigger the default case for both '-y' and '-z'
echo "Command: gcov-dump -r -s -y -z test.gcda"
output=$(gcov-dump -r -s -y -z test.gcda 2>&1)
echo "$output"

# Count occurrences of "unknown flag" error
flag_count=$(echo "$output" | grep -c "unknown flag")
if [ "$flag_count" -ge 2 ]; then
    echo "✓ Successfully triggered multiple unknown flag errors (count: $flag_count)"
else
    echo "✗ Expected at least 2 unknown flag errors, got $flag_count"
    exit 1
fi

echo ""
echo "5. Testing gcov-dump with only invalid flags..."
# This should trigger the default case for '-a', '-b', and '-c'
echo "Command: gcov-dump -a -b -c test.gcda"
output=$(gcov-dump -a -b -c test.gcda 2>&1)
echo "$output"

flag_count=$(echo "$output" | grep -c "unknown flag")
if [ "$flag_count" -ge 3 ]; then
    echo "✓ Successfully triggered multiple unknown flag errors with only invalid flags (count: $flag_count)"
else
    echo "✗ Expected at least 3 unknown flag errors, got $flag_count"
fi

echo ""
echo "6. Testing mixed valid/invalid flag combinations..."
# Test various combinations
echo "6.1 Testing: gcov-dump -h -x test.gcda"
if gcov-dump -h -x test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Combination -h -x triggers unknown flag error"
fi

echo "6.2 Testing: gcov-dump -v -? test.gcda"
if gcov-dump -v -? test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Combination -v -? triggers unknown flag error"
fi

echo "6.3 Testing: gcov-dump -l -p -r -s -@ test.gcda"
if gcov-dump -l -p -r -s -@ test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ All valid flags plus invalid -@ triggers unknown flag error"
fi

echo ""
echo "7. Testing edge cases..."
echo "7.1 Testing invalid flag at beginning: gcov-dump -q -l test.gcda"
if gcov-dump -q -l test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Invalid flag at beginning triggers error"
fi

echo "7.2 Testing invalid flag at end: gcov-dump -l -p -m test.gcda"
if gcov-dump -l -p -m test.gcda 2>&1 | grep -q "unknown flag"; then
    echo "✓ Invalid flag at end triggers error"
fi

echo ""
echo "8. Testing with .gcno file as well..."
echo "Command: gcov-dump -x test.gcno"
if gcov-dump -x test.gcno 2>&1 | grep -q "unknown flag"; then
    echo "✓ Invalid flag with .gcno file triggers error"
fi

echo ""
echo "Test Summary:"
echo "All tests completed. The default case for unknown flags should have been"
echo "triggered multiple times, covering lines 111-130 in gcov-dump.cc"
echo ""
echo "Generated files:"
ls -la test.c test test.gcno test.gcda

# Cleanup (optional)
# rm -f test test.c test.gcno test.gcda test.o
```

This script provides comprehensive testing of the uncovered lines by:

1. **Creating a valid coverage file**: Compiles and runs a simple C program to generate `.gcda` and `.gcno` files that `gcov-dump` can process.

2. **Testing invalid flags in various contexts**:
   - Single invalid flag mixed with valid flags (`-l -p -x`)
   - Multiple invalid flags (`-y -z`)
   - Only invalid flags (`-a -b -c`)
   - Different positions (beginning, middle, end)

3. **Using diverse invalid characters**: Tests `-x`, `-y`, `-z`, `-?`, `-@`, `-q`, `-m`, `-a`, `-b`, `-c` - all of which should trigger the default case.

4. **Verifying error messages**: Each test checks that the "unknown flag" error message is printed, confirming the code path was taken.

5. **Testing with both file types**: Uses both `.gcda` and `.gcno` files to ensure the flag parsing happens regardless of input file type.

The script ensures that `gcov-dump` will:
- Parse command-line arguments (entering the switch statement)
- Handle valid flags correctly
- Reach the `default` case for invalid flags
- Execute the `fprintf(stderr, "unknown flag '%c'\n", opt);` line
- Continue processing remaining arguments even after encountering invalid flags

This should provide comprehensive coverage of the target lines 111-130 in `gcov-dump.cc`.
