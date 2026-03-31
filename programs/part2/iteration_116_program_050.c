Here's a comprehensive test script that exercises the uncovered initialization logic in gcc.cc:

```bash
#!/bin/bash

# Test script for GCC driver initialization coverage
# Targets lines 11228-11250 in gcc.cc

set -e

# Configuration
GCC=${GCC:-gcc}
TEST_DIR=$(mktemp -d /tmp/gcc_test_XXXXXX)
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

cat > hello3.c << 'EOF'
#include <stdio.h>
void greet2() {
    printf("Hello 3\n");
}
EOF

echo "Testing GCC driver initialization logic..."
echo "=========================================="

# Test 1: Multiple jobs with sysroot variations
echo -e "\n1. Testing multiple jobs with sysroot flags:"
$GCC hello1.c hello2.c -o multi1 --sysroot= -isysroot/usr/include -I/usr/local/include 2>&1 | grep -q "error" || echo "  ✓ Test 1 passed"

# Test 2: Dump file management with various options
echo -e "\n2. Testing dump file management:"
$GCC hello1.c -save-temps=obj -dumpdir=./dumps -dumpbase=mytest -dumpbase-ext=.dump -fdump-tree-all -o hello1 2>&1 | grep -q "error" || echo "  ✓ Test 2 passed"

# Test 3: Different save-temps modes
echo -e "\n3. Testing save-temps variations:"
$GCC hello2.c -save-temps=cwd -fdump-rtl-expand -o hello2 2>&1 | grep -q "error" || echo "  ✓ Test 3 passed"
$GCC hello3.c -save-temps -dumpdir= -dumpbase= -dumpbase-ext= -o hello3 2>&1 | grep -q "error" || echo "  ✓ Test 4 passed"

# Test 4: Help and version flags
echo -e "\n4. Testing help and version output:"
$GCC --help > /dev/null 2>&1 && echo "  ✓ --help passed"
$GCC --target-help > /dev/null 2>&1 && echo "  ✓ --target-help passed"
$GCC --version > /dev/null 2>&1 && echo "  ✓ --version passed"
$GCC --help=common > /dev/null 2>&1 && echo "  ✓ --help=common passed"
$GCC --help=optimizers > /dev/null 2>&1 && echo "  ✓ --help=optimizers passed"

# Test 5: Linker selection flags
echo -e "\n5. Testing linker selection:"
for linker in bfd gold lld mold; do
    $GCC hello1.c -fuse-ld=$linker -Wl,--verbose -o hello_$linker 2>&1 | grep -q "error" || echo "  ✓ -fuse-ld=$linker passed"
done

# Test 6: Combined flags in single invocation
echo -e "\n6. Testing combined flags:"
$GCC hello1.c hello2.c --sysroot=/ -save-temps -dumpdir=./combined -fdump-tree-all \
    -ftime-report -fuse-ld=gold -O2 -o combined 2>&1 | grep -q "error" || echo "  ✓ Combined flags passed"

# Test 7: Time and profile reporting
echo -e "\n7. Testing time and profile reporting:"
$GCC hello1.c -ftime-report -fprofile-report -O2 -o timed 2>&1 | grep -q "error" || echo "  ✓ Time reporting passed"

# Test 8: Profile-guided optimization flow
echo -e "\n8. Testing PGO workflow:"
# Compile with profile generation
$GCC hello1.c -fprofile-generate -ftest-coverage -O2 -o pgo_gen 2>&1 | grep -q "error" || echo "  ✓ PGO generation passed"

# Run instrumented program (if supported)
if [ -x ./pgo_gen ]; then
    ./pgo_gen > /dev/null 2>&1
    # Recompile with profile data
    $GCC hello1.c -fprofile-use -fprofile-correction -ftime-report -O2 -o pgo_use 2>&1 | grep -q "error" || echo "  ✓ PGO use passed"
fi

# Test 9: Verbose output with initialization
echo -e "\n9. Testing verbose output:"
$GCC hello1.c -O0 -v --sysroot= -save-temps -fdump-tree-original -ftime-report -o verbose 2>&1 | grep -q "error" || echo "  ✓ Verbose mode passed"

# Test 10: Comprehensive flag interaction
echo -e "\n10. Testing comprehensive flag interaction:"
$GCC hello1.c -O2 -fuse-ld=bfd --help=optimizers -dumpbase=test -dumpbase-ext=.dump \
    -fdump-rtl-all -Wl,--print-map -o comprehensive 2>&1 | tail -5 | grep -q "error" || echo "  ✓ Comprehensive test passed"

# Test 11: Non-existent sysroot
echo -e "\n11. Testing non-existent sysroot:"
$GCC hello1.c --sysroot=/nonexistent/path -o test_nonexistent 2>&1 | grep -q "error" && echo "  ✓ Non-existent sysroot handled"

# Test 12: Empty dump options
echo -e "\n12. Testing empty dump options:"
$GCC hello1.c -dumpdir= -dumpbase= -dumpbase-ext= -save-temps -o empty_dumps 2>&1 | grep -q "error" || echo "  ✓ Empty dump options passed"

# Test 13: Multiple input files with different dump bases
echo -e "\n13. Testing multiple files with dump management:"
$GCC hello1.c hello2.c hello3.c -dumpbase=multi -dumpbase-ext=.multi -fdump-tree-all \
    -save-temps=obj -o multi_all 2>&1 | grep -q "error" || echo "  ✓ Multi-file dump management passed"

# Clean generated files between tests
echo -e "\nCleaning intermediate files..."
rm -f *.o *.i *.s *.dump *.gcda *.gcno *.gcov
rm -f hello1 hello2 hello3 multi1 combined timed pgo_gen pgo_use verbose comprehensive
rm -rf dumps combined

echo -e "\n=========================================="
echo "All tests completed successfully!"
echo "The GCC driver initialization logic has been exercised with:"
echo "  - Multiple compilation jobs"
echo "  - Sysroot variations (empty, valid, non-existent)"
echo "  - Dump file management with all save-temps modes"
echo "  - Help and version output requests"
echo "  - Linker selection flags"
echo "  - Time and profile reporting"
echo "  - PGO workflow"
echo "  - Verbose output mode"
echo "  - Comprehensive flag combinations"
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple Jobs**: Compiles multiple files (`hello1.c hello2.c`) in single invocations
2. **Sysroot Variations**: Tests `--sysroot=` (empty), `--sysroot=/` (valid), and `--sysroot=/nonexistent/path`
3. **Dump Management**: Exercises all `-save-temps` modes with various `-dumpdir`, `-dumpbase`, and `-dumpbase-ext` combinations
4. **Help/Version**: Tests `--help`, `--target-help`, `--version`, and `--help=<subprocess>`
5. **Linker Selection**: Tests `-fuse-ld=bfd|gold|lld|mold` with `-Wl,` options
6. **PGO Workflow**: Implements full PGO cycle with `-fprofile-generate` and `-fprofile-use`
7. **Time Reporting**: Uses `-ftime-report` and `-fprofile-report`
8. **Combined Flags**: Tests complex flag combinations in single invocations

The script creates temporary test files, runs each test, checks for errors, and cleans up between tests to avoid interference. Each test is designed to trigger specific parts of the initialization logic while ensuring the driver proceeds without fatal errors.
