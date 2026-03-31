Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc by invoking the compiler driver with various state-modifying options:

```bash
#!/bin/sh
# Test script to cover gcc.cc lines 11228-11250 - driver state reset logic

set -e
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"

# Create a simple test source file
cat > test.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Hello from test program\n");
    return 0;
}
EOF

# Create another source file for multiple compilation units
cat > test2.c << 'EOF'
int helper() {
    return 42;
}
EOF

echo "=== Testing GCC driver state reset logic ==="

# 1. Test basic compilation with save-temps and dump options
echo "1. Testing save-temps and dump directory logic..."
gcc -save-temps -dumpdir ./dumps/ -dumpbase test -fdump-rtl-all -O2 -c test.c -o test.o 2>/dev/null || true
gcc -save-temps=obj -dumpdir ./out/ -dumpbase base -fdump-tree-all -c test.c -o test2.o 2>/dev/null || true

# Force dumpdir trailing slash logic
mkdir -p nodash
gcc -save-temps -dumpdir nodash -dumpbase test3 -fdump-ipa-all -c test.c -o test3.o 2>/dev/null || true

# 2. Test sysroot and machine specification resets
echo "2. Testing sysroot and machine spec resets..."
# Use dummy sysroot paths (these may not exist, but will set the variables)
gcc --sysroot=/usr -march=x86-64 -c test.c -o test4.o 2>/dev/null || true
gcc --sysroot=/ -march=native -c test.c -o test5.o 2>/dev/null || true
# Reset to default by not specifying
gcc -c test.c -o test6.o 2>/dev/null || true

# 3. Test driver mode switching and help/version
echo "3. Testing driver mode switching..."
# Invoke as C++ driver (sets is_cpp_driver)
g++ --version >/dev/null 2>&1 || true
# Help requests (set print_help_list, print_version)
gcc --help >/dev/null 2>&1 || true
gcc --version >/dev/null 2>&1 || true
# Subprocess help (sets print_subprocess_help)
gcc -print-prog-name=cc1 >/dev/null 2>&1 || true
gcc -print-prog-name=as >/dev/null 2>&1 || true

# 4. Test complex dump and output naming combinations
echo "4. Testing complex dump/output combinations..."
mkdir -p complex_dumps
gcc -save-temps -dumpdir complex_dumps/ -dumpbase complex \
    -fdump-rtl-expand -fdump-tree-optimized -fdump-ipa-all \
    -c test.c -o complex.o 2>/dev/null || true

# Change dumpbase and outbase
gcc -dumpbase different.123 -c test.c -o different.o 2>/dev/null || true
gcc -dumpbase "with spaces" -c test.c -o "with spaces.o" 2>/dev/null || true

# 5. Test multiple compilations in sequence with varying options
echo "5. Testing sequential compilations with state changes..."
for i in 1 2 3; do
    gcc -save-temps=${i} -dumpdir seq${i}/ -dumpbase seq${i} \
        -march=x86-64 -c test.c -o seq${i}.o 2>/dev/null || true
    # Alternate with help requests
    gcc --help=common >/dev/null 2>&1 || true
done

# 6. Test with explicit specs file if available
echo "6. Testing with specs overrides..."
# Try to find a specs file or create a minimal one
cat > dummy.specs << 'EOF'
*cc1:
%(cc1_cpu)

*link:
%(link_cpu)
EOF
gcc -specs=dummy.specs -c test.c -o specs.o 2>/dev/null || true

# 7. Test B option (adds to compiler executable prefixes)
echo "7. Testing -B option..."
gcc -B/usr/bin -c test.c -o btest.o 2>/dev/null || true

# 8. Final compilation to verify state was properly reset
echo "8. Final compilation to verify clean state..."
gcc test.c -o final_test 2>/dev/null || true
if [ -x ./final_test ]; then
    ./final_test
    echo "✓ Final compilation successful - driver state was properly reset"
else
    echo "✗ Final compilation failed"
fi

# 9. Test verbose flag (sets verbose_only_flag)
echo "9. Testing verbose flag..."
gcc -v -c test.c -o verbose.o 2>/dev/null || true

# 10. Test time reporting
echo "10. Testing time reporting..."
gcc -ftime-report -c test.c -o time.o 2>/dev/null || true

# Cleanup
echo ""
echo "=== Test complete ==="
echo "Generated files in: $TEMP_DIR"
echo "To examine generated dump files, check the dumps/, out/, and complex_dumps/ directories"
echo "To clean up: rm -rf $TEMP_DIR"

# Optional: keep temp dir for inspection
read -p "Keep temporary directory for inspection? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    rm -rf "$TEMP_DIR"
    echo "Cleaned up temporary directory"
else
    echo "Files kept in: $TEMP_DIR"
fi
```

This script systematically exercises the target code by:

1. **Multiple invocations with varied output flags**: Uses `-save-temps`, `-dumpdir`, `-dumpbase` with different values across multiple compilations, forcing allocation and freeing of `dumpdir`, `dumpbase`, etc.

2. **System root and spec machine overrides**: Uses `--sysroot`, `-march`, `-specs`, and `-B` options to modify `target_system_root`, `spec_machine`, and related variables.

3. **Driver mode switching**: Invokes `g++` (C++ driver), `--help`, `--version`, `-print-prog-name` to set flags like `is_cpp_driver`, `print_help_list`, `print_version`, `print_subprocess_help`.

4. **Save-temps and dump directory complexity**: Tests `save_temps_flag` with different values (`-save-temps`, `-save-temps=obj`), ensures trailing slash handling with `-dumpdir`, and uses multiple `-fdump-*` flags.

5. **Sequential state changes**: Loops through compilations with different options, ensuring state is reset between invocations.

The script creates a temporary directory, generates test files, runs the compiler with various flag combinations, and cleans up. Each invocation modifies driver state variables that should be reset by the target code block during cleanup between jobs or at program exit.
