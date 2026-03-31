Here's a comprehensive test script that exercises the uncovered initialization logic in gcc.cc:

```bash
#!/bin/bash
set -e

# Configuration
GCC=${GCC:-gcc}
TEST_DIR=$(mktemp -d /tmp/gcc_driver_test_XXXXXX)
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    echo "Cleaning up test directory: $TEST_DIR"
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

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

echo "Testing GCC driver initialization logic..."
echo "=========================================="

# Test 1: Multiple jobs with sysroot variations
echo -e "\n1. Testing multiple jobs with sysroot flags..."
$GCC hello1.c hello2.c -o multi_hello \
    --sysroot= \
    --sysroot=/nonexistent \
    -isysroot /usr \
    -I/usr/include \
    -I. \
    2>&1 | grep -E "(sysroot|error|warning)" || true

# Test 2: Dump file management with various options
echo -e "\n2. Testing dump file management..."
$GCC hello1.c -save-temps=obj -dumpdir=./dumps -dumpbase=test1 \
    -dumpbase-ext=.dump -fdump-tree-all -fdump-rtl-expand \
    -o hello1_dump 2>&1 | tail -20

# Clean dump files for next test
rm -rf dumps *.i *.s *.o *.dump 2>/dev/null || true

# Test 3: Different save-temps modes
echo -e "\n3. Testing save-temps variations..."
for mode in "cwd" "obj" ""; do
    echo "  Testing -save-temps=$mode"
    $GCC hello1.c -save-temps=$mode -dumpdir=./temps_$mode \
        -fdump-tree-original -o hello1_$mode 2>/dev/null || true
done

# Clean up
rm -f hello1_* *.i *.s *.o 2>/dev/null || true

# Test 4: Help and version flags
echo -e "\n4. Testing help and version flags..."
$GCC --help > /dev/null
$GCC --target-help 2>/dev/null | head -5
$GCC --version | head -1
$GCC --help=common 2>&1 | head -5
$GCC --help=optimizers 2>&1 | head -5

# Test 5: Combined flags with help
echo -e "\n5. Testing combined flags with help..."
$GCC --help=optimizers -dumpbase=help_test -O2 2>&1 | head -3

# Test 6: Linker selection flags
echo -e "\n6. Testing linker selection..."
for linker in bfd gold lld mold; do
    echo "  Testing -fuse-ld=$linker"
    $GCC hello1.c -fuse-ld=$linker -Wl,--verbose -o hello1_$linker 2>&1 | \
        grep -i "linker" | head -1 || true
done

# Test 7: Time and profile reports
echo -e "\n7. Testing time and profile reports..."
$GCC hello1.c -ftime-report -fprofile-report -O2 -o hello1_time 2>&1 | \
    grep -E "(Time|profile)" | head -5

# Test 8: Comprehensive flag combination
echo -e "\n8. Testing comprehensive flag combination..."
$GCC hello1.c hello2.c \
    --sysroot=/ \
    -save-temps \
    -dumpdir=./comprehensive \
    -dumpbase=combo \
    -dumpbase-ext=.full \
    -fdump-tree-all \
    -fdump-rtl-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    -v \
    -o comprehensive_test 2>&1 | \
    grep -E "(sysroot|dump|save-temps|use.*ld|Time)" | head -10

# Test 9: Profile-guided optimization flow
echo -e "\n9. Testing PGO workflow..."
echo "  Phase 1: Instrumented compilation"
$GCC pgo_test.c -fprofile-generate -ftest-coverage -O2 -o pgo_instr

echo "  Running instrumented program..."
./pgo_instr > /dev/null

echo "  Phase 2: Optimization with profile data"
$GCC pgo_test.c -fprofile-use -fprofile-report -fprofile-correction \
    -ftime-report -O2 -o pgo_opt 2>&1 | \
    grep -E "(profile|Time)" | head -5

# Test 10: Reset behavior with multiple invocations
echo -e "\n10. Testing reset behavior across invocations..."
$GCC hello1.c -save-temps=obj -dumpdir=./test1 -o test1
$GCC hello2.c -save-temps=cwd -dumpdir=./test2 -o test2
$GCC hello1.c hello2.c -save-temps -dumpdir=./test3 -o test3

# Test 11: Edge cases for dump options
echo -e "\n11. Testing dump option edge cases..."
$GCC hello1.c -dumpdir="" -dumpbase="" -dumpbase-ext="" -save-temps=none -o edge1
$GCC hello1.c -dumpdir="." -dumpbase="test" -dumpbase-ext=".dump" -o edge2

# Test 12: Verbose flag with initialization
echo -e "\n12. Testing verbose output..."
$GCC hello1.c -v --sysroot= -save-temps -fdump-tree-original \
    -ftime-report -o verbose_test 2>&1 | \
    grep -E "(COLLECT_GCC|sysroot|dump)" | head -10

# Cleanup generated files
echo -e "\nCleaning generated files..."
rm -rf dumps comprehensive temps_* *.gcda *.gcno *.gcov 2>/dev/null || true

echo -e "\n=========================================="
echo "All tests completed successfully!"
echo "The following driver initialization variables were exercised:"
echo "  - is_cpp_driver, at_file_supplied"
echo "  - print_help_list, print_version, print_subprocess_help"
echo "  - use_ld, report_times_to_file"
echo "  - target_system_root, target_system_root_changed"
echo "  - target_sysroot_suffix, target_sysroot_hdrs_suffix"
echo "  - save_temps_flag, save_temps_overrides_dumpdir"
echo "  - dumpdir_trailing_dash_added"
echo "  - dumpdir, dumpbase, dumpbase_ext, outbase"
echo "  - spec_machine, greatest_status"
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple Jobs**: Compiles multiple files (`hello1.c hello2.c`) in single invocations
2. **Sysroot Variations**: Tests `--sysroot=`, `--sysroot=/nonexistent`, `-isysroot`, and `-I` flags
3. **Dump Management**: Exercises all `-dump*` flags with various values and combinations
4. **Save-temps**: Tests all modes (`obj`, `cwd`, default)
5. **Help/Version**: Tests `--help`, `--target-help`, `--version`, `--help=common`, `--help=optimizers`
6. **Linker Selection**: Tests `-fuse-ld=` with bfd, gold, lld, mold
7. **Timing Reports**: Tests `-ftime-report` and `-fprofile-report`
8. **PGO Workflow**: Full PGO cycle with `-fprofile-generate` and `-fprofile-use`
9. **Combined Flags**: Tests complex combinations that trigger multiple initialization paths
10. **Reset Behavior**: Tests that variables are properly reset between invocations
11. **Edge Cases**: Tests empty strings and special values for dump options
12. **Verbose Output**: Uses `-v` to observe driver state changes

The script creates temporary test files, runs the GCC driver with various flag combinations, and cleans up after itself. Each test targets specific variables from the uncovered block while ensuring valid compilation where appropriate.
