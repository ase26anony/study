Here's a comprehensive shell script that covers all the specified requirements for testing the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo "=== GCOV-TOOL Overlap Flag Coverage Test ==="

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d)
cd "$TEST_DIR"

# Create a simple C program with multiple functions and branches
cat > test.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

int func1(int x) {
    if (x > 0) {
        return x * 2;
    } else {
        return x / 2;
    }
}

int func2(int x, int y) {
    for (int i = 0; i < x; i++) {
        y += i;
    }
    return y;
}

void func3() {
    printf("Func3 called\n");
}

int main(int argc, char *argv[]) {
    int val = 10;
    if (argc > 1) {
        val = atoi(argv[1]);
    }
    
    int result1 = func1(val);
    int result2 = func2(val, result1);
    func3();
    
    printf("Result: %d\n", result2);
    return 0;
}
EOF

# Compile with coverage instrumentation
echo "Compiling test program with coverage..."
gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog

# Create multiple .gcda files with different execution patterns
echo "Generating profile data..."

# First run with normal input
echo "Run 1: Normal execution"
./test_prog 10

# Copy .gcda files for first profile set
mkdir -p profile_set1
cp *.gcda profile_set1/ 2>/dev/null || true

# Remove .gcda files for second run
rm -f *.gcda

# Second run with different input
echo "Run 2: Different input"
./test_prog 5

# Copy .gcda files for second profile set
mkdir -p profile_set2
cp *.gcda profile_set2/ 2>/dev/null || true

# Third run with zero input (to trigger different branch)
echo "Run 3: Zero input"
./test_prog 0

# Copy .gcda files for third profile set
mkdir -p profile_set3
cp *.gcda profile_set3/ 2>/dev/null || true

# Test individual flags
echo -e "\n${GREEN}Testing individual flags:${NC}"

# Test -v flag (verbose)
echo "Testing -v flag..."
gcov-tool overlap -v profile_set1/*.gcda profile_set2/*.gcda > verbose_output.txt 2>&1
if [ $? -eq 0 ]; then echo "✓ -v flag works"; else echo -e "${RED}✗ -v flag failed${NC}"; fi

# Test -f flag (function level)
echo "Testing -f flag..."
gcov-tool overlap -f profile_set1/*.gcda profile_set2/*.gcda > /dev/null 2>&1
if [ $? -eq 0 ]; then echo "✓ -f flag works"; else echo -e "${RED}✗ -f flag failed${NC}"; fi

# Test -F flag (fullname)
echo "Testing -F flag..."
gcov-tool overlap -F profile_set1/*.gcda profile_set2/*.gcda > /dev/null 2>&1
if [ $? -eq 0 ]; then echo "✓ -F flag works"; else echo -e "${RED}✗ -F flag failed${NC}"; fi

# Test -o flag (object level)
echo "Testing -o flag..."
gcov-tool overlap -o profile_set1/*.gcda profile_set2/*.gcda > /dev/null 2>&1
if [ $? -eq 0 ]; then echo "✓ -o flag works"; else echo -e "${RED}✗ -o flag failed${NC}"; fi

# Test -h flag (hot only)
echo "Testing -h flag..."
gcov-tool overlap -h profile_set1/*.gcda profile_set2/*.gcda > /dev/null 2>&1
if [ $? -eq 0 ]; then echo "✓ -h flag works"; else echo -e "${RED}✗ -h flag failed${NC}"; fi

# Test -t flag with threshold (requires argument)
echo "Testing -t flag with threshold 0.5..."
gcov-tool overlap -t 0.5 profile_set1/*.gcda profile_set2/*.gcda > /dev/null 2>&1
if [ $? -eq 0 ]; then echo "✓ -t 0.5 flag works"; else echo -e "${RED}✗ -t 0.5 flag failed${NC}"; fi

echo "Testing -t flag with threshold 1.0..."
gcov-tool overlap -t 1.0 profile_set1/*.gcda profile_set2/*.gcda > /dev/null 2>&1
if [ $? -eq 0 ]; then echo "✓ -t 1.0 flag works"; else echo -e "${RED}✗ -t 1.0 flag failed${NC}"; fi

echo "Testing -t flag with threshold 10.5..."
gcov-tool overlap -t 10.5 profile_set1/*.gcda profile_set2/*.gcda > /dev/null 2>&1
if [ $? -eq 0 ]; then echo "✓ -t 10.5 flag works"; else echo -e "${RED}✗ -t 10.5 flag failed${NC}"; fi

# Test flag combinations
echo -e "\n${GREEN}Testing flag combinations:${NC}"

# Test -f -o combination
echo "Testing -f -o combination..."
gcov-tool overlap -f -o profile_set1/*.gcda profile_set2/*.gcda profile_set3/*.gcda > /dev/null 2>&1
if [ $? -eq 0 ]; then echo "✓ -f -o combination works"; else echo -e "${RED}✗ -f -o combination failed${NC}"; fi

# Test -F -h -t combination
echo "Testing -F -h -t 1.0 combination..."
gcov-tool overlap -F -h -t 1.0 profile_set1/*.gcda profile_set2/*.gcda > /dev/null 2>&1
if [ $? -eq 0 ]; then echo "✓ -F -h -t 1.0 combination works"; else echo -e "${RED}✗ -F -h -t 1.0 combination failed${NC}"; fi

# Test -v -f -F -o -h -t combination
echo "Testing -v -f -F -o -h -t 5.0 combination..."
gcov-tool overlap -v -f -F -o -h -t 5.0 profile_set1/*.gcda profile_set2/*.gcda profile_set3/*.gcda > full_combo_output.txt 2>&1
if [ $? -eq 0 ]; then echo "✓ Full combination works"; else echo -e "${RED}✗ Full combination failed${NC}"; fi

# Test -f -o -t 0.8 combination (as specified)
echo "Testing -f -o -t 0.8 combination..."
gcov-tool overlap -f -o -t 0.8 profile_set1/*.gcda profile_set2/*.gcda > /dev/null 2>&1
if [ $? -eq 0 ]; then echo "✓ -f -o -t 0.8 combination works"; else echo -e "${RED}✗ -f -o -t 0.8 combination failed${NC}"; fi

# Test invalid flag to trigger usage (default case)
echo -e "\n${GREEN}Testing invalid flag to trigger usage:${NC}"
echo "Testing invalid -Z flag..."
gcov-tool overlap -Z profile_set1/*.gcda 2>&1 | grep -q "Usage:" && echo "✓ Invalid flag triggered usage" || echo -e "${RED}✗ Invalid flag did not trigger usage${NC}"

# Test with multiple input files explicitly
echo -e "\n${GREEN}Testing with multiple explicit .gcda files:${NC}"
echo "Testing with 3 profile sets..."
gcov-tool overlap -f -o profile_set1/*.gcda profile_set2/*.gcda profile_set3/*.gcda > /dev/null 2>&1
if [ $? -eq 0 ]; then echo "✓ Multiple file sets work"; else echo -e "${RED}✗ Multiple file sets failed${NC}"; fi

# Test with different .gcda file patterns
echo -e "\n${GREEN}Testing with mixed .gcda files:${NC}"
# Create a merged directory with all .gcda files
mkdir -p all_profiles
cp profile_set1/*.gcda profile_set2/*.gcda profile_set3/*.gcda all_profiles/ 2>/dev/null || true

echo "Testing with all merged profiles..."
gcov-tool overlap -v -f all_profiles/*.gcda > /dev/null 2>&1
if [ $? -eq 0 ]; then echo "✓ Merged profiles work"; else echo -e "${RED}✗ Merged profiles failed${NC}"; fi

# Test edge cases for -t flag
echo -e "\n${GREEN}Testing edge cases for -t flag:${NC}"

echo "Testing -t with 0.0 threshold..."
gcov-tool overlap -t 0.0 profile_set1/*.gcda profile_set2/*.gcda > /dev/null 2>&1
if [ $? -eq 0 ]; then echo "✓ -t 0.0 works"; else echo -e "${RED}✗ -t 0.0 failed${NC}"; fi

echo "Testing -t with 100.0 threshold..."
gcov-tool overlap -t 100.0 profile_set1/*.gcda profile_set2/*.gcda > /dev/null 2>&1
if [ $? -eq 0 ]; then echo "✓ -t 100.0 works"; else echo -e "${RED}✗ -t 100.0 failed${NC}"; fi

# Test missing argument for -t flag (should show error/usage)
echo "Testing -t without argument..."
gcov-tool overlap -t 2>&1 | grep -q "requires an argument" && echo "✓ -t without arg shows error" || echo -e "${RED}✗ -t without arg error not shown${NC}"

# Cleanup
echo -e "\n${GREEN}Cleaning up...${NC}"
cd ..
rm -rf "$TEST_DIR"

echo -e "\n${GREEN}=== Test completed successfully ==="
echo "Covered all target flags:"
echo "  -v (verbose)"
echo "  -f (function level)"
echo "  -F (fullname)"
echo "  -o (object level)"
echo "  -h (hot only)"
echo "  -t (threshold with various values)"
echo "  -Z (invalid flag to trigger default case)"
echo "  Multiple flag combinations"
echo "  Multiple .gcda file inputs${NC}"
```

This script provides comprehensive coverage of all the specified requirements:

1. **Individual Flag Testing**: Tests each uncovered flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) individually with valid arguments.

2. **Flag Combinations**: Tests various combinations including `-f -o`, `-F -h -t 1.0`, `-v -f -F -o -h -t 5.0`, and `-f -o -t 0.8`.

3. **Invalid Flag**: Tests `-Z` to trigger the `default` case and call `overlap_usage()`.

4. **Multiple Input Files**: Creates three sets of profile data and tests with multiple `.gcda` files.

5. **Valid GCOV Data**: Generates a C program, compiles it with coverage, runs it multiple times with different inputs to create distinct `.gcda` files.

6. **Verbose Output Handling**: Captures verbose output to files (`verbose_output.txt`, `full_combo_output.txt`).

7. **Edge Cases**: Tests edge cases for the `-t` flag including 0.0, 0.5, 1.0, 5.0, 10.5, and 100.0 thresholds.

8. **Error Testing**: Tests `-t` without an argument to ensure proper error handling.

The script is self-contained, creates a temporary directory for all test files, and cleans up after itself. Each test checks the exit status and provides clear pass/fail feedback with color coding.
