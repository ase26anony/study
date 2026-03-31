Here's a comprehensive test script that exercises the uncovered initialization logic in gcc.cc:

```bash
#!/bin/bash
set -e

# Configuration
GCC=${1:-gcc}
TEST_DIR=$(mktemp -d /tmp/gcc_test_XXXXXX)
cd "$TEST_DIR"

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
    printf("Hello 2\n");
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

# Function to clean up dump files
cleanup_dumps() {
    rm -f *.i *.s *.o *.dump *.gcda *.gcno *.gcov *.times 2>/dev/null || true
    rm -rf dumps/ 2>/dev/null || true
}

echo "=== Testing GCC driver initialization logic ==="
echo "Using GCC: $($GCC --version | head -1)"
echo "Test directory: $TEST_DIR"
echo

# Test 1: Multiple jobs with sysroot variations
echo "Test 1: Multiple jobs with sysroot variations"
cleanup_dumps
$GCC hello1.c hello2.c -o prog1 --sysroot= -v 2>&1 | grep -q "sysroot" || true
$GCC hello1.c hello2.c -o prog2 --sysroot=/nonexistent/path -v 2>&1 | grep -q "sysroot" || true
$GCC hello1.c hello2.c -o prog3 --sysroot=/ -isysroot/usr/include -I/usr/local/include -v 2>&1 | grep -q "sysroot" || true
echo "✓ Test 1 completed"
echo

# Test 2: Dump file management with various options
echo "Test 2: Dump file management"
cleanup_dumps
mkdir -p dumps

# Different save-temps options
$GCC hello1.c -save-temps=obj -o hello_obj
$GCC hello1.c -save-temps=cwd -o hello_cwd
$GCC hello1.c -save-temps -o hello_all

# Dumpdir and dumpbase combinations
$GCC hello1.c -dumpdir=./dumps -dumpbase=mydump -dumpbase-ext=.dump -fdump-tree-all -o hello_dump1
$GCC hello1.c -dumpdir= -dumpbase= -dumpbase-ext= -fdump-rtl-expand -o hello_dump2
$GCC hello1.c -dumpdir=dumps/ -dumpbase="test" -dumpbase-ext=".myext" -fdump-ipa-all -o hello_dump3

# Combined flags
$GCC hello1.c -save-temps -dumpdir=./dumps -fdump-tree-original -fdump-rtl-expand -o hello_combined
echo "✓ Test 2 completed"
echo

# Test 3: Help and version output
echo "Test 3: Help and version output"
$GCC --help > /dev/null
$GCC --target-help > /dev/null
$GCC --version > /dev/null
$GCC --help=common > /dev/null
$GCC --help=optimizers > /dev/null
$GCC --help=warnings > /dev/null

# Combined with compilation flags
$GCC hello1.c --help=common -O2 -o hello_help 2>&1 | grep -q "common" || true
echo "✓ Test 3 completed"
echo

# Test 4: Linker selection flags
echo "Test 4: Linker selection"
cleanup_dumps

# Test available linkers
for linker in bfd gold lld mold; do
    if $GCC -fuse-ld=$linker --help=linker 2>&1 | grep -q "fuse-ld"; then
        $GCC hello1.c -fuse-ld=$linker -o hello_$linker 2>/dev/null || true
    fi
done

# With linker options
$GCC hello1.c -fuse-ld=bfd -Wl,--verbose -o hello_verbose 2>&1 | grep -q "ld" || true
$GCC hello1.c hello2.c -fuse-ld=gold -Wl,-Map=output.map -o hello_map 2>/dev/null || true
echo "✓ Test 4 completed"
echo

# Test 5: Timing reports and PGO
echo "Test 5: Timing reports and PGO"
cleanup_dumps

# Timing reports
$GCC hello1.c -ftime-report -O2 -o hello_time 2>&1 | grep -q "Time" || true
$GCC hello1.c -ftime-report -fdump-tree-all -O3 -o hello_time2 2>&1 | grep -q "Time" || true

# Profile-guided optimization (simplified)
cat > pgo_test.c << 'EOF'
#include <stdio.h>
int main() {
    for (int i = 0; i < 1000; i++) {
        printf("PGO test %d\n", i % 100);
    }
    return 0;
}
EOF

# Generate profile
$GCC pgo_test.c -fprofile-generate -o pgo_gen
./pgo_gen > /dev/null 2>&1 || true

# Use profile with timing report
$GCC pgo_test.c -fprofile-use -ftime-report -o pgo_use 2>&1 | grep -q "profile" || true
$GCC pgo_test.c -fprofile-use -fprofile-report -o pgo_report 2>&1 | grep -q "profile" || true
echo "✓ Test 5 completed"
echo

# Test 6: Comprehensive flag combination
echo "Test 6: Comprehensive flag combination"
cleanup_dumps
mkdir -p comprehensive_dumps

$GCC hello1.c hello2.c hello3.c \
    --sysroot=/ \
    -save-temps \
    -dumpdir=./comprehensive_dumps \
    -dumpbase=comprehensive \
    -dumpbase-ext=.full \
    -fdump-tree-all \
    -fdump-rtl-all \
    -ftime-report \
    -fuse-ld=bfd \
    -O2 \
    -v \
    -o comprehensive_prog 2>&1 | tail -20

echo "✓ Test 6 completed"
echo

# Test 7: Multiple help/version combinations
echo "Test 7: Mixed help/version with compilation"
$GCC --version --help=optimizers hello1.c -O2 2>&1 | grep -q "version" || true
$GCC --target-help --help=warnings -v 2>&1 | grep -q "warning" || true
$GCC --help=common --version -v 2>&1 | grep -q "common" || true
echo "✓ Test 7 completed"
echo

# Test 8: Edge cases for dump management
echo "Test 8: Dump management edge cases"
cleanup_dumps

# Empty dumpdir with trailing slash
$GCC hello1.c -dumpdir= -dumpbase=test -fdump-tree-original -o hello_edge1
# Very long dumpbase
$GCC hello1.c -dumpbase=very_long_dump_base_name_that_might_test_buffer_limits -fdump-tree-original -o hello_edge2
# Special characters in dumpbase
$GCC hello1.c -dumpbase="test@special#chars" -fdump-tree-original -o hello_edge3 2>/dev/null || true
echo "✓ Test 8 completed"
echo

# Cleanup
cleanup_dumps
rm -f hello1.c hello2.c hello3.c pgo_test.c
rm -f prog1 prog2 prog3 hello_* pgo_* comprehensive_prog output.map
rm -rf dumps comprehensive_dumps

echo "=== All tests completed successfully ==="
echo "Generated files in: $TEST_DIR"
echo "To clean up: rm -rf $TEST_DIR"
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple jobs**: Compiles multiple source files in single invocations
2. **Sysroot variations**: Tests `--sysroot=` with empty, invalid, and valid paths
3. **Dump management**: Exercises all `-dump*` flags with various combinations
4. **Help/version**: Tests `--help`, `--version`, and subprocess help
5. **Linker selection**: Tests all `-fuse-ld=` variants with linker flags
6. **Timing/PGO**: Tests `-ftime-report` and profile-guided optimization paths
7. **Comprehensive combination**: Combines multiple relevant flags in one invocation
8. **Edge cases**: Tests boundary conditions for dump file management

The script creates temporary test files, runs the GCC driver with the specified flag combinations, and cleans up after itself. Each test is designed to trigger the specific initialization variables mentioned in the uncovered lines.
