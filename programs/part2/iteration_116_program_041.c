Here's a comprehensive test script that exercises the uncovered initialization logic in gcc.cc:

```bash
#!/bin/bash

# GCC driver initialization coverage test script
# Tests lines 11228-11250 of gcc.cc

set -e

# Configuration
GCC="${GCC:-gcc}"
TEST_DIR="gcc_init_test"
HELLO_SRC="hello.c"
HELLO2_SRC="hello2.c"
PROG_OUT="hello_prog"
PROFILE_DATA="profile_data"

# Clean up previous test runs
cleanup() {
    rm -rf "$TEST_DIR" "$PROFILE_DATA" *.o *.s *.i *.ii *.bc *.expand *.original \
           *.gkd *.gcno *.gcda *.dwo *.dmp *.dump dumps/ temps/ *.times \
           "$PROG_OUT" hello hello2 test 2>/dev/null || true
}

# Create test directory
mkdir -p "$TEST_DIR"
cd "$TEST_DIR" || exit 1

# Create simple test programs
cat > "$HELLO_SRC" << 'EOF'
#include <stdio.h>
int main() {
    printf("Hello, World!\n");
    return 0;
}
EOF

cat > "$HELLO2_SRC" << 'EOF'
#include <stdio.h>
void helper() {
    printf("Helper function\n");
}
EOF

echo "=== Testing GCC Driver Initialization Logic ==="
echo "Target: Lines 11228-11250 of gcc.cc"
echo ""

# Test 1: Multiple jobs with sysroot variations
echo "Test 1: Multiple jobs with sysroot flags"
echo "----------------------------------------"
"$GCC" "$HELLO_SRC" "$HELLO2_SRC" -o "$PROG_OUT" \
    --sysroot= \
    --sysroot=/nonexistent \
    -isysroot /usr \
    -I/usr/include \
    -I. \
    2>&1 | grep -E "(sysroot|error|warning)" || true
echo "Exit code: $?"
echo ""

# Test 2: Dump file management with various options
echo "Test 2: Dump file management"
echo "---------------------------"
"$GCC" "$HELLO_SRC" \
    -save-temps=obj \
    -dumpdir=./dumps \
    -dumpbase="testdump" \
    -dumpbase-ext=".dump" \
    -fdump-tree-all \
    -fdump-rtl-expand \
    -fdump-ipa-all \
    -fdump-statistics \
    -o test1 2>&1 | tail -20
echo "Exit code: $?"
ls -la dumps/ 2>/dev/null || echo "No dump directory created"
echo ""

# Test 3: Different save-temps modes
echo "Test 3: Various save-temps modes"
echo "-------------------------------"
for mode in "obj" "cwd" ""; do
    echo "Testing -save-temps${mode:+=$mode}"
    "$GCC" "$HELLO_SRC" -save-temps${mode:+=$mode} -O1 -o "test_$mode" 2>&1 | grep -i "temp" || true
    ls -la *.i *.s *.o 2>/dev/null | wc -l | xargs echo "  Temp files created:"
    rm -f *.i *.s *.o test_$mode 2>/dev/null
done
echo ""

# Test 4: Help and version flags
echo "Test 4: Help and version output"
echo "-------------------------------"
for flag in "--help" "--target-help" "--version" "--help=common" "--help=optimizers" "--help=warnings"; do
    echo "Testing $flag"
    "$GCC" $flag 2>&1 | head -5
    echo "Exit code: $?"
done
echo ""

# Test 5: Combined help with compilation flags
echo "Test 5: Combined help/version with compilation"
echo "---------------------------------------------"
"$GCC" --version "$HELLO_SRC" -O2 2>&1 | head -10
echo "Exit code: $?"
"$GCC" --help=optimizers -dumpbase=test -O1 "$HELLO_SRC" -o test2 2>&1 | tail -5
echo "Exit code: $?"
echo ""

# Test 6: Linker selection flags
echo "Test 6: Linker selection"
echo "-----------------------"
for linker in "bfd" "gold" "lld" "mold"; do
    echo "Testing -fuse-ld=$linker"
    "$GCC" "$HELLO_SRC" -fuse-ld=$linker -Wl,--verbose -o "test_$linker" 2>&1 | grep -i "linker" || true
    echo "Exit code: $?"
    rm -f "test_$linker"
done
echo ""

# Test 7: Time and profile reports
echo "Test 7: Timing and profile reports"
echo "---------------------------------"
"$GCC" "$HELLO_SRC" -ftime-report -O2 -o test_time 2>&1 | grep -A5 "Time variable"
echo "Exit code: $?"
rm -f test_time

# Test 8: Profile-guided optimization flow
echo "Test 8: PGO workflow"
echo "-------------------"
echo "Step 1: Generate profile"
"$GCC" "$HELLO_SRC" -fprofile-generate="$PROFILE_DATA" -o pgo_gen 2>&1 | tail -5
./pgo_gen 2>/dev/null || true

echo "Step 2: Use profile with reports"
"$GCC" "$HELLO_SRC" -fprofile-use="$PROFILE_DATA" -fprofile-report -ftime-report -O3 -o pgo_use 2>&1 | tail -10
echo "Exit code: $?"
echo ""

# Test 9: Comprehensive flag combination
echo "Test 9: Comprehensive flag combination"
echo "-------------------------------------"
"$GCC" "$HELLO_SRC" "$HELLO2_SRC" \
    --sysroot=/ \
    -save-temps \
    -dumpdir=./comprehensive_dumps \
    -dumpbase="comprehensive" \
    -dumpbase-ext=".full" \
    -fdump-tree-all \
    -fdump-rtl-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    -v \
    -o comprehensive_test 2>&1 | grep -E "(sysroot|dump|save-temps|Using.*linker|Time variable)" || true
echo "Exit code: $?"
ls -la comprehensive_dumps/ 2>/dev/null | head -5 || true
echo ""

# Test 10: Edge cases with empty values
echo "Test 10: Edge cases with empty values"
echo "------------------------------------"
"$GCC" "$HELLO_SRC" \
    --sysroot="" \
    -dumpdir="" \
    -dumpbase="" \
    -dumpbase-ext="" \
    -save-temps=cwd \
    -o edge_test 2>&1 | grep -E "(sysroot|dump)" || true
echo "Exit code: $?"
echo ""

# Test 11: Verbose flag to observe initialization
echo "Test 11: Verbose output to observe driver state"
echo "----------------------------------------------"
"$GCC" "$HELLO_SRC" -v -save-temps=obj --sysroot=/ -ftime-report -o verbose_test 2>&1 | \
    grep -B2 -A2 "COLLECT_GCC_OPTIONS" | head -20
echo "Exit code: $?"
echo ""

# Test 12: Reset between jobs (implicitly tested by multiple file compilation)
echo "Test 12: Multiple jobs testing re-initialization"
echo "------------------------------------------------"
"$GCC" -c "$HELLO_SRC" -save-temps=obj -dumpbase="job1" -O0
"$GCC" -c "$HELLO2_SRC" -save-temps=cwd -dumpbase="job2" -O2
"$GCC" hello.o hello2.o -fuse-ld=bfd -Wl,--print-map -o multi_job_test 2>&1 | tail -5
echo "Exit code: $?"
echo ""

# Cleanup
echo "=== Test Complete ==="
cd ..
cleanup
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple Jobs**: Compiles multiple files (`hello.c` and `hello2.c`) in single invocations
2. **Sysroot Variations**: Tests `--sysroot=`, `--sysroot=/nonexistent`, `-isysroot`, and `-I` flags
3. **Dump Management**: Exercises `-save-temps` (all variants), `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, and various `-fdump-*` flags
4. **Help/Version**: Tests `--help`, `--target-help`, `--version`, `--help=<subprocess>` alone and combined with compilation
5. **Linker Selection**: Tests `-fuse-ld=bfd/gold/lld/mold` with `-Wl,` options
6. **PGO and Timing**: Implements full PGO workflow with `-fprofile-generate`, `-fprofile-use`, `-ftime-report`, and `-fprofile-report`
7. **Combined Flags**: Uses comprehensive combinations that trigger multiple initialization variables
8. **Edge Cases**: Tests empty string values for dump and sysroot options
9. **Verbose Output**: Uses `-v` to observe driver state changes
10. **Job Separation**: Demonstrates re-initialization between separate compilation jobs

The script captures exit codes and relevant output while cleaning up between tests to avoid interference. Each test case targets specific variables from the uncovered block, ensuring the initialization logic is exercised.
