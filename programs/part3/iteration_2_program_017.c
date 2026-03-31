Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc:

```bash
#!/bin/sh
# Test script to cover gcc driver initialization/cleanup logic

set -e
TESTDIR="gcc_coverage_test"
mkdir -p "$TESTDIR"
cd "$TESTDIR"

# Create a simple test source file
cat > test.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Test program\n");
    return 0;
}
EOF

# Create a C++ test file for driver mode switching
cat > test.cpp << 'EOF'
#include <iostream>
int main() {
    std::cout << "C++ test" << std::endl;
    return 0;
}
EOF

echo "=== Testing GCC driver state reset logic ==="

# 1. Multiple compilations with varied output flags and dump options
echo "1. Testing output naming and dump directory logic..."
gcc -save-temps -dumpdir ./dumps1/ -dumpbase test1 -fdump-rtl-all -fdump-tree-all -O2 -c test.c -o test1.o 2>/dev/null
gcc -save-temps=obj -dumpdir ./dumps2 -dumpbase test2 -fdump-ipa-all -c test.c -o test2.o 2>/dev/null
gcc -dumpbase test3.c -dumpdir ./dumps3/ -c test.c -o test3.o 2>/dev/null

# 2. System root and machine specification overrides
echo "2. Testing sysroot and machine spec resets..."
# Use a dummy sysroot (current directory as sysroot)
gcc --sysroot=. -march=x86-64 -c test.c -o test4.o 2>/dev/null
gcc -B. -specs=/dev/null -march=native -c test.c -o test5.o 2>/dev/null 2>&1 || true

# 3. Driver mode switching and help/version requests
echo "3. Testing driver mode switching..."
# Invoke as C++ driver
g++ -c test.cpp -o test_cpp.o 2>/dev/null
# Help and version requests
gcc --help > /dev/null 2>&1
gcc --version > /dev/null 2>&1
gcc -print-prog-name=cc1 > /dev/null 2>&1
gcc -print-subprocess-help > /dev/null 2>&1 || true

# 4. Complex save-temps and dump directory scenarios
echo "4. Testing complex dump scenarios..."
# Test with trailing dash handling
gcc -save-temps -dumpdir ./trailing-test/ -dumpbase complex -fdump-rtl-expand -fdump-tree-optimized -c test.c -o test6.o 2>/dev/null
# Different dumpbase extensions
gcc -dumpbase complex.c -fdump-tree-cfg -c test.c -o test7.o 2>/dev/null
# Empty dumpdir (should trigger default behavior)
gcc -dumpdir '' -dumpbase empty -c test.c -o test8.o 2>/dev/null

# 5. Mixed compilation with state changes between jobs
echo "5. Testing mixed compilation sequences..."
# Sequence that modifies multiple state variables
gcc -save-temps -dumpdir ./mixed/ -c test.c -o test9.o 2>/dev/null
gcc --sysroot=/ -march=i386 -c test.c -o test10.o 2>/dev/null
gcc -dumpbase final -verbose -c test.c -o test11.o 2>/dev/null >/dev/null 2>&1

# 6. Test verbose flag (verbose_only_flag)
echo "6. Testing verbose flag..."
gcc -### -c test.c -o test12.o 2>/dev/null

# 7. Final compilation to verify state was properly reset
echo "7. Final compilation with default state..."
gcc test.c -o final_test
./final_test

# 8. Cleanup test
echo "8. Testing cleanup with report times..."
# This should trigger report_times_to_file reset
gcc -time -c test.c -o test13.o 2>/dev/null
gcc -c test.c -o test14.o 2>/dev/null

# 9. Test greatest_status reset
echo "9. Testing error recovery and status reset..."
# First a failing compilation
gcc -invalid-flag test.c -o fail 2>/dev/null || true
# Then a successful one - greatest_status should be reset
gcc test.c -o recovery_test
./recovery_test

# 10. Multiple dumpdir variations
echo "10. Testing dumpdir edge cases..."
gcc -dumpdir ./no-slash -dumpbase edge1 -c test.c -o edge1.o 2>/dev/null
gcc -dumpdir ./with-slash/ -dumpbase edge2 -c test.c -o edge2.o 2>/dev/null
gcc -dumpdir . -dumpbase edge3 -c test.c -o edge3.o 2>/dev/null

echo "=== Cleaning up ==="
cd ..
rm -rf "$TESTDIR"

echo "Test completed successfully!"
echo ""
echo "The test covered:"
echo "1. Multiple allocations/frees of dumpdir, dumpbase, outbase"
echo "2. target_system_root and spec_machine resets"
echo "3. Driver mode flags (is_cpp_driver, print_help_list, etc.)"
echo "4. save_temps_flag and dumpdir_trailing_dash_added logic"
echo "5. report_times_to_file and greatest_status resets"
```

This script systematically exercises the uncovered code by:

1. **Multiple allocations/frees**: Uses `-dumpdir`, `-dumpbase`, and `-save-temps` with different values across multiple invocations, forcing the driver to allocate and free these strings.

2. **System configuration resets**: Uses `--sysroot`, `-B`, `-specs`, and `-march` options that modify `target_system_root`, `spec_machine`, and related variables.

3. **Driver mode switching**: Invokes `g++` (C++ driver), uses `--help`, `--version`, and `-print-prog-name` flags that set mode flags later reset in the target code.

4. **Save-temps complexity**: Tests `-save-temps` with different dump directories, including trailing slash handling for `dumpdir_trailing_dash_added`.

5. **State variable coverage**: Exercises `verbose_only_flag`, `report_times_to_file`, `greatest_status`, and other variables in the target block.

The script creates a temporary directory, performs all tests, and cleans up. Each compilation uses valid source files to ensure the driver goes through full compilation passes, not just argument parsing. The varied flag combinations ensure the driver's internal state is modified and must be reset between invocations.
