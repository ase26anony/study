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
extern void helper();
int main() {
    printf("Hello World 3\n");
    helper();
    return 0;
}
EOF

echo "Testing GCC driver initialization logic..."
echo "=========================================="

# Function to run test and capture exit code
run_test() {
    local desc="$1"
    shift
    echo -e "\nTest: $desc"
    echo "Command: $GCC $*"
    "$GCC" "$@" 2>&1 | head -50
    local status=$?
    echo "Exit code: $status"
    return $status
}

# Clean state between tests
clean_files() {
    rm -f *.o *.s *.i *.ii *.bc *.expand *.original *.gimple *.cfg *.dot
    rm -f *.gcda *.gcno *.gcov *.d *.d.* *.time *.dump *.out
    rm -rf dumps/ temps/ profile_data/
    rm -f hello hello_pgo test *.so *.a
}

# Test 1: Multiple jobs with sysroot variations
echo -e "\n=== Test 1: Multiple jobs with sysroot ==="
clean_files
run_test "Multiple files with empty sysroot" hello1.c hello2.c -o multi1 --sysroot=
run_test "Multiple files with non-existent sysroot" hello1.c hello2.c -o multi2 --sysroot=/nonexistent/path/$(date +%s)
run_test "With valid sysroot and includes" hello1.c -o hello_sysroot --sysroot=/ -isysroot/usr/include -I/usr/local/include

# Test 2: Dump file management with various options
echo -e "\n=== Test 2: Dump file management ==="
clean_files
run_test "save-temps=obj" hello1.c -save-temps=obj -o hello_obj
run_test "save-temps=cwd" hello1.c -save-temps=cwd -o hello_cwd
run_test "save-temps with dumpdir" hello1.c -save-temps -dumpdir=./dumps -o hello_dump
run_test "dumpbase and dumpbase-ext" hello1.c -dumpbase=mydump -dumpbase-ext=.myext -fdump-tree-all -o hello_base
run_test "Combined dump options" hello1.c -save-temps=obj -dumpdir=./dumps -dumpbase=test -dumpbase-ext=.dump -fdump-rtl-expand -fdump-tree-optimized -o hello_comb

# Test 3: Help and version output
echo -e "\n=== Test 3: Help and version ==="
clean_files
run_test "Basic help" --help
run_test "Target help" --target-help
run_test "Version" --version
run_test "Subprocess help: common" --help=common
run_test "Subprocess help: optimizers" --help=optimizers
run_test "Help with other flags" --help -O2 --sysroot=/
run_test "Version with dump flags" --version -dumpbase=test -fdump-tree-all

# Test 4: Linker selection flags
echo -e "\n=== Test 4: Linker selection ==="
clean_files
run_test "Default linker" hello1.c -o hello_default
run_test "BFD linker" hello1.c -fuse-ld=bfd -o hello_bfd
run_test "Gold linker" hello1.c -fuse-ld=gold -o hello_gold 2>/dev/null || echo "Gold linker not available"
run_test "LLD linker" hello1.c -fuse-ld=lld -o hello_lld 2>/dev/null || echo "LLD linker not available"
run_test "Mold linker" hello1.c -fuse-ld=mold -o hello_mold 2>/dev/null || echo "Mold linker not available"
run_test "Linker with options" hello1.c -fuse-ld=bfd -Wl,--verbose -o hello_wl

# Test 5: Timing and profile reports
echo -e "\n=== Test 5: Timing and profile reports ==="
clean_files
run_test "Time report" hello1.c -ftime-report -o hello_time
run_test "Profile report" hello1.c -fprofile-report -o hello_profile

# Test 6: PGO workflow
echo -e "\n=== Test 6: Profile-Guided Optimization ==="
clean_files
echo "Creating PGO test program..."
cat > pgo_test.c << 'EOF'
#include <stdio.h>
int main(int argc, char **argv) {
    int i, sum = 0;
    for (i = 0; i < argc * 1000; i++) {
        sum += i;
    }
    printf("Sum: %d\n", sum);
    return 0;
}
EOF

echo "Step 1: Compile with profile generation"
run_test "PGO generate" pgo_test.c -fprofile-generate -o pgo_instrumented
echo "Step 2: Run instrumented program"
./pgo_instrumented 1 2 3 2>/dev/null || true
echo "Step 3: Compile with profile use and timing"
run_test "PGO use with reports" pgo_test.c -fprofile-use -ftime-report -fprofile-report -o pgo_optimized

# Test 7: Comprehensive flag combination
echo -e "\n=== Test 7: Comprehensive flag combination ==="
clean_files
run_test "All flags combined" \
    hello1.c hello2.c hello3.c \
    -O2 \
    --sysroot=/ \
    -save-temps=obj \
    -dumpdir=./dumps \
    -dumpbase=comprehensive \
    -dumpbase-ext=.test \
    -fdump-tree-all \
    -fdump-rtl-all \
    -ftime-report \
    -fuse-ld=bfd \
    -Wl,--print-map \
    -o comprehensive_test

# Test 8: Verbose output to see driver state
echo -e "\n=== Test 8: Verbose compilation ==="
clean_files
run_test "Verbose with initialization flags" \
    hello1.c \
    -v \
    -O0 \
    --sysroot= \
    -save-temps \
    -fdump-tree-original \
    -ftime-report \
    -o hello_verbose

# Test 9: Edge cases with empty values
echo -e "\n=== Test 9: Edge cases ==="
clean_files
run_test "Empty dumpdir" hello1.c -dumpdir= -dumpbase= -o hello_empty
run_test "Multiple sysroot changes" hello1.c --sysroot= --sysroot=/usr -o hello_multi_sysroot

# Test 10: Multiple help flags in one invocation
echo -e "\n=== Test 10: Multiple informational flags ==="
clean_files
run_test "Help and version together" --help --version --target-help 2>&1 | head -20

echo -e "\n=========================================="
echo "All tests completed. Test directory: $TEST_DIR"
echo "To examine generated files, delay cleanup with:"
echo "  trap - EXIT"
echo "  cd $TEST_DIR"
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple Jobs**: Compiles multiple files (`hello1.c hello2.c hello3.c`) in single invocations
2. **Sysroot Variations**: Tests `--sysroot=`, `--sysroot=/nonexistent`, `--sysroot=/`, `-isysroot`, and `-I`
3. **Dump Management**: Exercises all `-save-temps` variants, `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, and `-fdump-*` flags
4. **Help/Version**: Tests `--help`, `--target-help`, `--version`, `--help=common`, `--help=optimizers`
5. **Linker Selection**: Tests `-fuse-ld=bfd`, `-fuse-ld=gold`, `-fuse-ld=lld`, `-fuse-ld=mold`, and `-Wl,` options
6. **PGO Workflow**: Implements full PGO cycle with `-fprofile-generate`, execution, and `-fprofile-use` with `-ftime-report`
7. **Comprehensive Test**: Combines multiple flags in one invocation to test interactions
8. **Verbose Output**: Uses `-v` to observe driver state changes
9. **Edge Cases**: Tests empty values and multiple flag occurrences
10. **Flag Combinations**: Tests how help/version flags interact with compilation flags

The script cleans up between test sections to ensure independent execution and captures exit codes to verify successful driver initialization.
