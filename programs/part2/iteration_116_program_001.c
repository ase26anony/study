Here's a comprehensive test script that exercises the uncovered initialization logic in gcc.cc:

```bash
#!/bin/bash

# Test script for GCC driver initialization coverage
# Targets lines 11228-11250 in gcc.cc

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
void greet() {
    printf("Hello 2\n");
}
EOF

cat > pgo_test.c << 'EOF'
#include <stdio.h>
int main() {
    for (int i = 0; i < 1000; i++) {
        printf("PGO test iteration %d\n", i % 100);
    }
    return 0;
}
EOF

echo "Testing GCC driver initialization logic..."
echo "=========================================="

# Test 1: Multiple jobs with sysroot variations
echo -e "\n1. Testing multiple jobs with sysroot flags..."
$GCC hello1.c hello2.c -o multi --sysroot= -isysroot/usr/include -I/usr/local/include 2>&1 | head -20
rm -f multi

# Test 2: Empty sysroot
echo -e "\n2. Testing empty sysroot..."
$GCC hello1.c --sysroot= -o hello1 2>&1 | head -10
./hello1
rm -f hello1

# Test 3: Non-existent sysroot (should still initialize variables)
echo -e "\n3. Testing non-existent sysroot..."
$GCC hello1.c --sysroot=/nonexistent/path -o hello1 2>&1 | head -10 || true
rm -f hello1 2>/dev/null || true

# Test 4: Dump file generation with various options
echo -e "\n4. Testing dump file options..."
$GCC hello1.c -save-temps -dumpdir=./dumps -dumpbase=testbase -dumpbase-ext=.myext \
    -fdump-tree-all -fdump-rtl-expand -o hello1 2>&1 | head -20
rm -rf dumps hello1 *.i *.s *.o 2>/dev/null || true

# Test 5: Different save-temps modes
echo -e "\n5. Testing save-temps modes..."
for mode in "obj" "cwd" ""; do
    echo "  Testing -save-temps=$mode"
    $GCC hello1.c -save-temps=$mode -fdump-tree-original -o hello1 2>&1 | head -5
    rm -f hello1 *.i *.s *.o 2>/dev/null || true
done

# Test 6: Help and version flags
echo -e "\n6. Testing help and version flags..."
$GCC --help 2>&1 | head -5
$GCC --target-help 2>&1 | head -5
$GCC --version 2>&1 | head -5
$GCC --help=common 2>&1 | head -5
$GCC --help=optimizers 2>&1 | head -5

# Test 7: Combined help with compilation flags
echo -e "\n7. Testing help combined with other flags..."
$GCC --help --sysroot=/ -save-temps 2>&1 | head -5
$GCC --version -dumpbase=test 2>&1 | head -5

# Test 8: Linker selection flags
echo -e "\n8. Testing linker selection..."
for linker in bfd gold lld mold; do
    echo "  Testing -fuse-ld=$linker"
    $GCC hello1.c -fuse-ld=$linker -Wl,--verbose -o hello1 2>&1 | head -5 || true
    rm -f hello1 2>/dev/null || true
done

# Test 9: Time and profile reports
echo -e "\n9. Testing time and profile reports..."
$GCC hello1.c -ftime-report -fprofile-report -O2 -o hello1 2>&1 | head -20
./hello1
rm -f hello1

# Test 10: Comprehensive flag combination
echo -e "\n10. Testing comprehensive flag combination..."
$GCC hello1.c hello2.c \
    --sysroot=/ \
    -save-temps=obj \
    -dumpdir=./comprehensive_dumps \
    -dumpbase=comprehensive \
    -dumpbase-ext=.test \
    -fdump-tree-all \
    -fdump-rtl-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    -v \
    -o comprehensive_test 2>&1 | head -30
rm -rf comprehensive_dumps comprehensive_test *.i *.s *.o 2>/dev/null || true

# Test 11: Profile-guided optimization path
echo -e "\n11. Testing PGO workflow..."
echo "  Phase 1: Generate profile"
$GCC pgo_test.c -O2 -fprofile-generate -ftest-coverage -o pgo_instrumented 2>&1 | head -10
./pgo_instrumented > /dev/null
echo "  Phase 2: Use profile"
$GCC pgo_test.c -O2 -fprofile-use -fprofile-report -fprofile-correction -ftime-report -o pgo_optimized 2>&1 | head -20
./pgo_optimized > /dev/null
rm -f pgo_instrumented pgo_optimized *.gcda *.gcno 2>/dev/null || true

# Test 12: Reset variables with empty dump options
echo -e "\n12. Testing empty dump options..."
$GCC hello1.c -dumpdir= -dumpbase= -dumpbase-ext= -save-temps -o hello1 2>&1 | head -10
rm -f hello1 *.i *.s *.o 2>/dev/null || true

# Test 13: Multiple jobs with different optimization levels
echo -e "\n13. Testing multiple jobs with optimization variations..."
$GCC hello1.c -O0 -fdump-tree-optimized -o hello1_o0 \
    hello2.c -O3 -fdump-tree-optimized -o hello2_o3 2>&1 | head -15
rm -f hello1_o0 hello2_o3 2>/dev/null || true

# Test 14: Verbose flag to observe initialization
echo -e "\n14. Testing verbose output to observe driver state..."
$GCC hello1.c -v --sysroot= -save-temps -fdump-tree-original -ftime-report -o hello1 2>&1 | \
    grep -A5 -B5 "sysroot\|dumpdir\|Initializing" | head -20
rm -f hello1 *.i *.s *.o 2>/dev/null || true

echo -e "\n=========================================="
echo "All tests completed. Temporary directory: $TESTDIR"
echo "Note: Some tests may show warnings/errors for invalid paths or unsupported options."
echo "This is expected as we're testing the driver's initialization logic."
```

This script systematically tests the uncovered initialization block by:

1. **Multiple jobs**: Tests 1, 10, and 13 compile multiple files
2. **Sysroot variations**: Tests 1-3, 7, 10, 14 use `--sysroot=`, `-isysroot`, and invalid paths
3. **Dump file options**: Tests 4, 5, 10, 12, 13, 14 exercise `-dumpdir`, `-dumpbase`, `-save-temps`, `-fdump-*` flags
4. **Help/version output**: Tests 6-7 trigger `print_help_list`, `print_version`, `print_subprocess_help`
5. **Linker selection**: Test 8 sets `use_ld` with various `-fuse-ld=` options
6. **Timing reports**: Tests 9-11, 14 use `-ftime-report` and `-fprofile-report`
7. **PGO workflow**: Test 11 exercises the full profile-guided optimization path

The script creates temporary files, runs tests, and cleans up after itself. Some tests may produce warnings or errors (e.g., for invalid sysroot paths), but this is intentional to ensure the driver initializes all variables even in error conditions.
