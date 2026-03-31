I'll create a comprehensive test script that covers all the requirements for testing the uncovered lines in `gcov-dump.cc`. This script will generate a simple instrumented program, run it to create GCOV data files, and then test all the flag combinations.

```bash
#!/bin/bash

# Test script for gcov-dump.cc uncovered lines (111-130)
# This script tests the command-line argument parsing logic

set -e

echo "=== Setting up test environment ==="

# Create a simple C program that will generate GCOV data
cat > test_program.c << 'EOF'
#include <stdio.h>

int main() {
    int i, sum = 0;
    
    // Simple loop to generate some execution paths
    for (i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            sum += i;  // Even numbers
        } else {
            sum -= i;  // Odd numbers
        }
    }
    
    printf("Result: %d\n", sum);
    return 0;
}
EOF

# Create a header file to ensure we have .gcno file
cat > test_header.h << 'EOF'
#ifndef TEST_HEADER_H
#define TEST_HEADER_H

#define TEST_VALUE 42

#endif
EOF

# Update test program to include header
cat > test.c << 'EOF'
#include <stdio.h>
#include "test_header.h"

int helper_function(int x) {
    return x * TEST_VALUE;
}

int main() {
    int i, sum = 0;
    
    // Simple loop to generate some execution paths
    for (i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            sum += helper_function(i);  // Even numbers
        } else {
            sum -= i;  // Odd numbers
        }
    }
    
    printf("Result: %d\n", sum);
    return 0;
}
EOF

echo "=== Compiling test program with GCOV instrumentation ==="
# Compile with GCOV instrumentation
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog

echo "=== Running test program to generate .gcda file ==="
# Run the program to generate coverage data
./test_prog

echo "=== Verifying GCOV files were created ==="
# Check that the expected files exist
if [ -f "test.gcda" ]; then
    echo "✓ test.gcda created successfully"
else
    echo "✗ test.gcda not found!"
    exit 1
fi

if [ -f "test.gcno" ]; then
    echo "✓ test.gcno created successfully"
else
    echo "✗ test.gcno not found!"
    exit 1
fi

echo -e "\n=== Testing individual flags (primary target) ==="

echo "1. Testing -l flag (dump contents):"
gcov-dump -l test.gcda 2>&1 | head -20
echo "---"

echo "2. Testing -p flag (dump positions):"
gcov-dump -p test.gcda 2>&1 | head -20
echo "---"

echo "3. Testing -r flag (dump raw):"
gcov-dump -r test.gcda 2>&1 | head -20
echo "---"

echo "4. Testing -s flag (dump stable):"
gcov-dump -s test.gcda 2>&1 | head -20
echo "---"

echo -e "\n=== Testing flag combinations ==="

echo "5. Testing -l -p combination:"
gcov-dump -l -p test.gcda 2>&1 | head -20
echo "---"

echo "6. Testing -r -s combination:"
gcov-dump -r -s test.gcda 2>&1 | head -20
echo "---"

echo "7. Testing all flags together:"
gcov-dump -l -p -r -s test.gcda 2>&1 | head -20
echo "---"

echo -e "\n=== Testing with .gcno files ==="
echo "8. Testing -l with .gcno file:"
gcov-dump -l test.gcno 2>&1 | head -20
echo "---"

echo "9. Testing -p with .gcno file:"
gcov-dump -p test.gcno 2>&1 | head -20
echo "---"

echo -e "\n=== Testing help and version flags ==="
echo "10. Testing -h flag (help):"
gcov-dump -h 2>&1 | head -10
echo "---"

echo "11. Testing -v flag (version):"
gcov-dump -v 2>&1
echo "---"

echo -e "\n=== Testing invalid flags (to trigger default case) ==="
echo "12. Testing invalid flag -x:"
gcov-dump -x test.gcda 2>&1 || true
echo "---"

echo "13. Testing invalid flag -?:"
gcov-dump -? test.gcda 2>&1 || true
echo "---"

echo "14. Testing invalid uppercase flag -L:"
gcov-dump -L test.gcda 2>&1 || true
echo "---"

echo "15. Testing invalid flag -z:"
gcov-dump -z test.gcda 2>&1 || true
echo "---"

echo "16. Testing multiple flags with one invalid:"
gcov-dump -l -x -p test.gcda 2>&1 || true
echo "---"

echo -e "\n=== Testing edge cases ==="
echo "17. Testing no flags (should show basic info):"
gcov-dump test.gcda 2>&1 | head -10
echo "---"

echo "18. Testing with multiple files:"
gcov-dump -l test.gcda test.gcno 2>&1 | head -10
echo "---"

echo "19. Testing flag with no file argument:"
gcov-dump -l 2>&1 || true
echo "---"

echo -e "\n=== Testing with different file orders ==="
echo "20. Testing flag after filename:"
gcov-dump test.gcda -l 2>&1 | head -10 || true
echo "---"

echo -e "\n=== Creating additional test with more complex program ==="
# Create another test program with more branches
cat > test2.c << 'EOF'
#include <stdio.h>

void function1(int x) {
    if (x > 0) {
        printf("Positive\n");
    } else if (x < 0) {
        printf("Negative\n");
    } else {
        printf("Zero\n");
    }
}

void function2(int x) {
    switch (x % 3) {
        case 0: printf("Mod 0\n"); break;
        case 1: printf("Mod 1\n"); break;
        case 2: printf("Mod 2\n"); break;
    }
}

int main() {
    function1(5);
    function1(-3);
    function1(0);
    
    function2(7);
    function2(8);
    function2(9);
    
    return 0;
}
EOF

echo "=== Compiling and running second test program ==="
gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test2_prog
./test2_prog

echo "21. Testing with second .gcda file:"
gcov-dump -l -p test2.gcda 2>&1 | head -20
echo "---"

echo -e "\n=== Summary ==="
echo "All tests completed successfully!"
echo "The following cases were tested:"
echo "1. Individual flags: -l, -p, -r, -s"
echo "2. Flag combinations: -l -p, -r -s, -l -p -r -s"
echo "3. Help and version: -h, -v"
echo "4. Invalid flags: -x, -?, -L, -z"
echo "5. Edge cases: no flags, multiple files, flag after filename"
echo "6. Different file types: .gcda and .gcno"
echo ""
echo "To check coverage of the target lines in gcov-dump.cc:"
echo "1. Build gcov-dump with coverage: gcc -fprofile-arcs -ftest-coverage -O0 gcov-dump.cc -o gcov-dump"
echo "2. Run this test script"
echo "3. Run: gcov gcov-dump.cc"
echo "4. Check that lines 111-130 are covered in the .gcov output"
```

This comprehensive test script:

1. **Creates test programs** with sufficient complexity to generate meaningful GCOV data
2. **Tests each individual flag** (`-l`, `-p`, `-r`, `-s`) in isolation
3. **Tests flag combinations** as specified
4. **Tests invalid flags** to trigger the `default:` case
5. **Tests help and version flags** (`-h`, `-v`)
6. **Uses both .gcda and .gcno files** as input
7. **Tests edge cases** like no flags, multiple files, etc.
8. **Handles errors gracefully** with `|| true` to continue testing
9. **Provides clear output** showing what each test is doing

To use this script:

1. Save it as `test_gcov_dump.sh`
2. Make it executable: `chmod +x test_gcov_dump.sh`
3. Ensure `gcov-dump` is in your PATH or modify the script to use the full path
4. Run it: `./test_gcov_dump.sh`

After running, you can check coverage of the `gcov-dump` binary itself by building it with instrumentation and then running `gcov gcov-dump.cc` to see if lines 111-130 are covered.
