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

# Create another source file for multi-file compilation
cat > test2.c <<'EOF'
int helper() {
    return 42;
}
EOF

echo "=== Testing driver state reset logic ==="

# 1. Test basic compilation with output naming variations
echo "1. Testing output naming and dump directory logic..."
gcc -save-temps -dumpdir ./dumps/ -dumpbase test1 -fdump-rtl-all -O2 -c test.c -o test1.o 2>/dev/null || true
gcc -save-temps=obj -dumpdir ./dumps2 -dumpbase test2 -fdump-tree-all -c test.c -o test2.o 2>/dev/null || true
gcc -dumpbase test3.c -fdump-ipa-all -c test.c -o test3.o 2>/dev/null || true

# Force dumpdir trailing slash logic
mkdir -p dumps_with_slash
gcc -dumpdir dumps_with_slash -dumpbase test4 -fdump-rtl-expand -c test.c -o test4.o 2>/dev/null || true
gcc -dumpdir dumps_with_slash/ -dumpbase test5 -fdump-tree-optimized -c test.c -o test5.o 2>/dev/null || true

# 2. Test sysroot and machine specification overrides
echo "2. Testing sysroot and machine spec resets..."
# Try different sysroot values (some may be invalid but should still trigger the logic)
gcc --sysroot=/ -march=x86-64 -c test.c -o test_sysroot1.o 2>/dev/null || true
gcc --sysroot=/usr -march=native -c test.c -o test_sysroot2.o 2>/dev/null || true
# Reset to default by not specifying sysroot
gcc -march=x86-64 -c test.c -o test_default.o 2>/dev/null || true

# 3. Test driver mode switching and help/version requests
echo "3. Testing driver mode switching..."
# Invoke as C++ driver (g++ usually symlinks to gcc with different mode)
if command -v g++ >/dev/null 2>&1; then
    g++ -c test.c -o test_cpp.o 2>/dev/null || true
fi

# Help and version requests (these set print_help_list, print_version)
gcc --help > /dev/null 2>&1
gcc --version > /dev/null 2>&1
gcc -print-prog-name=cc1 > /dev/null 2>&1
gcc -print-subprocess-help > /dev/null 2>&1 || true

# Follow with actual compilation to ensure state reset
gcc -c test.c -o test_after_help.o 2>/dev/null || true

# 4. Test complex save-temps and dump combinations
echo "4. Testing complex dump configurations..."
mkdir -p complex_dumps
gcc -save-temps -dumpdir complex_dumps/ -dumpbase complex \
    -fdump-rtl-all -fdump-tree-all -fdump-ipa-all \
    -O2 -c test.c -o complex.o 2>/dev/null || true

# Change dumpbase and dumpdir in subsequent compilation
gcc -save-temps=obj -dumpdir ./ -dumpbase complex2 \
    -fdump-tree-optimized -fdump-rtl-final \
    -c test.c -o complex2.o 2>/dev/null || true

# Test dumpbase extension logic
gcc -dumpbase complex3.c -fdump-tree-cfg -c test.c -o complex3.o 2>/dev/null || true

# 5. Test multiple compilations in sequence with varying options
echo "5. Testing sequential compilations with state changes..."
for i in 1 2 3 4 5; do
    gcc -dumpdir "seq_$i" -dumpbase "seq$i" \
        -save-temps \
        -c test.c -o "seq$i.o" 2>/dev/null || true
done

# 6. Test linker invocation (use_ld variable)
echo "6. Testing linker-related state..."
gcc -fuse-ld=bfd -o test_prog1 test1.o 2>/dev/null || true
gcc -fuse-ld=gold -o test_prog2 test1.o 2>/dev/null 2>&1 || true
# Reset to default
gcc -o test_prog_default test1.o 2>/dev/null || true

# 7. Test verbose and reporting flags
echo "7. Testing verbose and reporting flags..."
gcc -v -c test.c -o verbose.o 2>/dev/null || true
gcc -ftime-report -c test.c -o time_report.o 2>/dev/null || true

# 8. Final compilation to verify driver still works correctly
echo "8. Final verification compilation..."
gcc test.c -o final_test 2>/dev/null || true
if [ -x ./final_test ]; then
    ./final_test && echo "Final test program executed successfully"
else
    echo "Warning: Could not create final executable (may be cross-compiler)"
fi

# 9. Cleanup demonstration - show files were created
echo -e "\nGenerated files in $TMPDIR:"
find . -type f -name "*.o" -o -name "*.i" -o -name "*.s" -o -name "*.dump*" 2>/dev/null | head -20

echo -e "\n=== Test completed ==="
echo "The driver has been invoked with various state-modifying options"
echo "that should trigger the target cleanup code in gcc.cc lines 11228-11250"
```

This script systematically tests the uncovered code by:

1. **Multiple output naming variations**: Uses `-dumpdir`, `-dumpbase`, `-save-temps` with different values to stress allocation/free logic
2. **Sysroot and machine spec overrides**: Uses `--sysroot` and `-march` to modify `target_system_root` and `spec_machine`
3. **Driver mode switching**: Invokes help/version flags and attempts C++ driver mode
4. **Complex dump configurations**: Combines multiple `-fdump-*` flags with varying `-dumpbase` extensions
5. **Sequential compilations**: Loops through multiple compilations to ensure state resets between them
6. **Linker options**: Tests `-fuse-ld` to modify `use_ld` variable
7. **Verbose/reporting flags**: Tests `-v` and `-ftime-report` for related state variables

The script creates temporary files and cleans up after itself. Each compiler invocation is designed to modify specific global state variables that should be reset by the target code block. The `2>/dev/null || true` pattern ensures the script continues even if some options aren't supported by the specific GCC build being tested.
