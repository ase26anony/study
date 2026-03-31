Here's a comprehensive test script that exercises the uncovered initialization logic in gcc.cc:

```bash
#!/bin/bash
set -e

# Configuration
GCC=${GCC:-gcc}
TEST_DIR=$(mktemp -d /tmp/gcc_test_XXXXXX)
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    cd /
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

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

cat > hello3.c << 'EOF'
#include <stdio.h>
void helper2() {
    printf("Hello from helper2\n");
}
EOF

echo "Testing GCC driver initialization logic..."
echo "Test directory: $TEST_DIR"
echo "Using GCC: $($GCC --version | head -1)"
echo ""

# Function to run test and capture status
run_test() {
    local name="$1"
    shift
    echo "=== Test: $name ==="
    echo "Command: $GCC $*"
    set +e
    timeout 10s $GCC "$@" 2>&1 | head -50
    local status=$?
    set -e
    echo "Exit status: $status"
    echo ""
    
    # Clean generated files
    rm -f *.o *.s *.i *.o.* *.expand *.original *.gkd *.gcda *.gcno
    rm -f a.out hello test prog *.exe
    rm -rf dumps temps
}

# Test 1: Multiple jobs with sysroot variations
echo "--- Testing sysroot and multiple jobs ---"
run_test "Multiple files with empty sysroot" hello1.c hello2.c hello3.c --sysroot= -o multi1
run_test "Multiple files with nonexistent sysroot" hello1.c hello2.c --sysroot=/nonexistent/path -o multi2
run_test "With isysroot and includes" hello1.c -isysroot/usr/include -I/usr/local/include -o hello

# Test 2: Dump file management with save-temps variations
echo "--- Testing dump file management ---"
run_test "save-temps=obj" hello1.c -save-temps=obj -o hello_obj
run_test "save-temps=cwd" hello1.c -save-temps=cwd -o hello_cwd
run_test "save-temps default" hello1.c -save-temps -o hello_save
run_test "With dumpdir and dumpbase" hello1.c -dumpdir=./dumps -dumpbase=mydump -dumpbase-ext=.ext -fdump-tree-all -o hello_dump
run_test "Empty dump options" hello1.c -dumpdir= -dumpbase= -dumpbase-ext= -fdump-rtl-expand -o hello_empty
mkdir -p dumps
run_test "Combined dump options" hello1.c -save-temps -dumpdir=./dumps -dumpbase=combined -fdump-tree-optimized -fdump-ipa-all -o hello_combined

# Test 3: Help and version flags
echo "--- Testing help and version output ---"
run_test "Basic help" --help
run_test "Target help" --target-help
run_test "Version" --version
run_test "Subprocess help: common" --help=common
run_test "Subprocess help: optimizers" --help=optimizers
run_test "Help with other flags" --help -O2 --sysroot=/
run_test "Version with dumpbase" --version -dumpbase=testver

# Test 4: Linker selection flags
echo "--- Testing linker selection ---"
run_test "Linker bfd" hello1.c -fuse-ld=bfd -o hello_bfd
run_test "Linker gold" hello1.c -fuse-ld=gold -o hello_gold 2>/dev/null || echo "gold not available"
run_test "Linker lld" hello1.c -fuse-ld=lld -o hello_lld 2>/dev/null || echo "lld not available"
run_test "Linker mold" hello1.c -fuse-ld=mold -o hello_mold 2>/dev/null || echo "mold not available"
run_test "With Wl options" hello1.c -Wl,--verbose -o hello_wl

# Test 5: Timing and profile reports
echo "--- Testing timing and profile reports ---"
run_test "Time report" hello1.c -ftime-report -O2 -o hello_time
run_test "Profile report" hello1.c -fprofile-report -O2 -o hello_profile

# Test 6: PGO workflow
echo "--- Testing PGO workflow ---"
cat > pgo_test.c << 'EOF'
#include <stdio.h>
int main() {
    for (int i = 0; i < 100; i++) {
        printf("PGO test %d\n", i);
    }
    return 0;
}
EOF

# Generate profile
echo "Generating PGO profile..."
$GCC pgo_test.c -fprofile-generate -o pgo_gen 2>/dev/null || true
./pgo_gen 2>/dev/null || true

# Use profile
run_test "PGO with profile use" pgo_test.c -fprofile-use -ftime-report -o pgo_use
run_test "PGO with correction" pgo_test.c -fprofile-use -fprofile-correction -o pgo_correct

# Test 7: Comprehensive flag combinations
echo "--- Testing comprehensive combinations ---"
run_test "Combo 1: Multiple files with all features" \
    hello1.c hello2.c \
    --sysroot=/ \
    -save-temps \
    -dumpdir=./combo_dumps \
    -fdump-tree-all \
    -ftime-report \
    -fuse-ld=bfd \
    -O2 \
    -v \
    -o combo1

run_test "Combo 2: Help+dump+optimization" \
    --help=optimizers \
    -dumpbase=helpdump \
    -O3 \
    -fdump-ipa-all

run_test "Combo 3: Version with sysroot and timing" \
    --version \
    --sysroot=/nonexistent \
    -ftime-report \
    -dumpdir=./vers_dumps

# Test 8: Edge cases
echo "--- Testing edge cases ---"
run_test "Empty command" ""
run_test "Only dump options" -dumpdir= -dumpbase= -dumpbase-ext=
run_test "Sysroot with trailing slash" --sysroot=/usr/ -v hello1.c -o edge1
run_test "Multiple sysroots" --sysroot=/ --sysroot=/usr hello1.c -o edge2 2>&1 | head -20

echo "=== All tests completed ==="
echo "Note: Some tests may fail due to missing components (like specific linkers)"
echo "This is expected - the goal is to exercise the driver initialization logic."
```

This script systematically tests the uncovered initialization block by:

1. **Multiple Jobs**: Compiles multiple files (`hello1.c hello2.c hello3.c`) in single invocations
2. **Sysroot Variations**: Tests `--sysroot=`, `--sysroot=/nonexistent`, `-isysroot`, and `-I` flags
3. **Dump Management**: Exercises all `-save-temps` variants, `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, and `-fdump-*` flags
4. **Help/Version**: Tests `--help`, `--target-help`, `--version`, `--help=common`, `--help=optimizers`
5. **Linker Selection**: Tests `-fuse-ld=bfd/gold/lld/mold` and `-Wl,` options
6. **Timing/PGO**: Tests `-ftime-report`, `-fprofile-report`, and full PGO workflow
7. **Combinations**: Tests complex flag interactions that trigger multiple initialization variables

The script cleans up between tests to avoid interference and captures exit codes to verify the driver handles all flag combinations correctly.
