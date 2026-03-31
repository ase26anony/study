Here's a shell script that implements the test requirements to cover the uncovered lines in `gcov-dump.cc`:

```bash
#!/bin/bash

# Test script for gcov-dump to cover the option parsing switch statement
# Creates a temporary directory, generates a GCOV data file, and tests all flags

set -e  # Exit on any command failure

# Create and move to temporary directory
TEMP_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEMP_DIR"
echo "Working in temporary directory: $TEMP_DIR"

# Create a simple C test program
cat > test.c << 'EOF'
#include <stdio.h>

int main() {
    printf("Hello from test program for gcov-dump\n");
    return 0;
}
EOF

# Compile with coverage instrumentation
echo "Compiling test program with coverage instrumentation..."
if ! gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test.exe; then
    echo "ERROR: Failed to compile test program"
    exit 1
fi

# Run the program to generate .gcda file
echo "Running test program to generate GCOV data..."
./test.exe

# Verify .gcda file was created
if [ ! -f test.gcda ]; then
    echo "ERROR: test.gcda file was not created"
    exit 1
fi

echo "GCOV data file created successfully: test.gcda"
echo ""

# Test 1: -h flag (help/usage)
echo "=== Test 1: -h flag (help/usage) ==="
gcov-dump -h 2>&1 | head -5
echo ""

# Test 2: -v flag (version)
echo "=== Test 2: -v flag (version) ==="
gcov-dump -v 2>&1
echo ""

# Test 3: -l flag (dump contents)
echo "=== Test 3: -l flag (dump contents) ==="
gcov-dump -l test.gcda 2>&1 | head -10
echo ""

# Test 4: -p flag (dump positions)
echo "=== Test 4: -p flag (dump positions) ==="
gcov-dump -p test.gcda 2>&1 | head -10
echo ""

# Test 5: -r flag (dump raw)
echo "=== Test 5: -r flag (dump raw) ==="
gcov-dump -r test.gcda 2>&1 | head -10
echo ""

# Test 6: -s flag (dump stable)
echo "=== Test 6: -s flag (dump stable) ==="
gcov-dump -s test.gcda 2>&1 | head -10
echo ""

# Test 7: Combined flags (-l -p -s)
echo "=== Test 7: Combined flags (-l -p -s) ==="
gcov-dump -l -p -s test.gcda 2>&1 | head -10
echo ""

# Test 8: Invalid flag (triggers default case)
echo "=== Test 8: Invalid flag -x (triggers default case) ==="
gcov-dump -x test.gcda 2>&1 || true  # Don't fail on expected error
echo ""

# Test 9: Another invalid flag
echo "=== Test 9: Invalid flag -Z (triggers default case) ==="
gcov-dump -Z test.gcda 2>&1 || true
echo ""

# Test 10: Test with no arguments (should show usage or error)
echo "=== Test 10: No arguments ==="
gcov-dump 2>&1 | head -5 || true
echo ""

# Cleanup
echo "Cleaning up..."
cd ..
rm -rf "$TEMP_DIR"

echo ""
echo "========================================"
echo "SUCCESS: All gcov-dump tests completed!"
echo "Covered all switch cases:"
echo "  - -h (help)"
echo "  - -v (version)"
echo "  - -l (dump contents)"
echo "  - -p (dump positions)"
echo "  - -r (dump raw)"
echo "  - -s (dump stable)"
echo "  - default case (invalid flags -x, -Z)"
echo "  - combined flags (-l -p -s)"
echo "========================================"
```

This script:

1. **Creates a temporary directory** to avoid polluting the current directory
2. **Generates a simple C program** that prints a message
3. **Compiles with coverage instrumentation** using `-fprofile-arcs -ftest-coverage`
4. **Runs the program** to generate the `.gcda` file
5. **Tests each individual flag** (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`)
6. **Tests combined flags** (`-l -p -s`)
7. **Tests invalid flags** (`-x`, `-Z`) to trigger the `default` case
8. **Tests with no arguments** for completeness
9. **Cleans up** the temporary directory
10. **Prints a success message** confirming all cases were covered

The script uses `head` to limit output for readability, but you can remove those pipes if you want to see full output. The `|| true` after invalid flag tests ensures the script doesn't fail when `gcov-dump` correctly returns an error for invalid options.

To run this test:
1. Save it as `test_gcov_dump.sh`
2. Make it executable: `chmod +x test_gcov_dump.sh`
3. Run it: `./test_gcov_dump.sh`

This will exercise all the uncovered lines in the switch statement, including each case and the default branch.
