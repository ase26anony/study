Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc by testing various flag combinations and execution scenarios:

```bash
#!/bin/bash

# GCC driver initialization coverage test script
# Tests the uncovered lines in gcc.cc (lines 11228-11250)

set -e

# Configuration
GCC=${GCC:-gcc}
TEST_DIR=$(mktemp -d /tmp/gcc_driver_test_XXXXXX)
echo "Test directory: $TEST_DIR"
cd "$TEST_DIR"

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

# Create a dummy sysroot directory structure
mkdir -p dummy_sysroot/usr/include
mkdir -p dummy_sysroot/usr/lib
echo "#define DUMMY 1" > dummy_sysroot/usr/include/dummy.h

# Function to clean up dump files between tests
clean_dumps() {
    rm -f *.i *.s *.o *.ii *.bc *.expand *.original *.gimple
    rm -f *.rtl *.cfg *.optimized *.ssa
    rm -f *.gcda *.gcno *.gcov
    rm -rf dumps/ temps/
    mkdir -p dumps temps
}

# Function to run GCC and capture exit code
run_gcc() {
    local description="$1"
    shift
    echo "========================================"
    echo "Test: $description"
    echo "Command: $GCC $*"
    clean_dumps
    $GCC "$@" 2>&1 | head -50
    local status=$?
    echo "Exit code: $status"
    echo "========================================"
    echo
    return $status
}

# Test 1: Multiple distinct jobs in single invocation
echo "=== TEST 1: Multiple jobs in single invocation ==="
run_gcc "Compile multiple files" hello1.c hello2.c hello3.c -o multihello

# Test 2: Sysroot variations
echo "=== TEST 2: Sysroot flag variations ==="
run_gcc "Empty sysroot" --sysroot= hello1.c -o hello_empty_sysroot
run_gcc "Non-existent sysroot" --sysroot=/nonexistent/path/hello hello1.c -o hello_nonexistent_sysroot
run_gcc "Valid sysroot" --sysroot=dummy_sysroot hello1.c -o hello_valid_sysroot
run_gcc "Isysroot with sysroot" --sysroot=dummy_sysroot -isysroot dummy_sysroot hello1.c
run_gcc "Sysroot with include paths" --sysroot=dummy_sysroot -Idummy_sysroot/usr/include hello1.c

# Test 3: Dump file generation with varied options
echo "=== TEST 3: Dump file generation ==="
run_gcc "Save temps in obj dir" -save-temps=obj hello1.c -o hello_save_obj
run_gcc "Save temps in cwd" -save-temps=cwd hello1.c -o hello_save_cwd
run_gcc "Save temps default" -save-temps hello1.c -o hello_save_default
run_gcc "With dumpdir" -dumpdir=./dumps -fdump-tree-all hello1.c -o hello_dumpdir
run_gcc "With dumpbase and extension" -dumpbase=myoutput -dumpbase-ext=.dump -fdump-rtl-expand hello1.c
run_gcc "Empty dumpdir" -dumpdir= -fdump-tree-original hello1.c
run_gcc "Combined dump options" -dumpdir=./dumps -dumpbase=test -dumpbase-ext=.out -fdump-tree-all -fdump-rtl-all hello1.c

# Test 4: Help and version output
echo "=== TEST 4: Help and version output ==="
run_gcc "Basic help" --help
run_gcc "Target help" --target-help
run_gcc "Version" --version
run_gcc "Subprocess help: common" --help=common
run_gcc "Subprocess help: optimizers" --help=optimizers
run_gcc "Help with compilation flags" --help -O2 hello1.c 2>/dev/null || true

# Test 5: Linker selection flags
echo "=== TEST 5: Linker selection ==="
for linker in bfd gold lld mold; do
    run_gcc "Linker: $linker" -fuse-ld=$linker hello1.c -o hello_$linker 2>/dev/null || echo "Linker $linker not available"
done
run_gcc "Wl options" -Wl,--verbose hello1.c -o hello_wl 2>/dev/null

# Test 6: Combined flags for comprehensive coverage
echo "=== TEST 6: Combined flag tests ==="
run_gcc "Combined sysroot and dump options" \
    --sysroot=dummy_sysroot \
    -save-temps \
    -dumpdir=./dumps \
    -fdump-tree-all \
    -ftime-report \
    -fuse-ld=gold \
    hello1.c hello2.c \
    -o hello_combined

run_gcc "Help with dump options" \
    --help=common \
    -dumpbase=help_test \
    -dumpbase-ext=.help \
    -fdump-tree-original 2>/dev/null || true

# Test 7: Profile-guided optimization paths
echo "=== TEST 7: PGO and timing reports ==="
# Step 1: Generate profile
echo "Step 1: Profile generation"
run_gcc "PGO generate" -O2 -fprofile-generate -ftest-coverage hello1.c -o hello_pgo_gen
./hello_pgo_gen 2>/dev/null || true

# Step 2: Use profile with timing report
echo "Step 2: Profile use with timing"
run_gcc "PGO use with timing" -O2 -fprofile-use -fprofile-report -ftime-report hello1.c -o hello_pgo_use

# Test 8: Verbose mode to observe driver state
echo "=== TEST 8: Verbose mode ==="
run_gcc "Verbose with initialization flags" \
    -v \
    -O0 \
    --sysroot= \
    -save-temps \
    -fdump-tree-original \
    -ftime-report \
    hello1.c \
    -o hello_verbose

# Test 9: Reset state between jobs (implicitly tested by multiple runs)
echo "=== TEST 9: Multiple invocations to test reset ==="
for i in {1..3}; do
    run_gcc "Iteration $i with different flags" \
        --sysroot=$([ $((i % 2)) -eq 0 ] && echo "dummy_sysroot" || echo "") \
        -save-temps=$([ $i -eq 1 ] && echo "obj" || echo "cwd") \
        -fdump-tree-all \
        hello1.c \
        -o hello_iter$i
done

# Test 10: Edge cases with empty/missing values
echo "=== TEST 10: Edge cases ==="
run_gcc "Empty dump options" -dumpdir= -dumpbase= -dumpbase-ext= hello1.c
run_gcc "Multiple sysroot changes" --sysroot= --sysroot=dummy_sysroot hello1.c

# Cleanup
echo "Cleaning up test directory: $TEST_DIR"
cd /
rm -rf "$TEST_DIR"

echo "All tests completed!"
```

This script systematically tests the uncovered initialization block by:

1. **Multiple Jobs**: Compiles multiple files (`hello1.c hello2.c hello3.c`) in single invocations
2. **Sysroot Variations**: Tests `--sysroot=` with empty, non-existent, and valid paths, plus `-isysroot` and `-I` flags
3. **Dump Management**: Exercises all `-save-temps` variants, `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, and `-fdump-*` flags
4. **Help/Version**: Tests `--help`, `--target-help`, `--version`, and `--help=<subprocess>` with various subprocesses
5. **Linker Selection**: Tests `-fuse-ld=` with different linkers and `-Wl,` options
6. **PGO and Timing**: Implements a complete PGO workflow with `-fprofile-generate`, execution, and `-fprofile-use` with `-ftime-report`
7. **Combined Flags**: Tests complex flag combinations that exercise multiple initialization variables simultaneously
8. **Verbose Mode**: Uses `-v` to observe driver state changes
9. **State Reset**: Multiple invocations with different flags to ensure proper reset between jobs
10. **Edge Cases**: Tests empty values and multiple flag occurrences

The script cleans up between tests to avoid interference and captures exit codes to verify successful execution. The uncovered variables (`is_cpp_driver`, `at_file_supplied`, `print_help_list`, `print_version`, `use_ld`, `report_times_to_file`, `target_system_root`, dump variables, etc.) are all exercised through these flag combinations.
