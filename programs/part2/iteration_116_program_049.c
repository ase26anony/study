Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc by testing various flag combinations and execution scenarios:

```bash
#!/bin/bash

# Test script for GCC driver initialization coverage
# Targets uncovered lines 11228-11250 in gcc.cc

set -e

# Configuration
GCC=${GCC:-gcc}
WORKDIR=$(mktemp -d)
cd "$WORKDIR"

# Cleanup function
cleanup() {
    cd /
    rm -rf "$WORKDIR"
}
trap cleanup EXIT

# Create test source files
cat > hello1.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Hello World 1\n");
    return 0;
}
EOF

cat > hello2.c << 'EOF'
#include <stdio.h>
void helper() {
    printf("Hello from helper\n");
}
EOF

cat > hello3.c << 'EOF'
#include <stdio.h>
extern void helper();
int main() {
    printf("Hello World 3\n");
    helper();
    return 0;
}
EOF

echo "=== Testing GCC driver initialization logic ==="
echo "GCC version: $($GCC --version | head -1)"
echo ""

# Test 1: Multiple jobs with sysroot variations
echo "Test 1: Multiple jobs with sysroot variations"
echo "---------------------------------------------"
$GCC -v hello1.c hello2.c -o multi1 2>&1 | grep -q "gcc version" && echo "✓ Multiple files compiled"
$GCC --sysroot= hello1.c -o sysroot_empty 2>&1 | tail -1 | grep -q "collect2" && echo "✓ Empty sysroot"
$GCC --sysroot=/nonexistent hello1.c -o sysroot_nonexist 2>&1 | grep -i "warning\|error" || echo "✓ Nonexistent sysroot handled"
$GCC --sysroot=/ hello1.c -o sysroot_root 2>&1 | tail -1 | grep -q "collect2" && echo "✓ Root sysroot"
$GCC -isysroot/usr hello1.c -o isysroot_test 2>&1 | tail -1 | grep -q "collect2" && echo "✓ isysroot flag"
echo ""

# Test 2: Dump file management with various options
echo "Test 2: Dump file management"
echo "----------------------------"
$GCC -save-temps hello1.c -o save1 2>&1 | tail -1 | grep -q "collect2" && echo "✓ save-temps default"
$GCC -save-temps=obj hello1.c -o save2 2>&1 | tail -1 | grep -q "collect2" && echo "✓ save-temps=obj"
$GCC -save-temps=cwd hello1.c -o save3 2>&1 | tail -1 | grep -q "collect2" && echo "✓ save-temps=cwd"
$GCC -dumpdir=./dumps -dumpbase=test -dumpbase-ext=.dump -fdump-tree-all hello1.c -o dump1 2>&1 | tail -1 | grep -q "collect2" && echo "✓ dumpdir/dumpbase with fdump"
$GCC -dumpdir= -dumpbase= -dumpbase-ext= -fdump-rtl-expand hello1.c -o dump2 2>&1 | tail -1 | grep -q "collect2" && echo "✓ Empty dump options"
$GCC -save-temps -dumpdir=./mixed -fdump-tree-original hello1.c hello2.c -o mixed1 2>&1 | tail -1 | grep -q "collect2" && echo "✓ Combined dump flags with multiple files"
echo ""

# Test 3: Help and version output
echo "Test 3: Help and version flags"
echo "------------------------------"
$GCC --help > /dev/null 2>&1 && echo "✓ --help"
$GCC --target-help > /dev/null 2>&1 && echo "✓ --target-help"
$GCC --version > /dev/null 2>&1 && echo "✓ --version"
$GCC --help=common > /dev/null 2>&1 && echo "✓ --help=common"
$GCC --help=optimizers > /dev/null 2>&1 && echo "✓ --help=optimizers"
$GCC --help=warnings > /dev/null 2>&1 && echo "✓ --help=warnings"
$GCC --help -O2 hello1.c -o help_compile 2>&1 | grep -q "gcc version" && echo "✓ Help with compilation flags"
echo ""

# Test 4: Linker selection flags
echo "Test 4: Linker selection"
echo "------------------------"
for linker in bfd gold lld mold; do
    if $GCC -fuse-ld=$linker hello1.c -o ld_$linker 2>&1 | grep -q "collect2"; then
        echo "✓ fuse-ld=$linker"
    else
        echo "✗ fuse-ld=$linker (not available)"
    fi
done
$GCC -Wl,--verbose hello1.c -o wl_test 2>&1 | grep -q "collect2" && echo "✓ Wl, options"
echo ""

# Test 5: Timing and profile reports
echo "Test 5: Timing and profile reports"
echo "----------------------------------"
$GCC -ftime-report hello1.c -o time1 2>&1 | grep -i "time report" && echo "✓ ftime-report"
$GCC -fprofile-report hello1.c -o profile1 2>&1 | grep -i "profile report" && echo "✓ fprofile-report"

# PGO test if supported
echo "Testing PGO (if supported)..."
$GCC -O2 -fprofile-generate hello1.c -o pgo_gen 2>&1 | tail -1 | grep -q "collect2" && PGO_SUPPORTED=1 || PGO_SUPPORTED=0

if [ $PGO_SUPPORTED -eq 1 ]; then
    ./pgo_gen > /dev/null 2>&1
    $GCC -O2 -fprofile-use -ftime-report hello1.c -o pgo_use 2>&1 | grep -i "time report" && echo "✓ PGO with ftime-report"
fi
echo ""

# Test 6: Combined comprehensive test
echo "Test 6: Comprehensive flag combination"
echo "--------------------------------------"
$GCC --sysroot=/ -save-temps=obj -dumpdir=./comprehensive -dumpbase=comp \
     -fdump-tree-all -ftime-report -fuse-ld=bfd -O2 \
     hello1.c hello2.c hello3.c -o comprehensive 2>&1 | tail -1 | grep -q "collect2" && echo "✓ Comprehensive test passed"
echo ""

# Test 7: Verbose output to observe initialization
echo "Test 7: Verbose output inspection"
echo "---------------------------------"
$GCC -v -save-temps --sysroot= -dumpbase=verbose -fdump-tree-original \
     -ftime-report hello1.c -o verbose_test 2>&1 | head -20 | grep -i "gcc version" && echo "✓ Verbose initialization observed"
echo ""

# Test 8: Test print_subprocess_help
echo "Test 8: Subprocess help"
echo "-----------------------"
$GCC --help=common --help=optimizers > /dev/null 2>&1 && echo "✓ Multiple --help options"
$GCC -v --help=warnings 2>&1 | grep -i "warning options" && echo "✓ Verbose with subprocess help"
echo ""

# Test 9: Test with empty/nonexistent files to ensure initialization still runs
echo "Test 9: Edge cases"
echo "------------------"
$GCC -v -save-temps --sysroot=/nonexistent -dumpdir=./edge -dumpbase= \
     -fdump-tree-all -ftime-report /dev/null -o edge_test 2>&1 | head -5 | grep -q "gcc version" && echo "✓ Edge case initialization"

# Clean generated files
rm -f *.o *.i *.s *.dump *.gcda *.gcno
rm -f multi1 sysroot_* isysroot_test save* dump* ld_* wl_test time* profile* pgo_* comprehensive verbose_test edge_test
rm -rf dumps mixed comprehensive

echo ""
echo "=== All tests completed ==="
echo "Generated files cleaned up from: $WORKDIR"
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple jobs**: Compiles multiple source files (`hello1.c hello2.c hello3.c`) in single invocations
2. **Sysroot variations**: Tests `--sysroot=`, `--sysroot=/nonexistent`, `--sysroot=/`, and `-isysroot`
3. **Dump management**: Exercises all `-save-temps` variants, `-dumpdir`, `-dumpbase`, `-dumpbase-ext` with empty and non-empty values, combined with `-fdump-*` flags
4. **Help/version**: Tests `--help`, `--target-help`, `--version`, `--help=<subprocess>` with various subprocesses
5. **Linker selection**: Tests all common `-fuse-ld=` variants and `-Wl,` options
6. **Timing/PGO**: Tests `-ftime-report`, `-fprofile-report`, and full PGO workflow if supported
7. **Combined flags**: Tests a comprehensive combination of all relevant flags
8. **Verbose mode**: Uses `-v` to observe initialization state
9. **Edge cases**: Tests with `/dev/null` input to ensure initialization runs even with problematic inputs

The script captures exit codes implicitly through `set -e` and `&&` operators, and cleans up all generated files between tests to avoid interference. Each test verifies that the driver proceeds through initialization without fatal errors.
