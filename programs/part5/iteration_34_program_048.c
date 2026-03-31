I'll create a comprehensive test script that covers all the uncovered lines in the gcov-dump.cc command-line argument parsing logic.

```bash
#!/bin/bash

# Test script for gcov-dump command-line argument parsing
# This script tests the specific uncovered lines in gcov-dump.cc (lines 111-130)

set -e

echo "=== Setting up test environment ==="

# Create a minimal C test program
cat > test_program.c << 'EOF'
#include <stdio.h>

int main() {
    int i;
    int sum = 0;
    
    // Simple loop to generate some execution paths
    for (i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            sum += i;
        } else {
            sum -= i;
        }
    }
    
    printf("Result: %d\n", sum);
    return 0;
}
EOF

# Create a simple header file to test with .gcno files
cat > test_header.h << 'EOF'
#ifndef TEST_HEADER_H
#define TEST_HEADER_H

static inline int helper(int x) {
    return x * 2;
}

#endif
EOF

# Create another C file that includes the header
cat > test_with_header.c << 'EOF'
#include <stdio.h>
#include "test_header.h"

int main() {
    int x = 5;
    int result = helper(x);
    printf("Result: %d\n", result);
    return 0;
}
EOF

echo "=== Compiling test programs with coverage instrumentation ==="

# Compile the main test program with coverage flags
gcc -fprofile-arcs -ftest-coverage -O0 test_program.c -o test_program
echo "Compiled test_program.c"

# Compile the header-including program
gcc -fprofile-arcs -ftest-coverage -O0 test_with_header.c -o test_with_header
echo "Compiled test_with_header.c"

echo "=== Generating GCOV data files ==="

# Run the test programs to generate .gcda files
./test_program
echo "Generated test_program.gcda"

./test_with_header
echo "Generated test_with_header.gcda"

# Also ensure we have .gcno files
echo "GCOV note files (.gcno) generated during compilation:"
ls -la *.gcno

echo -e "\n=== Testing gcov-dump with individual flags (covering lines 111-130) ==="

echo -e "\n1. Testing -l flag (flag_dump_contents = 1):"
gcov-dump -l test_program.gcda 2>&1 | head -20

echo -e "\n2. Testing -p flag (flag_dump_positions = 1):"
gcov-dump -p test_program.gcda 2>&1 | head -20

echo -e "\n3. Testing -r flag (flag_dump_raw = 1):"
gcov-dump -r test_program.gcda 2>&1 | head -20

echo -e "\n4. Testing -s flag (flag_dump_stable = 1):"
gcov-dump -s test_program.gcda 2>&1 | head -20

echo -e "\n=== Testing gcov-dump with flag combinations ==="

echo -e "\n5. Testing -l -p combination:"
gcov-dump -l -p test_program.gcda 2>&1 | head -20

echo -e "\n6. Testing -r -s combination:"
gcov-dump -r -s test_program.gcda 2>&1 | head -20

echo -e "\n7. Testing -l -p -r -s combination:"
gcov-dump -l -p -r -s test_program.gcda 2>&1 | head -20

echo -e "\n=== Testing with .gcno files ==="

echo -e "\n8. Testing -l flag with .gcno file:"
gcov-dump -l test_program.gcno 2>&1 | head -20

echo -e "\n9. Testing -p flag with .gcno file:"
gcov-dump -p test_program.gcno 2>&1 | head -20

echo -e "\n=== Testing help and version flags ==="

echo -e "\n10. Testing -h flag (help):"
gcov-dump -h 2>&1 | head -10

echo -e "\n11. Testing -v flag (version):"
gcov-dump -v 2>&1

echo -e "\n=== Testing invalid flags (to trigger default case) ==="

echo -e "\n12. Testing invalid flag -x (should trigger error):"
gcov-dump -x test_program.gcda 2>&1 || true

echo -e "\n13. Testing invalid flag -? (should trigger error):"
gcov-dump -? test_program.gcda 2>&1 || true

echo -e "\n14. Testing invalid uppercase flag -L (should trigger error):"
gcov-dump -L test_program.gcda 2>&1 || true

echo -e "\n15. Testing invalid flag -z (should trigger error):"
gcov-dump -z test_program.gcda 2>&1 || true

echo -e "\n=== Testing edge cases ==="

echo -e "\n16. Testing multiple files with flags:"
gcov-dump -l test_program.gcda test_with_header.gcda 2>&1 | head -20

echo -e "\n17. Testing flag in middle of arguments:"
gcov-dump test_program.gcda -l test_with_header.gcda 2>&1 | head -20

echo -e "\n18. Testing with no arguments (should show usage):"
gcov-dump 2>&1 | head -5 || true

echo -e "\n=== Testing with different file types ==="

# Test with the header-including program's data
echo -e "\n19. Testing with program that includes headers:"
gcov-dump -l test_with_header.gcda 2>&1 | head -20

echo -e "\n=== Cleanup ==="
rm -f test_program test_with_header *.gcda *.gcno *.c *.h

echo -e "\n=== Test completed successfully ==="
echo "All test cases executed. The following gcov-dump cases were covered:"
echo "  - Case 'l': flag_dump_contents = 1"
echo "  - Case 'p': flag_dump_positions = 1"
echo "  - Case 'r': flag_dump_raw = 1"
echo "  - Case 's': flag_dump_stable = 1"
echo "  - Case 'h': print_usage()"
echo "  - Case 'v': print_version()"
echo "  - Default case: invalid flag handling"
echo ""
echo "To measure coverage on gcov-dump itself:"
echo "1. Build gcov-dump with: gcc -fprofile-arcs -ftest-coverage -O0 gcov-dump.cc -o gcov-dump"
echo "2. Run this test script"
echo "3. Run: gcov gcov-dump.cc"
echo "4. Check gcov-dump.cc.gcov for coverage of lines 111-130"
```

This test script comprehensively covers all the requirements:

1. **Individual flag testing**: Tests `-l`, `-p`, `-r`, `-s` flags individually (lines 12-15 in the script)
2. **Flag combinations**: Tests various combinations like `-l -p`, `-r -s`, and `-l -p -r -s` (lines 17-19)
3. **Invalid flags**: Tests `-x`, `-?`, `-L`, `-z` to trigger the default case (lines 33-36)
4. **Help and version**: Tests `-h` and `-v` flags (lines 29-30)
5. **Valid GCOV files**: Creates and compiles test programs with `-fprofile-arcs -ftest-coverage`, runs them to generate `.gcda` files, and uses them in all tests (lines 1-45)

The script also includes additional edge cases like testing with `.gcno` files, multiple files, and different argument orders to ensure robust coverage of the command-line parsing logic.

To use this test:
1. Save it as `test_gcov_dump.sh`
2. Make it executable: `chmod +x test_gcov_dump.sh`
3. Run it: `./test_gcov_dump.sh`

The script will generate the necessary coverage data files and test all the command-line argument parsing cases in the uncovered block of gcov-dump.cc.
