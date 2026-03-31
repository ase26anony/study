Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc by testing various flag combinations and execution scenarios:

```bash
#!/bin/bash

# GCC driver test script for coverage of initialization logic
# Targets lines 11228-11250 in gcc.cc

set -e

# Configuration
GCC=${GCC:-gcc}
TEST_DIR=$(mktemp -d /tmp/gcc_driver_test_XXXXXX)
trap "rm -rf $TEST_DIR" EXIT

cd $TEST_DIR

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

cat > hello3.c << 'EOF'
#include <stdio.h>
extern void helper();
int main() {
    printf("Hello 3\n");
    helper();
    return 0;
}
EOF

echo "Testing GCC driver initialization logic..."
echo "Using GCC: $($GCC --version | head -1)"
echo "Test directory: $TEST_DIR"
echo ""

# Helper function to run GCC and capture exit code
run_gcc() {
    local desc="$1"
    shift
    echo "=== Test: $desc ==="
    echo "Command: $GCC $@"
    set +e
    $GCC "$@" 2>&1 | head -50  # Limit output
    local exit_code=$?
    set -e
    echo "Exit code: $exit_code"
    echo ""
    return $exit_code
}

# Clean up between tests
cleanup() {
    rm -f *.o *.s *.i *.ii *.o.* *.gcda *.gcno *.gcov *.dmp *.dump
    rm -f hello hello_pgo *.times *.profile *.d
    rm -rf dumps temps
}

# Test 1: Multiple jobs with sysroot variations
echo "--- Test 1: Multiple jobs with sysroot ---"
cleanup
run_gcc "Multiple files with empty sysroot" hello1.c hello2.c -o hello_multi --sysroot=
run_gcc "Multiple files with non-existent sysroot" hello1.c hello2.c -o hello_multi --sysroot=/nonexistent/path/here || true
run_gcc "With isysroot and -I" hello1.c -o hello1 --sysroot=/ -isysroot/usr/include -I/usr/local/include

# Test 2: Dump file generation with various options
echo "--- Test 2: Dump file management ---"
cleanup
mkdir -p dumps
run_gcc "save-temps=obj" hello1.c -save-temps=obj -o hello1
run_gcc "save-temps=cwd" hello1.c -save-temps=cwd -o hello1
run_gcc "save-temps with dumpdir" hello1.c -save-temps -dumpdir=./dumps -o hello1
run_gcc "dumpbase and dumpbase-ext" hello1.c -dumpbase=mydump -dumpbase-ext=.dmp -fdump-tree-all -o hello1
run_gcc "empty dump options" hello1.c -dumpdir= -dumpbase= -dumpbase-ext= -fdump-rtl-expand -o hello1
run_gcc "combined dump flags" hello1.c -save-temps -dumpdir=dumps/ -dumpbase=test -dumpbase-ext=.dump -fdump-tree-original -fdump-rtl-all -o hello1

# Test 3: Help and version output
echo "--- Test 3: Help and version ---"
run_gcc "Version" --version
run_gcc "Help" --help
run_gcc "Target help" --target-help
run_gcc "Help with subprocess" --help=common
run_gcc "Optimizers help" --help=optimizers
run_gcc "Combined help with other flags" --help -O2 --sysroot=/
run_gcc "Version with dump flags" --version -dumpbase=test -save-temps

# Test 4: Linker selection
echo "--- Test 4: Linker selection ---"
cleanup
run_gcc "Default linker" hello1.c hello2.c -o hello -v 2>&1 | grep -i "collect2\|ld" || true
run_gcc "bfd linker" hello1.c -fuse-ld=bfd -o hello1 -Wl,--verbose 2>&1 | head -20
run_gcc "gold linker" hello1.c -fuse-ld=gold -o hello1 2>/dev/null || echo "gold not available"
run_gcc "lld linker" hello1.c -fuse-ld=lld -o hello1 2>/dev/null || echo "lld not available"
run_gcc "mold linker" hello1.c -fuse-ld=mold -o hello1 2>/dev/null || echo "mold not available"
run_gcc "Linker with sysroot" hello1.c -fuse-ld=bfd --sysroot=/ -Wl,-rpath,/usr/lib -o hello1

# Test 5: Timing and profile reports
echo "--- Test 5: Timing and profile reports ---"
cleanup
run_gcc "Time report" hello1.c -ftime-report -O2 -o hello1 2>&1 | grep -i "time" || true
run_gcc "Profile report" hello1.c -fprofile-report -O2 -o hello1 2>&1 | grep -i "profile" || true

# Test 6: PGO workflow (if supported)
echo "--- Test 6: PGO workflow ---"
cleanup
cat > pgo_test.c << 'EOF'
#include <stdio.h>
int main(int argc, char **argv) {
    for (int i = 0; i < argc * 10; i++) {
        printf("Iteration %d\n", i);
    }
    return 0;
}
EOF

# Generate profile data
run_gcc "PGO generate" pgo_test.c -fprofile-generate -o pgo_gen
./pgo_gen some args 2>/dev/null || true

# Use profile data
run_gcc "PGO use with time report" pgo_test.c -fprofile-use -ftime-report -o pgo_use 2>/dev/null || echo "PGO not fully supported"

# Test 7: Combined comprehensive test
echo "--- Test 7: Comprehensive combined test ---"
cleanup
run_gcc "Mega-combination" hello1.c hello2.c hello3.c \
    --sysroot=/ \
    -save-temps=obj \
    -dumpdir=./dumps \
    -dumpbase=combo \
    -dumpbase-ext=.test \
    -fdump-tree-all \
    -fdump-rtl-expand \
    -ftime-report \
    -fuse-ld=bfd \
    -O2 \
    -v \
    -o hello_combo

# Test 8: Verbose flag to observe initialization
echo "--- Test 8: Verbose output for observation ---"
cleanup
run_gcc "Verbose with multiple flags" hello1.c \
    -v \
    --sysroot= \
    -save-temps \
    -dumpdir= \
    -fdump-tree-original \
    -ftime-report \
    -o hello_verbose 2>&1 | grep -i "sysroot\|dump\|collect2" | head -20

# Test 9: Test greatest_status initialization
echo "--- Test 9: Error handling and status ---"
cleanup
run_gcc "Invalid option" -invalid-option 2>/dev/null || true
run_gcc "After error" hello1.c -o hello1  # Should reset status

# Test 10: Multiple invocations to test re-initialization
echo "--- Test 10: Sequential invocations ---"
cleanup
for i in 1 2 3; do
    run_gcc "Invocation $i with different flags" hello1.c \
        --sysroot=/ \
        -save-temps=$([ $i -eq 1 ] && echo "cwd" || ([ $i -eq 2 ] && echo "obj" || echo "")) \
        -dumpbase=run$i \
        -o hello$i
done

echo "=== Summary ==="
echo "All tests completed. Check for:"
echo "1. Proper initialization/reset between jobs"
echo "2. Sysroot variables handled correctly"
echo "3. Dump file variables managed properly"
echo "4. Help/version flags working"
echo "5. Linker selection effective"
echo "6. Timing/profile reports generated"
echo ""
echo "Generated files in $TEST_DIR:"
ls -la $TEST_DIR 2>/dev/null | head -20
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple Jobs**: Compiles multiple `.c` files in single invocations
2. **Sysroot Variations**: Tests `--sysroot=`, `--sysroot=/nonexistent`, `-isysroot`, and `-I`
3. **Dump Management**: Exercises all `-save-temps` variants, `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, and `-fdump-*` flags
4. **Help/Version**: Tests `--help`, `--target-help`, `--help=<subprocess>`, `--version` alone and combined
5. **Linker Selection**: Tests `-fuse-ld=bfd/gold/lld/mold` and `-Wl,` options
6. **Timing/Profiling**: Tests `-ftime-report`, `-fprofile-report`, and full PGO workflow
7. **Combined Tests**: Includes comprehensive flag combinations to test interactions
8. **Verbose Output**: Uses `-v` to observe driver behavior
9. **Error Handling**: Tests `greatest_status` reset after errors
10. **Sequential Invocations**: Tests re-initialization between multiple calls

The script creates temporary test programs, runs GCC with various flag combinations, captures exit codes, and cleans up between tests to avoid interference. Each test targets specific variables in the uncovered initialization block.
