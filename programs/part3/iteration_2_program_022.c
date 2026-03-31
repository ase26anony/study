Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc:

```bash
#!/bin/sh
# Test script to cover gcc driver initialization/cleanup logic
# Targets lines 11228-11250 in gcc.cc

set -e
TESTDIR=$(mktemp -d /tmp/gcc_driver_test_XXXXXX)
trap 'rm -rf "$TESTDIR"' EXIT

cd "$TESTDIR"

# Create a simple test source file
cat > test.c <<'EOF'
#include <stdio.h>
int main() {
    printf("Hello from test program\n");
    return 0;
}
EOF

# Create a second test file
cat > test2.c <<'EOF'
#include <stdio.h>
int helper() {
    return 42;
}
EOF

echo "=== Testing gcc driver state reset logic ==="

# 1. Test basic compilation with save-temps and dump options
echo "1. Testing save-temps and dump directory logic..."
mkdir -p dumps out

# This will allocate dumpdir, dumpbase, etc.
gcc -save-temps -dumpdir ./dumps/ -dumpbase test1 -fdump-rtl-all -O2 -c test.c -o test1.o 2>/dev/null

# Different dumpdir and dumpbase to force reallocation
gcc -save-temps=obj -dumpdir ./out/ -dumpbase test2 -fdump-tree-all -fdump-ipa-all -c test.c -o test2.o 2>/dev/null

# Test dumpdir trailing dash logic with explicit trailing slash
gcc -dumpdir ./dumps -dumpbase test3 -c test.c -o test3.o 2>/dev/null

# Test without dumpdir to trigger cleanup
gcc -dumpbase test4 -c test.c -o test4.o 2>/dev/null

# 2. Test sysroot and machine specification
echo "2. Testing sysroot and machine spec reset..."
# Modify target_system_root
gcc --sysroot=/usr -march=x86-64 -c test.c -o test_sysroot1.o 2>/dev/null

# Different sysroot
gcc --sysroot=/ -march=native -c test.c -o test_sysroot2.o 2>/dev/null

# Back to default (should trigger reset to DEFAULT_TARGET_SYSTEM_ROOT)
gcc -march=x86-64 -c test.c -o test_sysroot3.o 2>/dev/null

# 3. Test driver mode switching and help/version
echo "3. Testing driver mode switching..."
# Invoke as C++ driver (sets is_cpp_driver)
g++ -c test.c -o test_cpp.o 2>/dev/null

# Request help (sets print_help_list)
gcc --help > /dev/null 2>&1

# Request version (sets print_version)
gcc --version > /dev/null 2>&1

# Request subprocess help (sets print_subprocess_help)
gcc -print-prog-name=cc1 > /dev/null 2>&1

# Now compile normally (should reset all above flags)
gcc -c test.c -o test_normal.o 2>/dev/null

# 4. Test complex combination of flags
echo "4. Testing complex flag combinations..."
# Multiple output files with different bases
gcc -save-temps -dumpdir ./complex/ -dumpbase complex1 -fdump-rtl-expand \
    -fdump-tree-optimized -c test.c -o complex1.o 2>/dev/null

# Different combination
gcc -save-temps=obj -dumpbase complex2 -fdump-tree-cfg -c test2.c -o complex2.o 2>/dev/null

# With spec machine override
gcc -specs=/dev/null -dumpdir ./specs/ -dumpbase specs_test -c test.c -o specs_test.o 2>/dev/null

# 5. Test batch compilation (multiple files in one invocation)
echo "5. Testing batch compilation..."
gcc -save-temps -dumpdir ./batch/ -dumpbase batch \
    -fdump-rtl-all -fdump-tree-all \
    -c test.c test2.c 2>/dev/null

# 6. Test verbose flag (verbose_only_flag)
echo "6. Testing verbose flag..."
gcc -v -c test.c -o test_verbose.o 2>/dev/null

# 7. Test linker specification (use_ld)
echo "7. Testing linker specification..."
gcc -fuse-ld=bfd -c test.c -o test_ld1.o 2>/dev/null 2>&1 || true
gcc -fuse-ld=gold -c test.c -o test_ld2.o 2>/dev/null 2>&1 || true

# 8. Final compilation to verify state is properly reset
echo "8. Final verification compilation..."
gcc test1.o test2.o test3.o test4.o \
    test_sysroot1.o test_sysroot2.o test_sysroot3.o \
    test_cpp.o test_normal.o \
    complex1.o complex2.o specs_test.o \
    test_verbose.o test_ld1.o test_ld2.o \
    -o final_test 2>/dev/null

# Run the final program to ensure everything compiled correctly
if [ -x ./final_test ]; then
    ./final_test
    echo "=== All tests completed successfully ==="
else
    echo "Error: Final test program not created"
    exit 1
fi

# Cleanup (handled by trap)
echo "Test directory: $TESTDIR"
```

This script systematically tests the uncovered code by:

1. **Multiple compilations with varied output flags**: Uses different combinations of `-save-temps`, `-dumpdir`, `-dumpbase`, and `-fdump-*` flags to stress the allocation and freeing of `dumpdir`, `dumpbase`, etc.

2. **System root and spec machine overrides**: Uses `--sysroot`, `-march`, and `-specs` to modify `target_system_root` and `spec_machine`, then returns to defaults.

3. **Driver mode switching**: Invokes `g++` (C++ driver), help, version, and subprocess help requests, then returns to normal compilation.

4. **Save-temps and dump directory complexity**: Tests `save_temps_flag` with different values, `dumpdir` with and without trailing slashes, and multiple dump generation flags.

5. **Batch processing**: Compiles multiple files in one invocation to test cleanup between internal jobs.

The script creates a temporary directory, generates test files, runs the compiler with various flag combinations, and verifies the final program works correctly. Each compilation triggers the driver's initialization/cleanup cycle, exercising the target code block.
