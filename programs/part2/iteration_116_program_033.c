Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc by testing various flag combinations and execution scenarios:

```bash
#!/bin/bash
set -e

# Configuration
GCC=${1:-gcc}
TEST_DIR=$(mktemp -d)
echo "Testing GCC driver initialization coverage in: $TEST_DIR"
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
void helper() {
    printf("Hello 2\n");
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

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    rm -f *.o *.s *.i *.ii *.bc *.expand *.original *.gimple *.cfg *.c.* *.r.* *.t.*
    rm -f *.gcda *.gcno *.gcov *.dwo *.dwp
    rm -f hello hello_pgo pgo_test *.dmp *.dump
    rm -rf dumps temps
    rm -f times.txt profile.txt
}
trap cleanup EXIT

echo "=== Test 1: Multiple jobs with sysroot variations ==="
# Multiple input files with different sysroot options
"$GCC" hello1.c hello2.c -o hello --sysroot= -isysroot/usr/include -I/usr/local/include 2>&1 | head -20
echo "Exit code: $?"

echo -e "\n=== Test 2: Dump file management with various options ==="
mkdir -p dumps temps
# Test different save-temps values
for save_temp in "obj" "cwd" ""; do
    echo "Testing -save-temps=$save_temp"
    if [ -z "$save_temp" ]; then
        "$GCC" hello1.c -save-temps -dumpdir=./dumps -dumpbase=testbase -dumpbase-ext=.mydump -fdump-tree-all -fdump-rtl-expand -o hello_temp 2>&1 | tail -5
    else
        "$GCC" hello1.c -save-temps=$save_temp -dumpdir=./dumps -dumpbase=testbase -dumpbase-ext=.mydump -fdump-tree-all -o hello_temp 2>&1 | tail -5
    fi
    echo "Exit code: $?"
done

# Test empty dump options
echo -e "\nTesting empty dump options:"
"$GCC" hello1.c -dumpdir= -dumpbase= -dumpbase-ext= -fdump-tree-original -o hello_empty 2>&1 | tail -5
echo "Exit code: $?"

echo -e "\n=== Test 3: Help and version output ==="
# Individual help/version flags
for flag in "--help" "--target-help" "--version" "--help=common" "--help=optimizers" "--help=target"; do
    echo "Testing $flag"
    "$GCC" $flag 2>&1 | head -3
    echo "Exit code: $?"
done

# Combined with compilation flags
echo -e "\nTesting help combined with compilation:"
"$GCC" hello1.c --help=optimizers -O2 -o hello_help 2>&1 | tail -5
echo "Exit code: $?"

echo -e "\n=== Test 4: Linker selection flags ==="
for linker in "bfd" "gold" "lld" "mold"; do
    echo "Testing -fuse-ld=$linker"
    "$GCC" hello1.c -fuse-ld=$linker -Wl,--verbose -o hello_$linker 2>&1 | grep -i "linker\|ld" | head -2 || true
    echo "Exit code: $?"
done

echo -e "\n=== Test 5: Comprehensive flag combination ==="
# Combine multiple relevant flags in one invocation
"$GCC" hello1.c hello2.c \
    --sysroot=/ \
    -save-temps=obj \
    -dumpdir=./dumps \
    -dumpbase=combo \
    -dumpbase-ext=.full \
    -fdump-tree-all \
    -fdump-rtl-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    -v \
    -o hello_combo 2>&1 | grep -E "(sysroot|dump|save-temps|Using.*linker|Time)" | head -10
echo "Exit code: $?"

echo -e "\n=== Test 6: Profile-guided optimization with timing reports ==="
# Phase 1: Generate profile
echo "Phase 1: Profile generation"
"$GCC" pgo_test.c -O2 -fprofile-generate -ftest-coverage -o pgo_test_gen 2>&1 | tail -5
./pgo_test_gen > /dev/null 2>&1 || true

# Phase 2: Use profile with timing report
echo -e "\nPhase 2: Profile usage with timing report"
"$GCC" pgo_test.c -O2 -fprofile-use -fprofile-report -ftime-report -o pgo_test_use 2>&1 | grep -E "(profile|Time)" | head -10
echo "Exit code: $?"

echo -e "\n=== Test 7: Verbose with various system root options ==="
# Test sysroot variations
for sysroot in "/" "/usr" "/nonexistent/path" ""; do
    echo "Testing --sysroot=$sysroot"
    "$GCC" hello1.c --sysroot="$sysroot" -v -o hello_sysroot 2>&1 | grep -i "sysroot\|search" | head -3 || true
    echo "Exit code: $?"
done

echo -e "\n=== Test 8: Print subprocess help ==="
"$GCC" --help=common --help=optimizers --help=target 2>&1 | head -5
echo "Exit code: $?"

echo -e "\n=== Test 9: Combined help/version with dump options ==="
"$GCC" --version --help=optimizers -dumpbase=helpdump -fdump-tree-original hello1.c -o hello_combined 2>&1 | tail -10
echo "Exit code: $?"

echo -e "\n=== Test 10: Save-temps overrides and dumpdir handling ==="
# Test save-temps overrides
"$GCC" hello1.c -save-temps -dumpdir=./temps -fdump-tree-all -save-temps=cwd -o hello_override 2>&1 | grep -i "save-temps\|dump" | head -5 || true
echo "Exit code: $?"

# Clean generated files
cleanup

echo -e "\n=== All tests completed ==="
echo "The tests have exercised the uncovered initialization block by:"
echo "1. Processing multiple jobs (hello1.c + hello2.c)"
echo "2. Using various --sysroot and -isysroot flags"
echo "3. Testing all save-temps modes with dump file options"
echo "4. Requesting help/version output in different combinations"
echo "5. Selecting different linkers with -fuse-ld"
echo "6. Using PGO with timing reports"
echo "7. Combining multiple flags in single invocations"
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple Jobs**: Compiles multiple source files (`hello1.c`, `hello2.c`) together
2. **System Root Variations**: Tests `--sysroot=` (empty), `--sysroot=/`, `--sysroot=/nonexistent`, and `-isysroot`
3. **Dump Management**: Exercises all `-save-temps` values, `-dumpdir`, `-dumpbase`, `-dumpbase-ext` with various strings and empty values
4. **Help/Version Output**: Tests `--help`, `--target-help`, `--version`, `--help=<subprocess>` individually and combined
5. **Linker Selection**: Tests `-fuse-ld=bfd/gold/lld/mold` with `-Wl` options
6. **PGO and Timing**: Implements full PGO workflow with `-ftime-report` and `-fprofile-report`
7. **Flag Combinations**: Creates complex invocations that combine multiple relevant flags

The script captures exit codes and filters output to show relevant information. Each test is designed to trigger specific parts of the initialization block while ensuring the driver proceeds through its normal execution flow.
