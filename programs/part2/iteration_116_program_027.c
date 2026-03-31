Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc by testing various flag combinations and execution scenarios:

```bash
#!/bin/bash
set -e

# Configuration
GCC=${1:-gcc}
TEST_DIR=$(mktemp -d)
cd "$TEST_DIR"
echo "Testing in directory: $TEST_DIR"

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

echo "=== Testing GCC driver initialization logic ==="
echo "Using GCC: $($GCC --version | head -1)"

# Cleanup function
cleanup() {
    rm -f *.o *.s *.i *.ii *.o *.so *.a hello prog test *.gcda *.gcno *.gcov
    rm -rf dumps temps
}

# Test 1: Multiple jobs with sysroot variations
echo -e "\n1. Testing multiple jobs with sysroot flags"
cleanup
$GCC hello1.c hello2.c -o hello \
    --sysroot= \
    --sysroot=/nonexistent \
    -isysroot /usr \
    -I/usr/include \
    -I. \
    -v 2>&1 | grep -q "gcc version" && echo "  ✓ Test 1 passed" || echo "  ✗ Test 1 failed"

# Test 2: Dump file generation with various options
echo -e "\n2. Testing dump file generation"
cleanup
mkdir -p dumps
$GCC hello1.c \
    -save-temps=obj \
    -dumpdir=./dumps \
    -dumpbase=testdump \
    -dumpbase-ext=.myext \
    -fdump-tree-all \
    -fdump-rtl-expand \
    -fdump-ipa-all \
    -o hello 2>&1 | tail -5
echo "  Checking dump files..."
ls -la dumps/*.myext* 2>/dev/null | head -3 && echo "  ✓ Dump files created"

# Test 3: Different save-temps modes
echo -e "\n3. Testing save-temps variations"
for mode in "cwd" "obj" ""; do
    cleanup
    if [ -z "$mode" ]; then
        $GCC hello1.c -save-temps -o hello >/dev/null 2>&1
    else
        $GCC hello1.c -save-temps=$mode -o hello >/dev/null 2>&1
    fi
    echo "  -save-temps${mode:+=$mode}: $(ls *.i *.s 2>/dev/null | wc -l) temp files"
done

# Test 4: Help and version flags
echo -e "\n4. Testing help and version output"
$GCC --help >/dev/null 2>&1 && echo "  ✓ --help"
$GCC --version >/dev/null 2>&1 && echo "  ✓ --version"
$GCC --target-help 2>&1 | head -5 >/dev/null && echo "  ✓ --target-help"
$GCC --help=common 2>&1 | head -5 >/dev/null && echo "  ✓ --help=common"
$GCC --help=optimizers 2>&1 | head -5 >/dev/null && echo "  ✓ --help=optimizers"

# Test 5: Help flags combined with compilation
echo -e "\n5. Testing help flags with compilation args"
$GCC --help=warnings hello1.c 2>&1 | head -10 >/dev/null && echo "  ✓ --help=warnings with source"
$GCC --version -O2 hello1.c 2>&1 | grep -q "gcc version" && echo "  ✓ --version with optimization"

# Test 6: Linker selection flags
echo -e "\n6. Testing linker selection"
for linker in bfd gold lld mold; do
    if $GCC -fuse-ld=$linker hello1.c -o hello 2>/dev/null; then
        echo "  ✓ -fuse-ld=$linker"
        rm -f hello
    else
        echo "  ✗ -fuse-ld=$linker (not available)"
    fi
done

# Test with -Wl, options
$GCC hello1.c -Wl,--verbose -o hello 2>&1 | grep -q "collect2" && echo "  ✓ -Wl,--verbose"

# Test 7: Time and profile reports
echo -e "\n7. Testing timing and profile reports"
cleanup
$GCC hello1.c -ftime-report -o hello 2>&1 | grep -i "time" >/dev/null && echo "  ✓ -ftime-report"

# Test 8: Complex combined invocation
echo -e "\n8. Testing complex combined invocation"
cleanup
$GCC hello1.c hello2.c \
    --sysroot=/ \
    -save-temps \
    -dumpdir=./dumps \
    -fdump-tree-all \
    -ftime-report \
    -fuse-ld=bfd \
    -O2 \
    -o prog \
    -v 2>&1 | tail -20 > combined_output.txt
if [ -f prog ]; then
    echo "  ✓ Combined compilation successful"
    ./prog 2>/dev/null && echo "  ✓ Program executes correctly"
fi

# Test 9: Profile-guided optimization flow
echo -e "\n9. Testing PGO flow (if supported)"
cleanup
# Phase 1: Generate profile
if $GCC pgo_test.c -fprofile-generate -o pgo_gen 2>/dev/null; then
    ./pgo_gen 2>/dev/null
    # Phase 2: Use profile with reports
    $GCC pgo_test.c -fprofile-use -fprofile-report -o pgo_use 2>/dev/null && \
        echo "  ✓ PGO flow completed"
    rm -f pgo_gen pgo_use *.gcda *.gcno
fi

# Test 10: Empty and special values for dump options
echo -e "\n10. Testing edge cases for dump options"
cleanup
$GCC hello1.c \
    -dumpdir= \
    -dumpbase= \
    -dumpbase-ext= \
    -save-temps=cwd \
    -o hello 2>/dev/null && echo "  ✓ Empty dump options"
$GCC hello1.c -dumpdir=./dumps/ -dumpbase="test with spaces" -o hello 2>/dev/null && \
    echo "  ✓ Special characters in dumpbase"

# Test 11: Reset behavior with multiple invocations
echo -e "\n11. Testing reset behavior across invocations"
cleanup
$GCC --sysroot=/nonexistent --help=optimizers >/dev/null 2>&1
$GCC hello1.c -save-temps -o hello >/dev/null 2>&1
$GCC --version >/dev/null 2>&1
$GCC hello1.c hello2.c -o final >/dev/null 2>&1 && echo "  ✓ Sequential invocations work"

# Test 12: Verbose and debug output
echo -e "\n12. Testing verbose output"
$GCC hello1.c -v -da -dp -o hello 2>&1 | grep -q "COLLECT_GCC" && echo "  ✓ Verbose with debug flags"

# Cleanup
echo -e "\n=== Cleaning up ==="
cd ..
rm -rf "$TEST_DIR"
echo "Test directory removed"
echo -e "\n=== All tests completed ==="
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple jobs with sysroot variations** - Exercises `target_system_root`, `target_system_root_changed`
2. **Dump file generation** - Tests `dumpdir`, `dumpbase`, `dumpbase_ext`, `save_temps_flag`
3. **Help and version flags** - Exercises `print_help_list`, `print_version`, `print_subprocess_help`
4. **Linker selection** - Tests `use_ld` variable
5. **Timing reports** - Exercises `report_times_to_file` path
6. **Complex combinations** - Tests interaction of multiple flags
7. **PGO flow** - Tests profile-guided optimization paths
8. **Edge cases** - Tests empty values and special characters
9. **Reset behavior** - Ensures variables are properly reset between invocations

The script creates temporary test programs, runs GCC with various flag combinations, and cleans up after itself. Each test is designed to trigger specific parts of the initialization logic while ensuring the driver can proceed without fatal errors.
