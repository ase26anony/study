Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc:

```bash
#!/bin/sh

# Test script to cover GCC driver initialization/cleanup logic
# Specifically targets lines 11228-11250 in gcc.cc

set -e

# Create a simple test source file
cat > test_source.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Hello from GCC driver test\n");
    return 0;
}
EOF

# Create a second test file for multiple compilations
cat > test_source2.c << 'EOF'
int add(int a, int b) {
    return a + b;
}
EOF

# Clean up any previous test artifacts
rm -rf test_dumps test_out test_temp 2>/dev/null || true
mkdir -p test_dumps test_out test_temp

echo "=== Testing GCC driver state reset logic ==="

# 1. Test basic compilation with save-temps and dump options
# This allocates dumpdir, dumpbase, etc.
echo "1. Testing save-temps and dump directory logic..."
gcc -save-temps -dumpdir ./test_dumps/ -dumpbase test1 -fdump-rtl-all -fdump-tree-all \
    -O2 -c test_source.c -o test_temp/test1.o 2>/dev/null

# Force cleanup by running another compilation with different options
gcc -dumpbase test2 -c test_source.c -o test_temp/test2.o 2>/dev/null

# 2. Test sysroot and machine specification overrides
# This modifies target_system_root and spec_machine
echo "2. Testing sysroot and machine specification..."
# Try with a dummy sysroot (using / as it always exists)
gcc --sysroot=/ -march=x86-64 -c test_source.c -o test_temp/test3.o 2>/dev/null

# Reset by compiling without sysroot
gcc -c test_source.c -o test_temp/test4.o 2>/dev/null

# 3. Test driver mode switching and help/version requests
# This sets flags like print_help_list, print_version, etc.
echo "3. Testing driver mode switching..."
gcc --help > /dev/null 2>&1
gcc --version > /dev/null 2>&1
gcc -print-prog-name=cc1 > /dev/null 2>&1
gcc -print-prog-name=as > /dev/null 2>&1

# Now compile normally to ensure state reset
gcc -c test_source.c -o test_temp/test5.o 2>/dev/null

# 4. Test complex dump and output naming scenarios
# Exercises save_temps_flag, dumpdir_trailing_dash_added logic
echo "4. Testing complex dump scenarios..."
# Test with save-temps=obj
gcc -save-temps=obj -dumpdir ./test_out -dumpbase complex \
    -fdump-tree-all -fdump-ipa-all -c test_source.c -o test_temp/complex.o 2>/dev/null

# Test with different dumpbase extension
gcc -dumpbase complex.2 -c test_source.c -o test_temp/complex2.o 2>/dev/null

# Test dumpdir without trailing slash (tests dumpdir_trailing_dash_added)
gcc -dumpdir ./test_dumps -dumpbase notrail -c test_source.c -o test_temp/notrail.o 2>/dev/null

# 5. Test multiple compilations in sequence with varying options
# This forces repeated initialization/cleanup cycles
echo "5. Testing multiple sequential compilations..."
for i in 1 2 3 4 5; do
    gcc -save-temps -dumpdir "./test_dumps/run$i/" -dumpbase "seq$i" \
        -march=x86-64 -O$i -c test_source.c -o "test_temp/seq$i.o" 2>/dev/null
done

# 6. Test as C++ driver (is_cpp_driver)
echo "6. Testing C++ driver mode..."
# Check if g++ is available
if command -v g++ >/dev/null 2>&1; then
    g++ -c test_source.c -o test_temp/cpp.o 2>/dev/null
    # Switch back to C mode
    gcc -c test_source.c -o test_temp/backtoc.o 2>/dev/null
fi

# 7. Test with specs file if available
echo "7. Testing with specs overrides..."
# Create a minimal specs file
cat > test_temp/test.specs << 'EOF'
*cc1:
%(cc1_cpu)
EOF

gcc -specs=test_temp/test.specs -c test_source.c -o test_temp/withspecs.o 2>/dev/null

# 8. Final compilation to verify everything still works
echo "8. Final verification compilation..."
gcc test_temp/test1.o test_temp/test2.o test_temp/test5.o \
    -o test_program 2>/dev/null

# Run the test program if compilation succeeded
if [ -f test_program ]; then
    ./test_program
    echo "=== Test completed successfully ==="
else
    echo "=== Final compilation failed ==="
    exit 1
fi

# 9. Cleanup
echo "Cleaning up test files..."
rm -f test_source.c test_source2.c test_program
rm -rf test_dumps test_out test_temp

echo "All tests completed. The GCC driver's initialization/cleanup logic"
echo "should have been exercised multiple times with different state configurations."
```

This script systematically tests the uncovered code by:

1. **Multiple compilations with varied output flags**: Uses `-c`, `-o`, `-dumpdir`, `-dumpbase` with different values across multiple invocations.

2. **System root and machine overrides**: Uses `--sysroot=/` and `-march=x86-64` to modify `target_system_root` and machine specs.

3. **Driver mode switching**: Calls `gcc` with `--help`, `--version`, `-print-prog-name` flags, then performs actual compilations.

4. **Save-temps and dump directory complexity**: Tests `-save-temps`, `-save-temps=obj`, `-dumpdir` with/without trailing slashes, and multiple `-fdump-*` flags.

5. **C++ driver mode**: Tests compilation with `g++` (if available) to trigger `is_cpp_driver` flag.

6. **Specs file usage**: Uses `-specs=` option to modify toolchain specifications.

7. **Sequential testing**: Runs multiple compilations in a loop with varying optimization levels and dump directories.

The script creates temporary directories for output, compiles simple test files, and ensures the driver goes through full initialization/cleanup cycles between different flag combinations. Each section forces the driver to allocate and free the global variables mentioned in the uncovered block.
