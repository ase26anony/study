Here's a comprehensive test script that exercises all the uncovered lines in `gcov-dump.cc`:

```bash
#!/bin/bash
# Test script for gcov-dump to cover lines 111-130 in gcov-dump.cc

set -e

# Create temporary directory for test artifacts
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"

# Create simple test source files
cat > test1.c << 'EOF'
#include <stdio.h>

int main() {
    int x = 5;
    int y = 10;
    int z = x + y;
    printf("Result: %d\n", z);
    return 0;
}
EOF

cat > test2.c << 'EOF'
#include <stdio.h>

int helper(int a, int b) {
    return a * b;
}

int main() {
    int result = helper(3, 4);
    printf("Multiplication: %d\n", result);
    return 0;
}
EOF

# Function to compile with coverage
compile_with_coverage() {
    local src=$1
    local base=${src%.c}
    
    echo "Compiling $src with coverage..."
    gcc -fprofile-arcs -ftest-coverage -O0 -g "$src" -o "${base}_prog"
    
    if [ ! -f "${base}.gcno" ]; then
        echo "ERROR: Failed to generate ${base}.gcno"
        exit 1
    fi
}

# Function to run program and generate gcda files
run_and_generate_gcda() {
    local base=$1
    
    echo "Running ${base}_prog to generate gcda..."
    "./${base}_prog" > /dev/null 2>&1
    
    if [ ! -f "${base}.gcda" ]; then
        echo "ERROR: Failed to generate ${base}.gcda"
        exit 1
    fi
}

# Compile test programs
compile_with_coverage test1.c
compile_with_coverage test2.c

# Generate gcda files
run_and_generate_gcda test1
run_and_generate_gcda test2

echo ""
echo "=== Testing gcov-dump with various flags ==="
echo ""

# Test 1: Help flag (-h)
echo "Test 1: -h flag (should call print_usage())"
gcov-dump -h 2>&1 | head -5
echo "Exit code: $?"
echo ""

# Test 2: Help flag (--help)
echo "Test 2: --help flag"
gcov-dump --help 2>&1 | head -5
echo "Exit code: $?"
echo ""

# Test 3: Version flag (-v)
echo "Test 3: -v flag (should call print_version())"
gcov-dump -v 2>&1
echo "Exit code: $?"
echo ""

# Test 4: Version flag (--version)
echo "Test 4: --version flag"
gcov-dump --version 2>&1
echo "Exit code: $?"
echo ""

# Test 5: -l flag (dump contents)
echo "Test 5: -l flag (should set flag_dump_contents = 1)"
gcov-dump -l test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 6: -p flag (dump positions)
echo "Test 6: -p flag (should set flag_dump_positions = 1)"
gcov-dump -p test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 7: -r flag (dump raw)
echo "Test 7: -r flag (should set flag_dump_raw = 1)"
gcov-dump -r test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 8: -s flag (dump stable)
echo "Test 8: -s flag (should set flag_dump_stable = 1)"
gcov-dump -s test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 9: Multiple flags combined
echo "Test 9: Multiple flags combined (-l -p -r -s)"
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -15
echo "Exit code: $?"
echo ""

# Test 10: Invalid flag (should trigger default case)
echo "Test 10: Invalid flag -x (should trigger default case)"
gcov-dump -x test1.gcda 2>&1
echo "Exit code: $?"
echo ""

# Test 11: No flags with gcno file
echo "Test 11: No flags with gcno file"
gcov-dump test1.gcno 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 12: Multiple input files with flag
echo "Test 12: Multiple input files with -l flag"
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -15
echo "Exit code: $?"
echo ""

# Test 13: Different combinations of flags
echo "Test 13: Different flag combination (-l -s)"
gcov-dump -l -s test2.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 14: Flag with gcno file
echo "Test 14: -p flag with gcno file"
gcov-dump -p test2.gcno 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 15: All flags with gcno file
echo "Test 15: All flags with gcno file"
gcov-dump -l -p -r -s test2.gcno 2>&1 | head -15
echo "Exit code: $?"
echo ""

# Test 16: Another invalid flag
echo "Test 16: Another invalid flag -z"
gcov-dump -z test1.gcda 2>&1
echo "Exit code: $?"
echo ""

# Test 17: Mixed valid and invalid flags (invalid first)
echo "Test 17: Mixed flags (-x -l) - invalid first"
gcov-dump -x -l test1.gcda 2>&1
echo "Exit code: $?"
echo ""

# Test 18: Mixed valid and invalid flags (valid first)
echo "Test 18: Mixed flags (-l -x) - valid first"
gcov-dump -l -x test1.gcda 2>&1
echo "Exit code: $?"
echo ""

echo "=== All tests completed ==="
echo ""

# Cleanup
cd /
rm -rf "$TEST_DIR"
echo "Cleaned up test directory: $TEST_DIR"
```

This test script comprehensively covers all the uncovered lines:

1. **Lines 111-112 (`case 'h'`):** Covered by tests 1 and 2 with `-h` and `--help`
2. **Lines 113-115 (`case 'v'`):** Covered by tests 3 and 4 with `-v` and `--version`
3. **Lines 116-118 (`case 'l'`):** Covered by tests 5, 9, 12, 13, 15, 17, and 18
4. **Lines 119-121 (`case 'p'`):** Covered by tests 6, 9, 14, and 15
5. **Lines 122-124 (`case 'r'`):** Covered by tests 7, 9, and 15
6. **Lines 125-127 (`case 's'`):** Covered by tests 8, 9, 13, and 15
7. **Lines 128-130 (`default` case):** Covered by tests 10, 16, 17, and 18 with invalid flags `-x` and `-z`

The script also tests various combinations and edge cases:
- Single flags with `.gcda` files
- Single flags with `.gcno` files
- Multiple flags combined
- Multiple input files
- Invalid flags (triggering the default case)
- Mixed valid and invalid flags

To run this test, you need to:
1. Ensure `gcov-dump` is in your PATH or modify the script to use the full path
2. Have GCC installed with coverage support
3. Make the script executable: `chmod +x test_gcov_dump.sh`
4. Run it: `./test_gcov_dump.sh`

The script creates a temporary directory, generates test programs with coverage instrumentation, runs them to produce `.gcda` files, then exercises `gcov-dump` with all the flag combinations needed to cover the target lines.
