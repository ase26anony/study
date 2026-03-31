Looking at the uncovered lines in `gcov-dump.cc`, I need to create a comprehensive test that exercises all the command-line flag handling logic. Here's a shell script that will generate the necessary GCOV data files and test all the uncovered cases:

```bash
#!/bin/bash
# Test script for gcov-dump command-line argument parsing

set -e  # Exit on any critical failure

# Create a temporary directory for test artifacts
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"

# Create a simple C source file that will generate GCOV data
cat > test1.c << 'EOF'
#include <stdio.h>

int main() {
    int x = 0;
    for (int i = 0; i < 10; i++) {
        x += i;
    }
    printf("Result: %d\n", x);
    return 0;
}
EOF

# Create another test file for multiple file testing
cat > test2.c << 'EOF'
#include <stdio.h>

void helper_function() {
    printf("Helper called\n");
}

int main() {
    helper_function();
    return 0;
}
EOF

# Compile with coverage instrumentation
echo "Compiling test programs with coverage..."
gcc -fprofile-arcs -ftest-coverage -O0 test1.c -o test1_prog
gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test2_prog

# Run the programs to generate .gcda files
echo "Running test programs to generate coverage data..."
./test1_prog > /dev/null
./test2_prog > /dev/null

# Verify that GCOV files were created
echo "Generated files:"
ls -la *.gcno *.gcda

# Test 1: Help flag (-h)
echo -e "\n=== Test 1: Help flag (-h) ==="
gcov-dump -h 2>&1 | head -5

# Test 2: Help flag (--help)
echo -e "\n=== Test 2: Help flag (--help) ==="
gcov-dump --help 2>&1 | head -5

# Test 3: Version flag (-v)
echo -e "\n=== Test 3: Version flag (-v) ==="
gcov-dump -v

# Test 4: Version flag (--version)
echo -e "\n=== Test 4: Version flag (--version) ==="
gcov-dump --version

# Test 5: Dump contents flag (-l)
echo -e "\n=== Test 5: Dump contents flag (-l) ==="
gcov-dump -l test1.gcda 2>&1 | head -10

# Test 6: Dump positions flag (-p)
echo -e "\n=== Test 6: Dump positions flag (-p) ==="
gcov-dump -p test1.gcda 2>&1 | head -10

# Test 7: Dump raw flag (-r)
echo -e "\n=== Test 7: Dump raw flag (-r) ==="
gcov-dump -r test1.gcda 2>&1 | head -10

# Test 8: Dump stable flag (-s)
echo -e "\n=== Test 8: Dump stable flag (-s) ==="
gcov-dump -s test1.gcda 2>&1 | head -10

# Test 9: Multiple flags combined
echo -e "\n=== Test 9: Multiple flags combined (-l -p -r -s) ==="
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -10

# Test 10: Invalid flag (triggers default case)
echo -e "\n=== Test 10: Invalid flag (-x) triggers default case ==="
gcov-dump -x test1.gcda 2>&1 | grep "unknown flag" || true

# Test 11: No flags, just .gcda file
echo -e "\n=== Test 11: No flags, just .gcda file ==="
gcov-dump test1.gcda 2>&1 | head -10

# Test 12: No flags, just .gcno file
echo -e "\n=== Test 12: No flags, just .gcno file ==="
gcov-dump test1.gcno 2>&1 | head -10

# Test 13: Multiple input files with flag
echo -e "\n=== Test 13: Multiple input files with flag (-l) ==="
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -10

# Test 14: Flag with .gcno file
echo -e "\n=== Test 14: Flag with .gcno file (-p) ==="
gcov-dump -p test1.gcno 2>&1 | head -10

# Test 15: Mixed .gcda and .gcno files
echo -e "\n=== Test 15: Mixed .gcda and .gcno files ==="
gcov-dump -l test1.gcda test1.gcno 2>&1 | head -10

# Test 16: Long option with equals sign (if supported)
echo -e "\n=== Test 16: Testing --help and --version long options ==="
gcov-dump --help 2>&1 | grep -i "usage" && echo "Help option works"
gcov-dump --version | grep -i "gcov-dump" && echo "Version option works"

# Test 17: Empty flag (just '-')
echo -e "\n=== Test 17: Empty flag (just '-') ==="
gcov-dump - test1.gcda 2>&1 | head -5 || true

# Test 18: Multiple invalid flags
echo -e "\n=== Test 18: Multiple invalid flags ==="
gcov-dump -xyz test1.gcda 2>&1 | grep "unknown flag" || true

# Test 19: Flag with no argument (should show error)
echo -e "\n=== Test 19: Flag with no argument ==="
gcov-dump -l 2>&1 | head -5 || true

# Test 20: Complex flag combination
echo -e "\n=== Test 20: Complex flag combination ==="
gcov-dump -lps test1.gcda 2>&1 | head -10

# Cleanup
echo -e "\n=== Cleaning up ==="
cd /
rm -rf "$TEST_DIR"
echo "Test completed successfully!"
```

This test script comprehensively covers all the uncovered lines:

1. **Lines 111-112 (`case 'h'`)** - Tested with `-h` and `--help` flags
2. **Lines 113-114 (`case 'v'`)** - Tested with `-v` and `--version` flags  
3. **Lines 115-116 (`case 'l'`)** - Tested with `-l` flag in multiple contexts
4. **Lines 117-118 (`case 'p'`)** - Tested with `-p` flag in multiple contexts
5. **Lines 119-120 (`case 'r'`)** - Tested with `-r` flag in multiple contexts
6. **Lines 121-122 (`case 's'`)** - Tested with `-s` flag in multiple contexts
7. **Lines 123-124 (`default` case)** - Tested with invalid flags `-x` and `-xyz`

The script also tests various combinations and edge cases:
- Single flags on `.gcda` files
- Single flags on `.gcno` files  
- Multiple flags combined (`-l -p -r -s`)
- Multiple input files with flags
- Mixed `.gcda` and `.gcno` files
- Invalid flags to trigger the default case
- Flag grouping (`-lps`)

To run this test, you need to:
1. Ensure `gcov-dump` is in your PATH (built from GCC source with coverage instrumentation)
2. Have GCC installed to compile the test programs
3. Make the script executable: `chmod +x test_gcov_dump.sh`
4. Run it: `./test_gcov_dump.sh`

The script creates temporary test files, compiles them with coverage instrumentation, runs them to generate `.gcda` files, then exercises `gcov-dump` with all the flag combinations that cover the target lines.
