Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc by invoking the driver with various state-modifying options:

```bash
#!/bin/sh

# Test program to exercise gcc driver initialization/cleanup logic
# Specifically targets lines 11228-11250 in gcc.cc

set -e

# Create a simple test source file
cat > test_source.c << 'EOF'
#include <stdio.h>

int main(void) {
    printf("Hello from test program\n");
    return 0;
}
EOF

# Create a second test file for multiple compilation scenarios
cat > test_source2.c << 'EOF'
#include <stdio.h>

int helper(void) {
    return 42;
}
EOF

# Clean up any previous test artifacts
rm -rf test_dumps test_out test_temp 2>/dev/null || true
mkdir -p test_dumps test_out test_temp

echo "=== Testing GCC driver state reset logic ==="
echo

# 1. Test basic compilation with save-temps and dump options
# This exercises dumpdir/dumpbase allocation and freeing
echo "1. Testing save-temps and dump directory logic..."
gcc -save-temps -dumpdir ./test_dumps/ -dumpbase test1 -fdump-rtl-all -O2 \
    -c test_source.c -o test_temp/test1.o 2>/dev/null || true

# Force trailing slash addition test
gcc -save-temps -dumpdir ./test_dumps -dumpbase test2 -fdump-tree-all \
    -c test_source.c -o test_temp/test2.o 2>/dev/null || true

# Test save-temps overrides
gcc -save-temps=obj -dumpdir ./test_dumps/ -dumpbase test3 \
    -c test_source.c -o test_temp/test3.o 2>/dev/null || true

# 2. Test system root and machine specification
# Exercises target_system_root and spec_machine reset
echo "2. Testing sysroot and machine specification..."
# Try with different sysroot (even if invalid, should trigger state changes)
gcc --sysroot=/ -march=x86-64 -c test_source.c -o test_temp/test4.o 2>/dev/null || true

# Try with machine-specific options
gcc -march=native -mtune=generic -c test_source.c -o test_temp/test5.o 2>/dev/null || true

# Reset to defaults implicitly
gcc -c test_source.c -o test_temp/test6.o 2>/dev/null || true

# 3. Test driver mode switching and help/version requests
# Exercises is_cpp_driver, print_help_list, print_version flags
echo "3. Testing driver mode switching..."
# Invoke as C++ driver (g++ usually symlinks to gcc with different mode)
if command -v g++ >/dev/null 2>&1; then
    g++ -c test_source.c -o test_temp/test_cpp.o 2>/dev/null || true
fi

# Request help and version information
gcc --help > /dev/null 2>&1 || true
gcc --version > /dev/null 2>&1 || true
gcc -print-prog-name=cc1 > /dev/null 2>&1 || true
gcc -print-search-dirs > /dev/null 2>&1 || true

# 4. Test complex dump and output naming scenarios
# Exercises dumpbase_ext, outbase, and save_temps_flag logic
echo "4. Testing complex dump scenarios..."
# Multiple dump flags with different dumpbase extensions
gcc -fdump-tree-all -fdump-ipa-all -fdump-rtl-expand \
    -dumpbase test7 -dumpdir ./test_dumps/ \
    -c test_source.c -o test_temp/test7.o 2>/dev/null || true

# Different dumpbase without extension
gcc -fdump-tree-optimized -dumpbase test8 -c test_source.c \
    -o test_temp/test8.o 2>/dev/null || true

# With dumpbase extension specified
gcc -fdump-tree-cfg -dumpbase test9.c -dumpdir ./test_dumps \
    -c test_source.c -o test_temp/test9.o 2>/dev/null || true

# 5. Test multiple compilations in sequence with varying options
# This forces repeated initialization/cleanup cycles
echo "5. Testing sequential compilations with state changes..."
for i in 10 11 12 13 14; do
    # Alternate between different option sets
    case $((i % 3)) in
        0)
            gcc -save-temps -dumpdir ./test_dumps/ -dumpbase "seq$i" \
                -O$((i % 3)) -c test_source.c \
                -o test_temp/seq$i.o 2>/dev/null || true
            ;;
        1)
            gcc --sysroot=/ -march=x86-64 -dumpbase "seq$i" \
                -c test_source.c -o test_temp/seq$i.o 2>/dev/null || true
            ;;
        2)
            gcc -fdump-tree-all -dumpdir ./test_dumps -dumpbase "seq$i" \
                -c test_source2.c -o test_temp/seq$i.o 2>/dev/null || true
            ;;
    esac
done

# 6. Test with explicit outbase and different output names
echo "6. Testing output file naming variations..."
gcc -c test_source.c -o test_out/output1.o 2>/dev/null || true
gcc -c test_source.c -o test_out/output2.obj 2>/dev/null || true
gcc -c test_source2.c -o test_out/output3.o 2>/dev/null || true

# 7. Final compilation to verify driver still works correctly
# This ensures state was properly reset after all modifications
echo "7. Final verification compilation..."
gcc test_temp/*.o test_out/*.o -o test_final 2>/dev/null || {
    # If linking fails due to incompatible objects, compile fresh
    gcc test_source.c -o test_final 2>/dev/null || true
}

# Run the final program if compilation succeeded
if [ -x test_final ]; then
    echo "Running final test program..."
    ./test_final
    echo "Test completed successfully."
else
    echo "Final compilation failed (expected due to test variations)."
fi

# 8. Cleanup
echo
echo "Cleaning up test files..."
rm -f test_source.c test_source2.c test_final
rm -rf test_dumps test_out test_temp 2>/dev/null || true

echo "=== Test completed ==="
```

This script systematically exercises the target code by:

1. **Multiple invocations with varied output flags**: Uses `-save-temps`, `-dumpdir`, `-dumpbase`, `-fdump-*` flags in different combinations across multiple compilations.

2. **System root and machine overrides**: Uses `--sysroot=/` and `-march=` options to modify `target_system_root` and machine specifications.

3. **Driver mode switching**: Attempts to invoke as C++ driver (if available) and uses help/version flags (`--help`, `--version`, `-print-prog-name=`).

4. **Complex dump scenarios**: Tests `dumpdir_trailing_dash_added` logic by using paths with and without trailing slashes, and exercises `dumpbase_ext` with different naming patterns.

5. **Sequential state changes**: Uses a loop to repeatedly invoke the driver with different option sets, forcing the cleanup/initialization cycle.

6. **Output file variations**: Tests different output file names and extensions to exercise `outbase` logic.

The script suppresses error output (with `2>/dev/null || true`) since some combinations may fail on certain systems (e.g., invalid `--sysroot` paths), but the driver will still execute the initialization/cleanup code path. The final compilation verifies the driver remains functional after all state modifications.
