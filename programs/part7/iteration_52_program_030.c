Here's a comprehensive test script that covers all the uncovered lines in the `gcov-dump.cc` switch statement:

```bash
#!/bin/bash
set -e

# Create a temporary directory for test artifacts
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test-XXXXXX)
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    cd /
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

# Create a simple C test program
cat > test1.c << 'EOF'
#include <stdio.h>

int main() {
    printf("Test program 1\n");
    return 0;
}
EOF

cat > test2.c << 'EOF'
#include <stdio.h>

int helper() {
    return 42;
}

int main() {
    printf("Test program 2: %d\n", helper());
    return 0;
}
EOF

# Compile test programs with coverage instrumentation
echo "Compiling test programs with coverage..."
gcc -fprofile-arcs -ftest-coverage -O0 test1.c -o test1_prog
gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test2_prog

# Run programs to generate .gcda files
echo "Running test programs to generate coverage data..."
./test1_prog > /dev/null
./test2_prog > /dev/null

# Verify that .gcno and .gcda files exist
if [ ! -f test1.gcno ] || [ ! -f test1.gcda ]; then
    echo "ERROR: test1 coverage files not generated"
    exit 1
fi

if [ ! -f test2.gcno ] || [ ! -f test2.gcda ]; then
    echo "ERROR: test2 coverage files not generated"
    exit 1
fi

echo "Generated coverage files:"
ls -la *.gcno *.gcda

# Test 1: Help flag (-h)
echo -e "\n=== Test 1: Help flag (-h) ==="
gcov-dump -h 2>&1 | head -5
gcov-dump --help 2>&1 | head -5

# Test 2: Version flag (-v)
echo -e "\n=== Test 2: Version flag (-v) ==="
gcov-dump -v
gcov-dump --version

# Test 3: Dump contents flag (-l)
echo -e "\n=== Test 3: Dump contents flag (-l) ==="
gcov-dump -l test1.gcda 2>&1 | head -10

# Test 4: Dump positions flag (-p)
echo -e "\n=== Test 4: Dump positions flag (-p) ==="
gcov-dump -p test1.gcda 2>&1 | head -10

# Test 5: Dump raw flag (-r)
echo -e "\n=== Test 5: Dump raw flag (-r) ==="
gcov-dump -r test1.gcda 2>&1 | head -10

# Test 6: Dump stable flag (-s)
echo -e "\n=== Test 6: Dump stable flag (-s) ==="
gcov-dump -s test1.gcda 2>&1 | head -10

# Test 7: Multiple flags combined
echo -e "\n=== Test 7: Multiple flags combined (-l -p -r -s) ==="
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -15

# Test 8: No flags (default behavior)
echo -e "\n=== Test 8: No flags (default behavior) ==="
gcov-dump test1.gcno 2>&1 | head -10

# Test 9: Multiple input files with flag
echo -e "\n=== Test 9: Multiple input files with flag ==="
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -15

# Test 10: .gcno file with flag
echo -e "\n=== Test 10: .gcno file with flag ==="
gcov-dump -p test1.gcno 2>&1 | head -10

# Test 11: Invalid flag (triggers default case)
echo -e "\n=== Test 11: Invalid flag (-x) triggers default case ==="
if gcov-dump -x test1.gcda 2>&1 | grep -q "unknown flag"; then
    echo "SUCCESS: Invalid flag correctly detected"
else
    echo "WARNING: Invalid flag error message not found"
fi

# Test 12: Multiple invalid flags
echo -e "\n=== Test 12: Multiple invalid flags ==="
if gcov-dump -x -y -z test1.gcda 2>&1 | grep -q "unknown flag"; then
    echo "SUCCESS: Multiple invalid flags detected"
fi

# Test 13: Mixed valid and invalid flags
echo -e "\n=== Test 13: Mixed valid and invalid flags ==="
if gcov-dump -l -x -p test1.gcda 2>&1 | grep -q "unknown flag"; then
    echo "SUCCESS: Mixed flags with invalid detected"
fi

# Test 14: Flag with no argument (should process next file)
echo -e "\n=== Test 14: Flag with no argument ==="
gcov-dump -l -p test1.gcda test2.gcno 2>&1 | head -5

# Test 15: Long options that map to short flags
echo -e "\n=== Test 15: Testing various flag combinations ==="
# Test each flag individually
for flag in l p r s; do
    echo "Testing -$flag flag:"
    gcov-dump -$flag test1.gcda 2>&1 | head -3
done

echo -e "\n=== All tests completed successfully ==="
```

This test script comprehensively covers all the uncovered lines:

1. **Lines 111-112 (`case 'h'`):** Covered by `gcov-dump -h` and `gcov-dump --help`
2. **Lines 113-114 (`case 'v'`):** Covered by `gcov-dump -v` and `gcov-dump --version`
3. **Lines 115-116 (`case 'l'`):** Covered by `gcov-dump -l test1.gcda` and multiple other tests
4. **Lines 117-118 (`case 'p'`):** Covered by `gcov-dump -p test1.gcda` and multiple other tests
5. **Lines 119-120 (`case 'r'`):** Covered by `gcov-dump -r test1.gcda` and multiple other tests
6. **Lines 121-122 (`case 's'`):** Covered by `gcov-dump -s test1.gcda` and multiple other tests
7. **Lines 123-125 (`default` case):** Covered by `gcov-dump -x test1.gcda` and other invalid flag tests

The script also tests:
- Multiple flag combinations (`-l -p -r -s`)
- Different input file types (`.gcda` and `.gcno`)
- Multiple input files in one command
- Edge cases like flags with no arguments
- Mixed valid and invalid flags

To compile and run the test, you need to:
1. Ensure `gcov-dump` is in your PATH (built from GCC source with coverage enabled)
2. Make the script executable: `chmod +x test_gcov_dump.sh`
3. Run it: `./test_gcov_dump.sh`

The script creates temporary test files, compiles them with coverage instrumentation, runs them to generate `.gcda` files, then exercises `gcov-dump` with all the flag combinations needed to cover the target lines.
