Looking at the uncovered lines in `gcov-tool.cc`, I need to create a comprehensive test that exercises all the command-line options for the `overlap` subcommand. Here's a complete test script that generates coverage data and tests all the uncovered cases:

```bash
#!/bin/bash

# Test script for gcov-tool overlap functionality
# This script tests the specific uncovered lines in gcov-tool.cc (lines 534-554)

set -e

# Create a temporary directory for our test
TEST_DIR=$(mktemp -d)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"

# Create a simple C test program with multiple code paths
cat > test.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

void func1(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else {
        printf("Non-positive: %d\n", x);
    }
}

void func2(int x) {
    for (int i = 0; i < x; i++) {
        printf("Loop iteration: %d\n", i);
    }
}

int main(int argc, char *argv[]) {
    int value = 1;
    
    if (argc > 1) {
        value = atoi(argv[1]);
    }
    
    func1(value);
    func2(value);
    
    return 0;
}
EOF

# Compile with coverage instrumentation
echo "Compiling test program with coverage..."
gcc -fprofile-arcs -ftest-coverage test.c -o test_prog

# Create multiple .gcda files with different execution paths
echo "Generating coverage data with different execution paths..."

# First run: value = 1
echo "Run 1: value = 1"
./test_prog 1
mv test.gcda test_run1.gcda

# Second run: value = 3  
echo "Run 2: value = 3"
./test_prog 3
mv test.gcda test_run2.gcda

# Third run: value = 0 (different path through func1)
echo "Run 3: value = 0"
./test_prog 0
mv test.gcda test_run3.gcda

# Fourth run: value = 5 (more loop iterations)
echo "Run 4: value = 5"
./test_prog 5
mv test.gcda test_run4.gcda

echo "Generated .gcda files:"
ls -la *.gcda

# Test 1: Execute gcov-tool overlap with ALL target flags to cover all cases
# This covers cases: 'v', 'f', 'F', 'o', 'h', 't'
echo -e "\n=== Test 1: Testing all target flags ==="
echo "Command: gcov-tool overlap -v -f -F -o -h -t 0.5 test_run1.gcda test_run2.gcda test_run3.gcda test_run4.gcda"
if gcov-tool overlap -v -f -F -o -h -t 0.5 test_run1.gcda test_run2.gcda test_run3.gcda test_run4.gcda 2>&1; then
    echo "✓ Successfully executed with all target flags"
else
    echo "✗ Failed to execute with all target flags"
fi

# Test 2: Test with just verbose and threshold flags
echo -e "\n=== Test 2: Testing verbose and threshold flags ==="
echo "Command: gcov-tool overlap -v -t 1.0 test_run1.gcda test_run2.gcda"
if gcov-tool overlap -v -t 1.0 test_run1.gcda test_run2.gcda 2>&1; then
    echo "✓ Successfully executed with -v and -t flags"
else
    echo "✗ Failed to execute with -v and -t flags"
fi

# Test 3: Test with function-level and object-level flags
echo -e "\n=== Test 3: Testing function and object level flags ==="
echo "Command: gcov-tool overlap -f -o test_run1.gcda test_run2.gcda test_run3.gcda"
if gcov-tool overlap -f -o test_run1.gcda test_run2.gcda test_run3.gcda 2>&1; then
    echo "✓ Successfully executed with -f and -o flags"
else
    echo "✗ Failed to execute with -f and -o flags"
fi

# Test 4: Test with fullname flag
echo -e "\n=== Test 4: Testing fullname flag ==="
echo "Command: gcov-tool overlap -F test_run1.gcda test_run2.gcda"
if gcov-tool overlap -F test_run1.gcda test_run2.gcda 2>&1; then
    echo "✓ Successfully executed with -F flag"
else
    echo "✗ Failed to execute with -F flag"
fi

# Test 5: Test with hot-only flag
echo -e "\n=== Test 5: Testing hot-only flag ==="
echo "Command: gcov-tool overlap -h -t 0.1 test_run1.gcda test_run2.gcda test_run3.gcda test_run4.gcda"
if gcov-tool overlap -h -t 0.1 test_run1.gcda test_run2.gcda test_run3.gcda test_run4.gcda 2>&1; then
    echo "✓ Successfully executed with -h flag"
else
    echo "✗ Failed to execute with -h flag"
fi

# Test 6: Trigger the default case with an invalid option
# This should call overlap_usage() and exit
echo -e "\n=== Test 6: Testing invalid option (to trigger default case) ==="
echo "Command: gcov-tool overlap -z test_run1.gcda 2>&1"
if ! gcov-tool overlap -z test_run1.gcda 2>&1; then
    echo "✓ Successfully triggered default case with invalid option -z"
else
    echo "✗ Failed to trigger default case"
fi

# Test 7: Test with different threshold values
echo -e "\n=== Test 7: Testing various threshold values ==="
for threshold in 0.1 0.25 0.5 0.75 0.9; do
    echo "Testing with threshold: $threshold"
    if gcov-tool overlap -t $threshold test_run1.gcda test_run2.gcda 2>&1 >/dev/null; then
        echo "  ✓ Threshold $threshold works"
    else
        echo "  ✗ Threshold $threshold failed"
    fi
done

# Test 8: Test combination of flags in different order
echo -e "\n=== Test 8: Testing flag combinations in different order ==="
echo "Command: gcov-tool overlap -t 0.3 -v -f -o test_run1.gcda test_run2.gcda"
if gcov-tool overlap -t 0.3 -v -f -o test_run1.gcda test_run2.gcda 2>&1 >/dev/null; then
    echo "✓ Successfully executed with flags in different order"
else
    echo "✗ Failed to execute with flags in different order"
fi

# Cleanup
echo -e "\n=== Cleaning up ==="
cd ..
rm -rf "$TEST_DIR"
echo "Test directory cleaned up"

echo -e "\n=== Test Summary ==="
echo "All tests completed. The following cases from lines 534-554 were tested:"
echo "1. case 'v': verbose mode"
echo "2. case 'f': function-level overlap"
echo "3. case 'F': use full filenames"
echo "4. case 'o': object-level reporting"
echo "5. case 'h': hot-only filtering"
echo "6. case 't': hotness threshold with argument"
echo "7. default: invalid option handling"
```

This script comprehensively tests all the uncovered lines:

1. **Generates valid .gcda files**: Creates a simple C program, compiles it with coverage flags, and runs it multiple times with different inputs to generate varied coverage data.

2. **Tests all target flags**: The main test (`Test 1`) uses all the flags in a single command: `-v -f -F -o -h -t 0.5`, which exercises all the case statements in the switch block.

3. **Tests individual flags**: Additional tests exercise different combinations of flags to ensure each case is properly handled.

4. **Triggers the default case**: `Test 6` uses an invalid option `-z` to trigger the default branch that calls `overlap_usage()`.

5. **Tests threshold with different values**: Exercises the `-t` flag with various numeric arguments.

6. **Tests flag ordering**: Ensures flags work correctly in different orders.

The script creates a self-contained test environment in a temporary directory, generates the necessary coverage data files, runs all the test cases, and cleans up after itself. Each test case is clearly labeled and reports success/failure.
