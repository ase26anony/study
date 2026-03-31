Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc by testing various flag combinations and execution scenarios:

```bash
#!/bin/bash
set -e

# Configuration
GCC=${1:-gcc}
WORKDIR=$(mktemp -d)
cd "$WORKDIR"
echo "Working directory: $WORKDIR"

# Cleanup function
cleanup() {
    echo "Cleaning up..."
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

cat > pgo_test.c << 'EOF'
#include <stdio.h>
int main() {
    int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += i;
    }
    printf("Sum: %d\n", sum);
    return 0;
}
EOF

echo "=== Testing basic driver initialization with multiple jobs ==="
echo "Test 1: Multiple source files with sysroot and dump options"
$GCC hello1.c hello2.c -o multi_hello \
    --sysroot=/ \
    -save-temps \
    -dumpdir=./dumps \
    -fdump-tree-all \
    -ftime-report \
    -fuse-ld=gold \
    -v 2>&1 | grep -E "(sysroot|dump|save-temps)" || true
rm -f multi_hello *.o *.i *.s *.dump 2>/dev/null

echo -e "\nTest 2: Empty sysroot and varied dump options"
$GCC hello1.c hello2.c -o test2 \
    --sysroot= \
    -save-temps=obj \
    -dumpbase="test_dump" \
    -dumpbase-ext=".myext" \
    -fdump-rtl-expand \
    -fdump-ipa-all \
    -Wl,--verbose 2>&1 | tail -20
rm -f test2 *.o *.i *.s 2>/dev/null

echo -e "\nTest 3: Non-existent sysroot with isysroot"
$GCC hello1.c \
    --sysroot=/nonexistent/path \
    -isysroot /usr \
    -I/usr/include \
    -save-temps=cwd \
    -dumpdir="" \
    -o test3 2>&1 | grep -i "sysroot" || true
rm -f test3 *.o *.i *.s 2>/dev/null

echo -e "\n=== Testing help and version flags ==="
echo "Test 4: Help list and version combinations"
$GCC --help --target-help --version 2>&1 | head -5
echo "---"

echo -e "\nTest 5: Subprocess help with compilation flags"
$GCC --help=common --help=optimizers hello1.c -O2 2>&1 | head -10
echo "---"

echo -e "\nTest 6: Combined help and compilation"
$GCC hello1.c --help -dumpbase=help_test -save-temps 2>&1 | grep -E "(help|dump)" | head -5
rm -f *.o *.i *.s 2>/dev/null

echo -e "\n=== Testing linker selection flags ==="
echo "Test 7: Different linker backends"
for linker in bfd gold lld mold; do
    echo "Testing -fuse-ld=$linker"
    $GCC hello1.c -fuse-ld=$linker -Wl,--verbose -o test_$linker 2>&1 | grep -i "linker" | head -2 || true
    rm -f test_$linker 2>/dev/null
done

echo -e "\nTest 8: Linker flags with dump options"
$GCC hello1.c hello2.c \
    -fuse-ld=bfd \
    -Wl,-Map=output.map \
    -dumpbase="linker_test" \
    -save-temps \
    -o linker_test 2>&1 | tail -10
rm -f linker_test *.o *.i *.s output.map 2>/dev/null

echo -e "\n=== Testing dump file management ==="
echo "Test 9: Complex dump combinations"
$GCC hello1.c \
    -dumpdir="./my_dumps/" \
    -dumpbase="complex" \
    -dumpbase-ext=".dump" \
    -fdump-tree-all \
    -fdump-rtl-all \
    -fdump-ipa-all \
    -save-temps=obj \
    -o complex_test 2>&1 | grep -i "dump" | head -5
ls -la ./my_dumps/ 2>/dev/null || true
rm -rf ./my_dumps complex_test *.o *.i *.s 2>/dev/null

echo -e "\nTest 10: Empty dump options"
$GCC hello1.c \
    -dumpbase="" \
    -dumpbase-ext="" \
    -dumpdir="" \
    -fdump-tree-original \
    -o empty_dump_test 2>&1 | grep -i "dump" || true
rm -f empty_dump_test *.dump 2>/dev/null

echo -e "\n=== Testing timing and profile reports ==="
echo "Test 11: Time report with optimization"
$GCC hello1.c -O3 -ftime-report -o time_test 2>&1 | grep -A5 "Time report"
rm -f time_test 2>/dev/null

echo -e "\nTest 12: Profile-guided optimization workflow"
echo "Phase 1: Generate profile"
$GCC pgo_test.c -O2 -fprofile-generate -ftest-coverage -o pgo_instr
./pgo_instr > /dev/null 2>&1 || true

echo "Phase 2: Use profile with reports"
$GCC pgo_test.c -O2 -fprofile-use -fprofile-report -fprofile-correction \
    -ftime-report -o pgo_opt 2>&1 | grep -i "profile" | head -5

rm -f pgo_instr pgo_opt *.gcda *.gcno 2>/dev/null

echo -e "\n=== Testing comprehensive flag combinations ==="
echo "Test 13: All relevant flags in one invocation"
$GCC hello1.c hello2.c \
    --sysroot=/ \
    -save-temps \
    -dumpdir="./final_dumps" \
    -dumpbase="final" \
    -dumpbase-ext=".ext" \
    -fdump-tree-all \
    -fdump-rtl-expand \
    -ftime-report \
    -fuse-ld=lld \
    -O2 \
    --help=common \
    -v \
    -o final_program 2>&1 | tail -30

echo -e "\nTest 14: Reset behavior with verbose flag"
$GCC -v hello1.c --sysroot=/ -save-temps 2>&1 | grep -E "(sysroot|save-temps|drv)" | head -10

echo -e "\n=== Cleanup ==="
rm -rf *.o *.i *.s *.dump *.gcda *.gcno final_program ./final_dumps 2>/dev/null

echo -e "\nAll tests completed. Check output for driver initialization coverage."
```

This script systematically tests the uncovered initialization block by:

1. **Multiple Jobs**: Compiles multiple source files (`hello1.c hello2.c`) in single invocations
2. **Sysroot Variations**: Tests `--sysroot=/`, `--sysroot=`, `--sysroot=/nonexistent` with `-isysroot` and `-I`
3. **Dump Management**: Exercises `-dumpdir`, `-dumpbase`, `-dumpbase-ext` with various values and combinations with `-save-temps` and `-fdump-*` flags
4. **Help/Version Output**: Tests `--help`, `--target-help`, `--version`, `--help=common`, `--help=optimizers` alone and with compilation flags
5. **Linker Selection**: Tests `-fuse-ld=bfd/gold/lld/mold` with `-Wl,` options
6. **Timing/Profile Reports**: Tests `-ftime-report` and full PGO workflow with `-fprofile-generate`/`-fprofile-use`
7. **Combination Tests**: Includes invocations with multiple relevant flags to test interactions

The script captures output and cleans up between tests to avoid interference. The `-v` flag helps observe driver state changes, and error output is captured but doesn't stop execution (using `|| true` for expected failures like non-existent sysroot).
