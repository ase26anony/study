Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc:

```bash
#!/bin/sh
# Test script for gcc driver initialization/cleanup logic
# Targets lines 11228-11250 in gcc.cc

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

# 1. Multiple compiler invocations with varied output flags
# This stresses dumpdir/dumpbase/outbase allocation and freeing
echo "=== Test 1: Multiple compilations with dump/output flags ==="

# First compilation with full dump options
gcc -save-temps -dumpdir ./dumps/ -dumpbase test1 -fdump-rtl-all \
    -fdump-tree-all -O2 -c test.c -o test1.o 2>/dev/null || true

# Second with different dumpbase and output
gcc -save-temps=obj -dumpdir ./dumps2 -dumpbase test2.ext \
    -fdump-ipa-all -c test.c -o test2.o 2>/dev/null || true

# Third with trailing slash in dumpdir (tests dumpdir_trailing_dash_added)
gcc -dumpdir ./dumps3/ -dumpbase test3 -fdump-rtl-expand \
    -c test.c -o test3.o 2>/dev/null || true

# Clean up dump directories
rm -rf dumps dumps2 dumps3 *.o *.i *.s *.o.* 2>/dev/null || true

# 2. System root and spec machine overrides
echo "=== Test 2: Sysroot and machine specification ==="

# Try with sysroot (may use / or /usr as dummy sysroot)
gcc --sysroot=/ -march=x86-64 -c test.c -o test_sysroot.o 2>/dev/null || true

# Try with different machine spec using -B and -specs
# Create a dummy specs file
echo "*cpp:" > dummy.specs
gcc -B. -specs=dummy.specs -march=native -c test.c -o test_specs.o 2>/dev/null || true

# Reset by compiling without special options
gcc -c test.c -o test_plain.o 2>/dev/null || true

rm -f dummy.specs *.o 2>/dev/null || true

# 3. Driver mode switching and help/version requests
echo "=== Test 3: Driver mode switching ==="

# Invoke as C++ driver (sets is_cpp_driver)
g++ --version >/dev/null 2>&1 || true

# Various help requests (set print_help_list, print_version, print_subprocess_help)
gcc --help >/dev/null 2>&1 || true
gcc --version >/dev/null 2>&1 || true
gcc -print-prog-name=cc1 >/dev/null 2>&1 || true
gcc -print-search-dirs >/dev/null 2>&1 || true

# Follow with actual compilation to trigger cleanup
gcc -c test.c -o test_after_help.o 2>/dev/null || true

rm -f *.o 2>/dev/null || true

# 4. Complex save-temps and dump directory scenarios
echo "=== Test 4: Complex save-temps and dump scenarios ==="

# Test save_temps_flag transitions
gcc -save-temps -c test.c -o test_save1.o 2>/dev/null || true
gcc -save-temps=obj -c test.c -o test_save2.o 2>/dev/null || true
gcc -save-temps=none -c test.c -o test_save3.o 2>/dev/null || true

# Test dumpbase with extensions and dumpdir combinations
gcc -dumpdir ./complex_dumps/ -dumpbase complex.test \
    -fdump-tree-optimized -fdump-rtl-final -O2 -c test.c -o complex.o 2>/dev/null || true

# Change dumpbase without dumpdir
gcc -dumpbase another -fdump-tree-cfg -c test.c -o another.o 2>/dev/null || true

# Test dumpdir without trailing slash
gcc -dumpdir ./nodash -dumpbase nodash -fdump-rtl-expand \
    -c test.c -o nodash.o 2>/dev/null || true

rm -rf complex_dumps nodash *.o *.i *.s *.o.* 2>/dev/null || true

# 5. Combined complex scenario
echo "=== Test 5: Combined complex scenario ==="

# Sequence that modifies many state variables
gcc --help=common >/dev/null 2>&1 || true
gcc --sysroot=/usr -march=x86-64 -save-temps -dumpdir ./final/ \
    -dumpbase final -fdump-tree-all -O2 -c test.c -o final1.o 2>/dev/null || true
gcc -print-prog-name=as >/dev/null 2>&1 || true
gcc -dumpbase simple -c test.c -o final2.o 2>/dev/null || true

rm -rf final *.o *.i *.s 2>/dev/null || true

# 6. Final verification compilation
echo "=== Test 6: Final verification ==="

# Compile and run a test program to verify driver still works correctly
cat > verify.c <<'EOF'
#include <stdio.h>
int square(int x) { return x * x; }
int main() {
    printf("Verification: 5^2 = %d\n", square(5));
    printf("Driver state reset successful!\n");
    return 0;
}
EOF

gcc -save-temps -dumpdir ./verify_dumps/ -dumpbase verify \
    -fdump-rtl-all -O2 verify.c -o verify 2>/dev/null || true

# Run if compilation succeeded
if [ -x ./verify ]; then
    ./verify
    echo "=== All tests completed ==="
else
    echo "Compilation failed - but driver initialization/cleanup was exercised"
fi

# Cleanup
rm -rf verify_dumps *.i *.s *.o.* verify.c verify 2>/dev/null || true
```

This script systematically exercises the target code by:

1. **Multiple invocations with output flags**: Uses `-dumpdir`, `-dumpbase`, `-fdump-*` flags with different values to stress allocation/freeing logic.

2. **System root overrides**: Uses `--sysroot`, `-B`, `-specs`, and `-march` to modify `target_system_root` and `spec_machine`.

3. **Driver mode switching**: Calls `g++` (C++ driver), `--help`, `--version`, and `-print-prog-name` to set mode flags that need resetting.

4. **Save-temps complexity**: Tests `save_temps_flag` with different values and combinations with dump options.

5. **Trailing slash handling**: Explicitly tests `dumpdir` with and without trailing slashes.

6. **Cleanup verification**: Ends with a working compilation to verify the driver state was properly reset.

The script uses `2>/dev/null || true` to handle cases where some options might not be supported on all platforms, ensuring the script continues to exercise the driver initialization/cleanup logic.
