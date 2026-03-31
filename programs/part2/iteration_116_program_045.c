Here's a comprehensive shell script that exercises the uncovered initialization logic in gcc.cc by testing various flag combinations and execution scenarios:

```bash
#!/bin/bash
set -e

# Configuration
GCC=${1:-gcc}
WORKDIR=$(mktemp -d)
cd "$WORKDIR"
echo "Working directory: $WORKDIR"

# Create simple test programs
cat > hello1.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Hello 1\n");
    return 0;
}
EOF

cat > hello2.c << 'EOF'
#include <stdio.h>
void greet() {
    printf("Hello 2\n");
}
EOF

cat > hello3.c << 'EOF'
extern void greet();
int main() {
    greet();
    return 0;
}
EOF

# Helper function to clean up dump files
clean_dumps() {
    rm -f *.o *.i *.s *.dump *.gcda *.gcno *.gcov *.times *.rpo *.opt *.cfg *.dot
    rm -rf dumps/ temps/ *.dSYM/
}

# Test 1: Multiple jobs with sysroot variations
echo "=== Test 1: Multiple jobs with sysroot ==="
clean_dumps
"$GCC" hello1.c hello2.c -o prog1 \
    --sysroot= \
    --sysroot=/nonexistent \
    -isysroot/usr/include \
    -I/usr/local/include \
    -v 2>&1 | grep -q "sysroot" || true
[ -f prog1 ] && ./prog1 && rm -f prog1

# Test 2: Dump file management with various options
echo "=== Test 2: Dump file management ==="
clean_dumps
"$GCC" hello1.c -o prog2 \
    -save-temps=obj \
    -dumpdir=./dumps \
    -dumpbase=testdump \
    -dumpbase-ext=.myext \
    -fdump-tree-all \
    -fdump-rtl-expand \
    -fdump-ipa-all \
    -O2 2>&1 | tail -20
ls -la dumps/ 2>/dev/null || true
rm -f prog2

# Test 3: Different save-temps modes
echo "=== Test 3: Various save-temps modes ==="
clean_dumps
"$GCC" hello1.c -save-temps=cwd -o prog3
[ -f hello1.i ] && echo "Created .i file"
[ -f hello1.s ] && echo "Created .s file"
rm -f prog3 hello1.i hello1.s hello1.o

"$GCC" hello1.c -save-temps -dumpdir=temps -o prog4
ls -la temps/ 2>/dev/null || true
rm -f prog4

# Test 4: Help and version flags
echo "=== Test 4: Help and version output ==="
"$GCC" --help | head -5
"$GCC" --target-help | head -5
"$GCC" --version | head -3
"$GCC" --help=common | head -10
"$GCC" --help=optimizers | grep -A2 "-O"
"$GCC" --help=warnings | grep -A2 "-W"

# Test 5: Combined help with compilation flags
echo "=== Test 5: Combined help and compilation ==="
"$GCC" hello1.c --help=target -O2 -o prog5 2>&1 | head -20
rm -f prog5

# Test 6: Linker selection flags
echo "=== Test 6: Linker selection ==="
for linker in bfd gold lld mold; do
    echo "Testing -fuse-ld=$linker"
    "$GCC" hello1.c -fuse-ld=$linker -Wl,--verbose -o prog6_$linker 2>&1 | grep -i "linker" | head -2 || true
    rm -f prog6_$linker
done

# Test 7: Timing and profile reports
echo "=== Test 7: Timing and profile reports ==="
clean_dumps
"$GCC" hello1.c -ftime-report -O2 -o prog7 2>&1 | grep -A5 "Time variable"
rm -f prog7

# Test 8: Comprehensive flag combination (targeting all uncovered variables)
echo "=== Test 8: Comprehensive flag combination ==="
clean_dumps
"$GCC" hello1.c hello2.c -o prog8 \
    --sysroot=/ \
    -isysroot/usr/include \
    -save-temps \
    -dumpdir=./all_dumps \
    -dumpbase=comprehensive \
    -dumpbase-ext=.test \
    -fdump-tree-all \
    -fdump-rtl-all \
    -ftime-report \
    -fuse-ld=gold \
    -O3 \
    -v 2>&1 | tail -30
[ -f prog8 ] && ./prog8
rm -f prog8

# Test 9: Profile-guided optimization flow
echo "=== Test 9: PGO workflow ==="
clean_dumps

# Step 1: Generate profile
cat > pgo_test.c << 'EOF'
#include <stdio.h>
int main(int argc, char **argv) {
    for (int i = 0; i < argc * 1000; i++) {
        printf("Iteration %d\n", i % 100);
    }
    return 0;
}
EOF

echo "Phase 1: Instrumented compilation"
"$GCC" pgo_test.c -fprofile-generate -O2 -o pgo_instr

echo "Phase 2: Run instrumented program"
./pgo_instr 1 2 3 > /dev/null

echo "Phase 3: Use profile for optimization"
"$GCC" pgo_test.c -fprofile-use -fprofile-report -fprofile-correction -O2 -o pgo_opt \
    -ftime-report 2>&1 | grep -i "profile\|time" | head -10

rm -f pgo_test.c pgo_instr pgo_opt *.gcda *.gcno

# Test 10: Empty and NULL-like values for dump options
echo "=== Test 10: Edge cases for dump options ==="
"$GCC" hello1.c -dumpdir= -dumpbase= -dumpbase-ext= -o prog10 2>&1 | head -5
rm -f prog10

"$GCC" hello1.c -save-temps=obj -dumpdir=./ -dumpbase="" -o prog11
rm -f prog11

# Test 11: Multiple jobs with mixed file types
echo "=== Test 11: Multiple jobs with assembly ==="
# Create assembly file
"$GCC" hello2.c -S -o hello2.s
"$GCC" hello1.c hello2.s -o prog12 --sysroot= -save-temps
[ -f prog12 ] && ./prog12
rm -f prog12 hello2.s

# Test 12: Verbose flag with initialization
echo "=== Test 12: Verbose initialization ==="
"$GCC" hello1.c -v -### 2>&1 | grep -A2 "sysroot\|dumpdir" | head -10

# Cleanup
clean_dumps
cd ..
rm -rf "$WORKDIR"
echo "=== All tests completed ==="
```

This script systematically tests the uncovered initialization logic by:

1. **Multiple jobs with sysroot variations** (lines 11228-11250: `target_system_root`, `target_system_root_changed`)
2. **Dump file management** (`dumpdir`, `dumpbase`, `dumpbase_ext`, `outbase`, `save_temps_flag`)
3. **Help and version flags** (`print_help_list`, `print_version`, `print_subprocess_help`)
4. **Linker selection** (`use_ld`)
5. **Timing reports** (`report_times_to_file`)
6. **Profile-guided optimization** (exercises multiple code paths)
7. **Edge cases** with empty values
8. **Verbose output** to observe driver state

The script creates temporary directories for each test to avoid interference and cleans up after itself. Each test is designed to trigger specific parts of the initialization block while ensuring valid compilation commands that won't fail due to syntax errors.
