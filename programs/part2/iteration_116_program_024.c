Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc by testing various flag combinations and execution scenarios:

```bash
#!/bin/bash
set -e

# Configuration
GCC=${1:-gcc}
TESTDIR=$(mktemp -d /tmp/gcc_test_XXXXXX)
echo "Test directory: $TESTDIR"
cd "$TESTDIR"

# Create simple test programs
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
    for (int i = 0; i < 100; i++) {
        sum += i;
    }
    printf("Sum: %d\n", sum);
    return 0;
}
EOF

# Function to clean up dump files
cleanup_dumps() {
    rm -f *.i *.s *.o *.ii *.bc *.expand *.original *.gimple *.cfg *.optimized
    rm -f *.c.* *.c.*.* *.times *.gcda *.gcno
    rm -rf dumps/
}

# Test 1: Multiple jobs with sysroot variations
echo "=== Test 1: Multiple jobs with sysroot ==="
cleanup_dumps
$GCC hello1.c hello2.c -o test1 \
    --sysroot= \
    -isysroot /usr \
    -I/usr/include \
    -v 2>&1 | grep -q "sysroot" || true
echo "Exit code: $?"

# Test 2: Dump file management with various options
echo -e "\n=== Test 2: Dump file management ==="
cleanup_dumps
$GCC hello1.c -o test2 \
    -save-temps=obj \
    -dumpdir=./dumps \
    -dumpbase="mydump" \
    -dumpbase-ext=".ext" \
    -fdump-tree-all \
    -fdump-rtl-expand \
    -fdump-ipa-all 2>&1 | tail -5
ls -la dumps/ 2>/dev/null || true

# Test 3: Different save-temps modes
echo -e "\n=== Test 3: Various save-temps modes ==="
for mode in "cwd" "obj" ""; do
    cleanup_dumps
    if [ -z "$mode" ]; then
        $GCC hello1.c -save-temps -o test3_ 2>&1 >/dev/null
    else
        $GCC hello1.c -save-temps=$mode -o test3_ 2>&1 >/dev/null
    fi
    echo "Mode '$mode': $(ls *.i *.s *.o 2>/dev/null | wc -l) temp files"
done

# Test 4: Help and version flags
echo -e "\n=== Test 4: Help and version output ==="
$GCC --help > /dev/null
echo "Basic help exit: $?"
$GCC --target-help 2>&1 | head -5
$GCC --version
$GCC --help=common 2>&1 | head -3
$GCC --help=optimizers 2>&1 | head -3
$GCC --help=warnings 2>&1 | head -3

# Test 5: Linker selection flags
echo -e "\n=== Test 5: Linker selection ==="
for linker in bfd gold lld mold; do
    if $GCC -fuse-ld=$linker --help=linker 2>&1 | grep -q "supported"; then
        echo "Testing linker: $linker"
        $GCC hello1.c -fuse-ld=$linker -Wl,--verbose -o test5_$linker 2>&1 >/dev/null || true
    fi
done

# Test 6: Combined flags in single invocation
echo -e "\n=== Test 6: Combined flags ==="
cleanup_dumps
$GCC hello1.c hello2.c -o test6 \
    --sysroot=/ \
    -save-temps \
    -dumpdir="./combined_dumps" \
    -fdump-tree-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    -v 2>&1 | grep -E "(sysroot|dump|time report|Using built-in specs)" | head -10

# Test 7: Profile-guided optimization path
echo -e "\n=== Test 7: PGO and timing reports ==="
cleanup_dumps
# Generate profile
$GCC pgo_test.c -O2 -fprofile-generate -o pgo_instrumented
./pgo_instrumented 2>&1 >/dev/null

# Use profile with timing report
$GCC pgo_test.c -O2 -fprofile-use \
    -ftime-report \
    -fprofile-report \
    -o pgo_optimized 2>&1 | grep -E "(time report|profile)" || true

# Test 8: Empty and non-existent sysroot
echo -e "\n=== Test 8: Sysroot edge cases ==="
$GCC hello1.c --sysroot="" -o test8_empty 2>&1 | grep -i "sysroot" || true
$GCC hello1.c --sysroot="/nonexistent/path/$(date +%s)" -o test8_nonexist 2>&1 | grep -i "sysroot" || true

# Test 9: Print subprocess help during compilation
echo -e "\n=== Test 9: Subprocess help with compilation ==="
$GCC hello1.c --help=common -o test9 2>&1 | head -5
echo "Exit code: $?"

# Test 10: Verbose only flag simulation
echo -e "\n=== Test 10: Verbose output ==="
$GCC hello1.c -v -### -o test10 2>&1 | grep -E "(COLLECT_GCC|sysroot)" | head -5

# Test 11: Reset variables with multiple sequential jobs
echo -e "\n=== Test 11: Sequential compilations ==="
for i in 1 2 3; do
    $GCC hello1.c -save-temps=$([ $i -eq 2 ] && echo "cwd") \
        --sysroot=$([ $i -eq 3 ] && echo "/") \
        -dumpbase="seq$i" \
        -o test11_$i
done
ls -la test11_* 2>/dev/null | wc -l

# Test 12: All dump-related flags together
echo -e "\n=== Test 12: Comprehensive dump flags ==="
cleanup_dumps
mkdir -p alldumps
$GCC hello1.c -o test12 \
    -save-temps=obj \
    -dumpdir="alldumps/" \
    -dumpbase="full" \
    -dumpbase-ext=".dump" \
    -fdump-tree-all \
    -fdump-rtl-all \
    -fdump-ipa-all \
    -fdump-noaddr \
    -fdump-unnumbered \
    -fdump-passes 2>&1 >/dev/null
find alldumps -name "*.dump*" 2>/dev/null | wc -l

# Cleanup
echo -e "\n=== Cleaning up ==="
cd /
rm -rf "$TESTDIR"
echo "Test completed successfully"
```

This script systematically tests the uncovered initialization block by:

1. **Multiple jobs**: Compiles multiple files (`hello1.c hello2.c`) in single invocations
2. **Sysroot variations**: Tests `--sysroot=`, `--sysroot=/`, `-isysroot`, and non-existent paths
3. **Dump management**: Exercises `-save-temps` with all modes, `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, and various `-fdump-*` flags
4. **Help/version output**: Tests `--help`, `--target-help`, `--version`, `--help=common`, etc.
5. **Linker selection**: Tests `-fuse-ld=` with all supported linkers
6. **Timing reports**: Uses `-ftime-report` and `-fprofile-report`
7. **PGO workflow**: Demonstrates `-fprofile-generate` → execution → `-fprofile-use` cycle
8. **Combined flags**: Tests multiple relevant flags in single invocations
9. **Sequential jobs**: Ensures variables reset between compilations

The script creates temporary directories, cleans up between tests, and handles errors gracefully to ensure maximum coverage of the initialization logic.
