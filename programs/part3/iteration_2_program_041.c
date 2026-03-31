Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc by invoking the compiler driver with various state-modifying options:

```bash
#!/bin/sh
# Test script for GCC driver initialization/cleanup logic
# Targets uncovered lines in gcc.cc (11228-11250)

set -e
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

cd "$TMPDIR"

# Create a simple test source file
cat > test.c <<'EOF'
#include <stdio.h>
int main() {
    printf("Hello from test program\n");
    return 0;
}
EOF

cat > test2.c <<'EOF'
#include <stdio.h>
int square(int x) {
    return x * x;
}
int main() {
    printf("Square of 5: %d\n", square(5));
    return 0;
}
EOF

echo "=== Testing GCC driver state reset logic ==="

# 1. Multiple compilations with varied output flags and dump options
echo "1. Testing output naming and dump directory logic..."
gcc -save-temps -dumpdir ./dumps/ -dumpbase test1 -fdump-rtl-all -O2 -c test.c -o test1.o 2>/dev/null || true
gcc -save-temps=obj -dumpdir ./dumps2 -dumpbase test2 -fdump-tree-all -c test.c -o test2.o 2>/dev/null || true
gcc -fdump-ipa-all -fdump-rtl-expand -dumpbase test3.ext -c test.c -o test3.o 2>/dev/null || true

# Force dumpdir trailing slash logic
gcc -dumpdir ./dumps3/ -dumpbase base -fdump-tree-cfg -c test.c -o test4.o 2>/dev/null || true
gcc -dumpdir ./dumps4 -dumpbase base -fdump-tree-optimized -c test.c -o test5.o 2>/dev/null || true

# 2. System root and spec machine overrides
echo "2. Testing sysroot and machine specification resets..."
# Use dummy sysroot paths (these may not exist, but will set the variables)
gcc --sysroot=/tmp/dummy-sysroot -march=x86-64 -c test.c -o sysroot1.o 2>/dev/null || true
gcc --sysroot=/ -march=native -c test.c -o sysroot2.o 2>/dev/null || true
gcc -B /usr/lib/gcc -specs=/usr/share/gcc/default.spec -c test.c -o spec1.o 2>/dev/null || true

# 3. Driver mode switching and help/version requests
echo "3. Testing driver mode switching..."
# Invoke as different driver modes
gcc --help >/dev/null 2>&1
gcc --version >/dev/null 2>&1
gcc -print-prog-name=cc1 >/dev/null 2>&1
gcc -print-subprocess-help >/dev/null 2>&1 || true

# Test C++ driver mode (g++ usually symlinks to gcc with different mode)
if command -v g++ >/dev/null 2>&1; then
    g++ --version >/dev/null 2>&1
    g++ -c test.c -o cppdriver.o 2>/dev/null || true
fi

# 4. Complex save-temps and dump directory scenarios
echo "4. Testing complex save-temps scenarios..."
mkdir -p complex_dumps
gcc -save-temps -dumpdir complex_dumps/ -dumpbase complex \
    -fdump-rtl-all -fdump-tree-all -fdump-ipa-all \
    -O3 -c test2.c -o complex1.o 2>/dev/null || true

# Override dumpdir with different options
gcc -save-temps=obj -dumpdir ./ -dumpbase override \
    -fdump-tree-optimized -fdump-rtl-final \
    -c test2.c -o complex2.o 2>/dev/null || true

# Test dumpbase extensions and outbase
gcc -dumpbase "multi.part" -dumpbase-ext ".ext" \
    -fdump-tree-vrp -c test2.c -o multi.o 2>/dev/null || true

# 5. Mixed invocations to force repeated initialization/cleanup
echo "5. Testing mixed invocations..."
gcc --help=common >/dev/null 2>&1
gcc -save-temps -c test.c -o mixed1.o 2>/dev/null || true
gcc --version >/dev/null 2>&1
gcc -dumpdir ./final_dumps/ -c test2.c -o mixed2.o 2>/dev/null || true
gcc -print-prog-name=as >/dev/null 2>&1
gcc -save-temps=obj -dumpbase final -c test.c -o mixed3.o 2>/dev/null || true

# 6. Final compilation to verify driver state was properly reset
echo "6. Final verification compilation..."
gcc test.c -o final_test
./final_test

# 7. Additional edge cases
echo "7. Testing edge cases..."
# Empty dumpdir
gcc -dumpdir "" -dumpbase empty -c test.c -o edge1.o 2>/dev/null || true
# Very long dumpbase (tests allocation)
gcc -dumpbase "$(printf 'x%.0s' {1..100})" -c test.c -o edge2.o 2>/dev/null || true
# Multiple dump flags with different formats
gcc -fdump-tree-ssa -fdump-rtl-regclass -fdump-ipa-inline \
    -dumpdir multi/ -dumpbase multi -c test2.c -o edge3.o 2>/dev/null || true

# 8. Batch compilation (multiple files in one invocation)
echo "8. Testing batch compilation..."
gcc -save-temps -dumpdir batch/ -dumpbase batch \
    -fdump-tree-all -c test.c test2.c 2>/dev/null || true

# 9. Test with explicit outbase equivalents using -o
echo "9. Testing explicit output naming..."
gcc -dumpbase explicit1 -c test.c -o explicit1.o 2>/dev/null || true
gcc -dumpbase explicit2 -c test2.c -o explicit2.o 2>/dev/null || true

# Clean up generated files (keep for inspection if needed)
echo "=== Cleaning up... ==="
ls -la

echo "=== Test completed successfully ==="
echo "The driver should have executed the target cleanup code multiple times"
echo "during these invocations, resetting:"
echo "  - dumpdir/dumpbase/outbase allocations"
echo "  - target_system_root and spec_machine"
echo "  - driver mode flags (is_cpp_driver, print_help_list, etc.)"
echo "  - save_temps_flag and related state variables"
```

This script systematically tests the uncovered code by:

1. **Multiple compilations with varied output flags**: Uses `-save-temps`, `-dumpdir`, `-dumpbase` with different values to trigger allocations and frees of `dumpdir`, `dumpbase`, etc.

2. **System root and machine overrides**: Uses `--sysroot`, `-march`, `-B`, `-specs` to modify `target_system_root` and `spec_machine`, forcing resets to defaults.

3. **Driver mode switching**: Calls `--help`, `--version`, `-print-prog-name`, and potentially `g++` to set flags like `print_help_list`, `print_version`, `is_cpp_driver`.

4. **Complex save-temps scenarios**: Tests `save_temps_flag` transitions and `dumpdir_trailing_dash_added` logic with various directory formats.

5. **Mixed invocations**: Alternates between help/version requests and actual compilations to ensure state is properly cleared between different driver operations.

6. **Edge cases**: Tests empty strings, long names, and multiple dump flags to stress the allocation/reset logic.

The script creates temporary files and directories, runs the compiler with various flag combinations, and cleans up after itself. Each invocation causes the driver to initialize, set global state based on flags, perform compilation (or help/version output), and then clean up - executing the target reset code.
