Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc:

```bash
#!/bin/sh
# Test script for GCC driver initialization/cleanup logic
# Targets lines 11228-11250 in gcc.cc

set -e
TMPDIR=$(mktemp -d)
cd "$TMPDIR"

# Cleanup function
cleanup() {
    cd /
    rm -rf "$TMPDIR"
}
trap cleanup EXIT

# Create a simple test source file
cat > test.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Hello from test program\n");
    return 0;
}
EOF

cat > test2.c << 'EOF'
#include <stdio.h>
int square(int x) {
    return x * x;
}
int main() {
    printf("Square of 5: %d\n", square(5));
    return 0;
}
EOF

echo "=== Testing GCC driver initialization/cleanup logic ==="
echo "Target: Lines 11228-11250 in gcc.cc"
echo

# 1. Test basic compilation with output naming variations
echo "1. Testing multiple compilations with varied output flags..."
gcc -c test.c -o test1.o
gcc -c test.c -o test2.o
gcc -c test2.c -o test3.o

# 2. Test dump directory and save-temps logic
echo "2. Testing dump directory and save-temps logic..."
mkdir -p dumps
gcc -save-temps -dumpdir ./dumps/ -dumpbase mytest -fdump-rtl-all -O2 -c test.c -o test4.o
gcc -save-temps=obj -dumpdir ./dumps2 -dumpbase mytest2 -fdump-tree-all -c test2.c -o test5.o

# Test dumpdir trailing slash logic
gcc -dumpdir ./dumps3 -dumpbase test -fdump-ipa-all -c test.c -o test6.o
gcc -dumpdir ./dumps4/ -dumpbase test -fdump-rtl-expand -c test.c -o test7.o

# 3. Test system root and machine specification overrides
echo "3. Testing system root and machine specification overrides..."
# Note: Using / as sysroot for testing (actual sysroot would be system-dependent)
gcc --sysroot=/ -march=x86-64 -c test.c -o test8.o
gcc -specs=/dev/null -march=native -c test2.c -o test9.o 2>/dev/null || true

# Test with -B option (add directory to compiler search path)
mkdir -p fake_lib
gcc -B ./fake_lib -c test.c -o test10.o

# 4. Test driver mode switching and help/version requests
echo "4. Testing driver mode switching and help/version..."
gcc --help > /dev/null
gcc --version > /dev/null
gcc -print-prog-name=cc1 > /dev/null
gcc -print-subprocess-helper > /dev/null 2>&1 || true

# Test as C++ driver (g++ should set is_cpp_driver)
g++ --version > /dev/null
g++ -c test.c -o test11.o 2>/dev/null || true

# 5. Complex combination test
echo "5. Testing complex combinations..."
gcc -save-temps -dumpdir ./combined/ -dumpbase complex \
    -fdump-rtl-all -fdump-tree-all -O2 \
    --sysroot=/ -march=x86-64 \
    -c test.c -o test12.o

# 6. Test multiple invocations in sequence to trigger cleanup between runs
echo "6. Testing sequential invocations..."
for i in $(seq 1 3); do
    gcc -save-temps -dumpdir "./seq$i/" -dumpbase "seq$i" \
        -fdump-rtl-expand -c test.c -o "seq$i.o"
done

# 7. Test with different dumpbase extensions
echo "7. Testing different dumpbase extensions..."
gcc -dumpbase main.c -c test.c -o test13.o
gcc -dumpbase main.123 -c test.c -o test14.o
gcc -dumpbase "test.with.dots" -c test.c -o test15.o

# 8. Final compilation to verify driver state is properly reset
echo "8. Final compilation to verify clean state..."
gcc test.c test2.c -o final_test
./final_test

# 9. Additional edge cases
echo "9. Testing edge cases..."
# Empty dumpdir
gcc -dumpdir "" -dumpbase empty -c test.c -o test16.o
# Very long dumpbase (tests allocation)
gcc -dumpbase "$(printf 'x%.0s' {1..100})" -c test.c -o test17.o

# 10. Test with verbose flag (verbose_only_flag)
echo "10. Testing verbose flag..."
gcc -v -c test.c -o test18.o 2>&1 | head -5 > /dev/null

echo
echo "=== All tests completed successfully ==="
echo "Generated files in: $TMPDIR"
echo "The driver's initialization/cleanup logic should have been exercised multiple times."
```

This script systematically tests the uncovered code by:

1. **Multiple Compiler Invocations**: Compiles multiple source files with different `-o` options and dump flags
2. **System Root and Machine Overrides**: Uses `--sysroot=`, `-march=`, `-specs=`, and `-B` options
3. **Driver Mode Switching**: Calls `gcc` and `g++`, uses help/version flags, and `-print-prog-name`
4. **Save-Temps and Dump Directory**: Tests `-save-temps`, `-dumpdir` (with and without trailing slash), `-dumpbase`, and various `-fdump-*` flags
5. **Complex Combinations**: Combines multiple flags that affect the target state variables
6. **Sequential Invocations**: Forces cleanup between multiple compiler runs
7. **Edge Cases**: Tests empty strings and long names for allocation/free logic

The script creates a temporary directory, generates test files, runs the compiler with various flag combinations, and cleans up after itself. Each invocation modifies the driver's global state, forcing the cleanup logic to execute between runs or at program termination.
