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
    cd /
    rm -rf "$WORKDIR"
    echo "Cleaned up $WORKDIR"
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

echo "=== Test 1: Multiple input files with sysroot variations ==="
# This triggers initialization for multiple jobs
$GCC hello1.c hello2.c -o multi \
    --sysroot= \
    --sysroot=/nonexistent \
    -isysroot /usr \
    -I/usr/include \
    -I. \
    2>&1 | grep -E "(sysroot|error|warning)" || true

echo -e "\n=== Test 2: Dump file management with various options ==="
# Test all save-temps values
for save_temp in "obj" "cwd" ""; do
    echo "Testing -save-temps=$save_temp"
    if [ -z "$save_temp" ]; then
        $GCC hello1.c -save-temps -dumpdir=./dumps -dumpbase=test1 \
            -dumpbase-ext=.dump -fdump-tree-all -o hello1 2>&1 | tail -5
    else
        $GCC hello1.c -save-temps=$save_temp -dumpdir=./dumps -dumpbase=test2 \
            -dumpbase-ext=.dump -fdump-rtl-expand -o hello2 2>&1 | tail -5
    fi
    rm -f *.i *.s *.o hello1 hello2 2>/dev/null || true
done

echo -e "\n=== Test 3: Help and version flags ==="
# Individual help/version flags
$GCC --help > /dev/null && echo "--help succeeded"
$GCC --target-help 2>&1 | head -5
$GCC --version | head -2

# Combined with other flags
$GCC --help=common --sysroot=/ -save-temps 2>&1 | head -5
$GCC --help=optimizers -O2 -dumpbase=help_test 2>&1 | head -5
$GCC --version -v --sysroot=/nonexistent 2>&1 | grep -E "(version|sysroot)" | head -5

echo -e "\n=== Test 4: Linker selection flags ==="
for linker in bfd gold lld mold; do
    echo "Testing -fuse-ld=$linker"
    $GCC hello1.c -fuse-ld=$linker -Wl,--verbose 2>&1 | grep -i "ld" | head -2 || true
done

echo -e "\n=== Test 5: Timing and profile reports ==="
$GCC hello1.c -ftime-report -O2 -o hello_time 2>&1 | grep -E "(Time|report)" || true
./hello_time 2>/dev/null || true

echo -e "\n=== Test 6: Combined comprehensive test ==="
$GCC hello1.c hello2.c \
    --sysroot=/ \
    -save-temps=obj \
    -dumpdir=./combined_dumps \
    -dumpbase=combined \
    -dumpbase-ext=.full \
    -fdump-tree-all \
    -fdump-rtl-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    -v \
    -o combined_prog 2>&1 | tail -20

echo -e "\n=== Test 7: Profile-Guided Optimization flow ==="
# Phase 1: Generate profile
echo "Phase 1: Instrumented compilation"
$GCC pgo_test.c -O2 -fprofile-generate -ftest-coverage -o pgo_instr
./pgo_instr > /dev/null

# Phase 2: Use profile with reports
echo "Phase 2: Profile-guided optimization"
$GCC pgo_test.c -O2 -fprofile-use -fprofile-report -ftime-report -o pgo_opt 2>&1 | \
    grep -E "(profile|Time|report)" || true

echo -e "\n=== Test 8: Edge cases ==="
# Empty dump options
$GCC hello1.c -dumpdir= -dumpbase= -dumpbase-ext= -o empty_dump 2>&1 | tail -3 || true

# Multiple sysroot changes
$GCC hello1.c --sysroot=/ --sysroot=/usr --sysroot= -v 2>&1 | grep -i "sysroot" | head -5

# Help with dump options
$GCC --help=warnings -dumpbase=helpdump -fdump-tree-original 2>&1 | head -3

echo -e "\n=== Test 9: Verbose flag to observe initialization ==="
$GCC hello1.c -v --sysroot= -save-temps -fdump-tree-original -ftime-report -o verbose_test 2>&1 | \
    grep -E "(sysroot|dump|Time)" | head -10

# Clean generated files
rm -rf *.o *.i *.s *.gcno *.gcda *.dump dumps combined_dumps 2>/dev/null || true
rm -f hello1 hello2 multi combined_prog pgo_instr pgo_opt verbose_test 2>/dev/null || true

echo -e "\n=== All tests completed ==="
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple Jobs**: Compiles multiple files (`hello1.c hello2.c`) in single invocations
2. **Sysroot Variations**: Tests `--sysroot=`, `--sysroot=/nonexistent`, `-isysroot`, and multiple sysroot changes
3. **Dump Management**: Exercises all `-save-temps` values, `-dumpdir`, `-dumpbase`, `-dumpbase-ext` with various combinations
4. **Help/Version**: Tests `--help`, `--target-help`, `--version`, `--help=common`, `--help=optimizers` both alone and combined with other flags
5. **Linker Selection**: Tests `-fuse-ld=` with bfd, gold, lld, mold variants
6. **Timing Reports**: Uses `-ftime-report` and `-fprofile-report`
7. **PGO Flow**: Implements full PGO workflow with `-fprofile-generate` and `-fprofile-use`
8. **Combined Test**: Single invocation with multiple relevant flags to test interactions
9. **Verbose Output**: Uses `-v` to observe driver state changes

The script captures exit codes implicitly via `set -e` and cleans up temporary files between tests to avoid interference. Each test is designed to trigger specific parts of the initialization block while ensuring the driver proceeds through its normal execution path.
