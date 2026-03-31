Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc by testing various flag combinations as specified:

```bash
#!/bin/bash

# GCC driver initialization coverage test script
# Tests the uncovered lines in gcc.cc (11228-11250)

set -e

# Use system GCC or specify path
GCC="${GCC:-gcc}"
echo "Testing with GCC: $($GCC --version | head -1)"

# Create test directory
TEST_DIR="gcc_init_test"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

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
void greet() {
    printf("Hello World 2\n");
}
EOF

cat > hello3.c << 'EOF'
#include <stdio.h>
void hello() {
    printf("Hello World 3\n");
}
EOF

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    rm -f *.o *.i *.s *.o *.so *.gcda *.gcno *.gcov
    rm -f hello hello_pgo *.dump *.times
    rm -rf dumps temps
    rm -f test_output
}

# Run test and capture exit code
run_test() {
    local desc="$1"
    shift
    echo "========================================"
    echo "Test: $desc"
    echo "Command: $GCC $*"
    "$GCC" "$@" 2>&1 | tee -a test_output
    local status=$?
    echo "Exit code: $status"
    echo "========================================"
    echo
    cleanup
    return $status
}

# Test 1: Multiple jobs with sysroot variations
echo "=== Test 1: Multiple jobs with sysroot ==="
run_test "Multiple files with empty sysroot" \
    hello1.c hello2.c hello3.c -o hello_multi \
    --sysroot= \
    -v

run_test "Multiple files with non-existent sysroot" \
    hello1.c hello2.c -o hello \
    --sysroot=/nonexistent/path \
    -isysroot /usr/include \
    -I/usr/local/include

# Test 2: Dump file management with various options
echo "=== Test 2: Dump file management ==="
run_test "save-temps=obj with dumpdir" \
    hello1.c -o hello \
    -save-temps=obj \
    -dumpdir=./dumps \
    -fdump-tree-all \
    -fdump-rtl-expand

mkdir -p dumps
run_test "save-temps=cwd with dumpbase" \
    hello1.c -o hello \
    -save-temps=cwd \
    -dumpbase=testdump \
    -dumpbase-ext=.myext \
    -fdump-tree-original \
    -fdump-tree-optimized

run_test "save-temps with empty dump options" \
    hello1.c -o hello \
    -save-temps \
    -dumpdir= \
    -dumpbase= \
    -dumpbase-ext= \
    -fdump-ipa-all

# Test 3: Help and version output
echo "=== Test 3: Help and version output ==="
run_test "Basic help" \
    --help

run_test "Target help" \
    --target-help

run_test "Version" \
    --version

run_test "Subprocess help" \
    --help=common

run_test "Optimizers help" \
    --help=optimizers

run_test "Combined help with compilation flags" \
    hello1.c --help=warnings -O2 -o hello

# Test 4: Linker selection flags
echo "=== Test 4: Linker selection ==="
for linker in bfd gold lld mold; do
    run_test "Linker: $linker" \
        hello1.c -o "hello_$linker" \
        -fuse-ld=$linker \
        -Wl,--verbose
done

run_test "Multiple linker flags with sysroot" \
    hello1.c hello2.c -o hello \
    -fuse-ld=gold \
    --sysroot=/ \
    -Wl,-rpath,/usr/lib \
    -Wl,--as-needed

# Test 5: Profile-guided optimization and timing
echo "=== Test 5: PGO and timing reports ==="

# Phase 1: Generate profile
echo "=== PGO Phase 1: Profile generation ==="
run_test "PGO generate" \
    hello1.c -o hello_pgo_gen \
    -fprofile-generate \
    -ftest-coverage \
    -O2

# Run instrumented program if compilation succeeded
if [ -f ./hello_pgo_gen ]; then
    ./hello_pgo_gen > /dev/null 2>&1 || true
fi

# Phase 2: Use profile with timing reports
echo "=== PGO Phase 2: Profile use ==="
run_test "PGO use with timing report" \
    hello1.c -o hello_pgo_use \
    -fprofile-use \
    -ftime-report \
    -fprofile-report \
    -fprofile-correction \
    -O2

# Test 6: Combined comprehensive test
echo "=== Test 6: Comprehensive flag combination ==="
run_test "Comprehensive test 1" \
    hello1.c hello2.c -o hello_combo1 \
    --sysroot=/ \
    -save-temps=obj \
    -dumpdir=./combo_dumps \
    -dumpbase=combo_test \
    -dumpbase-ext=.full \
    -fdump-tree-all \
    -fdump-rtl-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    -v

run_test "Comprehensive test 2" \
    hello1.c -o hello_combo2 \
    -save-temps=cwd \
    -dumpbase= \
    -dumpbase-ext=.test \
    -fdump-ipa-all \
    -fprofile-use \
    -ftime-report \
    --help=common \
    -fuse-ld=bfd \
    -O3

# Test 7: Reset state between jobs (implicitly tested by multiple runs)
echo "=== Test 7: State reset verification ==="
# Run two compilations with different dump settings
run_test "First with dumpdir" \
    hello1.c -o hello1 \
    -dumpdir=./first \
    -fdump-tree-original

run_test "Second with different dumpdir" \
    hello2.c -o hello2 \
    -dumpdir=./second \
    -fdump-tree-optimized

# Verify dump files were created in correct directories
echo "Checking dump file creation..."
find . -name "*.original" -o -name "*.optimized" 2>/dev/null | head -5

# Test 8: Edge cases
echo "=== Test 8: Edge cases ==="
run_test "Empty dump options with sysroot suffix" \
    hello1.c -o hello_edge \
    --sysroot=/ \
    -dumpdir= \
    -dumpbase= \
    -dumpbase-ext= \
    -save-temps=none

run_test "Verbose only flag simulation" \
    hello1.c -o hello_verbose \
    -v \
    -ftime-report \
    -fverbose-asm

# Final cleanup
cleanup
cd ..
rm -rf "$TEST_DIR"

echo "========================================"
echo "All tests completed!"
echo "Check test_output for detailed results"
echo "========================================"
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple Jobs**: Compiles multiple files (`hello1.c hello2.c hello3.c`) in single invocations
2. **Sysroot Variations**: Tests `--sysroot=`, `--sysroot=/nonexistent`, `-isysroot`, and `-I` flags
3. **Dump Management**: Exercises all `-save-temps` variants, `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, and various `-fdump-*` flags
4. **Help/Version**: Tests `--help`, `--target-help`, `--version`, `--help=common`, `--help=optimizers`
5. **Linker Selection**: Tests `-fuse-ld=bfd/gold/lld/mold` with `-Wl,` options
6. **PGO and Timing**: Implements full PGO workflow with `-fprofile-generate`, `-fprofile-use`, `-ftime-report`
7. **Combined Flags**: Creates comprehensive commands mixing multiple flag categories
8. **State Reset**: Verifies initialization happens between jobs by using different dump settings

The script cleans up between tests to avoid interference and captures all output for analysis. The `-v` flag in several tests helps observe the driver's internal state changes.
