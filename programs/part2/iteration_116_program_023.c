Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc by testing various flag combinations and execution scenarios:

```bash
#!/bin/bash
set -e

# Configuration
GCC=${1:-gcc}
TEST_DIR=$(mktemp -d)
cd "$TEST_DIR"

echo "Testing GCC driver initialization logic in directory: $TEST_DIR"
echo "Using GCC: $($GCC --version | head -1)"

# Create test source files
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
    printf("Hello from helper\n");
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
cleanup_dumps() {
    rm -f *.i *.s *.o *.dump *.gcda *.gcno *.gcov *.times *.rpo *.opt *.cfg
    rm -rf dumps/ ctemps/ otemps/
}

# Test 1: Multiple input files with sysroot variations
echo -e "\n=== Test 1: Multiple files with sysroot ==="
cleanup_dumps
$GCC hello1.c hello2.c -o prog1 \
    --sysroot= \
    -isysroot /usr \
    -I/usr/include \
    -v 2>&1 | grep -q "COLLECT_GCC_OPTIONS" && echo "Test 1 passed" || echo "Test 1 failed"

# Test 2: Dump file management with various options
echo -e "\n=== Test 2: Dump file management ==="
cleanup_dumps
$GCC hello1.c -o prog2 \
    -save-temps=obj \
    -dumpdir=./dumps \
    -dumpbase="testdump" \
    -dumpbase-ext=".myext" \
    -fdump-tree-all \
    -fdump-rtl-expand \
    -fdump-ipa-all 2>&1 | tail -5

# Check if dump files were created
if [ -f testdump.myext ] || [ -f hello1.i ] || [ -d dumps ]; then
    echo "Dump files created successfully"
fi

# Test 3: Empty dump options
echo -e "\n=== Test 3: Empty dump options ==="
cleanup_dumps
$GCC hello1.c -o prog3 \
    -dumpdir= \
    -dumpbase= \
    -dumpbase-ext= \
    -save-temps=cwd \
    -fdump-tree-original 2>&1 | grep -q "COLLECT_GCC_OPTIONS" && echo "Test 3 passed" || echo "Test 3 failed"

# Test 4: Help and version flags
echo -e "\n=== Test 4: Help and version output ==="
$GCC --help > /dev/null && echo "--help passed"
$GCC --version > /dev/null && echo "--version passed"
$GCC --target-help 2>&1 | head -5 && echo "--target-help passed"
$GCC --help=common 2>&1 | head -5 && echo "--help=common passed"
$GCC --help=optimizers 2>&1 | head -5 && echo "--help=optimizers passed"

# Test 5: Combined help with compilation flags
echo -e "\n=== Test 5: Combined help and compilation ==="
$GCC --help=warnings hello1.c -o prog5 2>&1 | head -10

# Test 6: Linker selection flags
echo -e "\n=== Test 6: Linker selection ==="
for linker in bfd gold lld mold; do
    if $GCC -fuse-ld=$linker hello1.c -o prog6_$linker 2>&1 | grep -q "collect2"; then
        echo "Linker $linker test passed"
    else
        echo "Linker $linker test failed (might not be available)"
    fi
done

# Test with -Wl options
$GCC -fuse-ld=bfd -Wl,--verbose hello1.c -o prog6_wl 2>&1 | grep -q "COLLECT_GCC_OPTIONS" && echo "-Wl test passed"

# Test 7: Timing and profile reports
echo -e "\n=== Test 7: Timing and profile reports ==="
cleanup_dumps
$GCC hello1.c -o prog7 \
    -ftime-report \
    -fprofile-report \
    -O2 2>&1 | grep -E "(Time variable|profile report)" | head -5

# Test 8: Profile-Guided Optimization (PGO) workflow
echo -e "\n=== Test 8: PGO workflow ==="
cleanup_dumps

# Step 1: Generate profile
echo "Step 1: Generating profile..."
$GCC pgo_test.c -o pgo_instrumented \
    -fprofile-generate \
    -ftest-coverage \
    -O2

# Run instrumented program to generate profile data
./pgo_instrumented > /dev/null

# Step 2: Use profile
echo "Step 2: Using profile..."
$GCC pgo_test.c -o pgo_optimized \
    -fprofile-use \
    -fprofile-correction \
    -ftime-report \
    -O2 2>&1 | grep -E "(profile use|Time variable)" | head -5

# Test 9: Comprehensive flag combination (single invocation)
echo -e "\n=== Test 9: Comprehensive flag combination ==="
cleanup_dumps
$GCC hello1.c hello2.c -o prog9 \
    --sysroot=/ \
    -save-temps \
    -dumpdir=./comprehensive_dumps \
    -dumpbase="comp" \
    -dumpbase-ext=".cdump" \
    -fdump-tree-all \
    -fdump-rtl-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    -v 2>&1 | tail -20

# Test 10: Reset state between jobs (simulated)
echo -e "\n=== Test 10: Multiple invocations to test reset ==="
cleanup_dumps

# First invocation with specific flags
$GCC --sysroot=/tmp -save-temps=obj hello1.c -o prog10a

# Second invocation with different flags (should reset state)
$GCC --sysroot= -save-temps=cwd hello2.c -o prog10b

# Third invocation with help (should also reset)
$GCC --help=target hello1.c -o prog10c 2>&1 | head -5

echo -e "\n=== Test 11: Edge cases ==="
# Test with non-existent sysroot
$GCC --sysroot=/nonexistent/path hello1.c -o prog11a 2>&1 | grep -i "warning\|error" | head -3 || true

# Test with empty string flags
$GCC -dumpdir="" -dumpbase="" hello1.c -o prog11b 2>&1 | grep -q "COLLECT_GCC_OPTIONS" && echo "Empty flag test passed"

# Test 12: Verbose only flag simulation
echo -e "\n=== Test 12: Verbose output ==="
$GCC hello1.c -o prog12 -v 2>&1 | grep "COLLECT_GCC_OPTIONS" | head -1

# Cleanup
echo -e "\n=== Cleaning up ==="
cleanup_dumps
rm -f prog* pgo_* hello*.i hello*.s hello*.o *.gcda *.gcno
cd ..
rm -rf "$TEST_DIR"

echo -e "\nAll tests completed. The GCC driver initialization logic has been exercised with:"
echo "1. Multiple input files"
echo "2. Various sysroot configurations"
echo "3. Dump file management options"
echo "4. Help and version requests"
echo "5. Linker selection flags"
echo "6. Timing and profile reports"
echo "7. PGO workflow"
echo "8. Comprehensive flag combinations"
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple Jobs**: Compiles multiple source files (`hello1.c`, `hello2.c`) in single invocations
2. **Sysroot Variations**: Tests `--sysroot=` (empty), `--sysroot=/`, `--sysroot=/nonexistent`, `-isysroot`
3. **Dump Management**: Exercises all dump-related variables with various combinations of `-save-temps`, `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, and `-fdump-*` flags
4. **Help/Version**: Tests `--help`, `--version`, `--target-help`, `--help=common`, `--help=optimizers`
5. **Linker Selection**: Tests `-fuse-ld=bfd/gold/lld/mold` and `-Wl,` options
6. **PGO & Timing**: Implements full PGO workflow with `-fprofile-generate`, `-fprofile-use`, `-ftime-report`, `-fprofile-report`
7. **State Reset**: Multiple invocations with different flags to ensure variables are reset between jobs

The script creates temporary files, runs tests, and cleans up after itself. Each test is designed to trigger specific parts of the initialization block while ensuring valid compilation where possible.
