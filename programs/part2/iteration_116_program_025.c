Here's a comprehensive test script that exercises the uncovered initialization logic in gcc.cc:

```bash
#!/bin/bash
set -e

# Configuration
GCC=${GCC:-gcc}
TESTDIR=$(mktemp -d /tmp/gcc_test_XXXXXX)
trap "rm -rf $TESTDIR" EXIT

cd $TESTDIR

# Create simple test programs
cat > hello1.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Hello 1\n");
    return 0;
}
EOF

cat > hello2.c << 'EOF'
#include <stdio.h>
void helper() {
    printf("Helper\n");
}
EOF

cat > hello3.c << 'EOF'
#include <stdio.h>
extern void helper();
int main() {
    printf("Hello 3\n");
    helper();
    return 0;
}
EOF

# Create a dummy sysroot directory structure
mkdir -p dummy_sysroot/usr/include
mkdir -p dummy_sysroot/usr/lib
echo "#define DUMMY 1" > dummy_sysroot/usr/include/dummy.h

echo "Testing GCC driver initialization logic..."
echo "=========================================="

# Test 1: Multiple jobs with sysroot variations
echo -e "\n1. Testing multiple jobs with sysroot flags..."
$GCC hello1.c hello2.c hello3.c -o multi_hello \
    --sysroot= \
    --sysroot=/nonexistent \
    -isysroot $TESTDIR/dummy_sysroot \
    -I$TESTDIR/dummy_sysroot/usr/include \
    -v 2>&1 | grep -q "sysroot" || true

# Test 2: Dump file management with various options
echo -e "\n2. Testing dump file management..."
$GCC hello1.c -save-temps=obj -dumpdir=./dumps -dumpbase=test1 \
    -dumpbase-ext=.dump -fdump-tree-all -fdump-rtl-expand \
    -o hello1 2>&1 | tail -5

# Clean up for next test
rm -f hello1 *.o *.i *.s *.dump* 2>/dev/null || true
rm -rf dumps 2>/dev/null || true

# Test 3: Different save-temps modes
echo -e "\n3. Testing save-temps variations..."
for mode in "cwd" "obj" ""; do
    echo "  Testing -save-temps=$mode"
    $GCC hello1.c -save-temps=$mode -fdump-tree-original -o hello1_$mode 2>/dev/null || true
    rm -f hello1_$mode *.i *.s *.o 2>/dev/null || true
done

# Test 4: Help and version flags
echo -e "\n4. Testing help and version output..."
$GCC --help > /dev/null
$GCC --target-help 2>&1 | head -5
$GCC --version | head -1
$GCC --help=common 2>&1 | grep -q "common" || true
$GCC --help=optimizers 2>&1 | grep -q "optimizers" || true

# Test 5: Combined help with compilation flags
echo -e "\n5. Testing help flags with compilation..."
$GCC --help -O2 hello1.c -o /dev/null 2>&1 | head -2

# Test 6: Linker selection flags
echo -e "\n6. Testing linker selection..."
for linker in bfd gold lld mold; do
    echo "  Testing -fuse-ld=$linker"
    $GCC hello1.c -fuse-ld=$linker -Wl,--verbose -o hello1_$linker 2>/dev/null || true
    rm -f hello1_$linker 2>/dev/null || true
done

# Test 7: Timing and profile reports
echo -e "\n7. Testing timing and profile reports..."
$GCC hello1.c -ftime-report -fprofile-report -O2 -o hello1_timing 2>&1 | grep -E "(time|profile)" || true
rm -f hello1_timing 2>/dev/null || true

# Test 8: PGO workflow (simplified)
echo -e "\n8. Testing PGO workflow..."
# Compile with profile generation
$GCC hello1.c -fprofile-generate -o hello1_instr 2>/dev/null || true
# Run instrumented program (if created)
if [ -x hello1_instr ]; then
    ./hello1_instr 2>/dev/null || true
    # Compile with profile use
    $GCC hello1.c -fprofile-use -ftime-report -fprofile-correction \
        -o hello1_pgo 2>/dev/null || true
    rm -f hello1_instr hello1_pgo *.gcda *.gcno 2>/dev/null || true
fi

# Test 9: Comprehensive flag combination
echo -e "\n9. Testing comprehensive flag combination..."
$GCC hello1.c hello2.c \
    --sysroot=$TESTDIR/dummy_sysroot \
    -save-temps \
    -dumpdir=./comprehensive_dumps \
    -dumpbase=comprehensive \
    -dumpbase-ext=.test \
    -fdump-tree-all \
    -fdump-rtl-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    -v \
    -o comprehensive_test 2>&1 | tail -10

# Test 10: Empty dump options
echo -e "\n10. Testing empty dump options..."
$GCC hello1.c -dumpdir= -dumpbase= -dumpbase-ext= -save-temps=cwd \
    -o empty_dump_test 2>/dev/null || true
rm -f empty_dump_test *.i *.s *.o 2>/dev/null || true

# Test 11: Multiple sysroot changes
echo -e "\n11. Testing multiple sysroot changes..."
$GCC hello1.c \
    --sysroot=/ \
    --sysroot=$TESTDIR/dummy_sysroot \
    -isysroot /usr \
    -target-sysroot=/ \
    -o sysroot_test 2>/dev/null || true
rm -f sysroot_test 2>/dev/null || true

# Test 12: Verbose only flag simulation
echo -e "\n12. Testing verbose output..."
$GCC hello1.c -v -### 2>&1 | grep -q "COLLECT_GCC_OPTIONS" || true

# Cleanup
echo -e "\nCleaning up..."
rm -rf *.o *.i *.s *.dump* *.test* comprehensive_dumps 2>/dev/null || true

echo -e "\nAll tests completed. Check that the driver executed the initialization block for each job."
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple Jobs**: Compiles multiple files (`hello1.c hello2.c hello3.c`) in single invocations
2. **Sysroot Variations**: Tests `--sysroot=`, `--sysroot=/nonexistent`, `-isysroot`, and `-I` flags
3. **Dump Management**: Exercises `-save-temps` with different values, `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, and `-fdump-*` flags
4. **Help/Version**: Tests `--help`, `--target-help`, `--version`, `--help=common`, `--help=optimizers`
5. **Linker Selection**: Tests `-fuse-ld=bfd/gold/lld/mold` with `-Wl,` options
6. **Timing/Profile**: Tests `-ftime-report`, `-fprofile-report`, and PGO workflow
7. **Combinations**: Tests comprehensive flag combinations that trigger multiple initialization variables

The script creates temporary directories, cleans up after itself, and handles cases where some flags might not be supported on all GCC builds. Each test invocation should cause the driver to execute the uncovered initialization block at lines 11228-11250.
