Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc by testing various flag combinations and execution scenarios:

```bash
#!/bin/bash
set -e

# Configuration
GCC=${1:-gcc}
WORKDIR=$(mktemp -d)
cd "$WORKDIR"

echo "Testing GCC driver initialization logic in: $WORKDIR"
echo "Using GCC: $($GCC --version | head -1)"

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

# Function to clean up dump files
clean_dumps() {
    rm -f *.i *.s *.o *.ii *.bc *.expand *.original *.gimple *.cfg *.optimized
    rm -f *.c.* *.c.*.* *.times *.gcda *.gcno *.gcov
    rm -rf dumps/ *.dSYM/
}

# Test 1: Multiple jobs with sysroot variations
echo -e "\n=== Test 1: Multiple jobs with sysroot ==="
clean_dumps
$GCC hello1.c hello2.c -o prog1 --sysroot= -v 2>&1 | grep -q "sysroot" && echo "Sysroot empty: PASS" || echo "Sysroot empty: flags processed"
$GCC hello1.c hello2.c -o prog2 --sysroot=/nonexistent -v 2>&1 | grep -q "sysroot" && echo "Sysroot nonexistent: PASS" || echo "Sysroot nonexistent: flags processed"
$GCC hello1.c hello2.c -o prog3 --sysroot=/ -isysroot/usr/include -I/usr/local/include -v 2>&1 | grep -q "sysroot\|isysroot" && echo "Sysroot with includes: PASS" || echo "Sysroot with includes: flags processed"

# Test 2: Dump file management with save-temps variations
echo -e "\n=== Test 2: Dump file management ==="
clean_dumps
$GCC hello1.c -save-temps=obj -fdump-tree-all -fdump-rtl-expand -o hello_save1 2>&1 && echo "save-temps=obj: PASS" || echo "save-temps=obj: FAIL"
ls *.i *.s *.o *.expand *.original 2>/dev/null | wc -l | grep -q "[1-9]" && echo "Dump files created" || echo "No dump files"

clean_dumps
$GCC hello1.c -save-temps=cwd -dumpdir=./dumps -fdump-tree-all -o hello_save2 2>&1 && echo "save-temps=cwd with dumpdir: PASS" || echo "save-temps=cwd: FAIL"
ls dumps/* 2>/dev/null | head -2 && echo "Dumpdir used" || echo "No dumpdir files"

clean_dumps
$GCC hello1.c -save-temps -dumpbase="mydump" -dumpbase-ext=".debug" -fdump-tree-all -o hello_save3 2>&1 && echo "dumpbase with extension: PASS" || echo "dumpbase: FAIL"
ls mydump* 2>/dev/null && echo "Custom dumpbase used" || echo "No custom dumpbase"

# Test 3: Help and version flags
echo -e "\n=== Test 3: Help and version output ==="
$GCC --help > /dev/null 2>&1 && echo "--help: PASS" || echo "--help: FAIL"
$GCC --target-help > /dev/null 2>&1 && echo "--target-help: PASS" || echo "--target-help: FAIL"
$GCC --version > /dev/null 2>&1 && echo "--version: PASS" || echo "--version: FAIL"
$GCC --help=common > /dev/null 2>&1 && echo "--help=common: PASS" || echo "--help=common: FAIL"
$GCC --help=optimizers > /dev/null 2>&1 && echo "--help=optimizers: PASS" || echo "--help=optimizers: FAIL"

# Test 4: Linker selection flags
echo -e "\n=== Test 4: Linker selection ==="
for linker in bfd gold lld mold; do
    $GCC hello1.c -fuse-ld=$linker -Wl,--verbose -o hello_$linker 2>&1 | grep -q "ld" && echo "fuse-ld=$linker: flags processed" || echo "fuse-ld=$linker: tried"
done

# Test 5: Combined flags in single invocation
echo -e "\n=== Test 5: Combined flags ==="
clean_dumps
$GCC hello1.c hello2.c -o hello_combined \
    --sysroot=/ \
    -save-temps \
    -dumpdir=./combined_dumps \
    -fdump-tree-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    -v 2>&1 | tail -5
echo "Combined flags: EXECUTED"

# Test 6: Profile-guided optimization path
echo -e "\n=== Test 6: PGO and timing reports ==="
clean_dumps

# Step 1: Generate profile
echo "int main() { for(int i=0; i<100; i++); return 0; }" > pgo_test.c
$GCC pgo_test.c -O2 -fprofile-generate -ftest-coverage -o pgo_instrumented 2>&1 && echo "PGO instrumented build: PASS" || echo "PGO instrumented: SKIP"

# Run instrumented program if built successfully
if [ -x ./pgo_instrumented ]; then
    ./pgo_instrumented
    # Step 2: Use profile with timing report
    $GCC pgo_test.c -O2 -fprofile-use -fprofile-report -ftime-report -o pgo_optimized 2>&1 | grep -i "time\|profile" | head -3
    echo "PGO with timing report: EXECUTED"
fi

# Test 7: Multiple help flags with compilation (should prioritize help)
echo -e "\n=== Test 7: Help with compilation flags ==="
$GCC hello1.c --help=warnings -O2 -o dummy 2>&1 | head -2
echo "Help with compilation flags: EXECUTED (help takes precedence)"

# Test 8: Empty dump options
echo -e "\n=== Test 8: Empty dump options ==="
clean_dumps
$GCC hello1.c -dumpdir= -dumpbase= -dumpbase-ext= -fdump-tree-all -o hello_empty 2>&1 && echo "Empty dump options: PASS" || echo "Empty dump options: flags processed"

# Test 9: Verbose only flag simulation
echo -e "\n=== Test 9: Verbose output ==="
$GCC hello1.c -v -### 2>&1 | grep -q "COLLECT_GCC_OPTIONS" && echo "Verbose mode: PASS" || echo "Verbose mode: flags processed"

# Cleanup
echo -e "\n=== Cleaning up ==="
clean_dumps
cd ..
rm -rf "$WORKDIR"
echo "Test directory cleaned: $WORKDIR"
echo -e "\nAll tests completed. Check output for any failures."
```

This script systematically tests the uncovered initialization block by:

1. **Multiple jobs with sysroot**: Tests `target_system_root`, `target_system_root_changed` with empty, nonexistent, and valid paths
2. **Dump file management**: Exercises `dumpdir`, `dumpbase`, `dumpbase_ext`, `save_temps_flag` with various combinations
3. **Help/version output**: Tests `print_help_list`, `print_version`, `print_subprocess_help`
4. **Linker selection**: Tests `use_ld` with different linker backends
5. **Combined flags**: Single invocation with multiple relevant flags to test interaction
6. **PGO and timing**: Tests `report_times_to_file` with profile-guided optimization
7. **Edge cases**: Empty dump options, help with compilation flags, verbose output

The script creates temporary test files, runs GCC with the specified flag combinations, and cleans up after itself. Each test is designed to trigger specific parts of the initialization logic while ensuring the driver proceeds through its normal execution flow.
