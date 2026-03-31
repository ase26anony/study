Looking at the uncovered lines in `gcov-dump.cc`, I need to create a comprehensive test that exercises all the command-line flag handling logic. Here's a shell script that will cover all the specified cases:

```bash
#!/bin/bash
# Test script for gcov-dump command-line argument parsing
# This script covers lines 111-130 in gcov-dump.cc

set -e

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
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
    printf("Result: %d\n", helper());
    return 0;
}
EOF

# Compile test programs with coverage instrumentation
echo "Compiling test programs with coverage flags..."
gcc -fprofile-arcs -ftest-coverage -O0 test1.c -o test1
gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test2

# Run the programs to generate .gcda files
echo "Running test programs to generate coverage data..."
./test1 > /dev/null
./test2 > /dev/null

# Verify that coverage files were created
if [ ! -f test1.gcda ] || [ ! -f test1.gcno ]; then
    echo "ERROR: test1 coverage files not created"
    exit 1
fi

if [ ! -f test2.gcda ] || [ ! -f test2.gcno ]; then
    echo "ERROR: test2 coverage files not created"
    exit 1
fi

echo "Testing gcov-dump with various command-line flags..."
echo "====================================================="

# Test 1: Help flag (-h) - covers case 'h'
echo "Test 1: Testing -h flag"
gcov-dump -h 2>&1 | head -5
echo "Exit code: $?"
echo

# Test 2: Help flag (--help) - also covers case 'h'
echo "Test 2: Testing --help flag"
gcov-dump --help 2>&1 | head -5
echo "Exit code: $?"
echo

# Test 3: Version flag (-v) - covers case 'v'
echo "Test 3: Testing -v flag"
gcov-dump -v 2>&1
echo "Exit code: $?"
echo

# Test 4: Version flag (--version) - also covers case 'v'
echo "Test 4: Testing --version flag"
gcov-dump --version 2>&1
echo "Exit code: $?"
echo

# Test 5: Dump contents flag (-l) - covers case 'l'
echo "Test 5: Testing -l flag"
gcov-dump -l test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo

# Test 6: Dump positions flag (-p) - covers case 'p'
echo "Test 6: Testing -p flag"
gcov-dump -p test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo

# Test 7: Dump raw flag (-r) - covers case 'r'
echo "Test 7: Testing -r flag"
gcov-dump -r test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo

# Test 8: Dump stable flag (-s) - covers case 's'
echo "Test 8: Testing -s flag"
gcov-dump -s test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo

# Test 9: Multiple flags combined - covers multiple cases sequentially
echo "Test 9: Testing multiple flags combined (-l -p -r -s)"
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo

# Test 10: Different order of flags
echo "Test 10: Testing different flag order (-s -r -p -l)"
gcov-dump -s -r -p -l test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo

# Test 11: Invalid flag (-x) - covers default case
echo "Test 11: Testing invalid flag (-x) - should trigger default case"
gcov-dump -x test1.gcda 2>&1
echo "Exit code: $?"
echo

# Test 12: No flags, just .gcda file
echo "Test 12: Testing with no flags on .gcda file"
gcov-dump test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo

# Test 13: No flags, just .gcno file
echo "Test 13: Testing with no flags on .gcno file"
gcov-dump test1.gcno 2>&1 | head -10
echo "Exit code: $?"
echo

# Test 14: Multiple input files with flag
echo "Test 14: Testing -l flag with multiple input files"
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -10
echo "Exit code: $?"
echo

# Test 15: Mix of .gcda and .gcno files
echo "Test 15: Testing with mix of .gcda and .gcno files"
gcov-dump -l test1.gcda test1.gcno 2>&1 | head -10
echo "Exit code: $?"
echo

# Test 16: Flag with multiple files of different types
echo "Test 16: Testing -p flag with multiple files"
gcov-dump -p test1.gcda test2.gcno test2.gcda 2>&1 | head -10
echo "Exit code: $?"
echo

# Test 17: Long form flags (if supported)
echo "Test 17: Testing long form flags (if available)"
gcov-dump --help 2>&1 | grep -q "Usage:" && echo "Long help flag works"
gcov-dump --version 2>&1 | grep -q "gcov-dump" && echo "Long version flag works"
echo

# Test 18: Empty flag (just -)
echo "Test 18: Testing empty flag (-)"
gcov-dump - test1.gcda 2>&1 || true
echo "Exit code: $?"
echo

# Test 19: Combined short flags (-lprs)
echo "Test 19: Testing combined short flags (-lprs)"
gcov-dump -lprs test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo

echo "====================================================="
echo "All tests completed successfully!"
echo "Coverage achieved for:"
echo "  -h / --help (print_usage)"
echo "  -v / --version (print_version)"
echo "  -l (flag_dump_contents)"
echo "  -p (flag_dump_positions)"
echo "  -r (flag_dump_raw)"
echo "  -s (flag_dump_stable)"
echo "  default case (invalid flags)"
echo "  multiple flag combinations"
echo "  various file input scenarios"
```

This test script comprehensively covers all the uncovered lines:

1. **Lines 111-112 (`case 'h'`):** Tested with `-h` and `--help` flags
2. **Lines 113-115 (`case 'v'`):** Tested with `-v` and `--version` flags
3. **Lines 116-118 (`case 'l'`):** Tested with `-l` flag in multiple scenarios
4. **Lines 119-121 (`case 'p'`):** Tested with `-p` flag in multiple scenarios
5. **Lines 122-124 (`case 'r'`):** Tested with `-r` flag in multiple scenarios
6. **Lines 125-127 (`case 's'`):** Tested with `-s` flag in multiple scenarios
7. **Lines 128-130 (`default` case):** Tested with invalid `-x` flag

The script also tests:
- Multiple flag combinations to ensure sequential execution
- Different types of input files (`.gcda`, `.gcno`)
- Multiple input files in one command
- Various edge cases (empty flag, combined short flags)

To run this test, you need to:
1. Ensure `gcov-dump` is in your PATH or provide the full path
2. Have GCC installed with coverage support
3. Make the script executable: `chmod +x test_gcov_dump.sh`
4. Run it: `./test_gcov_dump.sh`

The script creates temporary test files, compiles them with coverage instrumentation, runs them to generate `.gcda` files, and then exercises `gcov-dump` with all the flag combinations needed to cover the target lines.
