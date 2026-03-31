Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc by testing various flag combinations and execution scenarios:

```bash
#!/bin/bash

# GCC driver test script for uncovered initialization logic
# Targets lines 11228-11250 in gcc.cc

set -e

# Configuration
GCC=${GCC:-gcc}
TEST_DIR=$(mktemp -d /tmp/gcc_driver_test_XXXXXX)
echo "Test directory: $TEST_DIR"

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

# Create test source files
cat > "$TEST_DIR/hello1.c" << 'EOF'
#include <stdio.h>
int main() {
    printf("Hello World 1\n");
    return 0;
}
EOF

cat > "$TEST_DIR/hello2.c" << 'EOF'
#include <stdio.h>
void greet() {
    printf("Hello World 2\n");
}
EOF

cat > "$TEST_DIR/hello3.c" << 'EOF'
#include <stdio.h>
void farewell() {
    printf("Goodbye\n");
}
EOF

# Create a simple program for PGO
cat > "$TEST_DIR/pgo_test.c" << 'EOF'
#include <stdio.h>
int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    for (int i = 0; i < 10; i++) {
        printf("factorial(%d) = %d\n", i, factorial(i));
    }
    return 0;
}
EOF

cd "$TEST_DIR"

echo "=== Test 1: Multiple jobs with sysroot variations ==="
# This triggers initialization for each job with sysroot modifications
"$GCC" hello1.c hello2.c hello3.c \
    --sysroot= \
    --sysroot=/nonexistent/path \
    -isysroot /usr \
    -I/usr/include \
    -I. \
    -o multi_hello 2>&1 | head -20

echo -e "\n=== Test 2: Dump file management with various options ==="
# Test all save-temps variants and dump options
for save_opts in "obj" "cwd" ""; do
    echo "Testing -save-temps=$save_opts"
    if [ -z "$save_opts" ]; then
        SAVE_FLAG="-save-temps"
    else
        SAVE_FLAG="-save-temps=$save_opts"
    fi
    
    "$GCC" hello1.c \
        $SAVE_FLAG \
        -dumpdir=./dumps \
        -dumpbase=testdump \
        -dumpbase-ext=.myext \
        -fdump-tree-all \
        -fdump-rtl-expand \
        -fdump-ipa-all \
        -c 2>&1 | grep -E "(dump|save)" | head -5 || true
done

# Test empty dump options
echo -e "\nTesting empty dump options:"
"$GCC" hello1.c \
    -dumpdir= \
    -dumpbase= \
    -dumpbase-ext= \
    -fdump-tree-original \
    -c 2>&1 | grep -i dump | head -3 || true

echo -e "\n=== Test 3: Help and version output ==="
# Individual help/version flags
for flag in "--help" "--target-help" "--version"; do
    echo "Testing $flag:"
    "$GCC" $flag 2>&1 | head -2
done

# Help with subprocess specifications
for topic in "common" "optimizers" "target" "warnings"; do
    echo "Testing --help=$topic:"
    "$GCC" --help=$topic 2>&1 | head -2
done

# Combined with compilation flags
echo -e "\nTesting help flags with compilation:"
"$GCC" --help=common hello1.c -c 2>&1 | head -3

echo -e "\n=== Test 4: Linker selection flags ==="
for linker in "bfd" "gold" "lld" "mold"; do
    echo "Testing -fuse-ld=$linker:"
    "$GCC" hello1.c -fuse-ld=$linker -Wl,--verbose -o "test_$linker" 2>&1 | grep -i "linker" | head -1 || true
done

echo -e "\n=== Test 5: Timing and profile reports ==="
# Simple timing report
echo "Testing -ftime-report:"
"$GCC" hello1.c -O2 -ftime-report -c 2>&1 | grep -E "(Time|report)" | head -3 || true

# PGO workflow
echo -e "\nTesting PGO workflow:"
# Phase 1: Generate profile
echo "Phase 1: Profile generation"
"$GCC" pgo_test.c -O2 -fprofile-generate -o pgo_instrumented

# Run instrumented program to generate profile data
if [ -x ./pgo_instrumented ]; then
    ./pgo_instrumented > /dev/null 2>&1
    # Phase 2: Use profile with timing report
    echo "Phase 2: Profile usage with reports"
    "$GCC" pgo_test.c -O2 -fprofile-use -ftime-report -fprofile-report \
        -o pgo_optimized 2>&1 | grep -E "(profile|time)" | head -5 || true
fi

echo -e "\n=== Test 6: Combined flag stress test ==="
# Combine multiple relevant flags in one invocation
echo "Testing comprehensive flag combination:"
"$GCC" hello1.c hello2.c \
    --sysroot=/ \
    -save-temps=obj \
    -dumpdir=./combined_dumps \
    -dumpbase=combined \
    -dumpbase-ext=.test \
    -fdump-tree-all \
    -fdump-rtl-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    -v \
    -o combined_test 2>&1 | tail -20

echo -e "\n=== Test 7: Verbose output for initialization debugging ==="
# Use verbose flag to see driver's internal state
echo "Testing with verbose flag:"
"$GCC" hello1.c \
    -O0 \
    -v \
    --sysroot= \
    -save-temps \
    -fdump-tree-original \
    -ftime-report \
    -c 2>&1 | grep -i "sysroot\|dump\|collect2" | head -10

echo -e "\n=== Test 8: Optimization with help and dump combinations ==="
echo "Testing optimization with help and dump flags:"
"$GCC" hello1.c \
    -O2 \
    -fuse-ld=bfd \
    --help=optimizers \
    -dumpbase=opt_test \
    -dumpbase-ext=.dump \
    -fdump-rtl-all \
    -c 2>&1 | grep -E "(help|dump|optimiz)" | head -10

echo -e "\n=== Test 9: Clean compilation to verify exit codes ==="
# Simple compilation to ensure clean exit
echo "Testing clean compilation:"
"$GCC" hello1.c -o simple_hello
if [ $? -eq 0 ]; then
    echo "✓ Clean compilation successful"
fi

# Test exit codes for help/version
echo -e "\n=== Test 10: Exit code verification ==="
for flag in "--help" "--version" "--target-help"; do
    "$GCC" $flag > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "✓ $flag exited successfully"
    fi
done

echo -e "\n=== Cleaning generated files ==="
rm -f *.o *.s *.i *.ii *.bc *.expand *.original *.gkd
rm -f multi_hello test_* pgo_* combined_test simple_hello
rm -rf dumps combined_dumps
rm -f *.gcda *.gcno *.gcov

echo -e "\n=== All tests completed ==="
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple jobs with sysroot variations** - Exercises `target_system_root`, `target_system_root_changed`, and related variables
2. **Dump file management** - Tests `dumpdir`, `dumpbase`, `dumpbase_ext`, `outbase`, and `save_temps_flag` with various combinations
3. **Help and version output** - Triggers `print_help_list`, `print_version`, and `print_subprocess_help`
4. **Linker selection** - Sets `use_ld` via `-fuse-ld` options
5. **Timing and profile reports** - Exercises `report_times_to_file` with `-ftime-report` and PGO workflow
6. **Combined flag stress test** - Tests interaction of multiple flags in one invocation
7. **Verbose debugging** - Uses `-v` to observe driver state changes
8. **Optimization combinations** - Tests help and dump flags with optimization
9. **Exit code verification** - Ensures proper exit status for different invocations

The script creates temporary test files, runs GCC with the specified flag combinations, captures relevant output, and cleans up after itself. Each test is designed to trigger specific parts of the initialization logic while ensuring the driver can proceed without fatal errors.
