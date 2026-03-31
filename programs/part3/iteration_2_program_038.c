Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc by performing multiple compiler invocations with varied state-modifying flags:

```bash
#!/bin/sh

# Test script to cover GCC driver initialization/cleanup logic
# Specifically targets lines 11228-11250 in gcc.cc

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
gcc -save-temps -dumpdir "./dumps/" -dumpbase "test1" -fdump-rtl-all -O2 -c test.c -o test1.o 2>/dev/null
gcc -save-temps=obj -dumpdir "./out/" -dumpbase "test2" -fdump-tree-all -c test.c -o test2.o 2>/dev/null
gcc -dumpbase "test3.ext" -fdump-ipa-all -c test.c -o test3.o 2>/dev/null
gcc -dumpdir "./" -dumpbase "" -c test.c -o test4.o 2>/dev/null

# 2. System root and machine specification overrides
echo "2. Testing sysroot and machine spec reset..."
# Note: Using / as sysroot since it should exist on all systems
gcc --sysroot=/ -march=x86-64 -c test.c -o test5.o 2>/dev/null
gcc -B./ -specs=/dev/null -march=native -c test.c -o test6.o 2>/dev/null 2>&1 || true

# 3. Driver mode switching and help/version requests
echo "3. Testing driver mode switching..."
gcc --help > /dev/null 2>&1
gcc --version > /dev/null 2>&1
gcc -print-prog-name=cc1 > /dev/null 2>&1
gcc -print-subprocess-help > /dev/null 2>&1

# Test as C++ driver (g++ mode)
g++ -c test.cpp -o test_cpp.o 2>/dev/null

# 4. Complex save-temps and dump directory scenarios
echo "4. Testing complex dump scenarios..."
mkdir -p complex_dump
gcc -save-temps -dumpdir "complex_dump/" -dumpbase "complex" \
    -fdump-rtl-all -fdump-tree-all -fdump-ipa-all \
    -O2 -c test.c -o complex.o 2>/dev/null

# Test trailing slash handling (dumpdir_trailing_dash_added)
gcc -save-temps -dumpdir "complex_dump" -dumpbase "notrail" \
    -c test.c -o notrail.o 2>/dev/null

# 5. Mixed invocations to force multiple cleanup cycles
echo "5. Testing mixed invocations..."
# Help then compilation
gcc --help=common > /dev/null 2>&1
gcc -save-temps -c test.c -o mixed1.o 2>/dev/null

# Version then compilation with different options
gcc --version > /dev/null 2>&1
gcc --sysroot=/ -dumpdir "./" -c test.c -o mixed2.o 2>/dev/null

# Subprocess help then complex compilation
gcc -print-prog-name=as > /dev/null 2>&1
gcc -save-temps=obj -dumpbase "final" -fdump-tree-all -O3 -c test.c -o final.o 2>/dev/null

# 6. Final compilation to verify driver state was properly reset
echo "6. Final verification compilation..."
gcc test.c -o test_program 2>/dev/null

# Run the test program to verify everything works
if [ -x ./test_program ]; then
    ./test_program
    echo "✓ All compilations completed successfully"
else
    echo "✗ Final compilation failed"
    exit 1
fi

echo ""
echo "=== Generated files for inspection ==="
find . -type f -name "*.o" -o -name "*.i" -o -name "*.s" -o -name "*.dump*" | head -20
echo ""

echo "Test completed. The driver's initialization/cleanup logic should have been"
echo "exercised multiple times with different state configurations."
```

This script systematically tests the uncovered code by:

1. **Multiple output naming variations**: Uses `-dumpdir`, `-dumpbase`, `-save-temps` with different values to exercise the allocation/free logic for `dumpdir`, `dumpbase`, `dumpbase_ext`, and `outbase`.

2. **System root and machine spec overrides**: Uses `--sysroot=/`, `-march=`, `-B`, and `-specs=` to modify `target_system_root` and `spec_machine`, forcing resets to defaults.

3. **Driver mode switching**: Calls `gcc` with `--help`, `--version`, `-print-prog-name`, and uses `g++` to trigger `is_cpp_driver` and help/version flag resets.

4. **Complex dump scenarios**: Creates multiple dump files with different flags to test `save_temps_flag`, `dumpdir_trailing_dash_added`, and the various dump-related allocations.

5. **Mixed invocations**: Alternates between help/version requests and actual compilations to ensure state is properly cleared between different driver operations.

6. **Final verification**: Compiles and runs a test program to ensure the driver is in a clean state after all operations.

The script creates a temporary directory, cleans up after itself, and provides feedback about what was tested. Each compilation uses valid source files to ensure the driver goes through the full compilation pipeline, not just argument parsing.
