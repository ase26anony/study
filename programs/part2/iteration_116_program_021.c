Here's a comprehensive test script that exercises the uncovered initialization logic in gcc.cc:

```bash
#!/bin/bash

# Test script to exercise GCC driver initialization logic
# Targets lines 11228-11250 in gcc.cc

set -e

# Configuration
GCC=${GCC:-gcc}
TEST_DIR=$(mktemp -d /tmp/gcc_driver_test_XXXXXX)
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
void farewell() {
    printf("Goodbye\n");
}
EOF

echo "Testing GCC driver initialization logic..."
echo "Test directory: $TEST_DIR"
echo "Using GCC: $($GCC --version | head -1)"
echo ""

# Helper function to clean up dump files
cleanup_dumps() {
    rm -f *.i *.s *.o *.dump *.gcda *.gcno profile* 2>/dev/null || true
    rm -rf dumps 2>/dev/null || true
}

# Test 1: Multiple jobs with sysroot variations
echo "=== Test 1: Multiple jobs with sysroot variations ==="
cleanup_dumps
$GCC hello1.c hello2.c hello3.c -o multi_hello \
    --sysroot= \
    --sysroot=/nonexistent \
    -isysroot /usr \
    -I/usr/include \
    -I/usr/local/include \
    -v 2>&1 | grep -q "gcc version" && echo "Test 1 passed" || echo "Test 1 failed"

# Test 2: Dump file generation with various options
echo ""
echo "=== Test 2: Dump file generation ==="
cleanup_dumps
$GCC hello1.c -o hello_dump \
    -save-temps=obj \
    -dumpdir=./dumps \
    -dumpbase=testdump \
    -dumpbase-ext=.myext \
    -fdump-tree-all \
    -fdump-rtl-expand \
    -fdump-ipa-all \
    -O2 2>&1 | tail -5

# Check if dump files were created
if [ -f "hello1.i" ] || [ -d "dumps" ]; then
    echo "Dump files created successfully"
else
    echo "Warning: No dump files found"
fi

# Test 3: Different save-temps modes
echo ""
echo "=== Test 3: Different save-temps modes ==="
for mode in "cwd" "obj" ""; do
    cleanup_dumps
    if [ -z "$mode" ]; then
        $GCC hello1.c -save-temps -o hello_temp_plain 2>/dev/null
    else
        $GCC hello1.c -save-temps=$mode -o hello_temp_$mode 2>/dev/null
    fi
    echo -n "save-temps${mode:+=$mode}: "
    if [ -f "hello1.i" ] || [ -f "hello1.s" ]; then
        echo "PASS"
    else
        echo "FAIL"
    fi
done

# Test 4: Help and version flags
echo ""
echo "=== Test 4: Help and version output ==="
$GCC --version >/dev/null && echo "--version: PASS" || echo "--version: FAIL"
$GCC --help >/dev/null && echo "--help: PASS" || echo "--help: FAIL"
$GCC --target-help 2>/dev/null && echo "--target-help: PASS" || echo "--target-help: FAIL"
$GCC --help=common >/dev/null && echo "--help=common: PASS" || echo "--help=common: FAIL"
$GCC --help=optimizers >/dev/null && echo "--help=optimizers: PASS" || echo "--help=optimizers: FAIL"

# Test 5: Help/version combined with compilation
echo ""
echo "=== Test 5: Help/version with compilation flags ==="
$GCC --version --sysroot=/ -save-temps hello1.c 2>&1 | grep -q "gcc version" && echo "Combined flags: PASS" || echo "Combined flags: FAIL"

# Test 6: Linker selection flags
echo ""
echo "=== Test 6: Linker selection ==="
for linker in bfd gold lld mold; do
    if $GCC -fuse-ld=$linker --help=common 2>&1 | grep -q "fuse-ld"; then
        echo "-fuse-ld=$linker: SUPPORTED"
    else
        echo "-fuse-ld=$linker: NOT SUPPORTED (or error)"
    fi
done

# Test with actual compilation if supported
$GCC -fuse-ld=bfd hello1.c -o hello_bfd -Wl,--verbose 2>/dev/null && echo "bfd linker test: PASS" || echo "bfd linker test: FAIL"

# Test 7: Timing and profile reports
echo ""
echo "=== Test 7: Timing and profile reports ==="
cleanup_dumps
$GCC hello1.c -o hello_time -ftime-report -O2 2>&1 | grep -q "Time variable" && echo "-ftime-report: PASS" || echo "-ftime-report: FAIL"

# Test 8: Profile-guided optimization flow
echo ""
echo "=== Test 8: PGO workflow ==="
cleanup_dumps

# Step 1: Generate profile
echo "Step 1: Instrumented compilation"
$GCC hello1.c -o hello_pgo -fprofile-generate -O2 2>/dev/null && echo "Instrumentation: PASS" || echo "Instrumentation: FAIL"

# Step 2: Run to generate profile data (if compilation succeeded)
if [ -f "hello_pgo" ]; then
    ./hello_pgo >/dev/null 2>&1
    # Step 3: Use profile
    echo "Step 2: Profile use"
    $GCC hello1.c -o hello_pgo_opt -fprofile-use -ftime-report -fprofile-report -O2 2>/dev/null && echo "Profile use: PASS" || echo "Profile use: FAIL"
fi

# Test 9: Comprehensive flag combination
echo ""
echo "=== Test 9: Comprehensive flag combination ==="
cleanup_dumps
$GCC hello1.c hello2.c -o hello_comprehensive \
    --sysroot=/ \
    -save-temps \
    -dumpdir=./comprehensive_dumps \
    -dumpbase=comp \
    -dumpbase-ext=.full \
    -fdump-tree-all \
    -fdump-rtl-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    -v 2>&1 | tail -10

# Test 10: Empty/null dump options
echo ""
echo "=== Test 10: Empty dump options ==="
cleanup_dumps
$GCC hello1.c -o hello_empty \
    -dumpdir= \
    -dumpbase= \
    -dumpbase-ext= \
    -save-temps \
    -O0 2>/dev/null && echo "Empty dump options: PASS" || echo "Empty dump options: FAIL"

# Test 11: Multiple sysroot changes
echo ""
echo "=== Test 11: Multiple sysroot flags ==="
$GCC hello1.c -o hello_sysroot \
    --sysroot=/ \
    --sysroot=/usr \
    -isysroot /var \
    -v 2>&1 | grep -q "sysroot" && echo "Multiple sysroots: PASS" || echo "Multiple sysroots: FAIL"

# Test 12: Reset behavior with multiple invocations
echo ""
echo "=== Test 12: Sequential invocations (testing reset) ==="
cleanup_dumps
$GCC --help=common >/dev/null
$GCC --version >/dev/null
$GCC hello1.c -o hello_final -O2 2>/dev/null && echo "Sequential reset: PASS" || echo "Sequential reset: FAIL"

# Cleanup
echo ""
echo "=== Cleanup ==="
cleanup_dumps
rm -f hello1.c hello2.c hello3.c
rm -f hello_* multi_hello *.o 2>/dev/null || true
rm -rf dumps comprehensive_dumps 2>/dev/null || true

echo ""
echo "Tests completed in: $TEST_DIR"
echo "Note: Some tests may show 'FAIL' if certain features are not supported"
echo "by your GCC version. This is normal - the important part is that the"
echo "driver initialization code was executed."
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple jobs**: Compiles multiple files (`hello1.c hello2.c hello3.c`) in single invocations
2. **Sysroot variations**: Tests `--sysroot=`, `--sysroot=/nonexistent`, `-isysroot`, and `-I` flags
3. **Dump file management**: Exercises `-save-temps` with different modes, `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, and various `-fdump-*` flags
4. **Help/version output**: Tests `--help`, `--version`, `--target-help`, `--help=common`, `--help=optimizers`
5. **Linker selection**: Tests `-fuse-ld=bfd/gold/lld/mold` and `-Wl,` options
6. **Timing/PGO**: Tests `-ftime-report`, `-fprofile-generate`, `-fprofile-use`, `-fprofile-report`
7. **Combination tests**: Puts multiple flags together to test interaction effects
8. **Reset behavior**: Tests sequential invocations to ensure proper reinitialization

The script creates temporary files, runs tests, and cleans up after itself. Some tests may show as "FAIL" if certain features aren't available in your GCC version, but the driver initialization code will still be executed.
