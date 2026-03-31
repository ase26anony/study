Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc by testing various flag combinations and execution scenarios:

```bash
#!/bin/bash

# GCC driver initialization coverage test script
# Tests the uncovered lines in gcc.cc (lines 11228-11250)

set -e

# Configuration
GCC=${GCC:-gcc}
TESTDIR=$(mktemp -d /tmp/gcc_driver_test_XXXXXX)
echo "Test directory: $TESTDIR"
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

# Function to clean up dump files between tests
clean_dumps() {
    rm -f *.o *.s *.i *.ii *.bc *.expand *.original *.gimple *.cfg *.optimized
    rm -f *.r*.dump *.t*.dump *.c*.dump *.ltrans*.dump
    rm -rf dumps/ temps/ profile_data/
    rm -f *.gcda *.gcno *.gcov
    rm -f times.txt profile_report.txt
}

# Test 1: Basic multiple file compilation with sysroot and dump options
echo "=== Test 1: Multiple files with sysroot and dump options ==="
clean_dumps
$GCC hello1.c hello2.c hello3.c -o prog1 \
    --sysroot=/ \
    -save-temps \
    -dumpdir=./dumps \
    -fdump-tree-all \
    -ftime-report \
    -v 2>&1 | grep -E "(sysroot|dump|TIME)" || true
./prog1 2>/dev/null && echo "Test 1 program executed successfully"

# Test 2: Empty sysroot and various dumpbase options
echo -e "\n=== Test 2: Empty sysroot with dumpbase variants ==="
clean_dumps
$GCC hello1.c -o prog2 \
    --sysroot= \
    -dumpbase="myprog" \
    -dumpbase-ext=".debug" \
    -fdump-rtl-expand \
    -fdump-rtl-combine \
    -save-temps=obj \
    -v 2>&1 | grep -E "(sysroot|dump|Output)" || true

# Test 3: Non-existent sysroot with different save-temps modes
echo -e "\n=== Test 3: Non-existent sysroot and save-temps modes ==="
clean_dumps
$GCC hello1.c hello2.c -o prog3 \
    --sysroot=/nonexistent/path/$(date +%s) \
    -save-temps=cwd \
    -fdump-tree-original \
    -fdump-tree-gimple \
    -Wl,--verbose \
    2>&1 | grep -i "sysroot\|warning\|error" || true

# Test 4: Combined sysroot options with isysroot
echo -e "\n=== Test 4: Combined sysroot options ==="
clean_dumps
$GCC hello1.c -o prog4 \
    --sysroot=/usr \
    -isysroot / \
    -I/usr/include \
    -I/usr/local/include \
    -save-temps \
    -dumpdir="" \
    -dumpbase="" \
    -dumpbase-ext="" \
    -v 2>&1 | grep -E "(sysroot|include.*path)" || true

# Test 5: Help and version flags (should exit early)
echo -e "\n=== Test 5: Help and version flags ==="
$GCC --help > /dev/null && echo "Help flag succeeded"
$GCC --version > /dev/null && echo "Version flag succeeded"
$GCC --target-help 2>&1 | head -5 && echo "Target help succeeded"
$GCC --help=common > /dev/null && echo "Common help succeeded"
$GCC --help=optimizers 2>&1 | grep -i "optimization" | head -2 && echo "Optimizers help succeeded"

# Test 6: Help flags combined with compilation (should prioritize help)
echo -e "\n=== Test 6: Help combined with compilation flags ==="
$GCC --help -O2 hello1.c -o dummy 2>&1 | grep -i "optimization options" | head -2 && echo "Help with compilation flags succeeded"

# Test 7: Different linker selections
echo -e "\n=== Test 7: Linker selection flags ==="
for linker in bfd gold lld mold; do
    echo -n "Testing -fuse-ld=$linker: "
    $GCC hello1.c -fuse-ld=$linker -o prog7_$linker 2>&1 | grep -i "linker\|error" | head -1 || echo "Attempted (may not be available)"
done

# Test 8: Linker flags with Wl
echo -e "\n=== Test 8: Linker flags with Wl ==="
$GCC hello1.c -Wl,--verbose -Wl,--print-map -o prog8 2>&1 | grep -i "linker\|map" | head -2 || true

# Test 9: Profile-guided optimization with timing reports
echo -e "\n=== Test 9: PGO with timing reports ==="
clean_dumps
mkdir -p profile_data

# Phase 1: Generate profile
echo "Phase 1: Profile generation"
$GCC hello1.c -O2 -fprofile-generate=./profile_data -ftest-coverage -o prog9_instr
./prog9_instr 2>/dev/null

# Phase 2: Use profile with timing report
echo "Phase 2: Profile usage with timing"
$GCC hello1.c -O2 -fprofile-use=./profile_data -ftime-report -fprofile-report -o prog9_opt 2>&1 | \
    grep -E "(TIME|profile|execution)" || true

# Test 10: Complex combination of all relevant flags
echo -e "\n=== Test 10: Complex flag combination ==="
clean_dumps
$GCC hello1.c hello2.c hello3.c -o prog10 \
    --sysroot=/ \
    -save-temps=obj \
    -dumpdir="./dumps" \
    -dumpbase="combined" \
    -dumpbase-ext=".test" \
    -fdump-tree-all \
    -fdump-rtl-all \
    -ftime-report \
    -fuse-ld=gold \
    -O2 \
    --help=optimizers \
    -Wl,--verbose 2>&1 | \
    grep -E "(sysroot|dump|TIME|linker|optimization)" | head -10 || true

# Test 11: Multiple jobs with different output bases
echo -e "\n=== Test 11: Multiple outputs with outbase ==="
clean_dumps
$GCC hello1.c -c -o hello1.o \
    -dumpbase="obj1" \
    -outbase="obj1"
$GCC hello2.c -c -o hello2.o \
    -dumpbase="obj2" \
    -outbase="obj2"
$GCC hello1.o hello2.o hello3.c -o prog11 \
    -save-temps \
    -fdump-tree-original

# Test 12: Test print_subprocess_help with various subprocesses
echo -e "\n=== Test 12: Subprocess help ==="
for topic in common warnings optimize params target; do
    echo -n "Testing --help=$topic: "
    $GCC --help=$topic 2>&1 | head -2 | grep -i "." || echo "No output"
done

# Test 13: Verbose only flag simulation
echo -e "\n=== Test 13: Verbose output ==="
$GCC hello1.c -o prog13 -v -### 2>&1 | grep -E "(sysroot|collect2|cc1)" | head -5 || true

# Test 14: Test greatest_status with error case
echo -e "\n=== Test 14: Error case to test status ==="
$GCC nonexistent.c -o dummy 2>&1 | grep -i "error" || echo "Error detected as expected"

# Test 15: Cleanup and reinitialization test
echo -e "\n=== Test 15: Sequential compilations testing reinitialization ==="
clean_dumps
for i in 1 2 3; do
    echo "Compilation $i:"
    $GCC hello1.c -o prog15_$i \
        --sysroot=/ \
        -save-temps \
        -dumpdir="run$i" \
        -fdump-tree-original \
        -v 2>&1 | grep -E "(Output file|dump)" | head -2 || true
    clean_dumps
done

# Final cleanup
echo -e "\n=== Cleaning up ==="
cd /
rm -rf "$TESTDIR"
echo "Test directory removed"
echo -e "\nAll tests completed. The uncovered initialization block should have been exercised through:"
echo "1. Multiple job processing (tests 1, 3, 10)"
echo "2. Sysroot variations (tests 1-4, 10)"
echo "3. Dump file management (tests 1-4, 7, 10, 11, 15)"
echo "4. Help/version output (tests 5-6, 10, 12)"
echo "5. Linker selection (tests 7-8, 10)"
echo "6. Timing and profile reports (test 9)"
echo "7. Multiple sequential compilations (test 15)"
```

This script comprehensively tests the uncovered initialization block by:

1. **Multiple Distinct Jobs**: Tests 1, 3, 10, and 11 compile multiple files in single invocations
2. **Sysroot Flags**: Tests 1-4 and 10 use `--sysroot=`, `--sysroot=/`, `--sysroot=/nonexistent`, `-isysroot`, and `-I` flags
3. **Dump File Options**: Tests 1-4, 7, 10, 11, and 15 use `-save-temps`, `-dumpdir`, `-dumpbase`, `-dumpbase-ext`, `-outbase`, and various `-fdump-*` flags
4. **Help/Version Output**: Tests 5-6, 10, and 12 use `--help`, `--version`, `--target-help`, `--help=common`, `--help=optimizers`
5. **Linker Selection**: Tests 7-8 and 10 use `-fuse-ld=bfd/gold/lld/mold` and `-Wl,` options
6. **PGO and Timing**: Test 9 implements full PGO workflow with `-fprofile-generate`, `-fprofile-use`, `-ftime-report`, and `-fprofile-report`
7. **Reinitialization**: Test 15 runs sequential compilations to ensure variables are reset between jobs

The script cleans up between tests to avoid interference and captures relevant output to verify the driver's behavior. The `-v` (verbose) flag helps confirm internal state changes, and error cases are handled gracefully to continue testing.
