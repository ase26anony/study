Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc by invoking the compiler driver with various state-modifying options:

```bash
#!/bin/sh
# Test script to cover gcc.cc driver initialization/cleanup logic
# Lines 11228-11250: resetting global state variables

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

# Create a C++ source file for driver mode testing
cat > test.cpp <<'EOF'
#include <iostream>
int main() {
    std::cout << "Hello from C++ test" << std::endl;
    return 0;
}
EOF

echo "=== Testing driver state reset logic ==="

# 1. Test save-temps and dump directory logic with multiple allocations
echo "1. Testing save-temps and dump directory logic..."
gcc -save-temps -dumpdir ./dumps/ -dumpbase test1 -fdump-rtl-all -fdump-tree-all \
    -O2 -c test.c -o test1.o 2>/dev/null || true

# Force cleanup by separate invocation with different dumpdir
gcc -dumpdir ./dumps2/ -dumpbase test2 -fdump-ipa-all -c test.c -o test2.o 2>/dev/null || true

# Test dumpdir trailing slash logic (dumpdir_trailing_dash_added)
gcc -dumpdir ./dumps3 -dumpbase test3 -c test.c -o test3.o 2>/dev/null || true
gcc -dumpdir ./dumps4/ -dumpbase test4 -c test.c -o test4.o 2>/dev/null || true

# 2. Test sysroot and machine specification overrides
echo "2. Testing sysroot and machine specification resets..."
# Use dummy sysroot paths
gcc --sysroot=/usr -march=x86-64 -c test.c -o test5.o 2>/dev/null || true
gcc --sysroot=/ -march=native -c test.c -o test6.o 2>/dev/null || true
# Reset to defaults
gcc -c test.c -o test7.o 2>/dev/null || true

# 3. Test driver mode switching and help/version requests
echo "3. Testing driver mode switching..."
# Invoke as C++ driver (is_cpp_driver)
g++ -c test.cpp -o test_cpp.o 2>/dev/null || true

# Help and version requests (print_help_list, print_version)
gcc --help > /dev/null 2>&1 || true
gcc --version > /dev/null 2>&1 || true

# Subprocess help (print_subprocess_help)
gcc -print-prog-name=cc1 > /dev/null 2>&1 || true
gcc -print-prog-name=ld > /dev/null 2>&1 || true

# Back to normal compilation after help/version
gcc -c test.c -o test8.o 2>/dev/null || true

# 4. Test complex dumpbase extensions and outbase
echo "4. Testing complex dump naming..."
gcc -dumpbase test.complex -dumpbase-ext .ext -fdump-tree-optimized \
    -c test.c -o test9.o 2>/dev/null || true

gcc -dumpbase test.normal -c test.c -o test10.o 2>/dev/null || true

# 5. Test save-temps variations (save_temps_flag)
echo "5. Testing save-temps variations..."
gcc -save-temps=obj -dumpdir ./save1/ -c test.c -o test11.o 2>/dev/null || true
gcc -save-temps=cwd -dumpdir ./save2/ -c test.c -o test12.o 2>/dev/null || true
gcc -save-temps=none -c test.c -o test13.o 2>/dev/null || true

# 6. Test specs overrides (spec_machine)
echo "6. Testing specs overrides..."
gcc -specs=/dev/null -c test.c -o test14.o 2>/dev/null || true
# Reset with -B option
gcc -B/usr/bin -c test.c -o test15.o 2>/dev/null || true

# 7. Combined complex test: multiple state changes in sequence
echo "7. Running combined complex test..."
gcc --sysroot=/usr -march=x86-64 -save-temps -dumpdir ./combined/ \
    -dumpbase combined -fdump-rtl-all -O2 -c test.c -o combined1.o 2>/dev/null || true

gcc --help=optimizers > /dev/null 2>&1 || true

gcc -dumpdir ./final/ -dumpbase final -c test.c -o final.o 2>/dev/null || true

# 8. Final compilation to verify driver still works correctly
echo "8. Final compilation and execution test..."
gcc test.c -o test_program
./test_program

echo "=== Test completed successfully ==="
echo "Generated files in: $TMPDIR"
echo "Driver state reset logic should have been exercised multiple times."

# Optional: Show dump files were created
if [ -d "./dumps" ]; then
    echo "Dump files created: $(find ./dumps -name "*.dump" 2>/dev/null | wc -l)"
fi
if [ -d "./combined" ]; then
    echo "Combined dump files: $(find ./combined -name "*.dump" 2>/dev/null | wc -l)"
fi
```

This script systematically exercises the target code by:

1. **Multiple allocations and frees of dumpdir/dumpbase**: Uses different `-dumpdir` and `-dumpbase` options across invocations, forcing the driver to allocate and free these strings.

2. **Sysroot and machine spec resets**: Uses `--sysroot` and `-march` options, then compiles without them to trigger reset to defaults.

3. **Driver mode switching**: Invokes `g++` (C++ driver), help/version flags, and `-print-prog-name` to set various boolean flags that need resetting.

4. **Save-temps variations**: Tests all `-save-temps` modes (`obj`, `cwd`, `none`) to exercise `save_temps_flag`.

5. **Complex dump scenarios**: Uses `-dumpbase-ext` and multiple dump flags (`-fdump-*`) to stress the dump file naming logic.

6. **Specs overrides**: Uses `-specs` and `-B` options to modify `spec_machine`.

7. **Combined test**: Puts multiple state changes together in sequence to ensure proper cleanup between them.

8. **Final verification**: Compiles and runs a simple program to ensure the driver works correctly after all state modifications.

The script uses `2>/dev/null || true` to handle cases where some options might not be supported on all platforms, ensuring the script continues to exercise the driver logic. The temporary directory cleanup ensures no files are left behind.
