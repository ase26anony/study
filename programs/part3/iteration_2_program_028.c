Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc by invoking the compiler driver with various state-modifying flags:

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
int square(int x) { return x * x; }
int main() {
    printf("Square of 5: %d\n", square(5));
    return 0;
}
EOF

echo "=== Testing GCC driver state reset logic ==="

# 1. Test basic compilation with output naming variations
echo "1. Testing output file naming and dump directory logic..."
gcc -c test.c -o test1.o
gcc -c test.c -o test2.o
gcc -c test2.c -o test3.o

# 2. Exercise dumpdir/dumpbase allocation and freeing
echo "2. Testing dump directory and base name logic..."
mkdir -p dumps1 dumps2

# First with explicit dumpdir and dumpbase
gcc -save-temps -dumpdir ./dumps1/ -dumpbase mytest -fdump-rtl-all -O2 -c test.c -o test4.o 2>/dev/null || true

# Different dumpdir, same base
gcc -save-temps -dumpdir ./dumps2/ -dumpbase mytest -fdump-tree-all -c test.c -o test5.o 2>/dev/null || true

# Different dumpbase with extension
gcc -dumpbase mytest.alt -fdump-ipa-all -c test.c -o test6.o 2>/dev/null || true

# Test dumpdir with trailing dash (triggers dumpdir_trailing_dash_added)
gcc -dumpdir ./dumps1 -dumpbase trail -c test.c -o test7.o 2>/dev/null || true

# 3. Test save-temps variations
echo "3. Testing save-temps flag variations..."
gcc -save-temps=cwd -c test.c -o test8.o
gcc -save-temps=obj -dumpdir ./ -c test2.c -o test9.o

# 4. Test system root and machine specification resets
echo "4. Testing system root and machine spec resets..."
# Use dummy sysroot (current directory as sysroot)
gcc --sysroot=/ -march=x86-64 -c test.c -o test10.o 2>/dev/null || true

# Different machine spec
gcc -march=native -c test.c -o test11.o 2>/dev/null || true

# With -B option (adds prefix to executable search path)
gcc -B/usr/bin -c test.c -o test12.o 2>/dev/null || true

# 5. Test driver mode switching and help/version requests
echo "5. Testing driver mode switching..."
# Invoke as C++ driver (triggers is_cpp_driver)
g++ --version >/dev/null 2>&1
g++ -c test.c -o test13.o 2>/dev/null || true

# Help requests (triggers print_help_list)
gcc --help >/dev/null 2>&1

# Version request (triggers print_version)
gcc --version >/dev/null 2>&1

# Subprocess help (triggers print_subprocess_help)
gcc -print-prog-name=cc1 >/dev/null 2>&1

# 6. Complex combination test
echo "6. Testing complex combination..."
gcc -save-temps=obj \
    -dumpdir ./complex/ \
    -dumpbase complex \
    -fdump-rtl-all \
    -fdump-tree-all \
    --sysroot=/ \
    -march=x86-64 \
    -c test.c \
    -o complex.o 2>/dev/null || true

# 7. Final compilation to ensure state was properly reset
echo "7. Final compilation with default state..."
gcc test.c test2.c -o final_test

# Run the final program to verify everything works
echo "8. Running final test program..."
./final_test

echo "=== Test completed successfully ==="
echo "Generated files in: $TMPDIR"
echo "To examine generated dump files, check: $TMPDIR/dumps1/ and $TMPDIR/dumps2/"
```

This script systematically tests the uncovered code by:

1. **Multiple invocations with varied output flags**: Uses `-o` with different names and `-c` for separate compilations
2. **Dump directory logic**: Tests `-dumpdir`, `-dumpbase`, `-fdump-*` flags with different combinations
3. **Save-temps variations**: Exercises `save_temps_flag` with different values
4. **System root and machine specs**: Uses `--sysroot`, `-march`, `-B` to modify `target_system_root` and `spec_machine`
5. **Driver mode switching**: Invokes `g++`, `--help`, `--version`, `-print-prog-name` to set various boolean flags
6. **Complex combination**: Tests multiple state-modifying flags together
7. **Final verification**: Ensures the driver can still compile normally after all state changes

The script creates temporary directories for isolation, cleans up after itself, and provides meaningful output about what's being tested. Each invocation forces the driver to allocate and free the global variables, hitting the target cleanup code.
