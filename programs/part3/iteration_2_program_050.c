Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc by invoking the compiler driver with various state-modifying options:

```bash
#!/bin/sh

# Test script to cover gcc.cc lines 11228-11250
# This script invokes gcc with combinations that modify global state variables
# and force their cleanup/reset between compilations

set -e
TESTDIR="gcc_coverage_test"
mkdir -p "$TESTDIR"
cd "$TESTDIR"

# Create a simple test source file
cat > simple.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Test program\n");
    return 0;
}
EOF

# Create another source file for multiple compilation units
cat > helper.c << 'EOF'
int helper() {
    return 42;
}
EOF

echo "=== Starting GCC driver state coverage test ==="

# 1. Test basic compilation with output naming and dump options
echo "1. Testing basic compilation with dump options..."
gcc -save-temps -dumpdir ./dumps/ -dumpbase test -fdump-rtl-all -O2 -c simple.c -o simple.o 2>/dev/null || true
gcc --sysroot=/ -march=x86-64 -c simple.c -o simple2.o 2>/dev/null || true

# 2. Test driver mode switching and help/version requests
echo "2. Testing driver mode switching..."
gcc --help > /dev/null 2>&1
gcc --version > /dev/null 2>&1
gcc -print-prog-name=cc1 > /dev/null 2>&1
gcc -c simple.c -o simple3.o 2>/dev/null || true

# 3. Test as C++ driver (is_cpp_driver)
echo "3. Testing C++ driver mode..."
g++ -c simple.c -o simple_cpp.o 2>/dev/null || true

# 4. Test complex dump and output naming with multiple invocations
echo "4. Testing complex dump configurations..."
gcc -save-temps=obj -dumpdir ./out -dumpbase base -fdump-tree-all -fdump-ipa-all -c simple.c -o simple4.o 2>/dev/null || true
gcc -dumpbase base.2 -c simple.c -o simple5.o 2>/dev/null || true

# 5. Test with different sysroot and machine specs
echo "5. Testing sysroot and machine spec overrides..."
gcc --sysroot=/usr -specs=/dev/null -march=native -c simple.c -o simple6.o 2>/dev/null || true
gcc -B/usr/bin -c simple.c -o simple7.o 2>/dev/null || true

# 6. Test print_subprocess_help and verbose flags
echo "6. Testing subprocess help and verbose flags..."
gcc -print-prog-name=as > /dev/null 2>&1
gcc -print-prog-name=ld > /dev/null 2>&1
gcc -dumpspecs > /dev/null 2>&1
gcc -v -c simple.c -o simple8.o 2>/dev/null || true

# 7. Test multiple compilations in sequence with varying options
echo "7. Testing sequential compilations with state changes..."
for i in 1 2 3; do
    gcc -save-temps -dumpdir "dump$i/" -dumpbase "seq$i" -fdump-rtl-expand -c simple.c -o "seq$i.o" 2>/dev/null || true
    gcc --sysroot=/ -march=x86-64 -c helper.c -o "helper$i.o" 2>/dev/null || true
done

# 8. Test with at_file_supplied (@file syntax)
echo "8. Testing @file syntax..."
echo "-c simple.c -o atfile.o" > args.txt
gcc @args.txt 2>/dev/null || true

# 9. Test save_temps_flag variations
echo "9. Testing save-temps variations..."
gcc -save-temps=cwd -c simple.c -o temps1.o 2>/dev/null || true
gcc -save-temps=obj -c simple.c -o temps2.o 2>/dev/null || true

# 10. Final compilation to verify driver state is properly reset
echo "10. Final verification compilation..."
gcc simple.o helper.o -o final_test 2>/dev/null || true

# Run the final test if compilation succeeded
if [ -x ./final_test ]; then
    echo "Running final test program..."
    ./final_test
    echo "=== Test completed successfully ==="
else
    echo "=== Test completed (some compilations may have failed on this system) ==="
fi

# Cleanup
echo "Cleaning up test files..."
cd ..
rm -rf "$TESTDIR"

echo "Coverage test script finished."
```

This script systematically tests the uncovered code by:

1. **Multiple compiler invocations with varied output flags**: Uses `-c`, `-o`, `-dumpdir`, `-dumpbase`, `-fdump-*` flags in different combinations across multiple invocations.

2. **System root and spec machine overrides**: Uses `--sysroot=`, `-B`, `-specs=`, `-march=` options to modify `target_system_root` and `spec_machine`.

3. **Driver mode switching**: Invokes `gcc` with `--help`, `--version`, `-print-prog-name=`, and uses `g++` to trigger `is_cpp_driver` changes.

4. **Save-temps and dump directory complexity**: Tests `-save-temps` with different values, `-dumpdir` with trailing slashes, and multiple `-fdump-*` options.

5. **At-file syntax**: Uses `@args.txt` to test `at_file_supplied` logic.

The script creates a temporary directory, generates test source files, runs the compiler with various flag combinations, and cleans up afterward. Each invocation modifies different global state variables in the GCC driver, forcing the cleanup code to execute between compilations or at program exit.

Note: Some options may fail on certain systems (e.g., invalid `--sysroot` paths or unsupported `-march` values), so errors are suppressed with `2>/dev/null || true` to allow the script to continue testing other combinations.
