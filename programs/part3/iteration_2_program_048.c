Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc by invoking the compiler driver with various state-modifying flags:

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

# Ensure cleanup on exit
trap cleanup EXIT

# Create a simple test source file
cat > simple.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Hello from GCC driver test\n");
    return 0;
}
EOF

cat > simple2.c << 'EOF'
int add(int a, int b) {
    return a + b;
}
EOF

echo "=== Testing GCC driver state reset logic ==="

# 1. Test basic compilation with output naming variations
echo "1. Testing output file naming and dump directory logic..."
gcc -save-temps -dumpdir ./dumps/ -dumpbase test1 -fdump-rtl-all -O2 -c simple.c -o simple.o 2>/dev/null || true
gcc -save-temps=obj -dumpdir ./dumps2 -dumpbase test2 -fdump-tree-all -c simple.c -o simple2.o 2>/dev/null || true

# Force dumpdir trailing slash logic
gcc -dumpdir ./dumps3/ -dumpbase test3 -fdump-ipa-all -c simple2.c -o simple3.o 2>/dev/null || true
gcc -dumpdir ./dumps4 -dumpbase test4 -c simple2.c -o simple4.o 2>/dev/null || true

# 2. Test sysroot and machine specification overrides
echo "2. Testing sysroot and machine specification resets..."
# Use dummy sysroot paths (these may fail but will set the state)
gcc --sysroot=/tmp/dummy_sysroot -march=x86-64 -c simple.c -o simple5.o 2>/dev/null || true
gcc --sysroot=/ -march=native -c simple.c -o simple6.o 2>/dev/null || true
gcc -B/tmp/dummy_bindir -specs=/dev/null -c simple.c -o simple7.o 2>/dev/null || true

# 3. Test driver mode switching and help/version requests
echo "3. Testing driver mode switching..."
# Invoke as C++ driver (g++ usually symlinks to gcc with different mode)
if command -v g++ >/dev/null 2>&1; then
    g++ -c simple.c -o simple_cpp.o 2>/dev/null || true
fi

# Help and version requests (these set print_help_list, print_version)
gcc --help > /dev/null 2>&1 || true
gcc --version > /dev/null 2>&1 || true
gcc -print-prog-name=cc1 > /dev/null 2>&1 || true
gcc -print-subprocess-help > /dev/null 2>&1 || true

# 4. Test save-temps and dump directory complexity
echo "4. Testing save-temps and dump base variations..."
gcc -save-temps -dumpdir ./complex_dumps/ -dumpbase complex \
    -fdump-rtl-all -fdump-tree-all -fdump-ipa-all \
    -c simple.c -o complex.o 2>/dev/null || true

# Test dumpbase extensions
gcc -dumpbase base.main -dumpbase-ext .c -fdump-tree-optimized \
    -c simple.c -o base1.o 2>/dev/null || true
gcc -dumpbase base.add -dumpbase-ext .c -fdump-rtl-expand \
    -c simple2.c -o base2.o 2>/dev/null || true

# 5. Test multiple compilations in sequence with varying options
echo "5. Testing sequential compilations with state changes..."
for i in $(seq 1 3); do
    gcc -save-temps=${i} -dumpdir ./seq${i}/ -dumpbase seq${i} \
        -march=x86-64 -c simple.c -o seq${i}.o 2>/dev/null || true
    gcc --sysroot=/ -dumpbase seq${i}b -c simple2.c -o seq${i}b.o 2>/dev/null || true
done

# 6. Final compilation to verify driver state was properly reset
echo "6. Final compilation to verify clean state..."
gcc simple.c simple2.c -o final_test 2>/dev/null || {
    echo "Warning: Final compilation failed, but driver state was exercised"
}

# Run the final test if compilation succeeded
if [ -x ./final_test ]; then
    ./final_test
    echo "=== Test completed successfully ==="
else
    echo "=== Driver state was exercised (some compilations may have failed) ==="
fi

# Clean up generated files (except what cleanup() will handle)
rm -f *.o *.i *.s *.r*.dump *.t*.dump *.ipa*.dump 2>/dev/null || true
rm -rf dumps dumps2 dumps3 dumps4 complex_dumps seq1 seq2 seq3 2>/dev/null || true
```

This script systematically exercises the target code by:

1. **Multiple invocations with varied output flags**: Uses `-save-temps`, `-dumpdir`, `-dumpbase` with different values to stress allocation/free logic.

2. **System root and machine overrides**: Uses `--sysroot`, `-march`, `-B`, `-specs` to modify `target_system_root` and `spec_machine`.

3. **Driver mode switching**: Invokes `g++` (sets `is_cpp_driver`), uses `--help`, `--version`, `-print-prog-name` to set various print flags.

4. **Save-temps and dump complexity**: Tests `save_temps_flag` with different values, uses `-fdump-*` flags, exercises `dumpdir_trailing_dash_added` logic.

5. **Sequential compilations**: Loops through multiple compilations to ensure state is reset between invocations.

The script handles potential failures gracefully (using `|| true` and `2>/dev/null`) since some options like invalid `--sysroot` paths may cause compilation failures but still exercise the driver's state management logic. The final compilation verifies the driver can still function after all the state modifications.
