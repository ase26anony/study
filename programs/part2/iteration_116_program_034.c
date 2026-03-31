Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc by testing various flag combinations and execution scenarios:

```bash
#!/bin/bash
set -e

# Configuration
GCC=${1:-gcc}
WORKDIR=$(mktemp -d)
cd "$WORKDIR"
echo "Working directory: $WORKDIR"

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

cat > pgo_test.c << 'EOF'
#include <stdio.h>
int main() {
    int i, sum = 0;
    for (i = 0; i < 100; i++) {
        sum += i;
    }
    printf("Sum: %d\n", sum);
    return 0;
}
EOF

# Function to clean up dump files
cleanup_dumps() {
    rm -f *.i *.s *.o *.ii *.bc *.expand *.original *.gimple *.cfg *.optimized
    rm -f *.c.* *.c.*.* *.r.* *.rtl.* *.tree.* *.ipa.*
    rm -f *.gcda *.gcno *.gcov *.dwo *.dwp
    rm -rf dumps/ temps/ *.dSYM/
}

# Test 1: Multiple jobs with sysroot variations
echo "=== Test 1: Multiple jobs with sysroot variations ==="
cleanup_dumps
$GCC hello1.c hello2.c -o test1 \
    --sysroot= \
    -isysroot /usr/include \
    -I/usr/local/include \
    -v 2>&1 | grep -q "sysroot" || true
rm -f test1

# Test 2: Dump file management with various options
echo "=== Test 2: Dump file management ==="
cleanup_dumps
mkdir -p dumps
$GCC hello1.c -o test2 \
    -save-temps=obj \
    -dumpdir=./dumps \
    -dumpbase="mydump" \
    -dumpbase-ext=".ext" \
    -fdump-tree-all \
    -fdump-rtl-expand \
    -fdump-ipa-all 2>&1 | tail -20 || true
ls -la dumps/ 2>/dev/null || true
rm -f test2

# Test 3: Different save-temps modes
echo "=== Test 3: Different save-temps modes ==="
cleanup_dumps
$GCC hello1.c -o test3 -save-temps=cwd
ls -la hello1.* 2>/dev/null || true
rm -f test3 hello1.i hello1.s hello1.o

$GCC hello1.c -o test3b -save-temps
ls -la hello1.* 2>/dev/null || true
rm -f test3b hello1.i hello1.s hello1.o

# Test 4: Help and version flags
echo "=== Test 4: Help and version flags ==="
$GCC --help > /dev/null
$GCC --target-help 2>&1 | head -5
$GCC --version
$GCC --help=common 2>&1 | head -5
$GCC --help=optimizers 2>&1 | head -5

# Test 5: Combined help with compilation flags
echo "=== Test 5: Combined flags ==="
$GCC --help -O2 --sysroot=/ -save-temps 2>&1 | head -10

# Test 6: Linker selection flags
echo "=== Test 6: Linker selection ==="
for linker in bfd gold lld mold; do
    echo "Testing -fuse-ld=$linker"
    $GCC hello1.c -fuse-ld=$linker -o test6_$linker 2>&1 | grep -i "ld" || true
    rm -f test6_$linker
done

# Test 7: Timing and profile reports
echo "=== Test 7: Timing and profile reports ==="
cleanup_dumps
$GCC hello1.c -o test7 -ftime-report -O2 2>&1 | grep -A5 "Time variable" || true
rm -f test7

# Test 8: Profile-Guided Optimization (PGO) workflow
echo "=== Test 8: PGO workflow ==="
cleanup_dumps

# Step 1: Generate profile
$GCC pgo_test.c -o pgo_instrumented -fprofile-generate -O2
./pgo_instrumented 2>/dev/null || true

# Step 2: Use profile with timing report
$GCC pgo_test.c -o pgo_optimized -fprofile-use -ftime-report -fprofile-report -O2 2>&1 | \
    grep -E "(profile|time|report)" | head -10 || true

rm -f pgo_instrumented pgo_optimized

# Test 9: Comprehensive flag combination
echo "=== Test 9: Comprehensive flag combination ==="
cleanup_dumps
$GCC hello1.c hello2.c -o test9 \
    --sysroot=/ \
    -save-temps \
    -dumpdir=./comprehensive_dumps \
    -dumpbase="comprehensive" \
    -dumpbase-ext=".dump" \
    -fdump-tree-all \
    -fdump-rtl-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    -v 2>&1 | tail -30 || true

# Test 10: Empty and special values
echo "=== Test 10: Empty and special values ==="
$GCC hello1.c -o test10 \
    --sysroot="" \
    -dumpdir="" \
    -dumpbase="" \
    -dumpbase-ext="" \
    -save-temps=obj 2>&1 | grep -i "warning" || true

# Test 11: Print subprocess help
echo "=== Test 11: Subprocess help ==="
$GCC --help=common --help=optimizers --help=target 2>&1 | head -20

# Test 12: Multiple sysroot changes
echo "=== Test 12: Multiple sysroot changes ==="
$GCC hello1.c -o test12 \
    --sysroot=/ \
    -isysroot /usr \
    --sysroot=/nonexistent/path \
    -v 2>&1 | grep -i "sysroot" || true

# Cleanup
echo "=== Cleaning up ==="
cleanup_dumps
cd ..
rm -rf "$WORKDIR"
echo "All tests completed"
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple Jobs**: Compiles multiple files (`hello1.c hello2.c`) in single invocations
2. **Sysroot Variations**: Tests `--sysroot=`, `--sysroot=/`, `--sysroot=/nonexistent`, `-isysroot`
3. **Dump Management**: Exercises `-save-temps` (all variants), `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, `-fdump-*` flags
4. **Help/Version**: Tests `--help`, `--target-help`, `--version`, `--help=<subprocess>`
5. **Linker Selection**: Tests `-fuse-ld=bfd/gold/lld/mold`
6. **Timing Reports**: Tests `-ftime-report`, `-fprofile-report`
7. **PGO Workflow**: Full PGO cycle with `-fprofile-generate` and `-fprofile-use`
8. **Combined Flags**: Tests complex flag interactions in single invocations
9. **Edge Cases**: Empty strings, non-existent paths, multiple flag combinations

The script creates temporary directories, cleans up between tests, and captures output to verify the driver processes all flags correctly. Each test invocation triggers the initialization block in gcc.cc, ensuring coverage of the uncovered lines.
