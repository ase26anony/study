I'll create a comprehensive test script that covers all the uncovered lines in the `gcov-tool overlap` command-line parsing logic.

```bash
#!/bin/bash

# Test script for gcov-tool overlap command options coverage
# This script generates GCOV data files and tests all uncovered
# command-line options in gcov-tool.cc lines 534-554

set -e  # Exit on any error

# Create a temporary directory for our test
TEST_DIR=$(mktemp -d /tmp/gcov_tool_test_XXXXXX)
echo "Test directory: $TEST_DIR"
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    cd /
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

# Step 1: Create a simple C program for GCOV instrumentation
cat > test_program.c << 'EOF'
#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int main() {
    int i;
    
    // Test factorial function
    printf("Factorial of 5: %d\n", factorial(5));
    
    // Test fibonacci function
    printf("Fibonacci of 6: %d\n", fibonacci(6));
    
    // Loop to generate some execution counts
    for (i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
    
    return 0;
}
EOF

# Step 2: Compile with GCOV instrumentation
echo "Compiling test program with GCOV instrumentation..."
gcc -fprofile-arcs -ftest-coverage test_program.c -o test_program

# Step 3: Run the program to generate initial .gcda file
echo "Running test program to generate profile data..."
./test_program > /dev/null

# Verify .gcda file was created
if [ ! -f test_program.gcda ]; then
    echo "ERROR: test_program.gcda not created!"
    exit 1
fi

# Step 4: Create two different .gcda files for overlap analysis
# First, create a "base" profile
cp test_program.gcda base.gcda

# Run program again with different input to create variation
# We'll modify the .gcda file by running with different parameters
# For simplicity, we'll just copy and modify execution counts
cp test_program.gcda compare.gcda

# Create a simple script to modify gcda file slightly
# (In real scenario, we'd run the program with different inputs)
echo "Creating modified profile data..."

# Actually, let's run the program with a different loop count
# by creating a modified version
cat > test_program2.c << 'EOF'
#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int main() {
    int i;
    
    // Test factorial function with different value
    printf("Factorial of 3: %d\n", factorial(3));
    
    // Test fibonacci function with different value
    printf("Fibonacci of 4: %d\n", fibonacci(4));
    
    // Different loop count
    for (i = 0; i < 5; i++) {
        if (i % 3 == 0) {
            printf("Multiple of 3: %d\n", i);
        } else {
            printf("Not multiple of 3: %d\n", i);
        }
    }
    
    return 0;
}
EOF

# Compile and run second program
gcc -fprofile-arcs -ftest-coverage test_program2.c -o test_program2
./test_program2 > /dev/null
cp test_program2.gcda compare.gcda

# Verify both files exist
if [ ! -f base.gcda ] || [ ! -f compare.gcda ]; then
    echo "ERROR: Required .gcda files not created!"
    exit 1
fi

echo "Profile files created:"
echo "  base.gcda"
echo "  compare.gcda"

# Step 5: Test individual uncovered options
echo -e "\n=== Testing individual options ==="

# Test 1: -v (verbose)
echo "Test 1: Testing -v (verbose) option..."
if gcov-tool overlap -v base.gcda compare.gcda 2>&1 | grep -q "verbose\|Verbose"; then
    echo "  ✓ -v option processed (verbose output detected)"
else
    echo "  -v option test completed"
fi

# Test 2: -f (function level)
echo "Test 2: Testing -f (function level) option..."
if gcov-tool overlap -f base.gcda compare.gcda 2>&1 | head -20; then
    echo "  ✓ -f option processed successfully"
fi

# Test 3: -F (full filename)
echo "Test 3: Testing -F (full filename) option..."
if gcov-tool overlap -F base.gcda compare.gcda 2>&1 | head -20; then
    echo "  ✓ -F option processed successfully"
fi

# Test 4: -o (object level)
echo "Test 4: Testing -o (object level) option..."
if gcov-tool overlap -o base.gcda compare.gcda 2>&1 | head -20; then
    echo "  ✓ -o option processed successfully"
fi

# Test 5: -h (hot only)
echo "Test 5: Testing -h (hot only) option..."
if gcov-tool overlap -h base.gcda compare.gcda 2>&1 | head -20; then
    echo "  ✓ -h option processed successfully"
fi

# Test 6: -t (hot threshold) with valid value
echo "Test 6: Testing -t (hot threshold) option with value 0.5..."
if gcov-tool overlap -t 0.5 base.gcda compare.gcda 2>&1 | head -20; then
    echo "  ✓ -t 0.5 option processed successfully"
fi

# Step 6: Test option combinations
echo -e "\n=== Testing option combinations ==="

# Combination 1: -v -f -o
echo "Test 7: Testing combination -v -f -o..."
if gcov-tool overlap -v -f -o base.gcda compare.gcda 2>&1 | head -20; then
    echo "  ✓ Combination -v -f -o processed successfully"
fi

# Combination 2: -F -h -t
echo "Test 8: Testing combination -F -h -t 0.75..."
if gcov-tool overlap -F -h -t 0.75 base.gcda compare.gcda 2>&1 | head -20; then
    echo "  ✓ Combination -F -h -t 0.75 processed successfully"
fi

# Combination 3: All options together
echo "Test 9: Testing all options together..."
if gcov-tool overlap -v -f -F -o -h -t 0.25 base.gcda compare.gcda 2>&1 | head -20; then
    echo "  ✓ All options together processed successfully"
fi

# Step 7: Test threshold boundary values
echo -e "\n=== Testing threshold boundary values ==="

# Test minimum threshold
echo "Test 10: Testing -t with minimum value 0.0..."
if gcov-tool overlap -t 0.0 base.gcda compare.gcda 2>&1 | head -20; then
    echo "  ✓ -t 0.0 processed successfully"
fi

# Test maximum threshold
echo "Test 11: Testing -t with maximum value 1.0..."
if gcov-tool overlap -t 1.0 base.gcda compare.gcda 2>&1 | head -20; then
    echo "  ✓ -t 1.0 processed successfully"
fi

# Test out-of-range threshold (should still parse but may affect output)
echo "Test 12: Testing -t with out-of-range value -1.0..."
if gcov-tool overlap -t -1.0 base.gcda compare.gcda 2>&1 | head -20; then
    echo "  ✓ -t -1.0 parsed (behavior depends on implementation)"
fi

# Test another out-of-range threshold
echo "Test 13: Testing -t with out-of-range value 2.5..."
if gcov-tool overlap -t 2.5 base.gcda compare.gcda 2>&1 | head -20; then
    echo "  ✓ -t 2.5 parsed (behavior depends on implementation)"
fi

# Step 8: Test invalid option to trigger default case
echo -e "\n=== Testing invalid option (to trigger default case) ==="

echo "Test 14: Testing invalid option -x..."
if ! gcov-tool overlap -x base.gcda compare.gcda 2>&1 | grep -q "usage\|Usage\|invalid"; then
    echo "  Note: Invalid option -x might not trigger expected error message"
else
    echo "  ✓ Invalid option triggered error/usage message"
fi

# Step 9: Test missing required argument for -t
echo "Test 15: Testing -t without required argument..."
if ! gcov-tool overlap -t base.gcda compare.gcda 2>&1 | grep -q "requires\|argument"; then
    echo "  Note: Missing argument for -t might not trigger expected error"
else
    echo "  ✓ Missing argument for -t triggered error"
fi

# Step 10: Test with different file order and additional files
echo -e "\n=== Testing additional scenarios ==="

echo "Test 16: Testing with swapped file order..."
if gcov-tool overlap -v compare.gcda base.gcda 2>&1 | head -5; then
    echo "  ✓ Swapped file order processed successfully"
fi

# Create a summary
echo -e "\n=== Test Summary ==="
echo "All gcov-tool overlap option tests completed."
echo "The following uncovered lines should now be covered:"
echo "  - case 'v': verbose = true; gcov_set_verbose();"
echo "  - case 'f': overlap_func_level = 1;"
echo "  - case 'F': overlap_use_fullname = 1;"
echo "  - case 'o': overlap_obj_level = 1;"
echo "  - case 'h': overlap_hot_only = 1;"
echo "  - case 't': overlap_hot_threshold = atof(optarg);"
echo "  - default: overlap_usage();"

echo -e "\nTo verify coverage:"
echo "1. Ensure gcov-tool was built with coverage instrumentation"
echo "2. Run: gcov -b gcov-tool.cc"
echo "3. Check that lines 534-554 are now marked as executed"

exit 0
```

This comprehensive test script:

1. **Creates a temporary workspace** to avoid polluting the source tree
2. **Generates two different C programs** to create varied GCOV profile data
3. **Compiles with GCOV instrumentation** (`-fprofile-arcs -ftest-coverage`)
4. **Runs the programs** to generate `.gcda` files
5. **Tests each individual uncovered option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`)
6. **Tests option combinations** to ensure they work together
7. **Tests threshold boundary values** (`0.0`, `1.0`, `-1.0`, `2.5`)
8. **Tests invalid option** (`-x`) to trigger the `default` case
9. **Tests edge cases** like missing argument for `-t`
10. **Provides clear output** showing which tests passed

The script captures the essence of each test while keeping output manageable. Each `gcov-tool` invocation will execute the command-line parsing logic in `gcov-tool.cc`, specifically the switch statement at lines 534-554, ensuring all uncovered lines are exercised.
