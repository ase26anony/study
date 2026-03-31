Here's a shell script that implements the requirements to test the uncovered lines in `gcov-dump.cc`:

```bash
#!/bin/bash

# Test script for gcov-dump to cover the switch statement in main()
# This tests all command-line flags including invalid ones

set -e  # Exit on any command failure

echo "=== Starting gcov-dump test ==="

# Create a temporary directory for our test
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test-XXXXXX)
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
echo "Running test program to generate coverage data..."
./test.exe
echo "Generated test.gcda file"

# Verify the .gcda file exists
if [ ! -f test.gcda ]; then
    echo "ERROR: test.gcda file was not created"
    exit 1
fi

echo "=== Testing gcov-dump with various flags ==="

# Step 4: Test each flag individually

# Test -h flag (help/usage)
echo "1. Testing -h flag (usage):"
gcov-dump -h 2>&1 | head -5
echo "---"

# Test -v flag (version)
echo "2. Testing -v flag (version):"
gcov-dump -v 2>&1
echo "---"

# Test -l flag (dump contents)
echo "3. Testing -l flag (dump contents):"
gcov-dump -l test.gcda 2>&1 | head -10
echo "---"

# Test -p flag (dump positions)
echo "4. Testing -p flag (dump positions):"
gcov-dump -p test.gcda 2>&1 | head -10
echo "---"

# Test -r flag (dump raw)
echo "5. Testing -r flag (dump raw):"
gcov-dump -r test.gcda 2>&1 | head -10
echo "---"

# Test -s flag (dump stable)
echo "6. Testing -s flag (dump stable):"
gcov-dump -s test.gcda 2>&1 | head -10
echo "---"

# Test combination of flags
echo "7. Testing combination of flags (-l -p -s):"
gcov-dump -l -p -s test.gcda 2>&1 | head -15
echo "---"

# Test invalid flag (triggers default case)
echo "8. Testing invalid flag -x (should trigger default case):"
gcov-dump -x test.gcda 2>&1 || true  # Don't fail on expected error
echo "---"

echo "9. Testing another invalid flag -Z:"
gcov-dump -Z test.gcda 2>&1 || true
echo "---"

# Additional test: No arguments (should show usage or error)
echo "10. Testing with no arguments:"
gcov-dump 2>&1 | head -5 || true
echo "---"

# Test with multiple files (if we create another .gcda)
echo "11. Testing with multiple .gcda files:"
# Create another test program
cat > test2.c << 'EOF'
#include <stdio.h>
void foo() { printf("foo\n"); }
int main() { foo(); return 0; }
EOF

gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test2.exe
./test2.exe
gcov-dump -l test.gcda test2.gcda 2>&1 | head -5
echo "---"

# Cleanup
echo "=== Cleaning up ==="
cd ..
rm -rf "$TEST_DIR"
echo "Removed test directory: $TEST_DIR"

echo ""
echo "=== TEST COMPLETE ==="
echo "All gcov-dump switch cases have been exercised:"
echo "  ✓ -h (help/usage)"
echo "  ✓ -v (version)"
echo "  ✓ -l (dump contents)"
echo "  ✓ -p (dump positions)"
echo "  ✓ -r (dump raw)"
echo "  ✓ -s (dump stable)"
echo "  ✓ Multiple flags combined"
echo "  ✓ Invalid flags (default case)"
echo ""
echo "SUCCESS: All required test cases executed!"
```

This script:

1. **Creates a temporary directory** for isolation
2. **Generates a minimal C program** that will produce GCOV data
3. **Compiles with coverage instrumentation** (`-fprofile-arcs -ftest-coverage`)
4. **Runs the program** to generate the `.gcda` file
5. **Tests each individual flag** (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`)
6. **Tests flag combinations** (`-l -p -s`)
7. **Tests invalid flags** (`-x`, `-Z`) to trigger the `default` case
8. **Includes additional tests** for edge cases
9. **Cleans up** the temporary directory

The script uses `2>&1` to capture both stdout and stderr, and `|| true` to prevent the script from failing when `gcov-dump` returns an error code (which is expected for invalid flags).

To run this test:
1. Save it as `test_gcov_dump.sh`
2. Make it executable: `chmod +x test_gcov_dump.sh`
3. Run it: `./test_gcov_dump.sh`

The script will exercise all the uncovered lines in the switch statement:
- Each `case` for valid flags
- The `default` case for invalid flags
- The `fprintf(stderr, "unknown flag...")` line
