Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc by testing various flag combinations and execution scenarios:

```bash
#!/bin/bash
set -e

# Configuration
GCC=${1:-gcc}
WORKDIR=$(mktemp -d)
cd "$WORKDIR"
echo "Working in: $WORKDIR"

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
extern void helper();
int main2() {
    helper();
    return 0;
}
EOF

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    rm -f *.o *.s *.i *.ii *.o *.so *.a hello prog test *.gcda *.gcno *.gcov
    rm -f dumps/* dumpdir/* *.dump *.times *.pass *.fail
    rm -rf dumps dumpdir
}
trap cleanup EXIT

# Test 1: Multiple jobs with sysroot variations
echo "=== Test 1: Multiple jobs with sysroot variations ==="
"$GCC" hello1.c hello2.c -o hello --sysroot= -v 2>&1 | grep -q "sysroot" || true
"$GCC" hello1.c hello2.c -o hello --sysroot=/nonexistent -v 2>&1 | grep -q "sysroot" || true
"$GCC" hello1.c hello2.c -o hello --sysroot=/ -isysroot/usr/include -I/usr/local/include -v 2>&1 | grep -q "sysroot" || true

# Test 2: Dump file generation with varied options
echo "=== Test 2: Dump file generation ==="
mkdir -p dumps dumpdir

# Test different save-temps values
for save_temp in "obj" "cwd" ""; do
    if [ -n "$save_temp" ]; then
        flag="-save-temps=$save_temp"
    else
        flag="-save-temps"
    fi
    echo "Testing $flag"
    "$GCC" hello1.c $flag -o hello 2>&1 | tail -5 || true
    cleanup
done

# Test dumpdir/dumpbase combinations
"$GCC" hello1.c -dumpdir=./dumps -dumpbase=mydump -dumpbase-ext=.ext \
    -fdump-tree-all -fdump-rtl-expand -o hello 2>&1 | tail -5 || true

"$GCC" hello1.c -dumpdir= -dumpbase= -dumpbase-ext= \
    -fdump-tree-original -o hello 2>&1 | tail -5 || true

# Test save_temps_flag interactions
"$GCC" hello1.c -save-temps=obj -dumpdir=./dumpdir \
    -fdump-tree-optimized -o hello 2>&1 | tail -5 || true

# Test 3: Help and version output
echo "=== Test 3: Help and version flags ==="
"$GCC" --help > /dev/null
"$GCC" --target-help 2>&1 | head -20
"$GCC" --version
"$GCC" --help=common 2>&1 | head -10
"$GCC" --help=optimizers 2>&1 | head -10

# Combine help with compilation flags
"$GCC" hello1.c --help=warnings -o hello 2>&1 | head -10 || true
"$GCC" --version --sysroot=/ -v 2>&1 | head -5

# Test 4: Linker selection flags
echo "=== Test 4: Linker selection ==="
for linker in bfd gold lld mold; do
    echo "Testing -fuse-ld=$linker"
    "$GCC" hello1.c -fuse-ld=$linker -Wl,--verbose -o hello 2>&1 | grep -i "linker" || true
    cleanup
done

# Test 5: Profile-guided optimization and timing reports
echo "=== Test 5: PGO and timing reports ==="

# Create PGO test program
cat > pgo_test.c << 'EOF'
#include <stdio.h>
int main(int argc, char **argv) {
    int i, sum = 0;
    for (i = 0; i < argc * 100; i++) {
        sum += i;
    }
    printf("Sum: %d\n", sum);
    return 0;
}
EOF

# Generate profile data
echo "Phase 1: Profile generation"
"$GCC" pgo_test.c -O2 -fprofile-generate -ftest-coverage -o pgo_test
./pgo_test 1 2 3 2>&1 | tail -1

# Use profile data with timing report
echo "Phase 2: Profile use with timing report"
"$GCC" pgo_test.c -O2 -fprofile-use -ftime-report -fprofile-report \
    -fprofile-correction -o pgo_test_opt 2>&1 | grep -A5 "Time report" || true

# Test 6: Comprehensive flag combination (as recommended)
echo "=== Test 6: Comprehensive flag combination ==="
"$GCC" hello1.c hello2.c -O0 -v --sysroot= -save-temps \
    -fdump-tree-original -ftime-report -o combined 2>&1 | tail -20

# Test 7: Another comprehensive combination
echo "=== Test 7: Another comprehensive combination ==="
"$GCC" hello1.c -O2 -fuse-ld=bfd --help=optimizers -dumpbase=test \
    -dumpbase-ext=.dump -fdump-rtl-all -o test_prog 2>&1 | head -30

# Test 8: Test greatest_status initialization with failing compilation
echo "=== Test 8: Testing error handling ==="
cat > bad.c << 'EOF'
int main() {
    this_function_does_not_exist();
    return 0;
}
EOF

"$GCC" bad.c -o bad 2>&1 | grep -i "error" || true
echo "Exit code: $?"

# Test 9: Multiple jobs with dump options and sysroot
echo "=== Test 9: Complex multi-job compilation ==="
"$GCC" hello1.c hello2.c hello3.c --sysroot=/ -save-temps=obj \
    -dumpdir=./dumps -fdump-tree-all -ftime-report -fuse-ld=gold \
    -o multijob 2>&1 | tail -15

# Test 10: Verbose only flag simulation
echo "=== Test 10: Verbose output tests ==="
"$GCC" hello1.c -v -### 2>&1 | head -20

# Final cleanup
cleanup
echo "=== All tests completed ==="
```

This script systematically tests the uncovered initialization block by:

1. **Multiple Jobs**: Compiles multiple source files (`hello1.c`, `hello2.c`, `hello3.c`) in single invocations
2. **Sysroot Variations**: Tests `--sysroot=`, `--sysroot=/nonexistent`, `--sysroot=/` with `-isysroot` and `-I`
3. **Dump File Options**: Exercises all `-save-temps` variants, `-dumpdir`, `-dumpbase`, `-dumpbase-ext` with empty/non-empty values, and `-fdump-*` flags
4. **Help/Version**: Tests `--help`, `--target-help`, `--version`, `--help=common`, `--help=optimizers` alone and combined with compilation flags
5. **Linker Selection**: Tests `-fuse-ld=bfd/gold/lld/mold` with `-Wl` options
6. **PGO and Timing**: Implements full PGO workflow with `-fprofile-generate`, execution, then `-fprofile-use` with `-ftime-report`
7. **Comprehensive Combinations**: Creates invocations that combine multiple relevant flags as specified in requirements
8. **Error Handling**: Tests `greatest_status` initialization with a failing compilation
9. **Verbose Output**: Uses `-v` and `-###` to observe driver state

The script cleans up between tests to avoid interference and captures relevant output while allowing non-fatal errors to continue testing other code paths.
