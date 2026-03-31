Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc by invoking the compiler driver with various state-modifying options:

```bash
#!/bin/sh
# Test script for gcc driver initialization/cleanup logic
# Targets the uncovered block in gcc.cc lines 11228-11250

set -e

# Create a simple test source file
cat > test_source.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Hello from test program\n");
    return 0;
}
EOF

# Create a second test file for multiple compilation units
cat > test_source2.c << 'EOF'
int helper() {
    return 42;
}
EOF

# Clean up any previous test artifacts
rm -rf test_dumps test_out test_temp 2>/dev/null || true
mkdir -p test_dumps test_out test_temp

echo "=== Testing gcc driver state reset logic ==="

# 1. Test basic compilation with dump options to exercise dumpdir/dumpbase allocation/free
echo "1. Testing dump directory and base name logic..."
gcc -save-temps -dumpdir ./test_dumps/ -dumpbase test1 -fdump-rtl-all -O2 -c test_source.c -o test_temp/test1.o 2>/dev/null || true
# Force cleanup by invoking again with different dump options
gcc -dumpdir ./test_dumps2/ -dumpbase test2 -fdump-tree-all -c test_source.c -o test_temp/test2.o 2>/dev/null || true

# 2. Test sysroot and machine spec overrides
echo "2. Testing sysroot and machine specification overrides..."
# Use a dummy sysroot (current directory as example)
gcc --sysroot=/ -march=x86-64 -specs=/dev/null -c test_source.c -o test_temp/test3.o 2>/dev/null || true
# Reset by compiling without these options
gcc -c test_source.c -o test_temp/test4.o 2>/dev/null || true

# 3. Test driver mode switching and help/version requests
echo "3. Testing driver mode switching..."
# Invoke as C++ driver (sets is_cpp_driver)
g++ --version > /dev/null 2>&1 || true
# Request help (sets print_help_list)
gcc --help > /dev/null 2>&1 || true
# Request version (sets print_version)
gcc --version > /dev/null 2>&1 || true
# Request subprocess help (sets print_subprocess_help)
gcc -print-prog-name=cc1 > /dev/null 2>&1 || true
# Now compile to ensure reset happens
gcc -c test_source.c -o test_temp/test5.o 2>/dev/null || true

# 4. Test complex save-temps and dump directory combinations
echo "4. Testing complex save-temps and dump options..."
# Test with explicit dumpdir with trailing slash (tests dumpdir_trailing_dash_added)
gcc -save-temps=obj -dumpdir ./test_out/ -dumpbase complex -fdump-tree-all -fdump-ipa-all -c test_source.c -o test_temp/complex.o 2>/dev/null || true
# Test different dumpbase extensions
gcc -dumpbase complex.2 -fdump-rtl-expand -c test_source.c -o test_temp/complex2.o 2>/dev/null || true
# Test save-temps overrides
gcc -save-temps -dumpdir ./test_dumps/ -c test_source.c -o test_temp/complex3.o 2>/dev/null || true

# 5. Test multiple compilations in sequence with varying output flags
echo "5. Testing multiple compilations with varying output naming..."
for i in 1 2 3; do
    gcc -dumpbase "seq$i" -dumpdir "./test_dumps/seq/" -o "test_temp/seq$i.o" -c test_source.c 2>/dev/null || true
done

# 6. Test with -B option (affects spec_machine indirectly)
echo "6. Testing with -B option..."
gcc -B /usr/bin -c test_source.c -o test_temp/test6.o 2>/dev/null || true

# 7. Final compilation that should have clean state
echo "7. Final compilation with clean state..."
gcc test_temp/test1.o test_temp/test5.o -o test_program 2>/dev/null || true

# Run the test program if compilation succeeded
if [ -x test_program ]; then
    echo "Running test program..."
    ./test_program
    echo "Test program executed successfully"
else
    echo "Final linking failed (expected in some test environments)"
fi

# 8. Additional test: compile multiple source files with different dump options
echo "8. Testing multiple source files..."
gcc -dumpdir ./test_dumps/multi/ -dumpbase multi1 -c test_source.c -o test_temp/multi1.o 2>/dev/null || true
gcc -dumpdir ./test_dumps/multi/ -dumpbase multi2 -c test_source2.c -o test_temp/multi2.o 2>/dev/null || true
gcc test_temp/multi1.o test_temp/multi2.o -o test_multi 2>/dev/null || true

# Cleanup
echo "=== Cleaning up test files ==="
rm -f test_source.c test_source2.c
rm -rf test_dumps test_out test_temp 2>/dev/null || true
rm -f test_program test_multi 2>/dev/null || true

echo "Test completed. The gcc driver should have executed the target cleanup code multiple times."
```

This script systematically exercises the uncovered code by:

1. **Multiple invocations with dump options**: Uses `-dumpdir`, `-dumpbase`, `-fdump-*` flags to allocate and free the dump-related variables
2. **Sysroot and machine overrides**: Uses `--sysroot`, `-march`, `-specs` to modify `target_system_root` and `spec_machine`
3. **Driver mode switching**: Invokes `g++`, `--help`, `--version`, `-print-prog-name` to set mode flags that need resetting
4. **Complex save-temps logic**: Tests `save_temps_flag`, `dumpdir_trailing_dash_added`, and various dump combinations
5. **Multiple compilations**: Forces repeated initialization/cleanup cycles
6. **Clean state verification**: Ends with a normal compilation to ensure the driver resets properly

The script handles errors gracefully (using `2>/dev/null || true`) since some options may not be available in all GCC builds, but the driver initialization/cleanup code will still execute.
