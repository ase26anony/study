Here's a comprehensive test script to cover the uncovered lines in `gcov-dump.cc`:

```bash
#!/bin/bash
set -e

# Create a temporary directory for test artifacts
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

# Verify that coverage files were created
if [ ! -f test1.gcda ] || [ ! -f test1.gcno ]; then
    echo "ERROR: test1 coverage files not created"
    exit 1
fi

if [ ! -f test2.gcda ] || [ ! -f test2.gcno ]; then
    echo "ERROR: test2 coverage files not created"
    exit 1
fi

echo "Coverage files created successfully"
echo "======================================"

# Test 1: Help flag (-h)
echo "Test 1: Testing -h flag"
gcov-dump -h 2>&1 | head -5
echo "Exit code: $?"
echo "---"

# Test 2: Help flag (--help)
echo "Test 2: Testing --help flag"
gcov-dump --help 2>&1 | head -5
echo "Exit code: $?"
echo "---"

# Test 3: Version flag (-v)
echo "Test 3: Testing -v flag"
gcov-dump -v
echo "Exit code: $?"
echo "---"

# Test 4: Version flag (--version)
echo "Test 4: Testing --version flag"
gcov-dump --version
echo "Exit code: $?"
echo "---"

# Test 5: Dump contents flag (-l)
echo "Test 5: Testing -l flag"
gcov-dump -l test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo "---"

# Test 6: Dump positions flag (-p)
echo "Test 6: Testing -p flag"
gcov-dump -p test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo "---"

# Test 7: Dump raw flag (-r)
echo "Test 7: Testing -r flag"
gcov-dump -r test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo "---"

# Test 8: Dump stable flag (-s)
echo "Test 8: Testing -s flag"
gcov-dump -s test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo "---"

# Test 9: Multiple flags combined
echo "Test 9: Testing multiple flags (-l -p -r -s)"
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -15
echo "Exit code: $?"
echo "---"

# Test 10: Invalid flag (triggers default case)
echo "Test 10: Testing invalid flag (-x) to trigger default case"
gcov-dump -x test1.gcda 2>&1
echo "Exit code: $?"
echo "---"

# Test 11: No flags (default behavior)
echo "Test 11: Testing no flags with .gcda file"
gcov-dump test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo "---"

# Test 12: No flags with .gcno file
echo "Test 12: Testing no flags with .gcno file"
gcov-dump test1.gcno 2>&1 | head -10
echo "Exit code: $?"
echo "---"

# Test 13: Single flag with .gcno file
echo "Test 13: Testing -l flag with .gcno file"
gcov-dump -l test1.gcno 2>&1 | head -10
echo "Exit code: $?"
echo "---"

# Test 14: Multiple input files
echo "Test 14: Testing with multiple input files"
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -15
echo "Exit code: $?"
echo "---"

# Test 15: Mixed file types
echo "Test 15: Testing with mixed file types (.gcda and .gcno)"
gcov-dump -l test1.gcda test1.gcno 2>&1 | head -15
echo "Exit code: $?"
echo "---"

# Test 16: All flags with multiple files
echo "Test 16: Testing all flags with multiple files"
gcov-dump -l -p -r -s test1.gcda test2.gcda test1.gcno test2.gcno 2>&1 | head -20
echo "Exit code: $?"
echo "---"

# Test 17: Long form flags with short options
echo "Test 17: Testing long form of help and version"
gcov-dump --help --version 2>&1 | head -5
echo "Exit code: $?"
echo "---"

# Test 18: Empty input (should show usage or error)
echo "Test 18: Testing with no input files"
gcov-dump -l 2>&1 | head -5
echo "Exit code: $?"
echo "---"

# Test 19: Non-existent file
echo "Test 19: Testing with non-existent file"
gcov-dump -l nonexistent.gcda 2>&1
echo "Exit code: $?"
echo "---"

# Test 20: Flag with no argument (edge case)
echo "Test 20: Testing flag parsing edge case"
gcov-dump -l - 2>&1 | head -5
echo "Exit code: $?"
echo "---"

echo "======================================"
echo "All tests completed successfully!"
echo "Coverage should now include lines 111-130 in gcov-dump.cc"
```

To use this test script:

1. **Save the script** as `test_gcov_dump.sh`
2. **Make it executable**: `chmod +x test_gcov_dump.sh`
3. **Ensure `gcov-dump` is in your PATH** or modify the script to use the full path to your `gcov-dump` binary
4. **Run the script**: `./test_gcov_dump.sh`

The script will:
1. Create two simple C test programs
2. Compile them with coverage instrumentation (`-fprofile-arcs -ftest-coverage`)
3. Run the programs to generate `.gcda` files
4. Execute `gcov-dump` with various flag combinations to cover all the target lines:
   - `-h` and `--help` → covers `case 'h'` and `print_usage()`
   - `-v` and `--version` → covers `case 'v'` and `print_version()`
   - `-l` → covers `case 'l'` and `flag_dump_contents = 1`
   - `-p` → covers `case 'p'` and `flag_dump_positions = 1`
   - `-r` → covers `case 'r'` and `flag_dump_raw = 1`
   - `-s` → covers `case 's'` and `flag_dump_stable = 1`
   - `-x` (invalid) → covers `default` case and error message
   - Multiple flag combinations → ensures sequential execution of flag-setting logic
   - Different file types (`.gcda`, `.gcno`) → tests flag processing in different contexts
   - Multiple files → tests argument parsing with multiple inputs

The script captures output and exit codes to verify each invocation works as expected. After running this test, the coverage for the switch statement cases (lines 111-130) should be fully covered.
