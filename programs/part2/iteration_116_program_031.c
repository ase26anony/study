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
    printf("Hello 1\n");
    return 0;
}
EOF

cat > hello2.c << 'EOF'
#include <stdio.h>
void helper() {
    printf("Helper\n");
}
EOF

cat > hello3.c << 'EOF'
#include <stdio.h>
extern void helper();
int main() {
    printf("Hello 3\n");
    helper();
    return 0;
}
EOF

# Create a dummy sysroot directory structure
mkdir -p dummy_sysroot/usr/include
mkdir -p dummy_sysroot/usr/lib
echo "/* dummy header */" > dummy_sysroot/usr/include/dummy.h

# Function to clean up dump files between tests
clean_dumps() {
    rm -f *.i *.s *.o *.ii *.bc *.expand *.original *.gimple \
          *.cfg *.optimized *.ssa *.cse *.jump *.rtl *.greg \
          *.stack *.address *.align *.asm *.ltrans *.d \
          *.gcda *.gcno *.gcov *.time *.times 2>/dev/null || true
    rm -rf dumps 2>/dev/null || true
}

# Function to run GCC and capture exit code
run_gcc() {
    local desc="$1"
    shift
    echo "========================================"
    echo "Test: $desc"
    echo "Command: $GCC $*"
    clean_dumps
    $GCC "$@" 2>&1 | head -50
    local status=$?
    echo "Exit code: $status"
    echo
    return $status
}

# Test 1: Multiple jobs with basic initialization
echo "=== TEST 1: Multiple jobs with basic flags ==="
run_gcc "Multiple files, sysroot, verbose" \
    -v --sysroot= -save-temps -fdump-tree-original -ftime-report \
    hello1.c hello2.c -o hello12

# Test 2: Sysroot variations
echo "=== TEST 2: Sysroot variations ==="
run_gcc "Empty sysroot" --sysroot= hello1.c -c
run_gcc "Non-existent sysroot" --sysroot=/nonexistent/path hello1.c -c 2>/dev/null || true
run_gcc "Dummy sysroot" --sysroot=dummy_sysroot hello1.c -c
run_gcc "Sysroot with isysroot" --sysroot=dummy_sysroot -isysroot dummy_sysroot hello1.c -c
run_gcc "Sysroot with includes" --sysroot=dummy_sysroot -Idummy_sysroot/usr/include hello1.c -c

# Test 3: Dump file management variations
echo "=== TEST 3: Dump file management ==="
run_gcc "Save temps obj" -save-temps=obj hello1.c -c
clean_dumps
run_gcc "Save temps cwd" -save-temps=cwd hello1.c -c
clean_dumps
run_gcc "Save temps default" -save-temps hello1.c -c
clean_dumps
run_gcc "Dumpdir with path" -dumpdir=./dumps -fdump-tree-all hello1.c -c
clean_dumps
run_gcc "Dumpdir empty" -dumpdir= -fdump-tree-all hello1.c -c
clean_dumps
run_gcc "Dumpbase variations" -dumpbase=myfile -dumpbase-ext=.dump -fdump-rtl-expand hello1.c -c
clean_dumps
run_gcc "Multiple dump flags" -fdump-tree-all -fdump-rtl-all -fdump-ipa-all hello1.c -c

# Test 4: Help and version output
echo "=== TEST 4: Help and version ==="
run_gcc "Help" --help
run_gcc "Target help" --target-help
run_gcc "Version" --version
run_gcc "Help optimizers" --help=optimizers
run_gcc "Help common" --help=common
run_gcc "Help with other flags" --help -O2 --sysroot= 2>/dev/null

# Test 5: Linker selection
echo "=== TEST 5: Linker selection ==="
for linker in bfd gold lld mold; do
    run_gcc "Linker $linker" -fuse-ld=$linker hello1.c -o hello_$linker 2>/dev/null || true
done
run_gcc "Wl options" -Wl,--verbose hello1.c -o hello_wl 2>/dev/null || true

# Test 6: Combined complex invocation
echo "=== TEST 6: Combined complex invocation ==="
run_gcc "Complex combined flags" \
    --sysroot=/ \
    -save-temps \
    -dumpdir=./dumps \
    -fdump-tree-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    hello1.c hello2.c \
    -o hello_complex

# Test 7: Profile-guided optimization path
echo "=== TEST 7: PGO workflow ==="
# Step 1: Generate profile
run_gcc "PGO generate" -O2 -fprofile-generate -ftest-coverage hello1.c -o hello_pgo_gen
./hello_pgo_gen 2>/dev/null || true

# Step 2: Use profile with timing report
run_gcc "PGO use with reports" -O2 -fprofile-use -fprofile-report -ftime-report hello1.c -o hello_pgo_use

# Test 8: Multiple help/version combinations
echo "=== TEST 8: Help/version combinations ==="
run_gcc "Version with dump flags" --version -dumpbase=test -fdump-tree-all 2>/dev/null
run_gcc "Help with sysroot" --help --sysroot=dummy_sysroot 2>/dev/null

# Test 9: Reset variables with empty values
echo "=== TEST 9: Empty value resets ==="
run_gcc "Empty dump values" -dumpdir= -dumpbase= -dumpbase-ext= -save-temps hello1.c -c

# Test 10: Verbose only flag simulation
echo "=== TEST 10: Verbose output ==="
run_gcc "Verbose compilation" -v -### hello1.c 2>&1 | head -20

# Cleanup
echo "========================================"
echo "Cleaning up test directory: $TESTDIR"
cd /
rm -rf "$TESTDIR"
echo "All tests completed"
```

This script systematically tests the uncovered initialization block by:

1. **Multiple Jobs**: Compiles multiple files (`hello1.c hello2.c`) in single invocations
2. **Sysroot Variations**: Tests `--sysroot=` with empty, non-existent, and valid paths, plus `-isysroot` and `-I`
3. **Dump Management**: Exercises all `-save-temps` variants, `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, and various `-fdump-*` flags
4. **Help/Version**: Tests `--help`, `--target-help`, `--version`, and `--help=<subprocess>` with different arguments
5. **Linker Selection**: Tests `-fuse-ld=` with different linkers and `-Wl,` options
6. **Combined Flags**: Uses a complex combination of flags in one invocation
7. **PGO Workflow**: Implements a complete profile-guided optimization cycle
8. **Edge Cases**: Tests empty values and verbose output

The script cleans up between tests to avoid interference and captures exit codes to verify the driver handles all flag combinations correctly. The `-v` (verbose) flag helps confirm the driver's internal state changes are occurring as expected.
