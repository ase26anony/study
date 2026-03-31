Here's a shell script that meets all the requirements to test the uncovered lines in `gcov-dump.cc`:

```bash
#!/bin/bash

# Test script for gcov-dump.cc uncovered lines (111-130)
# Tests all switch cases: -h, -v, -l, -p, -r, -s, and default case

set -e  # Exit on any command failure

echo "=== Starting gcov-dump test ==="

# Create a temporary directory for our test
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
echo "Created test directory: $TEST_DIR"
cd "$TEST_DIR"

# Step 1: Create a minimal C test program
cat > test.c << 'EOF'
#include <stdio.h>

int main() {
    printf("Hello from test program\n");
    return 0;
}
EOF

echo "Created test.c"

# Step 2: Compile with coverage instrumentation
echo "Compiling with coverage instrumentation..."
if ! gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test.exe; then
    echo "ERROR: Failed to compile test program"
    exit 1
fi

echo "Compilation successful"

# Step 3: Run the program to generate .gcda file
echo "Running test program to generate profile data..."
./test.exe
echo "Generated test.gcda file"

# Verify the .gcda file exists
if [ ! -f "test.gcda" ]; then
    echo "ERROR: test.gcda file was not created"
    exit 1
fi

echo "=== Testing gcov-dump with various flags ==="

# Step 4: Test each switch case individually

# Case 'h': Print usage
echo -e "\n1. Testing -h flag (print usage):"
gcov-dump -h 2>&1 | head -5

# Case 'v': Print version
echo -e "\n2. Testing -v flag (print version):"
gcov-dump -v 2>&1

# Case 'l': Dump contents
echo -e "\n3. Testing -l flag (dump contents):"
gcov-dump -l test.gcda 2>&1 | head -10

# Case 'p': Dump positions
echo -e "\n4. Testing -p flag (dump positions):"
gcov-dump -p test.gcda 2>&1 | head -10

# Case 'r': Dump raw
echo -e "\n5. Testing -r flag (dump raw):"
gcov-dump -r test.gcda 2>&1 | head -10

# Case 's': Dump stable
echo -e "\n6. Testing -s flag (dump stable):"
gcov-dump -s test.gcda 2>&1 | head -10

# Test multiple flags combined
echo -e "\n7. Testing combined flags (-l -p -s):"
gcov-dump -l -p -s test.gcda 2>&1 | head -10

# Default case: Invalid flag
echo -e "\n8. Testing invalid flag -x (should trigger default case):"
gcov-dump -x test.gcda 2>&1 || true  # Don't fail on expected error

# Test another invalid flag
echo -e "\n9. Testing invalid flag -Z (should trigger default case):"
gcov-dump -Z test.gcda 2>&1 || true  # Don't fail on expected error

# Test with no arguments (should show usage or error)
echo -e "\n10. Testing with no arguments:"
gcov-dump 2>&1 | head -5 || true

# Cleanup
echo -e "\n=== Cleaning up ==="
cd ..
rm -rf "$TEST_DIR"
echo "Removed test directory: $TEST_DIR"

echo -e "\n=== TEST COMPLETE ==="
echo "All switch cases in gcov-dump.cc lines 111-130 have been exercised:"
echo "  ✓ -h (usage)"
echo "  ✓ -v (version)"
echo "  ✓ -l (dump contents)"
echo "  ✓ -p (dump positions)"
echo "  ✓ -r (dump raw)"
echo "  ✓ -s (dump stable)"
echo "  ✓ Combined flags (-l -p -s)"
echo "  ✓ Default case (invalid flags -x and -Z)"
echo ""
echo "SUCCESS: All targeted lines should now be covered!"
```

This script:

1. **Creates a temporary directory** for isolation
2. **Generates a minimal C program** that prints "Hello from test program"
3. **Compiles with coverage instrumentation** using `-fprofile-arcs -ftest-coverage`
4. **Runs the program** to generate the `.gcda` file
5. **Tests each switch case individually**:
   - `-h`: Prints usage information
   - `-v`: Prints version information
   - `-l test.gcda`: Dumps contents
   - `-p test.gcda`: Dumps positions
   - `-r test.gcda`: Dumps raw data
   - `-s test.gcda`: Dumps stable output
6. **Tests combined flags**: `-l -p -s test.gcda`
7. **Triggers the default case** with invalid flags `-x` and `-Z`
8. **Handles errors properly**: Uses `|| true` for expected errors from invalid flags
9. **Cleans up** the temporary directory
10. **Provides clear output** showing which cases were tested

The script uses `head -10` on some outputs to keep the display concise while still showing that the commands executed successfully. The `2>&1` redirects stderr to stdout to capture all output.

To run this test:
1. Save it as `test_gcov_dump.sh`
2. Make it executable: `chmod +x test_gcov_dump.sh`
3. Run it: `./test_gcov_dump.sh`

This will exercise all the uncovered lines in the switch statement (lines 111-130) of `gcov-dump.cc`.
