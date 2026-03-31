Looking at the uncovered lines in `gcov-tool.cc`, I'll create a comprehensive test script that exercises all the command-line options for the `overlap` command. Here's a shell script that systematically tests each case:

```bash
#!/bin/bash
# test_gcov_tool_overlap.sh
# Comprehensive test for gcov-tool overlap command options

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== Starting gcov-tool overlap command tests ===${NC}"

# Create a temporary directory for test files
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"
echo "Working in temporary directory: $TEMP_DIR"

# Step 1: Create a simple C program for coverage testing
cat > test_program.c << 'EOF'
#include <stdio.h>

void function1() {
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
}

void function2(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else if (x < 0) {
        printf("Negative: %d\n", x);
    } else {
        printf("Zero\n");
    }
}

int main(int argc, char *argv[]) {
    function1();
    function2(5);
    function2(-3);
    function2(0);
    return 0;
}
EOF

# Step 2: Compile with coverage flags
echo -e "\n${YELLOW}Compiling test program with coverage...${NC}"
gcc -fprofile-arcs -ftest-coverage -O0 test_program.c -o test_program

# Step 3: Run program to generate initial .gcda file
echo -e "\n${YELLOW}Running test program to generate coverage data...${NC}"
./test_program > /dev/null

# Step 4: Create a second .gcda file with different coverage
echo -e "\n${YELLOW}Creating second coverage data file...${NC}"
# Copy the first .gcda file
cp test_program.gcda test_program2.gcda

# Modify the second .gcda file slightly to create differences
# We'll run the program with different inputs to get different coverage
cat > test_program2.c << 'EOF'
#include <stdio.h>

void function1() {
    for (int i = 0; i < 5; i++) {  // Different loop count
        if (i % 3 == 0) {          // Different condition
            printf("Divisible by 3: %d\n", i);
        }
    }
}

void function2(int x) {
    if (x > 10) {  // Different threshold
        printf("Greater than 10: %d\n", x);
    }
}

int main() {
    function1();
    function2(15);
    return 0;
}
EOF

# Compile and run second program
gcc -fprofile-arcs -ftest-coverage -O0 test_program2.c -o test_program2
./test_program2 > /dev/null

# Rename the second .gcda to avoid conflict
mv test_program2.gcda test_program_diff.gcda

# Step 5: Test individual flags (lines 534-554)
echo -e "\n${YELLOW}=== Testing individual flags ===${NC}"

# Test verbose flag (-v) - line 534-536
echo -e "\n${GREEN}Testing -v flag (verbose)${NC}"
gcov-tool overlap -v test_program.gcda test_program_diff.gcda || echo "Exit code: $?"

# Test function-level overlap (-f) - line 537-539
echo -e "\n${GREEN}Testing -f flag (function-level overlap)${NC}"
gcov-tool overlap -f test_program.gcda test_program_diff.gcda || echo "Exit code: $?"

# Test use full pathnames (-F) - line 540-542
echo -e "\n${GREEN}Testing -F flag (use full pathnames)${NC}"
gcov-tool overlap -F test_program.gcda test_program_diff.gcda || echo "Exit code: $?"

# Test object-level overlap (-o) - line 543-545
echo -e "\n${GREEN}Testing -o flag (object-level overlap)${NC}"
gcov-tool overlap -o test_program.gcda test_program_diff.gcda || echo "Exit code: $?"

# Test hot only (-h) - line 546-548
echo -e "\n${GREEN}Testing -h flag (hot only)${NC}"
gcov-tool overlap -h test_program.gcda test_program_diff.gcda || echo "Exit code: $?"

# Test hot threshold with argument (-t) - line 549-551
echo -e "\n${GREEN}Testing -t flag with argument (hot threshold 0.5)${NC}"
gcov-tool overlap -t 0.5 test_program.gcda test_program_diff.gcda || echo "Exit code: $?"

# Step 6: Test combined flags
echo -e "\n${YELLOW}=== Testing combined flags ===${NC}"

echo -e "\n${GREEN}Testing -v -f -F combination${NC}"
gcov-tool overlap -v -f -F test_program.gcda test_program_diff.gcda || echo "Exit code: $?"

echo -e "\n${GREEN}Testing -f -o -h combination${NC}"
gcov-tool overlap -f -o -h test_program.gcda test_program_diff.gcda || echo "Exit code: $?"

echo -e "\n${GREEN}Testing -v -F -o -t 0.75 combination${NC}"
gcov-tool overlap -v -F -o -t 0.75 test_program.gcda test_program_diff.gcda || echo "Exit code: $?"

echo -e "\n${GREEN}Testing all flags combined${NC}"
gcov-tool overlap -v -f -F -o -h -t 0.9 test_program.gcda test_program_diff.gcda || echo "Exit code: $?"

# Step 7: Test edge cases and error conditions
echo -e "\n${YELLOW}=== Testing edge cases and error conditions ===${NC}"

# Test default case (invalid option) - line 552-554
echo -e "\n${GREEN}Testing invalid option -z (should trigger default case)${NC}"
gcov-tool overlap -z test_program.gcda test_program_diff.gcda 2>&1 || echo "Exit code: $?"

# Test -t without argument (should trigger error)
echo -e "\n${GREEN}Testing -t without argument${NC}"
gcov-tool overlap -t test_program.gcda test_program_diff.gcda 2>&1 || echo "Exit code: $?"

# Test -t with non-numeric argument
echo -e "\n${GREEN}Testing -t with non-numeric argument${NC}"
gcov-tool overlap -t not_a_number test_program.gcda test_program_diff.gcda 2>&1 || echo "Exit code: $?"

# Test with same file twice
echo -e "\n${GREEN}Testing with same input file twice${NC}"
gcov-tool overlap -v test_program.gcda test_program.gcda || echo "Exit code: $?"

# Test with non-existent files
echo -e "\n${GREEN}Testing with non-existent files${NC}"
gcov-tool overlap -v nonexistent1.gcda nonexistent2.gcda 2>&1 || echo "Exit code: $?"

# Test with threshold edge values
echo -e "\n${GREEN}Testing with threshold 0.0${NC}"
gcov-tool overlap -t 0.0 test_program.gcda test_program_diff.gcda || echo "Exit code: $?"

echo -e "\n${GREEN}Testing with threshold 1.0${NC}"
gcov-tool overlap -t 1.0 test_program.gcda test_program_diff.gcda || echo "Exit code: $?"

echo -e "\n${GREEN}Testing with negative threshold${NC}"
gcov-tool overlap -t -0.5 test_program.gcda test_program_diff.gcda 2>&1 || echo "Exit code: $?"

echo -e "\n${GREEN}Testing with very large threshold${NC}"
gcov-tool overlap -t 100.0 test_program.gcda test_program_diff.gcda || echo "Exit code: $?"

# Step 8: Additional tests with different .gcda generation methods
echo -e "\n${YELLOW}=== Testing with merged .gcda files ===${NC}"

# Create a third .gcda file using gcov-tool merge
echo -e "\n${GREEN}Creating merged .gcda file${NC}"
gcov-tool merge test_program.gcda test_program_diff.gcda merged.gcda || echo "Merge failed"

# Test overlap with merged file
echo -e "\n${GREEN}Testing overlap with merged file${NC}"
gcov-tool overlap -v -f merged.gcda test_program.gcda || echo "Exit code: $?"

# Step 9: Cleanup
echo -e "\n${YELLOW}=== Cleaning up ===${NC}"
cd ..
rm -rf "$TEMP_DIR"
echo "Temporary directory removed: $TEMP_DIR"

echo -e "\n${GREEN}=== All tests completed ===${NC}"
echo "Note: To collect coverage data for gcov-tool itself,"
echo "ensure gcov-tool was built with -fprofile-arcs -ftest-coverage"
echo "and run this script in the appropriate environment."
```

This script systematically tests all the uncovered lines in the switch statement:

1. **Individual flag tests** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t` with argument) - Each directly exercises one case in the switch statement.

2. **Combined flag tests** - Tests multiple flags together to ensure they work in combination.

3. **Error/edge case tests**:
   - Invalid option `-z` triggers the `default` case (line 552-554)
   - `-t` without argument tests error handling
   - Non-numeric argument to `-t`
   - Same file twice
   - Non-existent files
   - Edge values for threshold (0.0, 1.0, negative, large)

4. **Valid GCDA file generation** - Creates two different `.gcda` files by compiling and running two slightly different C programs, ensuring the overlap command has meaningful data to process.

To use this script:
1. Save it as `test_gcov_tool_overlap.sh`
2. Make it executable: `chmod +x test_gcov_tool_overlap.sh`
3. Ensure `gcov-tool` is in your PATH and was built with coverage instrumentation
4. Run the script: `./test_gcov_tool_overlap.sh`

The script will generate coverage data for the test programs, then systematically invoke `gcov-tool overlap` with all the flag combinations needed to exercise the uncovered lines. Each test that triggers an error will still execute the relevant parsing code before failing, so coverage will be recorded.
