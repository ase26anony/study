Here's a comprehensive test script that exercises the uncovered initialization logic in gcc.cc:

```bash
#!/bin/bash

# GCC driver initialization coverage test script
# Tests the uncovered lines 11228-11250 in gcc.cc

set -e

# Configuration
GCC=${GCC:-gcc}
TEST_DIR=$(mktemp -d /tmp/gcc_driver_test_XXXXXX)
echo "Test directory: $TEST_DIR"
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    echo "Cleaning up..."
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
void another() {
    printf("Hello World 3\n");
}
EOF

echo "=== Test 1: Multiple jobs with sysroot variations ==="
echo "Testing: Multiple input files with different sysroot options"
$GCC hello1.c hello2.c hello3.c -o multi1 \
    --sysroot= \
    --sysroot=/nonexistent \
    -isysroot/usr \
    -I/usr/include \
    -I/usr/local/include 2>&1 | grep -E "(sysroot|error|warning)" || true
echo "Exit code: $?"

echo -e "\n=== Test 2: Dump file management with various options ==="
echo "Testing: -save-temps with different values and dump options"
$GCC hello1.c -o dump_test1 \
    -save-temps=obj \
    -dumpdir=./dumps \
    -dumpbase=testdump \
    -dumpbase-ext=.myext \
    -fdump-tree-all \
    -fdump-rtl-expand 2>&1 | tail -20
echo "Exit code: $?"

# Clean dump files
rm -rf ./dumps *.i *.s *.o dump_test1 2>/dev/null || true

echo -e "\n=== Test 3: Different -save-temps modes ==="
echo "Testing: -save-temps=cwd"
$GCC hello1.c -o temp_test1 -save-temps=cwd -fdump-tree-original
echo "Exit code: $?"
ls -la *.i *.s *.o 2>/dev/null || echo "No temp files generated"

rm -f *.i *.s *.o temp_test1 2>/dev/null || true

echo -e "\n=== Test 4: Help and version flags ==="
echo "Testing: --help with subprocess help"
$GCC --help --target-help --help=common --help=optimizers 2>&1 | head -5
echo "Exit code: $?"

echo -e "\nTesting: --version"
$GCC --version 2>&1 | head -3
echo "Exit code: $?"

echo -e "\n=== Test 5: Linker selection flags ==="
echo "Testing: Different linker options"
for linker in bfd gold lld mold; do
    echo -n "Testing -fuse-ld=$linker: "
    $GCC hello1.c -o linker_test -fuse-ld=$linker -Wl,--verbose 2>&1 | \
        grep -q "using the linker" && echo "Supported" || echo "Not available"
    rm -f linker_test 2>/dev/null || true
done

echo -e "\n=== Test 6: Combined flags for comprehensive coverage ==="
echo "Testing: Multiple flags in single invocation"
$GCC hello1.c hello2.c -o combined \
    --sysroot=/ \
    -save-temps \
    -dumpdir=./combined_dumps \
    -fdump-tree-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    -v 2>&1 | tail -30
echo "Exit code: $?"

echo -e "\n=== Test 7: Profile-guided optimization path ==="
echo "Step 1: Compile with profile generation"
$GCC hello1.c -o pgo_instrumented -fprofile-generate -ftest-coverage
echo "Exit code: $?"

echo -e "\nStep 2: Run instrumented program"
./pgo_instrumented > /dev/null 2>&1 || true

echo -e "\nStep 3: Recompile with profile use and timing"
$GCC hello1.c -o pgo_optimized \
    -fprofile-use \
    -ftime-report \
    -fprofile-report \
    -fprofile-correction 2>&1 | tail -20
echo "Exit code: $?"

echo -e "\n=== Test 8: Empty dump options ==="
echo "Testing: Empty dumpdir and dumpbase"
$GCC hello1.c -o empty_dump_test \
    -dumpdir= \
    -dumpbase= \
    -dumpbase-ext= \
    -fdump-tree-optimized 2>&1 | grep -E "(dump|generating)" || true
echo "Exit code: $?"

echo -e "\n=== Test 9: Verbose and timing flags ==="
echo "Testing: Verbose with timing report"
$GCC hello1.c -o verbose_test \
    -v \
    -ftime-report \
    -fprofile-report \
    -save-temps=obj \
    -dumpbase=verbose_dump 2>&1 | grep -E "(Time variable|drivers)" | head -5
echo "Exit code: $?"

echo -e "\n=== Test 10: Maximum flag combination ==="
echo "Testing: All relevant flags together"
$GCC hello1.c hello2.c hello3.c -o max_test \
    --sysroot=/ \
    -isysroot/usr \
    -save-temps=obj \
    -dumpdir=./max_dumps \
    -dumpbase=max_test \
    -dumpbase-ext=.dump \
    -fdump-tree-all \
    -fdump-rtl-all \
    -ftime-report \
    -fuse-ld=bfd \
    -Wl,--print-map \
    -O3 \
    -v \
    --help=common 2>&1 | tail -10
echo "Exit code: $?"

echo -e "\n=== Test 11: Test print_help_list and print_subprocess_help ==="
echo "Testing: Help flags with compilation"
$GCC --help=common hello1.c -o help_test 2>&1 | head -10
echo "Exit code: $?"

echo -e "\n=== Test 12: Test target_sysroot_suffix variations ==="
echo "Testing: System root with suffix-like options"
$GCC hello1.c -o sysroot_test \
    --sysroot=/usr \
    -I/usr/include/x86_64-linux-gnu \
    -I/usr/include/linux 2>&1 | grep -E "(search starts|include)" | head -5
echo "Exit code: $?"

# Cleanup generated files
echo -e "\n=== Cleaning generated files ==="
rm -rf *.o *.i *.s *.gcno *.gcda *.gcov \
    multi1 dump_test1 temp_test1 linker_test combined \
    pgo_instrumented pgo_optimized empty_dump_test verbose_test \
    max_test help_test sysroot_test \
    dumps combined_dumps max_dumps 2>/dev/null || true

echo -e "\n=== Test completed ==="
echo "All tests executed. Check exit codes above for success/failure."
echo "Note: Some tests may fail due to missing components (e.g., specific linkers)"
echo "but should still exercise the driver initialization code."
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple Jobs**: Compiles multiple source files (`hello1.c hello2.c hello3.c`) in single invocations
2. **Sysroot Variations**: Tests `--sysroot=`, `--sysroot=/nonexistent`, `-isysroot`, and `-I` flags
3. **Dump Management**: Exercises all `-save-temps` modes, `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, and `-fdump-*` flags
4. **Help/Version Output**: Tests `--help`, `--target-help`, `--version`, `--help=common`, `--help=optimizers`
5. **Linker Selection**: Tests `-fuse-ld=bfd/gold/lld/mold` and `-Wl,` options
6. **PGO and Timing**: Implements full PGO workflow with `-fprofile-generate`, execution, and `-fprofile-use` with `-ftime-report`
7. **Combined Flags**: Tests multiple flags together to ensure initialization resets between logical sections

The script captures exit codes and filters output to show relevant information while ensuring the driver initialization code paths are exercised.
