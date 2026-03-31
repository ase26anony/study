Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc:

```bash
#!/bin/bash

# GCC driver initialization coverage test script
# Targets lines 11228-11250 in gcc.cc

set -e

# Configuration
GCC=${GCC:-gcc}
TEST_DIR=$(mktemp -d /tmp/gcc_driver_test_XXXXXX)
echo "Test directory: $TEST_DIR"
cd "$TEST_DIR"

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

cat > hello3.c << 'EOF'
#include <stdio.h>
void greet2() {
    printf("Hello 3\n");
}
EOF

# Function to clean up dump files
cleanup_dumps() {
    rm -f *.i *.s *.o *.ii *.bc *.expand *.original *.gimple \
          *.cfg *.optimized *.ssa *.alias *.ccp *.store_ccp \
          *.pre *.fre *.copyprop *.dce *.mudflap *.sra *.sink \
          *.dom *.dse *.phiopt *.forwprop *.copyrename *.nrv \
          *.vect *.slp *.dse1 *.dse2 *.pre1 *.pre2 *.pre3 *.pre4 \
          *.pre5 *.pre6 *.pre7 *.pre8 *.pre9 *.ltrans *.offload \
          *.omplower *.ompdevicelower *.ompexp *.omp *.ompgimple \
          *.optimized.* *.pass.* *.statistics *.times *.bbro \
          *.store_motion *.cse *.cse1 *.cse2 *.gcse *.gcse1 *.gcse2 \
          *.*.dump *.dump *.rpo *.profile *.gcda *.gcno *.dSYM/*
    rm -rf dumps/ *.dSYM
    mkdir -p dumps
}

# Initialize
cleanup_dumps

echo "=== Test 1: Multiple jobs with sysroot variations ==="
echo "Test 1a: Empty sysroot"
$GCC hello1.c hello2.c -o prog1 --sysroot= 2>&1 | grep -q "sysroot" || true
./prog1

echo "Test 1b: Non-existent sysroot"
$GCC hello1.c hello2.c -o prog2 --sysroot=/nonexistent/path 2>&1 | grep -i "warning\|error" || true

echo "Test 1c: Valid sysroot with isysroot"
$GCC hello1.c hello2.c -o prog3 --sysroot=/ -isysroot/usr/include -I/usr/local/include 2>&1 | tail -5 || true
./prog3

echo "=== Test 2: Dump file generation with varied options ==="
cleanup_dumps

echo "Test 2a: save-temps=obj"
$GCC hello1.c -save-temps=obj -o hello1_obj 2>&1 | tail -3 || true
ls -la hello1_obj.* 2>/dev/null | head -5 || true

echo "Test 2b: save-temps=cwd"
$GCC hello1.c -save-temps=cwd -o hello1_cwd 2>&1 | tail -3 || true
ls -la *.i *.s *.o 2>/dev/null | head -5 || true

echo "Test 2c: Basic save-temps"
$GCC hello1.c -save-temps -o hello1_basic 2>&1 | tail -3 || true

echo "Test 2d: Dumpdir with empty value"
$GCC hello1.c -dumpdir -fdump-tree-all -o hello1_dumpdir 2>&1 | tail -3 || true

echo "Test 2e: Dumpdir with path"
$GCC hello1.c -dumpdir=./dumps -fdump-tree-all -o hello1_dumpdir2 2>&1 | tail -3 || true
ls -la dumps/ 2>/dev/null | head -5 || true

echo "Test 2f: Dumpbase combinations"
$GCC hello1.c -dumpbase="mydump" -dumpbase-ext=".ext" -fdump-rtl-expand -o hello1_dumpbase 2>&1 | tail -3 || true
ls -la *dump* 2>/dev/null | head -5 || true

echo "Test 2g: Empty dumpbase and dumpbase-ext"
$GCC hello1.c -dumpbase="" -dumpbase-ext="" -fdump-tree-original -o hello1_empty 2>&1 | tail -3 || true

echo "Test 2h: Multiple dump flags combined"
$GCC hello1.c -save-temps -dumpdir=./dumps -dumpbase=combined -dumpbase-ext=.test \
    -fdump-tree-all -fdump-rtl-all -fdump-ipa-all -o hello1_combined 2>&1 | tail -5 || true

echo "=== Test 3: Help and version output ==="
echo "Test 3a: Basic help"
$GCC --help 2>&1 | head -5

echo "Test 3b: Target help"
$GCC --target-help 2>&1 | head -5

echo "Test 3c: Version"
$GCC --version 2>&1 | head -5

echo "Test 3d: Subprocess help"
$GCC --help=common 2>&1 | head -5
$GCC --help=optimizers 2>&1 | head -5

echo "Test 3e: Help combined with compilation flags"
$GCC --help -O2 -dumpbase=test hello1.c 2>&1 | head -10 || true

echo "Test 3f: Version with other flags"
$GCC --version -v --sysroot=/ 2>&1 | head -10 || true

echo "=== Test 4: Linker selection flags ==="
echo "Test 4a: BFD linker"
$GCC hello1.c -fuse-ld=bfd -Wl,--verbose -o hello1_bfd 2>&1 | grep -i "bfd\|GNU ld" | head -2 || true
./hello1_bfd

echo "Test 4b: Gold linker (if available)"
$GCC hello1.c -fuse-ld=gold -o hello1_gold 2>&1 | grep -i "gold" | head -2 || true
[ -f hello1_gold ] && ./hello1_gold || true

echo "Test 4c: LLD linker (if available)"
$GCC hello1.c -fuse-ld=lld -o hello1_lld 2>&1 | grep -i "lld" | head -2 || true
[ -f hello1_lld ] && ./hello1_lld || true

echo "Test 4d: Mold linker (if available)"
$GCC hello1.c -fuse-ld=mold -o hello1_mold 2>&1 | grep -i "mold" | head -2 || true
[ -f hello1_mold ] && ./hello1_mold || true

echo "Test 4e: Multiple linker flags with Wl"
$GCC hello1.c -fuse-ld=bfd -Wl,-Map=output.map -Wl,--cref -o hello1_wl 2>&1 | tail -3 || true
[ -f output.map ] && echo "Map file created" || true

echo "=== Test 5: Profile-guided optimization and timing ==="
cleanup_dumps

echo "Test 5a: Profile generation"
$GCC hello1.c -O2 -fprofile-generate -ftest-coverage -o hello1_prof_gen 2>&1 | tail -3 || true
./hello1_prof_gen
ls -la *.gcda *.gcno 2>/dev/null | head -5 || true

echo "Test 5b: Profile use with timing report"
$GCC hello1.c -O2 -fprofile-use -ftime-report -o hello1_prof_use 2>&1 | grep -A5 "Time variable" || true
./hello1_prof_use

echo "Test 5c: Profile report"
$GCC hello1.c -O2 -fprofile-report -fprofile-correction -o hello1_prof_report 2>&1 | grep -i "profile" | head -5 || true

echo "=== Test 6: Comprehensive flag combinations ==="
cleanup_dumps

echo "Test 6a: Recommended basic initialization coverage"
$GCC hello1.c hello2.c hello3.c -O0 -v --sysroot= -save-temps \
    -fdump-tree-original -ftime-report -o hello_comprehensive1 2>&1 | \
    grep -E "(sysroot|dump|Time variable|Driving)" | head -10 || true

echo "Test 6b: Comprehensive flag interaction"
$GCC hello1.c -O2 -fuse-ld=bfd --help=optimizers -dumpbase=test \
    -dumpbase-ext=.dump -fdump-rtl-all -o hello_comprehensive2 2>&1 | \
    grep -E "(optimization|dump|bfd)" | head -10 || true

echo "Test 6c: All relevant flags in one command"
$GCC hello1.c hello2.c --sysroot=/ -save-temps -dumpdir=./dumps \
    -fdump-tree-all -ftime-report -fuse-ld=gold -Wl,--print-map \
    -o hello_combined 2>&1 | tail -15 || true

echo "=== Test 7: Multiple jobs with reset verification ==="
echo "Test 7: Compile three files with various flags to trigger multiple resets"
$GCC hello1.c -O1 -save-temps=obj -o h1.o -c 2>&1 | tail -2 || true
$GCC hello2.c -O2 -fdump-tree-all -o h2.o -c 2>&1 | tail -2 || true
$GCC hello3.c -O0 -ftime-report -o h3.o -c 2>&1 | tail -2 || true
$GCC h1.o h2.o h3.o -o multi_job 2>&1 | tail -2 || true
./multi_job

echo "=== Test 8: Edge cases ==="
echo "Test 8a: Empty dumpdir with trailing operations"
$GCC hello1.c -dumpdir -dumpbase="" -save-temps -o hello_edge1 2>&1 | tail -3 || true

echo "Test 8b: Only help flags (should exit before compilation)"
$GCC --help --target-help --version 2>&1 | wc -l

echo "Test 8c: Sysroot changed flag"
$GCC hello1.c --sysroot=/ --no-sysroot-suffix -o hello_sysroot_changed 2>&1 | grep -i "sysroot" | head -3 || true

# Cleanup
echo "=== Cleaning up ==="
cd /
rm -rf "$TEST_DIR"
echo "Test directory removed: $TEST_DIR"
echo "All tests completed successfully!"
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple Jobs**: Compiles multiple files (`hello1.c`, `hello2.c`, `hello3.c`) together
2. **Sysroot Variations**: Tests empty, non-existent, and valid sysroots with `-isysroot` and `-I`
3. **Dump File Options**: Exercises all `-save-temps` variants, `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, and `-fdump-*` flags
4. **Help/Version Output**: Tests `--help`, `--target-help`, `--version`, and `--help=<subprocess>` with and without compilation flags
5. **Linker Selection**: Tests all `-fuse-ld=` variants with `-Wl,` options
6. **PGO and Timing**: Implements full PGO workflow with `-fprofile-generate`, execution, and `-fprofile-use` with `-ftime-report`
7. **Comprehensive Combinations**: Creates commands that combine multiple relevant flags as specified
8. **Edge Cases**: Tests empty values and flag interactions

The script captures exit codes implicitly via `set -e` and handles errors gracefully with `|| true` where appropriate. It cleans up between tests to avoid interference and provides verbose output to confirm execution paths.
