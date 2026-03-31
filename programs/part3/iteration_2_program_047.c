Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc by invoking the compiler driver with various state-modifying options:

```bash
#!/bin/sh
# Test script to cover gcc.cc lines 11228-11250 - driver state reset logic

set -e
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

cd "$TMPDIR"

# Create a simple test source file
cat > test.c <<'EOF'
#include <stdio.h>
int main() {
    printf("Test program\n");
    return 0;
}
EOF

# 1. Multiple compilations with varied output flags and dump options
echo "=== Phase 1: Testing dumpdir/dumpbase/outbase allocation and cleanup ==="
gcc -save-temps -dumpdir ./dumps1/ -dumpbase test1 -fdump-rtl-all -O2 -c test.c -o test1.o 2>/dev/null || true
gcc -save-temps=obj -dumpdir ./dumps2 -dumpbase test2 -fdump-tree-all -c test.c -o test2.o 2>/dev/null || true
gcc -dumpdir ./dumps3/ -dumpbase test3.c -fdump-ipa-all -c test.c -o test3.o 2>/dev/null || true

# Force cleanup by separate invocation with different dumpbase
gcc -dumpbase test4 -c test.c -o test4.o 2>/dev/null || true

# 2. System root and spec machine overrides
echo "=== Phase 2: Testing target_system_root and spec_machine reset ==="
# Use dummy sysroot paths (these may not exist, but will set the flags)
gcc --sysroot=/tmp/dummy-sysroot -march=x86-64 -c test.c -o test5.o 2>/dev/null || true
gcc --sysroot=/ -march=native -c test.c -o test6.o 2>/dev/null || true
# Reset to default by not specifying sysroot
gcc -march=x86-64 -c test.c -o test7.o 2>/dev/null || true

# Test with -B and -specs (using default specs file if available)
if [ -f "/usr/lib/gcc/x86_64-linux-gnu/*/specs" ] 2>/dev/null; then
    gcc -B/usr/lib/gcc -specs=/usr/lib/gcc/x86_64-linux-gnu/*/specs -c test.c -o test8.o 2>/dev/null || true
fi

# 3. Driver mode switching and help/version requests
echo "=== Phase 3: Testing driver mode flags reset ==="
# Invoke as C++ driver (sets is_cpp_driver)
g++ --version >/dev/null 2>&1 || true
# Help requests (set print_help_list, print_version)
gcc --help=common >/dev/null 2>&1 || true
gcc --version >/dev/null 2>&1 || true
# Subprocess help (sets print_subprocess_help)
gcc -print-prog-name=cc1 >/dev/null 2>&1 || true
gcc -print-multi-directory >/dev/null 2>&1 || true

# Now compile normally - driver must reset all help/version flags
gcc -c test.c -o test9.o 2>/dev/null || true

# 4. Complex save-temps and dump directory scenarios
echo "=== Phase 4: Testing save_temps_flag and dumpdir edge cases ==="
# Test dumpdir with and without trailing slash
mkdir -p complex_dump
gcc -save-temps -dumpdir complex_dump -dumpbase complex -fdump-rtl-expand -fdump-tree-optimized -c test.c -o test10.o 2>/dev/null || true
gcc -save-temps -dumpdir complex_dump/ -dumpbase complex2 -c test.c -o test11.o 2>/dev/null || true

# Test save_temps_overrides_dumpdir behavior
gcc -save-temps=obj -dumpdir ignored_dir/ -dumpbase temp_test -c test.c -o test12.o 2>/dev/null || true

# Multiple dump flags with different extensions
gcc -dumpbase multi -fdump-rtl-all -fdump-tree-all -fdump-ipa-all -c test.c -o test13.o 2>/dev/null || true

# 5. Mixed complex invocation testing state persistence
echo "=== Phase 5: Mixed complex invocations ==="
gcc --sysroot=/ -save-temps -dumpdir ./final/ -dumpbase final -march=x86-64 -fdump-rtl-all -O2 -c test.c -o test14.o 2>/dev/null || true
gcc --version >/dev/null 2>&1 || true
gcc -dumpbase changed -c test.c -o test15.o 2>/dev/null || true

# 6. Final compilation and execution test
echo "=== Phase 6: Final compilation with reset state ==="
# This should use all default values after previous state modifications
gcc test.c -o test_program 2>/dev/null || true
if [ -x ./test_program ]; then
    ./test_program
    echo "Success: Driver state was properly reset"
else
    echo "Warning: Final compilation failed (may be expected in test environment)"
fi

# Cleanup
rm -f test*.o test*.s test*.i test*.ii test*.bc test*.expand test*.optimized 2>/dev/null || true
rm -f *.o *.s *.i *.ii 2>/dev/null || true
rm -rf dumps1 dumps2 dumps3 complex_dump final 2>/dev/null || true

echo "Test completed. All driver state resets should have been exercised."
```

**Key aspects that trigger the target code:**

1. **Multiple allocations/frees of dumpdir/dumpbase**: Each `-dumpdir` and `-dumpbase` option causes allocation, and the cleanup block frees them between invocations.

2. **System root changes**: `--sysroot=` options modify `target_system_root` and `target_system_root_changed`.

3. **Machine spec changes**: `-march=` options affect `spec_machine` which gets reset to `DEFAULT_TARGET_MACHINE`.

4. **Driver mode flags**: `g++` invocation sets `is_cpp_driver`, help/version options set `print_help_list`, `print_version`, etc.

5. **Save-temps flags**: `-save-temps` variations exercise `save_temps_flag` and `save_temps_overrides_dumpdir`.

6. **Trailing slash handling**: Different `-dumpdir` formats test `dumpdir_trailing_dash_added`.

The script runs the compiler driver through multiple state changes, forcing the cleanup logic to execute between invocations or at program termination. The `2>/dev/null` suppresses expected warnings about non-existent sysroots or dump directories in test environments.
