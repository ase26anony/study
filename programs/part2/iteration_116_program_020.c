Here's a comprehensive test script that exercises the uncovered initialization logic in gcc.cc:

```bash
#!/bin/bash
set -e

# Configuration
GCC=${GCC:-gcc}
TEST_DIR=$(mktemp -d /tmp/gcc_test_XXXXXX)
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    cd /
    rm -rf "$TEST_DIR"
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
    printf("Hello World 2\n");
}
EOF

cat > hello3.c << 'EOF'
#include <stdio.h>
void helper2() {
    printf("Hello World 3\n");
}
EOF

echo "Testing GCC driver initialization logic..."
echo "=========================================="

# Test 1: Multiple jobs with sysroot variations
echo -e "\n1. Testing multiple jobs with sysroot flags..."
$GCC hello1.c hello2.c hello3.c -o multi \
    --sysroot= \
    --sysroot=/nonexistent \
    -isysroot/usr \
    -I/usr/include \
    -I/usr/local/include 2>&1 | grep -q "error" || true

# Test 2: Dump file management with various options
echo -e "\n2. Testing dump file management..."
$GCC hello1.c -save-temps=obj -dumpdir=./dumps1 -dumpbase=test1 \
    -dumpbase-ext=.dump -fdump-tree-all -fdump-rtl-expand -o test1 2>&1 | tail -5

$GCC hello2.c -save-temps=cwd -dumpdir= -dumpbase=test2 \
    -fdump-ipa-all -o test2 2>&1 | tail -5

$GCC hello3.c -save-temps -dumpdir=./dumps2/ -dumpbase=test3 \
    -dumpbase-ext= -fdump-tree-original -o test3 2>&1 | tail -5

# Test 3: Help and version output
echo -e "\n3. Testing help/version flags..."
$GCC --help > /dev/null
$GCC --target-help > /dev/null
$GCC --version > /dev/null
$GCC --help=common > /dev/null
$GCC --help=optimizers > /dev/null
$GCC --help=warnings > /dev/null

# Test 4: Combined help with compilation flags
echo -e "\n4. Testing combined help/compilation..."
$GCC --help --sysroot=/ -save-temps hello1.c -o dummy1 2>&1 | head -20
$GCC --version -dumpbase=test -fdump-tree-all hello2.c -o dummy2 2>&1 | head -5

# Test 5: Linker selection flags
echo -e "\n5. Testing linker selection..."
for linker in bfd gold lld mold; do
    echo -n "  Testing -fuse-ld=$linker... "
    $GCC hello1.c -fuse-ld=$linker -Wl,--verbose -o test_$linker 2>&1 | grep -q "error" && echo "not available" || echo "ok"
done

# Test 6: Verbose flag with initialization
echo -e "\n6. Testing verbose output with initialization flags..."
$GCC -v -O0 --sysroot= -save-temps -fdump-tree-original \
    -ftime-report hello1.c -o verbose_test 2>&1 | grep -E "(sysroot|dump|Target)"

# Test 7: Comprehensive flag combination
echo -e "\n7. Testing comprehensive flag combination..."
$GCC -O2 -fuse-ld=bfd --help=optimizers -dumpbase=comprehensive \
    -dumpbase-ext=.dump -fdump-rtl-all hello1.c hello2.c \
    --sysroot=/ -save-temps -ftime-report -o comprehensive_test 2>&1 | tail -10

# Test 8: Profile-guided optimization path
echo -e "\n8. Testing PGO and timing reports..."
# Step 1: Generate profile
$GCC -O2 -fprofile-generate -ftest-coverage hello1.c -o pgo_gen
./pgo_gen 2>/dev/null || true

# Step 2: Use profile with timing report
$GCC -O2 -fprofile-use -fprofile-report -fprofile-correction \
    -ftime-report hello1.c -o pgo_use 2>&1 | grep -E "(profile|time)"

# Test 9: Multiple jobs with all relevant flags
echo -e "\n9. Testing multiple jobs with all relevant flags..."
$GCC hello1.c hello2.c hello3.c \
    --sysroot=/ \
    -save-temps \
    -dumpdir=./final_dumps \
    -fdump-tree-all \
    -ftime-report \
    -fuse-ld=gold \
    -v \
    -O2 \
    -o final_multi 2>&1 | grep -E "(sysroot|dump|Target|Using)" | head -20

# Test 10: Empty and NULL flag variations
echo -e "\n10. Testing edge cases with empty values..."
$GCC hello1.c \
    --sysroot= \
    -dumpdir= \
    -dumpbase= \
    -dumpbase-ext= \
    -save-temps=obj \
    -o edge_test 2>&1 | tail -5

# Clean generated files
rm -f *.o *.s *.i *.dump *.gcda *.gcno *.gcov
rm -f test* pgo* verbose* comprehensive* final* edge* dummy* multi
rm -rf dumps1 dumps2 final_dumps

echo -e "\n=========================================="
echo "All tests completed. Check for any errors above."
echo "The uncovered initialization block should have been executed multiple times."
echo "Test directory: $TEST_DIR"
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple jobs**: Compiles multiple source files in single invocations
2. **Sysroot variations**: Tests empty, non-existent, and default sysroots with `-isysroot` and `-I` flags
3. **Dump management**: Exercises all dump-related flags with various combinations
4. **Help/version output**: Tests all help variants and their interactions with compilation
5. **Linker selection**: Tests all available linker backends
6. **Verbose output**: Uses `-v` to observe internal state
7. **Comprehensive combination**: Tests complex flag interactions
8. **PGO path**: Exercises profile generation and use with timing reports
9. **Edge cases**: Tests empty values for dump-related flags

The script captures errors but continues execution, allowing observation of how the driver handles various flag combinations. Each test invocation triggers the initialization block, ensuring coverage of the reset logic for global variables.
