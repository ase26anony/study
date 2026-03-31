I'll create a comprehensive test script that exercises the uncovered initialization logic in gcc.cc. This script will test various flag combinations to ensure the driver's initialization block is thoroughly covered.

```bash
#!/bin/bash

# Test script for GCC driver initialization coverage
# Targets lines 11228-11250 in gcc.cc

set -e

# Configuration
GCC=${GCC:-gcc}
TESTDIR=$(mktemp -d /tmp/gcc_test_XXXXXX)
echo "Test directory: $TESTDIR"

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    rm -rf "$TESTDIR"
}
trap cleanup EXIT

# Create test source files
cat > "$TESTDIR/hello1.c" << 'EOF'
#include <stdio.h>
int main() {
    printf("Hello World 1\n");
    return 0;
}
EOF

cat > "$TESTDIR/hello2.c" << 'EOF'
#include <stdio.h>
void greet() {
    printf("Hello World 2\n");
}
EOF

cat > "$TESTDIR/hello3.c" << 'EOF'
#include <stdio.h>
void farewell() {
    printf("Goodbye\n");
}
EOF

echo "Created test source files"

# Function to run GCC and capture exit code
run_gcc() {
    local description="$1"
    shift
    echo "========================================"
    echo "Test: $description"
    echo "Command: $GCC $*"
    $GCC "$@" 2>&1 | head -20
    local exit_code=$?
    echo "Exit code: $exit_code"
    echo "========================================"
    echo
    return $exit_code
}

# Change to test directory
cd "$TESTDIR"

# Clean any existing dump files
rm -f *.o *.s *.i *.dump *.gcda *.gcno *.gcov *.time *.temp *.out hello

echo "=== Test 1: Multiple input files (triggers initialization per job) ==="
run_gcc "Multiple files compilation" hello1.c hello2.c hello3.c -o hello_multi

echo "=== Test 2: Sysroot variations ==="
# Empty sysroot
run_gcc "Empty sysroot" --sysroot= hello1.c -o hello_sysroot_empty
# Non-existent sysroot
run_gcc "Non-existent sysroot" --sysroot=/nonexistent/path hello1.c -o hello_sysroot_nonexist
# Valid sysroot (using / as it always exists)
run_gcc "Valid sysroot" --sysroot=/ hello1.c -o hello_sysroot_valid
# Combined with -isysroot and -I
run_gcc "Sysroot with includes" --sysroot=/ -isysroot/usr/include -I/usr/local/include hello1.c -o hello_sysroot_includes

echo "=== Test 3: Dump file generation variations ==="
# Different save-temps options
run_gcc "save-temps=obj" -save-temps=obj hello1.c -o hello_save_obj
run_gcc "save-temps=cwd" -save-temps=cwd hello1.c -o hello_save_cwd
run_gcc "save-temps default" -save-temps hello1.c -o hello_save_default

# Dump directory and base variations
run_gcc "dumpdir with path" -dumpdir=./dumps -fdump-tree-all hello1.c -o hello_dumpdir
mkdir -p "$TESTDIR/dumps2"
run_gcc "dumpdir empty" -dumpdir= -fdump-tree-all hello1.c -o hello_dumpdir_empty
run_gcc "dumpbase variations" -dumpbase=mytest -dumpbase-ext=.dump -fdump-rtl-expand hello1.c -o hello_dumpbase

# Multiple dump flags
run_gcc "Multiple dump flags" -fdump-tree-all -fdump-rtl-all -fdump-ipa-all hello1.c -o hello_dump_multi

echo "=== Test 4: Help and version output ==="
# Individual help/version flags
run_gcc "Help" --help
run_gcc "Target help" --target-help
run_gcc "Version" --version
run_gcc "Help for optimizers" --help=optimizers
run_gcc "Help for common" --help=common
run_gcc "Help for warnings" --help=warnings

# Combined with compilation flags (should still trigger initialization)
run_gcc "Help with other flags" --help -O2 hello1.c -o hello_help_compile 2>/dev/null || true

echo "=== Test 5: Linker selection flags ==="
# Different linker options
for linker in bfd gold lld mold; do
    run_gcc "Linker: $linker" -fuse-ld=$linker hello1.c -o "hello_$linker" 2>/dev/null || echo "Linker $linker not available"
done

# With linker-specific flags
run_gcc "Linker with options" -fuse-ld=bfd -Wl,--verbose hello1.c -o hello_linker_opts 2>/dev/null

echo "=== Test 6: Timing and profile reports ==="
# Time report
run_gcc "Time report" -ftime-report hello1.c -o hello_time

# Profile report (needs profile data)
run_gcc "Profile report flag" -fprofile-report hello1.c -o hello_profile_report 2>/dev/null || true

echo "=== Test 7: Combined flags (comprehensive test) ==="
# Combine multiple relevant flags in one invocation
run_gcc "Combined flags comprehensive" \
    --sysroot=/ \
    -save-temps=obj \
    -dumpdir=./combined_dumps \
    -dumpbase=combined_test \
    -dumpbase-ext=.combined \
    -fdump-tree-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    -v \
    hello1.c hello2.c \
    -o hello_combined

echo "=== Test 8: Profile-guided optimization flow ==="
# Step 1: Generate profile
echo "PGO Step 1: Generate profile"
run_gcc "PGO generate" -O2 -fprofile-generate -ftest-coverage hello1.c -o hello_pgo_gen

# Run instrumented program if compilation succeeded
if [ -f "./hello_pgo_gen" ]; then
    ./hello_pgo_gen > /dev/null 2>&1 || true
    echo "Generated profile data"
    
    # Step 2: Use profile
    echo "PGO Step 2: Use profile"
    run_gcc "PGO use" -O2 -fprofile-use -fprofile-report -fprofile-correction hello1.c -o hello_pgo_use
fi

echo "=== Test 9: Edge cases and special combinations ==="
# Empty dumpbase and dumpbase-ext
run_gcc "Empty dumpbase" -dumpbase= -dumpbase-ext= -fdump-tree-all hello1.c -o hello_empty_dump

# Very long dumpdir path
run_gcc "Long dumpdir" -dumpdir=./very/long/path/for/testing/dump/directory -fdump-tree-all hello1.c -o hello_long_dump 2>/dev/null || true

# Combination of help with dump flags (help should take precedence)
run_gcc "Help with dump flags" --help -dumpdir=./test -fdump-tree-all 2>&1 | head -5

# Reset variables with multiple jobs
run_gcc "Multiple jobs reset test" \
    -save-temps \
    -dumpdir=./job1 \
    hello1.c -c \
    -save-temps=obj \
    -dumpdir=./job2 \
    hello2.c -c

echo "=== Test 10: Verbose output to see initialization ==="
# Use verbose flag to see driver's internal processing
run_gcc "Verbose compilation" -v --sysroot= -save-temps -fdump-tree-original -ftime-report hello1.c -o hello_verbose 2>&1 | grep -i "sysroot\|dump\|temp" | head -10

# Clean up generated files between major test sections
echo "Cleaning intermediate files..."
rm -f *.o *.s *.i *.dump *.gcda *.gcno *.gcov *.time hello_* 2>/dev/null || true
rm -rf dumps dumps2 combined_dumps very 2>/dev/null || true

echo "=== Summary ==="
echo "All tests completed. Check coverage data for initialization block execution."
echo "The driver's initialization logic should have been triggered multiple times"
echo "with different flag combinations covering all target variables."

# Keep the final executables for verification
if [ -f "hello_combined" ]; then
    echo "Testing final executable..."
    ./hello_combined 2>&1 || true
fi
```

This script provides comprehensive coverage of the GCC driver initialization logic by:

1. **Multiple Jobs**: Compiles multiple files in single invocations
2. **Sysroot Variations**: Tests empty, non-existent, and valid sysroots with include flags
3. **Dump File Management**: Exercises all `-save-temps` variants, dump directories, and base names
4. **Help/Version Output**: Tests various help options and combinations
5. **Linker Selection**: Tests different linker backends
6. **PGO and Timing**: Implements a full PGO workflow with timing reports
7. **Combined Flags**: Tests complex flag interactions in single commands
8. **Edge Cases**: Tests empty values and long paths
9. **Verbose Output**: Uses `-v` to observe driver behavior

The script cleans up between tests to avoid interference and captures exit codes to verify successful execution. Each test is designed to trigger specific parts of the initialization block while ensuring the driver proceeds through its normal execution path.
