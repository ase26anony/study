Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc by invoking the compiler driver with various state-modifying options:

```bash
#!/bin/sh
# Test script for GCC driver initialization/cleanup logic coverage

set -e
TESTDIR="gcc_driver_test_$$"
mkdir -p "$TESTDIR"
cd "$TESTDIR" || exit 1

# Cleanup function
cleanup() {
    cd ..
    rm -rf "$TESTDIR"
}
trap cleanup EXIT

# Create a simple test source file
cat > test.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Test program\n");
    return 0;
}
EOF

# Create a simple C++ test file for driver mode switching
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
gcc -save-temps -dumpdir "./dumps/" -dumpbase "test1" -fdump-rtl-all -O2 -c test.c -o test1.o 2>/dev/null
gcc -save-temps=obj -dumpdir "./dumps2" -dumpbase "test2" -fdump-tree-all -fdump-ipa-all -c test.c -o test2.o 2>/dev/null
gcc -dumpbase "test3.c" -fdump-rtl-expand -c test.c -o test3.o 2>/dev/null

# Force dumpdir trailing slash logic
gcc -dumpdir "./dumps3" -dumpbase "test4" -fdump-tree-optimized -c test.c -o test4.o 2>/dev/null
gcc -dumpdir "./dumps3/" -dumpbase "test5" -fdump-rtl-final -c test.c -o test5.o 2>/dev/null

# 2. System root and machine specification overrides
echo "2. Testing sysroot and machine spec reset..."
# Use dummy sysroot paths (will fall back to default)
gcc --sysroot="/tmp/dummy_sysroot" -march=x86-64 -c test.c -o test_sysroot1.o 2>/dev/null
gcc --sysroot="" -march=native -c test.c -o test_sysroot2.o 2>/dev/null
gcc -B "/usr/bin" -specs="/dev/null" -c test.c -o test_specs.o 2>/dev/null 2>&1 || true

# 3. Driver mode switching and help/version requests
echo "3. Testing driver mode switching..."
# Invoke as C++ driver (touches is_cpp_driver)
g++ -c test.cpp -o test_cpp.o 2>/dev/null

# Help and version requests (set print_help_list, print_version)
gcc --help > /dev/null 2>&1
gcc --version > /dev/null 2>&1
gcc -print-prog-name=cc1 > /dev/null 2>&1
gcc -print-subprocess-help > /dev/null 2>&1

# Follow with actual compilation to trigger reset
gcc -c test.c -o test_after_help.o 2>/dev/null

# 4. Complex save-temps and dump combinations
echo "4. Testing save-temps and dump complexity..."
gcc -save-temps -dumpdir "./complex_dumps/" -dumpbase "complex" \
    -fdump-rtl-all -fdump-tree-all -fdump-ipa-all \
    -O3 -c test.c -o complex.o 2>/dev/null

# Change dumpbase extensions
gcc -dumpbase "complex.alt" -dumpbase-ext ".c" -fdump-tree-cfg -c test.c -o complex2.o 2>/dev/null

# Test save_temps_flag variations
gcc -save-temps=cwd -c test.c -o temp_cwd.o 2>/dev/null
gcc -save-temps=obj -dumpdir "." -c test.c -o temp_obj.o 2>/dev/null

# 5. Mixed invocations to stress cleanup between jobs
echo "5. Testing mixed invocations..."
# Sequence that modifies multiple state variables
gcc --sysroot="/" -march=x86-64 -save-temps -dumpdir "./mixed/" -dumpbase "mixed1" -c test.c -o mixed1.o 2>/dev/null
gcc --help=optimizers > /dev/null 2>&1
gcc -save-temps=obj -dumpbase "mixed2" -c test.c -o mixed2.o 2>/dev/null
gcc -print-prog-name=as > /dev/null 2>&1
gcc -dumpdir "./final/" -dumpbase "final" -c test.c -o final.o 2>/dev/null

# 6. Final compilation and link to verify proper reset
echo "6. Final compilation and execution test..."
# This should work with default state after all resets
gcc test1.o test2.o test3.o test4.o test5.o \
    test_sysroot1.o test_sysroot2.o test_specs.o \
    test_cpp.o test_after_help.o \
    complex.o complex2.o temp_cwd.o temp_obj.o \
    mixed1.o mixed2.o final.o \
    -o combined_test 2>/dev/null || true

# Create and run a simple final test to ensure driver works correctly
gcc test.c -o simple_test
./simple_test

echo "=== Test completed successfully ==="
echo "Generated files in: $TESTDIR"
echo "Driver state reset logic should have been exercised multiple times."

# List generated dump files to show dumpdir/dumpbase worked
echo -e "\nGenerated dump files (sample):"
find . -name "*.dump" -o -name "*.i" -o -name "*.s" -o -name "*.o" 2>/dev/null | head -20
```

This script systematically exercises the target code by:

1. **Multiple compilations with output/dump flags**: Uses `-dumpdir`, `-dumpbase`, `-fdump-*` flags with different values to stress allocation/free logic for `dumpdir`, `dumpbase`, `dumpbase_ext`, and `outbase`.

2. **System root and machine spec overrides**: Uses `--sysroot`, `-march`, `-B`, `-specs` to modify `target_system_root`, `spec_machine`, and related flags.

3. **Driver mode switching**: Invokes `g++` (C++ driver), `--help`, `--version`, `-print-prog-name`, `-print-subprocess-help` to set flags like `is_cpp_driver`, `print_help_list`, `print_version`, `print_subprocess_help`.

4. **Save-temps complexity**: Uses `-save-temps` with different modes (`cwd`, `obj`) and combinations with dump flags to exercise `save_temps_flag` and `save_temps_overrides_dumpdir` logic.

5. **Trailing slash handling**: Tests `dumpdir_trailing_dash_added` by using paths with and without trailing slashes.

6. **Mixed invocations**: Sequences that modify multiple state variables between compilations to ensure proper reset between driver invocations.

The script creates a temporary directory, generates test files, runs the compiler with various flag combinations, and cleans up after itself. Each `gcc` invocation modifies driver state, and the subsequent invocation (or program exit) triggers the cleanup code in the uncovered block.
