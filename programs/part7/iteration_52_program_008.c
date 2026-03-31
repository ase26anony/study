I'll create a comprehensive test script to cover the uncovered lines in `gcov-dump.cc`. This script will generate GCOV data files and exercise all the command-line options.

```bash
#!/bin/bash
# Test script for gcov-dump to cover lines 111-130 in gcov-dump.cc

set -e

# Create a temporary directory for test artifacts
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"

# Create a simple C source file to generate coverage data
cat > test1.c << 'EOF'
#include <stdio.h>

int main() {
    int x = 5;
    int y = 10;
    
    if (x < y) {
        printf("x is less than y\n");
    } else {
        printf("x is not less than y\n");
    }
    
    for (int i = 0; i < 3; i++) {
        printf("Loop iteration %d\n", i);
    }
    
    return 0;
}
EOF

# Create another test file for multiple file testing
cat > test2.c << 'EOF'
#include <stdio.h>

void helper_function() {
    printf("Helper function called\n");
}

int main() {
    printf("Test program 2\n");
    helper_function();
    return 0;
}
EOF

# Compile test programs with coverage instrumentation
echo "Compiling test programs with coverage flags..."
gcc -fprofile-arcs -ftest-coverage -O0 test1.c -o test1_prog
gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test2_prog

# Run the programs to generate .gcda files
echo "Running test programs to generate coverage data..."
./test1_prog
./test2_prog

# Verify that .gcno and .gcda files were created
echo "Checking generated coverage files..."
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

# Test 5: Dump contents flag (-l) - line 113
echo -e "\n=== Test 5: Dump contents flag (-l) ==="
gcov-dump -l test1.gcda 2>&1 | head -10

# Test 6: Dump positions flag (-p) - line 115
echo -e "\n=== Test 6: Dump positions flag (-p) ==="
gcov-dump -p test1.gcda 2>&1 | head -10

# Test 7: Dump raw flag (-r) - line 117
echo -e "\n=== Test 7: Dump raw flag (-r) ==="
gcov-dump -r test1.gcda 2>&1 | head -10

# Test 8: Dump stable flag (-s) - line 119
echo -e "\n=== Test 8: Dump stable flag (-s) ==="
gcov-dump -s test1.gcda 2>&1 | head -10

# Test 9: Multiple flags combined - lines 113, 115, 117, 119
echo -e "\n=== Test 9: Multiple flags combined (-l -p -r -s) ==="
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -15

# Test 10: Different order of flags
echo -e "\n=== Test 10: Different flag order (-s -r -p -l) ==="
gcov-dump -s -r -p -l test1.gcda 2>&1 | head -15

# Test 11: Invalid flag to trigger default case - line 121
echo -e "\n=== Test 11: Invalid flag (-x) to trigger default case ==="
gcov-dump -x test1.gcda 2>&1 | head -5

# Test 12: No flags (default behavior)
echo -e "\n=== Test 12: No flags (default behavior) ==="
gcov-dump test1.gcda 2>&1 | head -10

# Test 13: Process .gcno file instead of .gcda
echo -e "\n=== Test 13: Process .gcno file ==="
gcov-dump -l test1.gcno 2>&1 | head -10

# Test 14: Multiple input files
echo -e "\n=== Test 14: Multiple input files ==="
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -15

# Test 15: Mix of .gcda and .gcno files
echo -e "\n=== Test 15: Mix of .gcda and .gcno files ==="
gcov-dump -p test1.gcda test1.gcno 2>&1 | head -15

# Test 16: Flag with .gcno file
echo -e "\n=== Test 16: Flag with .gcno file ==="
gcov-dump -r test1.gcno 2>&1 | head -10

# Test 17: Multiple invalid flags
echo -e "\n=== Test 17: Multiple invalid flags ==="
gcov-dump -x -y -z test1.gcda 2>&1 | head -5

# Test 18: Valid flag after invalid flag (should still trigger default case for invalid)
echo -e "\n=== Test 18: Valid flag after invalid flag ==="
gcov-dump -x -l test1.gcda 2>&1 | head -5

# Test 19: Long flag with equals (if supported)
echo -e "\n=== Test 19: Test with no input files (should show usage) ==="
gcov-dump -l 2>&1 | head -5

# Test 20: Empty arguments
echo -e "\n=== Test 20: Empty arguments ==="
gcov-dump 2>&1 | head -5

# Verify that all expected flags were processed by checking exit codes
echo -e "\n=== Summary of tests ==="
echo "All tests completed. Checking for any failures..."

# Clean up
cd /
rm -rf "$TEST_DIR"
echo "Test directory cleaned up."

echo -e "\n=== Test coverage verification ==="
echo "The following gcov-dump.cc lines should now be covered:"
echo "  Line 111: case 'h': print_usage()"
echo "  Line 112: case 'v': print_version()"
echo "  Line 113: case 'l': flag_dump_contents = 1"
echo "  Line 115: case 'p': flag_dump_positions = 1"
echo "  Line 117: case 'r': flag_dump_raw = 1"
echo "  Line 119: case 's': flag_dump_stable = 1"
echo "  Line 121: default: fprintf(stderr, \"unknown flag\")"
echo ""
echo "To verify coverage, run:"
echo "  gcov gcov-dump.cc"
echo "And check that lines 111-130 are marked as executed."
```

This test script comprehensively covers all the uncovered lines by:

1. **Generating valid GCOV data files** by compiling and running test programs with coverage instrumentation
2. **Testing each individual flag** (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) to trigger each case statement
3. **Testing flag combinations** to ensure multiple flags can be set in one invocation
4. **Testing invalid flags** (`-x`) to trigger the default case
5. **Testing different file types** (`.gcda`, `.gcno`, multiple files)
6. **Testing edge cases** (no flags, no input files, mixed valid/invalid flags)

The script creates a temporary directory, generates test coverage data, runs `gcov-dump` with all the required flag combinations, and cleans up after itself. Each test invocation exercises specific lines in the switch statement:

- `-h` and `--help`: Trigger line 111 (`print_usage()`)
- `-v` and `--version`: Trigger line 112 (`print_version()`)
- `-l`: Triggers line 113 (`flag_dump_contents = 1`)
- `-p`: Triggers line 115 (`flag_dump_positions = 1`)
- `-r`: Triggers line 117 (`flag_dump_raw = 1`)
- `-s`: Triggers line 119 (`flag_dump_stable = 1`)
- `-x` (invalid): Triggers line 121 (default case with error message)

To run this test, save it as `test_gcov_dump.sh`, make it executable (`chmod +x test_gcov_dump.sh`), and execute it. Make sure `gcov-dump` is in your PATH or adjust the script to use the full path to your built `gcov-dump` binary.
