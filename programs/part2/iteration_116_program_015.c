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
    printf("Hello World 1\n");
    return 0;
}
EOF

cat > hello2.c << 'EOF'
#include <stdio.h>
void helper() {
    printf("Helper function\n");
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

# Function to clean up dump files
clean_dumps() {
    rm -f *.i *.s *.o *.ii *.bc *.expand *.original *.gimple *.cfg *.optimized
    rm -f *.c.* *.c.*.* *.rtl *.pass.* *.ipa-*
    rm -rf dumps/ *.dSYM/
}

# Test 1: Multiple jobs with sysroot variations
echo "=== Test 1: Multiple jobs with sysroot ==="
clean_dumps
$GCC hello1.c hello2.c -o prog1 \
    --sysroot= \
    -isysroot /usr/include \
    -I/usr/local/include \
    -v 2>&1 | grep -q "sysroot" || true
./prog1 2>/dev/null || true

# Test 2: Dump file management with various options
echo "=== Test 2: Dump file management ==="
clean_dumps
$GCC hello1.c -o prog2 \
    -save-temps=obj \
    -dumpdir=./dumps \
    -dumpbase="testdump" \
    -dumpbase-ext=".dump" \
    -fdump-tree-all \
    -fdump-rtl-expand \
    -fdump-ipa-all \
    -O0 2>&1 | tail -20 || true

# Test 3: Help and version flags
echo "=== Test 3: Help and version output ==="
$GCC --help > /dev/null
$GCC --target-help 2>&1 | head -5
$GCC --version
$GCC --help=common 2>&1 | head -5
$GCC --help=optimizers 2>&1 | head -5
$GCC --help=warnings 2>&1 | head -5

# Test 4: Linker selection flags
echo "=== Test 4: Linker selection ==="
clean_dumps
for linker in bfd gold lld mold; do
    echo "Testing -fuse-ld=$linker"
    $GCC hello1.c -fuse-ld=$linker -Wl,--verbose -o prog_$linker 2>&1 | \
        grep -i "linker" | head -2 || true
done

# Test 5: Combined flags in single invocation
echo "=== Test 5: Combined flags ==="
clean_dumps
$GCC hello1.c hello2.c -o prog_combined \
    --sysroot=/ \
    -save-temps \
    -dumpdir=./combined_dumps \
    -fdump-tree-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    -v 2>&1 | tail -30 || true

# Test 6: Different save-temps options
echo "=== Test 6: Various save-temps modes ==="
clean_dumps
for mode in "cwd" "obj" ""; do
    echo "Testing -save-temps=$mode"
    if [ -z "$mode" ]; then
        $GCC hello1.c -save-temps -o prog_temp_$mode 2>/dev/null || true
    else
        $GCC hello1.c -save-temps=$mode -o prog_temp_$mode 2>/dev/null || true
    fi
    ls -la *.i *.s *.o 2>/dev/null | head -3 || true
    clean_dumps
done

# Test 7: Profile-guided optimization with timing reports
echo "=== Test 7: PGO with timing reports ==="
clean_dumps

# Phase 1: Generate profile
echo "Phase 1: Profile generation"
$GCC pgo_test.c -O2 -fprofile-generate -ftest-coverage -o pgo_instr
./pgo_instr 2>/dev/null || true

# Phase 2: Use profile with timing report
echo "Phase 2: Profile usage with timing"
$GCC pgo_test.c -O2 -fprofile-use -fprofile-report -ftime-report -o pgo_opt 2>&1 | \
    grep -A5 -B5 "profile\|time report" || true

# Test 8: Empty and special values for dump options
echo "=== Test 8: Empty/special dump values ==="
clean_dumps
$GCC hello1.c -o prog_empty \
    -dumpdir="" \
    -dumpbase="" \
    -dumpbase-ext="" \
    -save-temps \
    -fdump-tree-original 2>&1 | tail -10 || true

# Test 9: Non-existent sysroot
echo "=== Test 9: Non-existent sysroot ==="
$GCC hello1.c --sysroot=/nonexistent/path -o prog_nonexist 2>&1 | \
    grep -i "error\|warning\|sysroot" | head -5 || true

# Test 10: Verbose only flag simulation
echo "=== Test 10: Verbose compilation ==="
$GCC hello1.c -v -### 2>&1 | grep -A2 "sysroot\|collect2" | head -10

# Test 11: Subprocess help with compilation flags
echo "=== Test 11: Help combined with compilation ==="
$GCC --help=common -E hello1.c 2>&1 | head -20
$GCC --version -c hello1.c 2>&1 | grep -i "version\|gcc" | head -3

# Test 12: Multiple dumpbase extensions
echo "=== Test 12: Multiple dump extensions ==="
clean_dumps
$GCC hello1.c -o prog_multi_dump \
    -dumpbase="multi" \
    -dumpbase-ext=".part1" \
    -fdump-tree-optimized \
    -fdump-rtl-expand \
    -dumpbase-ext=".part2" \
    -fdump-ipa-inline 2>&1 | tail -15 || true

# Test 13: Override dumpdir with save-temps
echo "=== Test 13: Dumpdir override behavior ==="
clean_dumps
mkdir -p custom_dump
$GCC hello1.c -o prog_override \
    -save-temps=cwd \
    -dumpdir=custom_dump \
    -fdump-tree-all 2>&1 | grep -i "dump\|save" | head -5 || true

# Test 14: System root suffix flags (if supported)
echo "=== Test 14: System root suffix flags ==="
$GCC hello1.c -o prog_suffix \
    --sysroot=/ \
    -isysroot /usr \
    -I /usr/local/include \
    -v 2>&1 | grep -i "sysroot\|include" | head -10 || true

# Cleanup
echo "=== Cleaning up ==="
clean_dumps
rm -f prog* pgo* hello*.o *.gcda *.gcno *.gcov
cd ..
rm -rf "$TEST_DIR"
echo "Test completed"
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple jobs with sysroot variations** (lines 11228-11250: `target_system_root`, `target_system_root_changed`)
2. **Dump file management** (`dumpdir`, `dumpbase`, `dumpbase_ext`, `outbase`, `save_temps_flag`)
3. **Help and version output** (`print_help_list`, `print_version`, `print_subprocess_help`)
4. **Linker selection** (`use_ld`)
5. **Timing reports** (`report_times_to_file`)
6. **Profile-guided optimization paths**
7. **Edge cases with empty/non-existent paths**
8. **Combinations of multiple flags in single invocations**

The script creates temporary test programs, runs GCC with various flag combinations, captures relevant output, and cleans up between tests to avoid interference. Each test targets specific variables in the uncovered initialization block while ensuring the driver proceeds through its normal execution flow.
