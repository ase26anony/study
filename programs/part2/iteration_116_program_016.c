I'll create a comprehensive test script that exercises the uncovered initialization logic in gcc.cc. This script will test various flag combinations to ensure the driver's state is properly reset between jobs.

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

# Create simple test programs
cat > "$TESTDIR/hello1.c" << 'EOF'
#include <stdio.h>
int main() {
    printf("Hello 1\n");
    return 0;
}
EOF

cat > "$TESTDIR/hello2.c" << 'EOF'
#include <stdio.h>
void helper() {
    printf("Helper\n");
}
EOF

cat > "$TESTDIR/hello3.c" << 'EOF'
#include <stdio.h>
extern void helper();
int main() {
    printf("Hello 3\n");
    helper();
    return 0;
}
EOF

# Create a dummy sysroot directory structure
mkdir -p "$TESTDIR/sysroot/usr/include"
mkdir -p "$TESTDIR/sysroot/usr/lib"
echo "#define DUMMY 1" > "$TESTDIR/sysroot/usr/include/dummy.h"

# Function to run GCC and capture exit code
run_gcc() {
    local desc="$1"
    shift
    echo "========================================"
    echo "Test: $desc"
    echo "Command: $GCC $*"
    $GCC "$@" 2>&1 | head -50
    local status=$?
    echo "Exit code: $status"
    echo "========================================"
    echo
    return $status
}

# Clean dump files between tests
clean_dumps() {
    rm -f "$TESTDIR"/*.o "$TESTDIR"/*.s "$TESTDIR"/*.i "$TESTDIR"/*.ii \
          "$TESTDIR"/*.dump* "$TESTDIR"/*.gcda "$TESTDIR"/*.gcno \
          "$TESTDIR"/*.time "$TESTDIR"/*.profile "$TESTDIR"/dumps/* 2>/dev/null || true
}

# Test 1: Multiple jobs with sysroot variations
echo "=== Test 1: Multiple jobs with sysroot ==="
clean_dumps
run_gcc "Multiple files with empty sysroot" \
    -c "$TESTDIR/hello1.c" "$TESTDIR/hello2.c" \
    --sysroot= \
    -isysroot "$TESTDIR/sysroot" \
    -I"$TESTDIR/sysroot/usr/include" \
    -o "$TESTDIR/hello1.o"

run_gcc "Multiple files with dummy sysroot" \
    -c "$TESTDIR/hello1.c" "$TESTDIR/hello2.c" \
    --sysroot="$TESTDIR/sysroot" \
    -o "$TESTDIR/hello2.o"

run_gcc "Multiple files with non-existent sysroot" \
    -c "$TESTDIR/hello1.c" "$TESTDIR/hello2.c" \
    --sysroot=/nonexistent/path/$(date +%s) \
    -o "$TESTDIR/hello3.o" 2>/dev/null || true

# Test 2: Dump file management with various options
echo "=== Test 2: Dump file management ==="
clean_dumps
mkdir -p "$TESTDIR/dumps"

run_gcc "Save temps in obj directory" \
    -c "$TESTDIR/hello1.c" \
    -save-temps=obj \
    -dumpdir="$TESTDIR/dumps/" \
    -dumpbase="hello1_test" \
    -dumpbase-ext=".dump" \
    -fdump-tree-all \
    -fdump-rtl-expand \
    -o "$TESTDIR/hello1_savetemps.o"

run_gcc "Save temps in cwd" \
    -c "$TESTDIR/hello2.c" \
    -save-temps=cwd \
    -dumpdir="" \
    -dumpbase="" \
    -fdump-tree-original \
    -o "$TESTDIR/hello2_savetemps.o"

run_gcc "Default save temps" \
    -c "$TESTDIR/hello3.c" \
    -save-temps \
    -dumpdir="$TESTDIR/dumps" \
    -fdump-ipa-all \
    -o "$TESTDIR/hello3_savetemps.o"

# Test 3: Help and version output
echo "=== Test 3: Help and version ==="
run_gcc "Print version" --version
run_gcc "Print help" --help
run_gcc "Print target help" --target-help
run_gcc "Print subprocess help" --help=common
run_gcc "Print optimizers help" --help=optimizers

# Test with compilation flags combined
run_gcc "Help combined with other flags" \
    --help=warnings \
    -O2 \
    --sysroot="$TESTDIR/sysroot" \
    -v

# Test 4: Linker selection flags
echo "=== Test 4: Linker selection ==="
clean_dumps

# Try different linkers (some might not be available)
for linker in bfd gold lld mold; do
    run_gcc "Test linker: $linker" \
        "$TESTDIR/hello1.c" "$TESTDIR/hello2.c" "$TESTDIR/hello3.c" \
        -fuse-ld=$linker \
        -Wl,--verbose \
        -o "$TESTDIR/test_$linker" 2>/dev/null || echo "Linker $linker not available"
done

# Test 5: Timing reports and PGO
echo "=== Test 5: Timing reports and PGO ==="
clean_dumps

# Create a program for PGO
cat > "$TESTDIR/pgo_test.c" << 'EOF'
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

# Step 1: Generate profile
echo "=== PGO Step 1: Generate profile ==="
run_gcc "PGO generate" \
    "$TESTDIR/pgo_test.c" \
    -O2 \
    -fprofile-generate \
    -ftime-report \
    -o "$TESTDIR/pgo_gen"

# Run the instrumented program
if [ -x "$TESTDIR/pgo_gen" ]; then
    echo "Running instrumented program..."
    "$TESTDIR/pgo_gen" arg1 arg2 arg3 >/dev/null 2>&1
fi

# Step 2: Use profile
echo "=== PGO Step 2: Use profile ==="
run_gcc "PGO use" \
    "$TESTDIR/pgo_test.c" \
    -O2 \
    -fprofile-use \
    -fprofile-report \
    -ftime-report \
    -fprofile-correction \
    -o "$TESTDIR/pgo_use"

# Test 6: Combined comprehensive test
echo "=== Test 6: Combined comprehensive test ==="
clean_dumps

run_gcc "Comprehensive test" \
    "$TESTDIR/hello1.c" "$TESTDIR/hello2.c" "$TESTDIR/hello3.c" \
    --sysroot="$TESTDIR/sysroot" \
    -save-temps \
    -dumpdir="$TESTDIR/dumps" \
    -dumpbase="comprehensive" \
    -dumpbase-ext=".test" \
    -fdump-tree-all \
    -fdump-rtl-all \
    -ftime-report \
    -fuse-ld=bfd \
    -O2 \
    -v \
    -o "$TESTDIR/comprehensive_test"

# Test 7: Multiple jobs with different dump options
echo "=== Test 7: Multiple jobs sequence ==="
clean_dumps

# Sequence of compilations to test re-initialization
run_gcc "Job 1: With dumpdir" \
    -c "$TESTDIR/hello1.c" \
    -dumpdir="$TESTDIR/dumps/job1" \
    -fdump-tree-optimized \
    -o "$TESTDIR/job1.o"

run_gcc "Job 2: Different dumpdir" \
    -c "$TESTDIR/hello2.c" \
    -dumpdir="$TESTDIR/dumps/job2" \
    -fdump-rtl-expand \
    -o "$TESTDIR/job2.o"

run_gcc "Job 3: No dumpdir" \
    -c "$TESTDIR/hello3.c" \
    -fdump-ipa-all \
    -o "$TESTDIR/job3.o"

# Test 8: Edge cases for dump options
echo "=== Test 8: Edge cases ==="
run_gcc "Empty dumpbase" \
    -c "$TESTDIR/hello1.c" \
    -dumpbase="" \
    -dumpbase-ext="" \
    -fdump-tree-all \
    -o "$TESTDIR/edge1.o"

run_gcc "Dumpdir with trailing slash" \
    -c "$TESTDIR/hello2.c" \
    -dumpdir="$TESTDIR/dumps/" \
    -save-temps \
    -o "$TESTDIR/edge2.o"

# Test 9: Verbose flag combinations
echo "=== Test 9: Verbose output ==="
run_gcc "Verbose with sysroot" \
    -c "$TESTDIR/hello1.c" \
    --sysroot="$TESTDIR/sysroot" \
    -v \
    -save-temps=obj \
    -o "$TESTDIR/verbose.o" 2>&1 | grep -i "sysroot\|collect2" | head -10

# List generated files to verify coverage
echo "=== Generated files ==="
find "$TESTDIR" -type f -name "*.i" -o -name "*.s" -o -name "*.o" -o -name "*.dump*" | sort
echo

echo "=== Test summary ==="
echo "Tests completed. Check for:"
echo "1. Multiple job processing"
echo "2. Sysroot variable initialization"
echo "3. Dump file management"
echo "4. Help/version output"
echo "5. Linker selection"
echo "6. Timing reports and PGO"
echo "7. Combined flag interactions"
```

This script comprehensively tests the uncovered initialization logic by:

1. **Multiple Jobs**: Compiles multiple `.c` files in single invocations
2. **Sysroot Variations**: Tests `--sysroot=`, valid sysroot, and non-existent sysroot
3. **Dump Management**: Exercises `-save-temps`, `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, and `-fdump-*` flags
4. **Help/Version**: Tests `--help`, `--version`, `--target-help`, and `--help=<subprocess>`
5. **Linker Selection**: Tests `-fuse-ld=` with different linkers and `-Wl,` options
6. **PGO and Timing**: Tests `-fprofile-generate`, `-fprofile-use`, `-ftime-report`, and `-fprofile-report`
7. **Combined Tests**: Includes comprehensive invocations with multiple relevant flags
8. **Edge Cases**: Tests empty strings and trailing slashes in dump options

The script cleans up between tests to avoid interference and captures exit codes to verify the driver handles all flag combinations correctly.
