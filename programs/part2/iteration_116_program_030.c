Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc by testing various flag combinations and execution scenarios:

```bash
#!/bin/bash
set -e

# Configuration
GCC=${1:-gcc}
WORKDIR=$(mktemp -d)
cd "$WORKDIR"

echo "Testing GCC driver initialization logic in: $WORKDIR"
echo "Using GCC: $($GCC --version | head -1)"

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

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    rm -f *.o *.i *.s *.o *.so *.gcda *.gcno *.gcov
    rm -f hello hello_pgo *.dump *.times *.opt *.expand
    rm -rf dumps temps
    rm -f test_output
}

# Test 1: Multiple jobs with sysroot variations
echo -e "\n=== Test 1: Multiple jobs with sysroot ==="
$GCC hello1.c hello2.c -o hello_multi --sysroot= -isysroot/usr/include -I/usr/local/include 2>&1 | head -20
./hello_multi 2>/dev/null && echo "Multi-job compilation successful"

cleanup

# Test 2: Dump file management with various options
echo -e "\n=== Test 2: Dump file management ==="
$GCC hello1.c -save-temps=obj -dumpdir=./dumps -dumpbase=testfile \
    -dumpbase-ext=.dump -fdump-tree-all -fdump-rtl-expand -o hello_dump 2>&1 | grep -i "dump\|save" | head -10

# Check if dump files were created
if [ -d "dumps" ] || [ -f "hello1.i" ] || [ -f "hello1.s" ]; then
    echo "Dump/temp files created successfully"
fi

cleanup

# Test 3: Different save-temps options
echo -e "\n=== Test 3: Various save-temps modes ==="
for mode in "cwd" "obj" ""; do
    echo "Testing -save-temps=$mode"
    $GCC hello1.c -save-temps=$mode -o hello_temp_$mode 2>&1 >/dev/null
    ls -la hello1.* 2>/dev/null | grep -E "\.i$|\.s$|\.o$" || true
    cleanup
done

# Test 4: Help and version flags
echo -e "\n=== Test 4: Help and version output ==="
$GCC --help | head -5
$GCC --target-help 2>&1 | head -5
$GCC --version
$GCC --help=common 2>&1 | head -5
$GCC --help=optimizers 2>&1 | head -5

# Test 5: Help flags combined with compilation
echo -e "\n=== Test 5: Help flags with compilation ==="
$GCC --help hello1.c 2>&1 | head -10
$GCC --version hello1.c 2>&1 | head -5

# Test 6: Linker selection flags
echo -e "\n=== Test 6: Linker selection ==="
for linker in bfd gold lld mold; do
    echo "Testing -fuse-ld=$linker"
    $GCC hello1.c -fuse-ld=$linker -Wl,--verbose -o hello_$linker 2>&1 >/dev/null || \
        echo "Linker $linker not available (continuing)"
    rm -f hello_$linker
done

# Test 7: Comprehensive flag combination
echo -e "\n=== Test 7: Comprehensive flag combination ==="
$GCC hello1.c hello2.c \
    --sysroot=/ \
    -save-temps \
    -dumpdir=./dumps \
    -fdump-tree-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    -v \
    -o hello_comprehensive 2>&1 | tail -20

./hello_comprehensive 2>/dev/null && echo "Comprehensive test successful"

cleanup

# Test 8: Profile-guided optimization path
echo -e "\n=== Test 8: PGO with timing reports ==="

# Create a test program that generates profile data
cat > pgo_test.c << 'EOF'
#include <stdio.h>
int main() {
    for (int i = 0; i < 1000; i++) {
        printf("Iteration %d\n", i % 100);
    }
    return 0;
}
EOF

# Step 1: Generate profile
echo "Step 1: Profile generation"
$GCC pgo_test.c -O2 -fprofile-generate -ftest-coverage -o pgo_gen
./pgo_gen > test_output 2>/dev/null

# Step 2: Use profile with timing report
echo "Step 2: Profile usage with timing report"
$GCC pgo_test.c -O2 -fprofile-use -fprofile-report -ftime-report -o pgo_use 2>&1 | \
    grep -E "time|profile|report" | head -10

./pgo_use 2>/dev/null | head -2 && echo "PGO test successful"

cleanup

# Test 9: Empty and non-existent sysroot
echo -e "\n=== Test 9: Sysroot edge cases ==="
$GCC hello1.c --sysroot= -o hello_empty_sysroot 2>&1 | grep -i "sysroot" || true
$GCC hello1.c --sysroot=/nonexistent/path -o hello_bad_sysroot 2>&1 | grep -i "sysroot\|warn\|error" | head -5 || true

# Test 10: Verbose only flag simulation
echo -e "\n=== Test 10: Verbose output ==="
$GCC hello1.c -v -o hello_verbose 2>&1 | grep -E "COLLECT_GCC_OPTIONS|sysroot|dump" | head -10

# Test 11: Multiple dumpbase variations
echo -e "\n=== Test 11: Dumpbase variations ==="
$GCC hello1.c -dumpbase="" -dumpbase-ext="" -fdump-tree-original -o hello_empty_dump 2>&1 >/dev/null
$GCC hello1.c -dumpbase="mydump" -dumpbase-ext=".myext" -fdump-tree-optimized -o hello_custom_dump 2>&1 >/dev/null

# Test 12: Combined help and dump flags
echo -e "\n=== Test 12: Combined help and dump flags ==="
$GCC --help=warnings -dumpbase=helpdump -fdump-tree-cfg hello1.c 2>&1 | head -15

# Final cleanup
cleanup
cd ..
rm -rf "$WORKDIR"

echo -e "\n=== All tests completed ==="
echo "The GCC driver initialization logic has been exercised with:"
echo "1. Multiple compilation jobs"
echo "2. Sysroot variations (empty, valid, invalid)"
echo "3. Dump file management flags"
echo "4. Help and version output requests"
echo "5. Linker selection flags"
echo "6. Timing and profile reports"
echo "7. Various flag combinations"
```

This script systematically tests the uncovered initialization block by:

1. **Multiple Jobs**: Compiles multiple source files in single invocations
2. **Sysroot Variations**: Tests `--sysroot=` (empty), `--sysroot=/`, `--sysroot=/nonexistent`, and `-isysroot`
3. **Dump Management**: Uses `-save-temps` with different values, `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, and `-fdump-*` flags
4. **Help/Version**: Tests `--help`, `--target-help`, `--version`, `--help=common`, `--help=optimizers`
5. **Linker Selection**: Tests `-fuse-ld=bfd/gold/lld/mold` with `-Wl` options
6. **Timing Reports**: Uses `-ftime-report` and `-fprofile-report` with PGO workflow
7. **Combinations**: Tests multiple flags together to exercise interaction logic

The script creates temporary files, runs tests, cleans up, and provides feedback on what was tested. Each test is designed to trigger specific parts of the initialization logic while ensuring the driver proceeds through its normal execution flow.
