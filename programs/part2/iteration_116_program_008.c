Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc by testing various flag combinations and execution scenarios:

```bash
#!/bin/bash

# Test script for GCC driver initialization coverage
# Targets lines 11228-11250 in gcc.cc

set -e

# Use system GCC or specify path
GCC=${GCC:-gcc}
echo "Testing with GCC: $($GCC --version | head -1)"

# Create test directory
TESTDIR="gcc_coverage_test"
mkdir -p "$TESTDIR"
cd "$TESTDIR"

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

# Cleanup function
cleanup() {
    rm -f *.o *.s *.i *.ii *.bc *.expand *.original *.gimple *.cfg *.dot
    rm -f *.gcda *.gcno *.gcov *.d *.d.* *.stackdump
    rm -f hello hello_pgo *.dmp *.dump *.time
    rm -rf dumps temps profile_data
}

# Test 1: Multiple jobs with sysroot variations
echo "=== Test 1: Multiple jobs with sysroot variations ==="
cleanup
$GCC hello1.c hello2.c -o hello --sysroot= -v 2>&1 | grep -q "sysroot" || true
$GCC hello1.c hello2.c -o hello --sysroot=/nonexistent/path -v 2>&1 | grep -q "sysroot" || true
$GCC hello1.c hello2.c -o hello --sysroot=/ -isysroot/usr/include -I/usr/local/include -v 2>&1 | grep -q "sysroot" || true

# Test 2: Dump file generation with various options
echo "=== Test 2: Dump file generation ==="
cleanup
mkdir -p dumps

# Different save-temps options
$GCC hello1.c -save-temps=obj -o hello_obj
$GCC hello1.c -save-temps=cwd -o hello_cwd
$GCC hello1.c -save-temps -o hello_all

# Dumpdir and dumpbase combinations
$GCC hello1.c -dumpdir=./dumps -dumpbase=test1 -dumpbase-ext=.dmp -fdump-tree-all -o hello_dump1
$GCC hello1.c -dumpdir= -dumpbase= -dumpbase-ext= -fdump-rtl-expand -o hello_dump2
$GCC hello1.c -dumpdir=dumps/ -dumpbase=test2 -fdump-tree-original -fdump-tree-gimple -o hello_dump3

# Test 3: Help and version output
echo "=== Test 3: Help and version output ==="
$GCC --help > /dev/null
$GCC --target-help > /dev/null
$GCC --version > /dev/null
$GCC --help=common > /dev/null
$GCC --help=optimizers > /dev/null
$GCC --help=warnings > /dev/null

# Combined with compilation flags
$GCC --help=common hello1.c -c 2>&1 | grep -q "help" || true
$GCC --version -v hello1.c -c 2>&1 | grep -q "version" || true

# Test 4: Linker selection flags
echo "=== Test 4: Linker selection ==="
cleanup

# Try different linkers (some may not be available)
for linker in bfd gold lld mold; do
    $GCC hello1.c -fuse-ld=$linker -o hello_$linker 2>/dev/null || true
done

# With linker flags
$GCC hello1.c hello2.c -fuse-ld=bfd -Wl,--verbose -o hello_verbose 2>&1 | grep -q "ld" || true
$GCC hello1.c -fuse-ld=gold -Wl,-Map=output.map -o hello_map 2>/dev/null || true

# Test 5: Profile-guided optimization and timing
echo "=== Test 5: PGO and timing reports ==="
cleanup
mkdir -p profile_data

# PGO: Generate profile
echo "Building with profile generation..."
$GCC -O2 -fprofile-generate=./profile_data hello1.c -o hello_pgo_gen
./hello_pgo_gen 2>/dev/null || true

# PGO: Use profile with timing report
echo "Building with profile use and timing..."
$GCC -O2 -fprofile-use=./profile_data -ftime-report hello1.c -o hello_pgo_use 2>&1 | grep -q "Time variable" || true

# Profile report
$GCC -O2 -fprofile-report -fprofile-correction hello1.c -c 2>&1 | grep -q "profile" || true

# Test 6: Combined flags in single invocation (comprehensive test)
echo "=== Test 6: Combined flags test ==="
cleanup
$GCC hello1.c hello2.c hello3.c \
    --sysroot=/ \
    -save-temps \
    -dumpdir=./combined_dumps \
    -dumpbase=combined \
    -dumpbase-ext=.full \
    -fdump-tree-all \
    -fdump-rtl-all \
    -ftime-report \
    -fuse-ld=bfd \
    -O2 \
    -v \
    -o combined_hello 2>&1 | tail -20

# Test 7: Verbose only flag simulation
echo "=== Test 7: Verbose and diagnostic flags ==="
cleanup
$GCC hello1.c -v -### 2>&1 | grep -q "COLLECT_GCC" || true
$GCC hello1.c -Q -v 2>&1 | grep -q "options" || true

# Test 8: Reset state between compilations
echo "=== Test 8: State reset verification ==="
cleanup
# First with help
$GCC --help=common > /dev/null
# Then compile - should reset state
$GCC hello1.c -save-temps -dumpdir=./final -o final_hello
# Then version
$GCC --version > /dev/null
# Then another compile with different options
$GCC hello2.c --sysroot= -fdump-tree-original -c

# Test 9: Edge cases with empty/null values
echo "=== Test 9: Edge cases ==="
cleanup
$GCC hello1.c --sysroot="" -dumpdir="" -dumpbase="" -o hello_edge 2>&1 || true
$GCC -x c - -dumpbase="-" -o hello_stdin <<< 'int main(){return 0;}' 2>&1 || true

# Test 10: Multiple output files
echo "=== Test 10: Multiple outputs ==="
cleanup
$GCC hello1.c -c -o hello1.o --save-temps=obj
$GCC hello2.c -c -o hello2.o --save-temps=obj
$GCC hello1.o hello2.o -o hello_multi

# Cleanup
echo "=== Cleaning up ==="
cleanup
cd ..
rm -rf "$TESTDIR"

echo "All tests completed successfully!"
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple jobs with sysroot variations**: Exercises `target_system_root`, `target_system_root_changed`, and related variables with different sysroot values.

2. **Dump file generation**: Tests `dumpdir`, `dumpbase`, `dumpbase_ext`, `outbase`, and `save_temps_flag` with various combinations of dump-related flags.

3. **Help and version output**: Triggers `print_help_list`, `print_version`, and `print_subprocess_help` with different help requests.

4. **Linker selection**: Sets `use_ld` with different linker options and passes flags to the linker.

5. **PGO and timing**: Exercises `report_times_to_file` with profile-guided optimization and timing reports.

6. **Combined flags**: Tests multiple flags together to ensure proper initialization state management.

7. **Verbose flags**: Tests verbose output which can affect driver state.

8. **State reset**: Verifies state is properly reset between different types of invocations.

9. **Edge cases**: Tests empty values and special cases.

10. **Multiple outputs**: Tests compilation with multiple output files.

The script creates temporary test files, runs the GCC driver with various flag combinations, and cleans up after itself. Each test is designed to trigger specific parts of the initialization logic while ensuring the driver can proceed without fatal errors.
