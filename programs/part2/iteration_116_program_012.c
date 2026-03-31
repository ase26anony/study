Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc:

```bash
#!/bin/bash
set -e

# Configuration
GCC=${GCC:-gcc}
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

cat > hello3.c << 'EOF'
#include <stdio.h>
void helper2() {
    printf("Hello World 3\n");
}
EOF

# Helper function to clean up dump files
cleanup_dumps() {
    rm -f *.i *.s *.o *.dump *.gcda *.gcno *.gcov 2>/dev/null || true
    rm -rf dumps 2>/dev/null || true
}

# Test 1: Multiple jobs with sysroot variations
echo "=== Test 1: Multiple jobs with sysroot ==="
cleanup_dumps
$GCC hello1.c hello2.c hello3.c -o prog1 \
    --sysroot= \
    -isysroot/usr/include \
    -I/usr/local/include \
    -v 2>&1 | grep -q "sysroot" || true
./prog1 2>/dev/null || true

# Test 2: Dump file management with various options
echo "=== Test 2: Dump file management ==="
cleanup_dumps
mkdir -p dumps
$GCC hello1.c -o prog2 \
    -save-temps=obj \
    -dumpdir=./dumps \
    -dumpbase=testdump \
    -dumpbase-ext=.myext \
    -fdump-tree-all \
    -fdump-rtl-expand \
    -fdump-ipa-all 2>&1 | tail -20 || true

# Test 3: Different save-temps modes
echo "=== Test 3: Various save-temps modes ==="
cleanup_dumps
$GCC hello1.c -o prog3a -save-temps=cwd 2>&1 | tail -5 || true
cleanup_dumps
$GCC hello1.c -o prog3b -save-temps 2>&1 | tail -5 || true
cleanup_dumps
$GCC hello1.c -o prog3c -save-temps=obj 2>&1 | tail -5 || true

# Test 4: Help and version flags (separate invocations)
echo "=== Test 4: Help and version flags ==="
$GCC --help >/dev/null 2>&1 || true
$GCC --target-help >/dev/null 2>&1 || true
$GCC --version >/dev/null 2>&1 || true
$GCC --help=common >/dev/null 2>&1 || true
$GCC --help=optimizers >/dev/null 2>&1 || true
$GCC --help=warnings >/dev/null 2>&1 || true

# Test 5: Combined help with compilation flags
echo "=== Test 5: Combined help/compilation ==="
$GCC --help -O2 hello1.c -o /dev/null 2>&1 | head -20 || true
$GCC --version -v --sysroot=/ hello1.c 2>&1 | head -10 || true

# Test 6: Linker selection flags
echo "=== Test 6: Linker selection ==="
for linker in bfd gold lld mold; do
    echo "Testing -fuse-ld=$linker"
    $GCC hello1.c -fuse-ld=$linker -o prog6_$linker 2>&1 | tail -3 || true
done
$GCC hello1.c -Wl,--verbose -o prog6_wl 2>&1 | grep -A2 "GNU ld" || true

# Test 7: Timing and profile reports
echo "=== Test 7: Timing and profile reports ==="
cleanup_dumps
$GCC hello1.c -O2 -ftime-report -o prog7a 2>&1 | grep -A5 "Time variable" || true
$GCC hello1.c -O2 -fprofile-report -o prog7b 2>&1 | tail -10 || true

# Test 8: Comprehensive flag combination (covers most variables)
echo "=== Test 8: Comprehensive flag combination ==="
cleanup_dumps
$GCC hello1.c hello2.c -o prog8 \
    --sysroot=/ \
    -save-temps \
    -dumpdir=./dumps \
    -dumpbase=comprehensive \
    -dumpbase-ext=.test \
    -fdump-tree-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    -v 2>&1 | tail -30 || true

# Test 9: Empty/non-existent sysroot
echo "=== Test 9: Edge case sysroots ==="
$GCC hello1.c --sysroot= -o prog9a 2>&1 | tail -5 || true
$GCC hello1.c --sysroot=/nonexistent/path -o prog9b 2>&1 | tail -5 || true

# Test 10: Profile-guided optimization path
echo "=== Test 10: PGO workflow ==="
cleanup_dumps

# Step 1: Generate profile
cat > pgo_test.c << 'EOF'
#include <stdio.h>
int main() {
    for (int i = 0; i < 1000; i++) {
        printf("PGO test %d\n", i % 10);
    }
    return 0;
}
EOF

$GCC pgo_test.c -O2 -fprofile-generate -o pgo_instrumented 2>&1 | tail -5 || true
./pgo_instrumented > /dev/null 2>&1 || true

# Step 2: Use profile with timing report
$GCC pgo_test.c -O2 -fprofile-use -ftime-report -fprofile-correction \
    -o pgo_optimized 2>&1 | grep -A3 "profile" || true

# Test 11: Reset variables with multiple sequential jobs
echo "=== Test 11: Sequential jobs resetting variables ==="
cleanup_dumps
$GCC --help=optimizers >/dev/null 2>&1
$GCC hello1.c --sysroot= -save-temps -o prog11a 2>&1 | tail -5 || true
$GCC hello2.c -fuse-ld=bfd -fdump-tree-all -o prog11b 2>&1 | tail -5 || true
$GCC --version >/dev/null 2>&1
$GCC hello3.c -ftime-report -o prog11c 2>&1 | tail -5 || true

# Test 12: Dumpdir edge cases
echo "=== Test 12: Dumpdir edge cases ==="
cleanup_dumps
$GCC hello1.c -dumpdir= -dumpbase= -o prog12a 2>&1 | tail -5 || true
$GCC hello1.c -dumpdir=. -dumpbase=test -dumpbase-ext= -o prog12b 2>&1 | tail -5 || true
$GCC hello1.c -dumpdir=./dumps/ -dumpbase=test.dump -o prog12c 2>&1 | tail -5 || true

# Cleanup
echo "=== Cleaning up ==="
cleanup_dumps
rm -f prog* pgo_* hello*.c 2>/dev/null || true
cd ..
rm -rf "$WORKDIR"
echo "Tests completed. Working directory cleaned."
```

This script systematically tests the uncovered initialization block by:

1. **Multiple Jobs**: Compiles multiple source files (`hello1.c hello2.c hello3.c`) in single invocations
2. **Sysroot Variations**: Tests `--sysroot=`, `--sysroot=/`, `--sysroot=/nonexistent`, `-isysroot`, and `-I` flags
3. **Dump Management**: Exercises `-save-temps` (all variants), `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, and various `-fdump-*` flags
4. **Help/Version**: Tests `--help`, `--target-help`, `--version`, `--help=<subprocess>` separately and combined
5. **Linker Selection**: Tests `-fuse-ld=bfd/gold/lld/mold` and `-Wl,` options
6. **Timing Reports**: Uses `-ftime-report` and `-fprofile-report`
7. **PGO Workflow**: Demonstrates full PGO cycle with `-fprofile-generate` and `-fprofile-use`
8. **Edge Cases**: Tests empty strings, non-existent paths, and sequential job resets

The script captures output but continues on errors (using `|| true`) to ensure all code paths are attempted. The `-v` flag in some tests helps verify internal state changes.
