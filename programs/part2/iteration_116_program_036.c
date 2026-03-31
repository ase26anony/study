I'll create a comprehensive test script that exercises the uncovered initialization logic in gcc.cc. This script will test various flag combinations to ensure the driver's initialization block is thoroughly covered.

```bash
#!/bin/bash

# Test script for GCC driver initialization coverage
# Targets the uncovered lines in gcc.cc (lines 11228-11250)

set -e

# Configuration
GCC=${GCC:-gcc}
TEST_DIR=$(mktemp -d /tmp/gcc_driver_test_XXXXXX)
echo "Test directory: $TEST_DIR"
cd "$TEST_DIR"

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
extern void helper();
int main() {
    printf("Hello World 3\n");
    helper();
    return 0;
}
EOF

# Function to clean up dump files between tests
clean_dumps() {
    rm -f *.o *.s *.i *.ii *.o.* *.expand *.original *.optimized *.gimple
    rm -f *.gcda *.gcno *.gcov
    rm -rf dumps/ temps/ *.dSYM/
    rm -f *.times *.report
}

# Function to run GCC and capture exit code
run_gcc() {
    local desc="$1"
    shift
    echo "========================================"
    echo "Test: $desc"
    echo "Command: $GCC $*"
    clean_dumps
    $GCC "$@" 2>&1 | head -50
    local status=$?
    echo "Exit code: $status"
    echo "========================================"
    echo
    return $status
}

# Test 1: Multiple distinct jobs (compiles multiple files)
echo "=== TEST 1: Multiple distinct jobs ==="
run_gcc "Multiple files compilation" hello1.c hello2.c hello3.c -o multi_hello

# Test 2: Sysroot variations
echo "=== TEST 2: Sysroot variations ==="
run_gcc "Empty sysroot" --sysroot= hello1.c -o hello_empty_sysroot
run_gcc "Non-existent sysroot" --sysroot=/nonexistent/path/here hello1.c -o hello_bad_sysroot
run_gcc "Valid sysroot with isysroot" --sysroot=/ -isysroot/usr/include hello1.c -o hello_sysroot
run_gcc "Sysroot with includes" --sysroot=/ -I/usr/include -I/usr/local/include hello1.c -o hello_includes

# Test 3: Dump file generation with varied options
echo "=== TEST 3: Dump file generation ==="
mkdir -p dumps temps

# Different save-temps options
run_gcc "save-temps=obj" -save-temps=obj hello1.c -o hello_save_obj
run_gcc "save-temps=cwd" -save-temps=cwd hello1.c -o hello_save_cwd
run_gcc "save-temps (default)" -save-temps hello1.c -o hello_save_default

# Dumpdir and dumpbase combinations
run_gcc "dumpdir with dumpbase" -dumpdir=./dumps -dumpbase=testdump -fdump-tree-all hello1.c -o hello_dump1
run_gcc "dumpdir with empty dumpbase" -dumpdir=./dumps -dumpbase= -fdump-rtl-expand hello1.c -o hello_dump2
run_gcc "dumpbase-ext with dumpdir" -dumpdir=./dumps -dumpbase=test -dumpbase-ext=.mydump -fdump-tree-original hello1.c -o hello_dump3
run_gcc "empty dumpdir" -dumpdir= -dumpbase=empty -fdump-tree-optimized hello1.c -o hello_dump4

# Multiple dump flags
run_gcc "Multiple fdump flags" -fdump-tree-all -fdump-rtl-all -fdump-ipa-all hello1.c -o hello_multi_dump

# Test 4: Help and version output
echo "=== TEST 4: Help and version output ==="
run_gcc "Basic help" --help
run_gcc "Target help" --target-help
run_gcc "Version" --version
run_gcc "Help for optimizers" --help=optimizers
run_gcc "Help for common options" --help=common
run_gcc "Help for target" --help=target
run_gcc "Help for warnings" --help=warnings

# Combined with compilation flags
run_gcc "Help with other flags" --help -O2 --sysroot=/
run_gcc "Version with dump flags" --version -dumpdir=./dumps -fdump-tree-all

# Test 5: Linker selection flags
echo "=== TEST 5: Linker selection ==="
run_gcc "Default linker" hello1.c -o hello_ld_default
run_gcc "bfd linker" -fuse-ld=bfd hello1.c -o hello_ld_bfd
run_gcc "gold linker" -fuse-ld=gold hello1.c -o hello_ld_gold
run_gcc "lld linker" -fuse-ld=lld hello1.c -o hello_ld_lld
# Try mold if available
run_gcc "mold linker" -fuse-ld=mold hello1.c -o hello_ld_mold 2>/dev/null || true

# Linker flags with -Wl
run_gcc "Linker with -Wl flags" -Wl,--verbose -Wl,--print-map hello1.c -o hello_wl

# Test 6: Profile-guided optimization and timing reports
echo "=== TEST 6: PGO and timing reports ==="

# Create PGO test program
cat > pgo_test.c << 'EOF'
#include <stdio.h>
int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}
int main() {
    int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += factorial(i % 10);
    }
    printf("Result: %d\n", sum);
    return 0;
}
EOF

# Step 1: Generate profile
echo "=== PGO Step 1: Generate profile ==="
run_gcc "PGO generate" -O2 -fprofile-generate pgo_test.c -o pgo_gen
# Run the instrumented program to generate profile data
./pgo_gen > /dev/null 2>&1 || true

# Step 2: Use profile with timing report
echo "=== PGO Step 2: Use profile ==="
run_gcc "PGO use with timing" -O2 -fprofile-use -ftime-report pgo_test.c -o pgo_use

# Test timing report alone
run_gcc "Time report only" -ftime-report hello1.c -o hello_time

# Test profile report
run_gcc "Profile report" -fprofile-report hello1.c -o hello_profile 2>/dev/null || true

# Test 7: Comprehensive flag combinations (covers multiple variables at once)
echo "=== TEST 7: Comprehensive combinations ==="

# Combination 1: Basic driver initialization coverage
run_gcc "Combo 1: Basic init coverage" -O0 -v --sysroot= -save-temps -fdump-tree-original -ftime-report hello1.c -o hello_combo1

# Combination 2: Flag interaction
run_gcc "Combo 2: Flag interaction" -O2 -fuse-ld=bfd --help=optimizers -dumpbase=test -dumpbase-ext=.dump -fdump-rtl-all hello1.c -o hello_combo2 2>&1 | grep -A20 "optimizers" || true

# Combination 3: Multiple jobs with complex flags
run_gcc "Combo 3: Multi-job complex" --sysroot=/ -save-temps -dumpdir=./dumps -fdump-tree-all -ftime-report -fuse-ld=gold hello1.c hello2.c -o hello_combo3

# Combination 4: Help with sysroot and dump options
run_gcc "Combo 4: Help with other options" --help --sysroot=/ -dumpdir=./dumps 2>&1 | head -30

# Test 8: Edge cases
echo "=== TEST 8: Edge cases ==="
run_gcc "Empty dumpbase and ext" -dumpbase= -dumpbase-ext= -fdump-tree-all hello1.c -o hello_edge1
run_gcc "Very long dumpbase" -dumpbase=very_long_dumpbase_name_that_might_test_buffer_limits -fdump-tree-all hello1.c -o hello_edge2
run_gcc "Multiple sysroots" --sysroot=/ -isysroot/usr --sysroot=/nonexistent hello1.c -o hello_edge3 2>&1 || true

# Test 9: Cleanup and re-initialization
echo "=== TEST 9: Sequential invocations ==="
# Multiple sequential calls to test re-initialization
for i in {1..3}; do
    run_gcc "Sequential call $i" -O$i --sysroot=/ -save-temps=cwd -dumpdir=./dumps_$i hello1.c -o hello_seq_$i
done

# Cleanup
clean_dumps
rm -f hello_* multi_hello pgo_* *.out

echo "All tests completed. Test directory: $TEST_DIR"
echo "To clean up: rm -rf $TEST_DIR"
```

This comprehensive test script covers all the requirements:

1. **Multiple Distinct Jobs**: Tests with multiple source files (`hello1.c hello2.c hello3.c`)
2. **Sysroot Variations**: Tests empty, non-existent, and valid sysroots with `-isysroot` and `-I` flags
3. **Dump File Generation**: Tests all `-save-temps` variants, `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, and various `-fdump-*` flags
4. **Help and Version**: Tests `--help`, `--target-help`, `--version`, and `--help=<subprocess>` with various subprocesses
5. **Linker Selection**: Tests `-fuse-ld=` with bfd, gold, lld, and mold linkers, plus `-Wl,` options
6. **PGO and Timing**: Implements full PGO workflow with `-fprofile-generate`, execution, and `-fprofile-use` with `-ftime-report`
7. **Comprehensive Combinations**: Tests the recommended compilation options and complex flag interactions
8. **Edge Cases**: Tests empty strings, long names, and conflicting options
9. **Sequential Invocations**: Tests that the initialization logic runs correctly for multiple sequential calls

The script creates temporary test files, runs GCC with various flag combinations, cleans up between tests, and captures exit codes. Each test is designed to exercise specific parts of the uncovered initialization block in gcc.cc.
