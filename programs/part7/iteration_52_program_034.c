Looking at the uncovered lines in `gcov-dump.cc`, I need to create a comprehensive test that exercises all the command-line flag handling logic. Here's a shell script that will generate the necessary GCOV data files and test all the uncovered switch cases:

```bash
#!/bin/bash
# Test script for gcov-dump command-line argument parsing
# This script tests the uncovered lines in gcov-dump.cc (lines 111-130)

set -e  # Exit on any error

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcov-dump-test.XXXXXX)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"

# Create a simple C test program
cat > test1.c << 'EOF'
#include <stdio.h>

int main() {
    int x = 5;
    int y = 10;
    int sum = x + y;
    
    if (sum > 10) {
        printf("Sum is greater than 10: %d\n", sum);
    } else {
        printf("Sum is 10 or less: %d\n", sum);
    }
    
    for (int i = 0; i < 3; i++) {
        printf("Loop iteration %d\n", i);
    }
    
    return 0;
}
EOF

# Create another test program for multiple file testing
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

echo "Compiling test programs with coverage instrumentation..."

# Compile test programs with coverage flags
gcc -fprofile-arcs -ftest-coverage -O0 -g test1.c -o test1_prog
gcc -fprofile-arcs -ftest-coverage -O0 -g test2.c -o test2_prog

echo "Running test programs to generate .gcda files..."

# Run the programs to generate coverage data
./test1_prog > /dev/null
./test2_prog > /dev/null

echo "Generated coverage files:"
ls -la *.gcda *.gcno

echo ""
echo "=========================================="
echo "Testing gcov-dump command-line arguments"
echo "=========================================="
echo ""

# Test 1: -h flag (help) - covers case 'h'
echo "Test 1: Testing -h flag (help)"
echo "------------------------------"
gcov-dump -h 2>&1 | head -5
echo "Exit code: $?"
echo ""

# Test 2: --help flag (should also trigger help)
echo "Test 2: Testing --help flag"
echo "---------------------------"
gcov-dump --help 2>&1 | head -5
echo "Exit code: $?"
echo ""

# Test 3: -v flag (version) - covers case 'v'
echo "Test 3: Testing -v flag (version)"
echo "---------------------------------"
gcov-dump -v 2>&1
echo "Exit code: $?"
echo ""

# Test 4: --version flag (should also trigger version)
echo "Test 4: Testing --version flag"
echo "------------------------------"
gcov-dump --version 2>&1
echo "Exit code: $?"
echo ""

# Test 5: -l flag (dump contents) - covers case 'l'
echo "Test 5: Testing -l flag (dump contents)"
echo "---------------------------------------"
gcov-dump -l test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 6: -p flag (dump positions) - covers case 'p'
echo "Test 6: Testing -p flag (dump positions)"
echo "----------------------------------------"
gcov-dump -p test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 7: -r flag (dump raw) - covers case 'r'
echo "Test 7: Testing -r flag (dump raw)"
echo "----------------------------------"
gcov-dump -r test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 8: -s flag (dump stable) - covers case 's'
echo "Test 8: Testing -s flag (dump stable)"
echo "-------------------------------------"
gcov-dump -s test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 9: Multiple flags combined - covers multiple cases
echo "Test 9: Testing multiple flags combined (-l -p -r -s)"
echo "----------------------------------------------------"
gcov-dump -l -p -r -s test1.gcda 2>&1 | head -15
echo "Exit code: $?"
echo ""

# Test 10: Invalid flag - triggers default case
echo "Test 10: Testing invalid flag (-x) - triggers default case"
echo "----------------------------------------------------------"
gcov-dump -x test1.gcda 2>&1
echo "Exit code: $?"
echo ""

# Test 11: No flags, just .gcda file
echo "Test 11: Testing with no flags (just .gcda file)"
echo "------------------------------------------------"
gcov-dump test1.gcda 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 12: No flags, just .gcno file
echo "Test 12: Testing with no flags (just .gcno file)"
echo "------------------------------------------------"
gcov-dump test1.gcno 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 13: Multiple input files with flag
echo "Test 13: Testing multiple input files with -l flag"
echo "--------------------------------------------------"
gcov-dump -l test1.gcda test2.gcda 2>&1 | head -15
echo "Exit code: $?"
echo ""

# Test 14: Mix of .gcda and .gcno files
echo "Test 14: Testing mix of .gcda and .gcno files"
echo "---------------------------------------------"
gcov-dump -l test1.gcda test1.gcno 2>&1 | head -15
echo "Exit code: $?"
echo ""

# Test 15: Long form of combined flags
echo "Test 15: Testing long form combined flags"
echo "-----------------------------------------"
gcov-dump -lprs test1.gcda 2>&1 | head -15
echo "Exit code: $?"
echo ""

# Test 16: Different order of flags
echo "Test 16: Testing different order of flags"
echo "-----------------------------------------"
gcov-dump -s -r -p -l test1.gcda 2>&1 | head -15
echo "Exit code: $?"
echo ""

# Test 17: Flag with .gcno file (not just .gcda)
echo "Test 17: Testing -p flag with .gcno file"
echo "----------------------------------------"
gcov-dump -p test1.gcno 2>&1 | head -10
echo "Exit code: $?"
echo ""

# Test 18: Another invalid flag combination
echo "Test 18: Testing another invalid flag (-z)"
echo "------------------------------------------"
gcov-dump -z test1.gcda 2>&1
echo "Exit code: $?"
echo ""

# Test 19: Flag with non-existent file (should still process flag first)
echo "Test 19: Testing flag with non-existent file"
echo "--------------------------------------------"
gcov-dump -l nonexistent.gcda 2>&1
echo "Exit code: $?"
echo ""

# Test 20: Empty arguments (should show usage)
echo "Test 20: Testing with no arguments"
echo "----------------------------------"
gcov-dump 2>&1 | head -5
echo "Exit code: $?"
echo ""

echo "=========================================="
echo "All tests completed"
echo "=========================================="

# Clean up
cd ..
rm -rf "$TEST_DIR"

echo "Test directory cleaned up: $TEST_DIR"
```

This comprehensive test script will:

1. **Generate valid GCOV data files** by compiling and running two simple C programs with coverage instrumentation.

2. **Test each individual flag** (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) to cover each switch case.

3. **Test flag combinations** to ensure multiple cases can be triggered in sequence.

4. **Test the default case** by using invalid flags (`-x`, `-z`).

5. **Test various file combinations**:
   - Single `.gcda` file
   - Single `.gcno` file  
   - Multiple `.gcda` files
   - Mix of `.gcda` and `.gcno` files
   - Non-existent file (to test error handling)

6. **Test different invocation patterns**:
   - Short flags separately
   - Combined short flags (`-lprs`)
   - Long forms (`--help`, `--version`)
   - No flags at all
   - No arguments at all

The script captures exit codes and shows snippets of output to verify each case was executed. Each test case will exercise the specific lines in the switch statement that were uncovered:

- `case 'h'`: Triggered by `-h` and `--help`
- `case 'v'`: Triggered by `-v` and `--version`  
- `case 'l'`: Triggered by `-l` flag
- `case 'p'`: Triggered by `-p` flag
- `case 'r'`: Triggered by `-r` flag
- `case 's'`: Triggered by `-s` flag
- `default`: Triggered by invalid flags like `-x` and `-z`

The script also tests edge cases like combined flags, different file types, and error conditions to ensure robust coverage of the command-line parsing logic.
