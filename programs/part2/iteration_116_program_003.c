Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc by testing various flag combinations and execution scenarios:

```bash
#!/bin/bash

# GCC driver initialization coverage test script
# Tests the uncovered lines 11228-11250 in gcc.cc

set -e

# Configuration
GCC=${GCC:-gcc}
TEST_DIR=$(mktemp -d /tmp/gcc_init_test_XXXXXX)
trap "rm -rf $TEST_DIR" EXIT

cd $TEST_DIR

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
    printf("Helper function\n");
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

echo "=== Testing GCC driver initialization logic ==="
echo "Test directory: $TEST_DIR"
echo "Using GCC: $($GCC --version | head -1)"
echo

# Helper function to run GCC and capture status
run_gcc() {
    local desc="$1"
    shift
    echo "Test: $desc"
    echo "Command: $GCC $@"
    $GCC "$@" 2>&1 | head -50
    local status=$?
    echo "Exit code: $status"
    echo "---"
    # Clean up generated files
    rm -f *.o *.s *.i *.ii *.o *.so *.a hello test prog *.gcda *.gcno *.gcov 2>/dev/null || true
    rm -rf dumps temps 2>/dev/null || true
}

# 1. Basic multi-job compilation (triggers initialization for each job)
echo "=== 1. Multi-job compilation ==="
run_gcc "Multiple source files" hello1.c hello2.c hello3.c -o multi_prog

# 2. Sysroot variations
echo "=== 2. Sysroot flag variations ==="
run_gcc "Empty sysroot" --sysroot= hello1.c -o hello
run_gcc "Non-existent sysroot" --sysroot=/nonexistent/path/hello hello1.c -o hello
run_gcc "Valid sysroot with isysroot" --sysroot=/ -isysroot/usr/include hello1.c -o hello
run_gcc "Sysroot with includes" --sysroot=/ -I/usr/include -I/usr/local/include hello1.c -o hello
run_gcc "Changed sysroot suffix" --sysroot=/usr --sysroot-suffix=/local hello1.c -o hello

# 3. Dump file management
echo "=== 3. Dump file management ==="
mkdir -p dumps temps

run_gcc "Save temps in obj dir" -save-temps=obj hello1.c -o hello
run_gcc "Save temps in cwd" -save-temps=cwd hello1.c -o hello
run_gcc "Save temps default" -save-temps hello1.c -o hello

run_gcc "Dumpdir with path" -dumpdir=./dumps -fdump-tree-all hello1.c -o hello
run_gcc "Dumpdir empty" -dumpdir= -fdump-tree-all hello1.c -o hello
run_gcc "Dumpbase with extension" -dumpbase=testfile -dumpbase-ext=.mydump -fdump-rtl-expand hello1.c -o hello
run_gcc "All dump options" -dumpdir=./dumps -dumpbase=mydump -dumpbase-ext=.dump -fdump-tree-all -fdump-rtl-all hello1.c -o hello

# 4. Help and version output
echo "=== 4. Help and version flags ==="
run_gcc "Help list" --help
run_gcc "Target help" --target-help
run_gcc "Version" --version
run_gcc "Subprocess help - common" --help=common
run_gcc "Subprocess help - optimizers" --help=optimizers
run_gcc "Subprocess help - params" --help=params
run_gcc "Verbose help" --help -v

# Help flags combined with compilation (should print help and exit)
run_gcc "Help with source file" --help hello1.c
run_gcc "Version with source file" --version hello1.c

# 5. Linker selection
echo "=== 5. Linker selection flags ==="
run_gcc "Default linker" hello1.c -o hello
run_gcc "BFD linker" -fuse-ld=bfd hello1.c -o hello
run_gcc "Gold linker" -fuse-ld=gold hello1.c -o hello 2>/dev/null || echo "Gold linker not available"
run_gcc "LLD linker" -fuse-ld=lld hello1.c -o hello 2>/dev/null || echo "LLD linker not available"
run_gcc "Mold linker" -fuse-ld=mold hello1.c -o hello 2>/dev/null || echo "Mold linker not available"
run_gcc "Linker with options" -Wl,--verbose -Wl,--print-map hello1.c -o hello

# 6. Timing and profile reports
echo "=== 6. Timing and profile reports ==="
run_gcc "Time report" -ftime-report -O2 hello1.c -o hello
run_gcc "Profile report" -fprofile-report hello1.c -o hello 2>/dev/null || echo "Profile report not available"

# PGO workflow
echo "=== 7. PGO workflow ==="
# Compile with profile generation
run_gcc "PGO generate" -fprofile-generate -O2 hello1.c -o hello_pgo
# Run to generate profile data (if program was created)
if [ -x ./hello_pgo ]; then
    ./hello_pgo 2>/dev/null || true
    run_gcc "PGO use with time report" -fprofile-use -ftime-report -O2 hello1.c -o hello_pgo_opt
    run_gcc "PGO with correction" -fprofile-use -fprofile-correction -ftime-report hello1.c -o hello
fi

# 8. Combined flag tests (comprehensive initialization)
echo "=== 8. Combined flag tests ==="
run_gcc "Combined 1" --sysroot=/ -save-temps -dumpdir=./dumps -fdump-tree-all -ftime-report -fuse-ld=bfd hello1.c hello2.c -o combined1
run_gcc "Combined 2" -O2 -fuse-ld=bfd --help=optimizers -dumpbase=test -dumpbase-ext=.dump -fdump-rtl-all hello1.c 2>&1 | head -30
run_gcc "Combined 3" -O0 -v --sysroot= -save-temps -fdump-tree-original -ftime-report hello1.c -o combined3
run_gcc "Combined 4" -save-temps=obj -dumpdir=./dumps -dumpbase=out -fdump-tree-all -fdump-rtl-expand -ftime-report -fuse-ld=gold hello1.c hello2.c -o combined4 2>/dev/null || true

# 9. Test verbose flag to see initialization
echo "=== 9. Verbose output (shows driver internals) ==="
run_gcc "Verbose compilation" -v hello1.c -o hello 2>&1 | grep -i "sysroot\|dump\|collect2" | head -20

# 10. Test with empty/missing arguments
echo "=== 10. Edge cases ==="
run_gcc "No input files" -save-temps --sysroot=/
run_gcc "Only dump flags" -dumpdir= -dumpbase= -dumpbase-ext=
run_gcc "Help with dump flags" --help -dumpdir=./test

echo "=== Cleaning up ==="
rm -rf dumps temps *.o *.s *.i *.ii *.so *.a hello* prog* multi_prog 2>/dev/null || true

echo "=== Test completed ==="
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple Jobs**: Compiles multiple source files in single invocations
2. **Sysroot Variations**: Tests `--sysroot=`, `-isysroot`, and related flags
3. **Dump Management**: Exercises `-save-temps`, `-dumpdir`, `-dumpbase`, `-dumpbase-ext` with various values
4. **Help/Version**: Tests `--help`, `--version`, `--target-help`, `--help=<subprocess>`
5. **Linker Selection**: Tests `-fuse-ld=` with different linkers
6. **PGO and Timing**: Tests `-ftime-report`, `-fprofile-report`, and PGO workflow
7. **Combined Flags**: Tests multiple flags together to trigger complex initialization paths
8. **Verbose Output**: Uses `-v` to observe driver behavior
9. **Edge Cases**: Tests boundary conditions

The script creates temporary test files, runs GCC with various flag combinations, captures output, and cleans up between tests to avoid interference. Each test is designed to exercise specific variables in the uncovered initialization block.
