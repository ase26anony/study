Looking at the uncovered lines in `gcov-dump.cc`, I need to create a comprehensive test that exercises all the command-line flag handling logic. Here's a shell script that will generate the necessary GCOV data files and test all the uncovered switch cases:

```bash
#!/bin/bash
set -e

# Create a temporary directory for test artifacts
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    cd /
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

echo "Test directory: $TEST_DIR"

# Create a simple C source file that will generate coverage data
cat > test1.c << 'EOF'
#include <stdio.h>

int main() {
    printf("Hello from test1\n");
    return 0;
}
EOF

cat > test2.c << 'EOF'
#include <stdio.h>

int helper() {
    return 42;
}

int main() {
    printf("Hello from test2, helper returned: %d\n", helper());
    return 0;
}
EOF

# Compile with coverage instrumentation
echo "Compiling test programs with coverage..."
gcc -fprofile-arcs -ftest-coverage -O0 test1.c -o test1_prog
gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test2_prog

# Run the programs to generate .gcda files
echo "Running test programs to generate coverage data..."
./test1_prog
./test2_prog

# Verify that the coverage files were created
echo "Generated coverage files:"
ls -la *.gcno *.gcda

# Test 1: Help flag (-h)
echo -e "\n=== Testing -h flag ==="
gcov-dump -h 2>&1 | head -5
echo "Exit code: $?"

# Test 2: Help flag (--help)
echo -e "\n=== Testing --help flag ==="
gcov-dump --help 2>&1 | head -5
echo "Exit code: $?"

# Test 3: Version flag (-v)
echo -e "\n=== Testing -v flag ==="
gcov-dump -v 2>&1
echo "Exit code: $?"

# Test 4: Version flag (--version)
echo -e "\n=== Testing --version flag ==="
gcov-dump --version 2>&1
echo "Exit code: $?"

# Test 5: List contents flag (-l)
echo -e "\n=== Testing -l flag ==="
gcov-dump -l test1.gcda 2>&1 | head -10
echo "Exit code: $?"

# Test 6: Dump positions flag (-p)
echo -e "\n=== Testing -p flag ==="
gcov-dump -p test1.gcda 2>&1 | head -10
echo "Exit code: $?"

# Test 7: Raw dump flag (-r)
echo -e "\n=== Testing -r flag ==="
gcov-dump -r test1.gcda 2>&1 | head -10
echo "Exit code: $?"

# Test 8: Stable dump flag (-s)
echo -e "\n=== Testing -s flag ==="
gcov-dump -s test1.gcda 2>&1 | head -10
echo "Exit code: $?"

# Test 9: Multiple flags combined
echo -e "\n=== Testing multiple flags (-l -p -r -s) ==="
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -15
echo "Exit code: $?"

# Test 10: Invalid flag (triggers default case)
echo -e "\n=== Testing invalid flag (-x) ==="
gcov-dump -x test1.gcda 2>&1
echo "Exit code: $?"

# Test 11: No flags, just .gcda file
echo -e "\n=== Testing with no flags (.gcda file) ==="
gcov-dump test1.gcda 2>&1 | head -10
echo "Exit code: $?"

# Test 12: No flags, just .gcno file
echo -e "\n=== Testing with no flags (.gcno file) ==="
gcov-dump test1.gcno 2>&1 | head -10
echo "Exit code: $?"

# Test 13: Multiple input files with flag
echo -e "\n=== Testing multiple input files (-l flag) ==="
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -15
echo "Exit code: $?"

# Test 14: Test with .gcno and .gcda together
echo -e "\n=== Testing .gcno and .gcda together ==="
gcov-dump -l test1.gcno test1.gcda 2>&1 | head -15
echo "Exit code: $?"

# Test 15: Long form flags with files
echo -e "\n=== Testing long flags with files ==="
gcov-dump --help test1.gcda 2>&1 | head -5
echo "Exit code: $?"

# Test 16: Empty arguments (should show usage)
echo -e "\n=== Testing with no arguments ==="
gcov-dump 2>&1 | head -5
echo "Exit code: $?"

# Test 17: Flag with no file argument
echo -e "\n=== Testing -l with no file ==="
gcov-dump -l 2>&1 | head -5
echo "Exit code: $?"

echo -e "\n=== All tests completed ==="
```

This test script comprehensively covers all the uncovered lines:

1. **Lines 111-112 (`case 'h'`):** Tested with `-h` and `--help` flags
2. **Lines 113-114 (`case 'v'`):** Tested with `-v` and `--version` flags  
3. **Lines 115-116 (`case 'l'`):** Tested with `-l` flag in multiple contexts
4. **Lines 117-118 (`case 'p'`):** Tested with `-p` flag
5. **Lines 119-120 (`case 'r'`):** Tested with `-r` flag
6. **Lines 121-122 (`case 's'`):** Tested with `-s` flag
7. **Lines 123-124 (`default` case):** Tested with invalid `-x` flag

The script also tests various combinations and edge cases:
- Multiple flags combined (`-l -p -r -s`)
- Different types of input files (`.gcda`, `.gcno`, both)
- Multiple input files in one command
- Edge cases like no arguments or flags without files

To run this test, you need to:
1. Ensure `gcov-dump` is in your PATH or provide the full path
2. Have GCC installed with coverage support
3. Make the script executable: `chmod +x test_gcov_dump.sh`
4. Run it: `./test_gcov_dump.sh`

The script creates a temporary directory, generates test coverage data, runs all the test cases, and cleans up automatically. Each test invocation will trigger the corresponding switch case in the `gcov-dump` command-line parsing logic, ensuring coverage of the previously uncovered lines.
