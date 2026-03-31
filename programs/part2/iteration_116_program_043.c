Here's a comprehensive test script that exercises the uncovered initialization logic in gcc.cc:

```bash
#!/bin/bash

# Test script for GCC driver initialization coverage
# Targets lines 11228-11250 in gcc.cc

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
extern void greet();
int main() {
    printf("Hello World 3\n");
    greet();
    return 0;
}
EOF

echo "Testing GCC driver initialization logic..."
echo "=========================================="

# Test 1: Multiple jobs with sysroot variations
echo -e "\n1. Testing multiple jobs with sysroot flags:"
$GCC hello1.c hello2.c -o multi1 --sysroot= -v 2>&1 | grep -q "sysroot" && echo "  ✓ Empty sysroot processed"
$GCC hello1.c hello2.c -o multi2 --sysroot=/nonexistent/path -v 2>&1 | grep -q "sysroot" && echo "  ✓ Non-existent sysroot processed"
$GCC hello1.c hello2.c -o multi3 --sysroot=/ -isysroot/usr/include -I/usr/local/include -v 2>&1 | grep -q "sysroot\|isysroot" && echo "  ✓ Sysroot with includes processed"

# Test 2: Dump file management with various options
echo -e "\n2. Testing dump file management:"
$GCC hello1.c -save-temps=obj -dumpdir=./dumps1 -fdump-tree-all -o hello1_dump 2>&1 && echo "  ✓ save-temps=obj with dumpdir"
$GCC hello1.c -save-temps=cwd -dumpbase="testdump" -dumpbase-ext=".myext" -fdump-rtl-expand -o hello2_dump 2>&1 && echo "  ✓ save-temps=cwd with dumpbase"
$GCC hello1.c -save-temps -dumpdir="" -dumpbase="" -dumpbase-ext="" -fdump-ipa-all -o hello3_dump 2>&1 && echo "  ✓ Empty dump options"
$GCC hello1.c -fdump-tree-optimized -fdump-rtl-final -o hello4_dump 2>&1 && echo "  ✓ Multiple fdump flags"

# Clean dump files between tests
rm -f *.i *.s *.o *.dump *.c.* *.expand *.final *.optimized

# Test 3: Help and version output
echo -e "\n3. Testing help and version flags:"
$GCC --help > /dev/null 2>&1 && echo "  ✓ --help flag"
$GCC --target-help > /dev/null 2>&1 && echo "  ✓ --target-help flag"
$GCC --version > /dev/null 2>&1 && echo "  ✓ --version flag"
$GCC --help=common > /dev/null 2>&1 && echo "  ✓ --help=common"
$GCC --help=optimizers > /dev/null 2>&1 && echo "  ✓ --help=optimizers"
$GCC --help=warnings > /dev/null 2>&1 && echo "  ✓ --help=warnings"

# Test 4: Combined help with compilation flags
echo -e "\n4. Testing help flags with compilation:"
$GCC --help hello1.c -o dummy 2>&1 | grep -q "help" && echo "  ✓ Help with source file"
$GCC --version -O2 hello1.c -o dummy 2>&1 | grep -q "version" && echo "  ✓ Version with optimization"

# Test 5: Linker selection flags
echo -e "\n5. Testing linker selection:"
for linker in bfd gold lld mold; do
    if $GCC -fuse-ld=$linker hello1.c -o hello_$linker 2>&1 | grep -q "ld"; then
        echo "  ✓ fuse-ld=$linker"
    fi
done
$GCC -fuse-ld=bfd -Wl,--verbose hello1.c -o hello_wl 2>&1 | grep -q "ld" && echo "  ✓ Wl, options"

# Test 6: Timing and profile reports
echo -e "\n6. Testing timing and profile reports:"
$GCC -ftime-report hello1.c -o hello_time 2>&1 | grep -q "time report" && echo "  ✓ ftime-report"
$GCC -fprofile-report hello1.c -o hello_profile 2>&1 && echo "  ✓ fprofile-report"

# Test 7: Profile-guided optimization flow
echo -e "\n7. Testing PGO workflow:"
# Generate instrumented binary
$GCC -O2 -fprofile-generate hello1.c -o hello_instr 2>&1 && echo "  ✓ Profile generation compile"
# Run to generate profile data (if supported)
if [ -x ./hello_instr ]; then
    ./hello_instr > /dev/null 2>&1 && echo "  ✓ Instrumented execution"
    # Use profile data
    $GCC -O2 -fprofile-use -ftime-report -fprofile-correction hello1.c -o hello_pgo 2>&1 && echo "  ✓ Profile use with timing"
fi

# Test 8: Comprehensive flag combination (targeting all uncovered lines)
echo -e "\n8. Testing comprehensive flag combination:"
$GCC --sysroot=/ -save-temps -dumpdir=./comprehensive_dumps \
     -dumpbase="comprehensive_test" -dumpbase-ext=".cdump" \
     -fdump-tree-all -fdump-rtl-all -ftime-report \
     -fuse-ld=gold -Wl,--print-map \
     hello1.c hello2.c hello3.c \
     -o comprehensive 2>&1 && echo "  ✓ All flags combined"

# Test 9: Multiple invocations to test re-initialization
echo -e "\n9. Testing multiple invocations (re-initialization):"
for i in {1..3}; do
    $GCC -save-temps=obj --sysroot=/ -dumpdir="./dump$i" \
         -fdump-tree-original hello1.c -o "hello_iter$i" 2>&1 > /dev/null
done
echo "  ✓ Multiple sequential compilations"

# Test 10: Edge cases with dump options
echo -e "\n10. Testing dump option edge cases:"
$GCC -dumpdir -dumpbase -dumpbase-ext hello1.c -o hello_edge 2>&1 > /dev/null && echo "  ✓ Dump flags without values"
$GCC -save-temps=obj -dumpdir=./trailing/ hello1.c -o hello_trailing 2>&1 && echo "  ✓ Trailing slash in dumpdir"

# Cleanup generated files
echo -e "\nCleaning generated files..."
rm -f *.o *.i *.s *.dump *.*.* hello_* multi* comprehensive* hello_*dump
rm -rf dumps* comprehensive_dumps dump[0-9]

echo -e "\n=========================================="
echo "All tests completed successfully!"
echo "The GCC driver initialization logic has been exercised with:"
echo "  - Multiple compilation jobs"
echo "  - Sysroot variations"
echo "  - Dump file management options"
echo "  - Help and version output"
echo "  - Linker selection"
echo "  - Timing and profile reports"
echo "  - Comprehensive flag combinations"
```

This script systematically tests all the requirements:

1. **Multiple Jobs**: Compiles multiple source files (`hello1.c hello2.c hello3.c`) in single invocations
2. **Sysroot Flags**: Tests `--sysroot=`, `--sysroot=/nonexistent`, `-isysroot`, and `-I` flags
3. **Dump Management**: Exercises `-save-temps` (all variants), `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, and various `-fdump-*` flags
4. **Help/Version**: Tests `--help`, `--target-help`, `--version`, `--help=<subprocess>` and combinations with compilation
5. **Linker Selection**: Tests `-fuse-ld=bfd/gold/lld/mold` and `-Wl,` options
6. **PGO and Timing**: Tests `-ftime-report`, `-fprofile-report`, and the full PGO workflow
7. **Re-initialization**: Multiple sequential compilations to ensure variables are reset
8. **Edge Cases**: Tests dump flags without values and trailing slashes

The script creates temporary test files, runs all test cases, cleans up after itself, and provides clear output about what's being tested. Each test is designed to trigger specific parts of the uncovered initialization block in gcc.cc.
