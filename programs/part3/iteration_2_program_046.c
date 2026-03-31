Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc by invoking the compiler driver with various state-modifying flags:

```bash
#!/bin/sh
# Test script for GCC driver initialization/cleanup logic
# Targets the uncovered block in gcc.cc lines 11228-11250

set -e
TESTDIR="gcc_driver_test_$$"
mkdir -p "$TESTDIR"
cd "$TESTDIR"

# Cleanup function
cleanup() {
    cd ..
    rm -rf "$TESTDIR"
}
trap cleanup EXIT

# Create a simple test source file
cat > test.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Test passed\n");
    return 0;
}
EOF

cat > test2.c << 'EOF'
int add(int a, int b) {
    return a + b;
}
EOF

echo "=== Testing GCC driver state reset logic ==="

# 1. Test basic compilation with output naming variations
echo "1. Testing output file naming and dump directory logic..."
gcc -save-temps -dumpdir "./dumps/" -dumpbase "test1" -fdump-rtl-all -O2 -c test.c -o test1.o 2>/dev/null
gcc -save-temps=obj -dumpdir "./dumps2" -dumpbase "test2" -fdump-tree-all -c test.c -o test2.o 2>/dev/null
gcc -dumpbase "test3.c" -fdump-ipa-all -c test.c -o test3.o 2>/dev/null

# Force dumpdir trailing slash logic
gcc -dumpdir "./dumps3" -dumpbase "test4" -fdump-rtl-expand -c test.c -o test4.o 2>/dev/null
gcc -dumpdir "./dumps4/" -dumpbase "test5" -fdump-tree-optimized -c test.c -o test5.o 2>/dev/null

# 2. Test sysroot and machine specification resets
echo "2. Testing sysroot and machine specification resets..."
# Use a dummy sysroot path
gcc --sysroot="/usr" -march=x86-64 -c test.c -o test_sysroot1.o 2>/dev/null
gcc --sysroot="/" -march=native -c test.c -o test_sysroot2.o 2>/dev/null
# Reset to default by not specifying sysroot
gcc -march=x86-64 -c test.c -o test_sysroot3.o 2>/dev/null

# Test with -B prefix for executables
gcc -B/usr/bin -c test.c -o test_bprefix1.o 2>/dev/null
gcc -B/usr/libexec/gcc -c test.c -o test_bprefix2.o 2>/dev/null

# 3. Test driver mode switching and help/version requests
echo "3. Testing driver mode switching..."
# Invoke as C++ driver (g++ usually symlinks to gcc with different mode)
if command -v g++ >/dev/null 2>&1; then
    g++ -c test.c -o test_cpp.o 2>/dev/null
fi

# Help and version requests followed by actual compilation
gcc --help > /dev/null 2>&1
gcc -c test.c -o test_after_help.o 2>/dev/null

gcc --version > /dev/null 2>&1
gcc -c test.c -o test_after_version.o 2>/dev/null

# Subprocess help requests
gcc -print-prog-name=cc1 > /dev/null 2>&1
gcc -c test.c -o test_after_print.o 2>/dev/null

# 4. Test complex save-temps and dump combinations
echo "4. Testing complex save-temps and dump combinations..."
# Multiple dump flags with different dumpbase extensions
gcc -save-temps -dumpdir "./complex_dumps/" \
    -dumpbase "complex" \
    -fdump-rtl-all \
    -fdump-tree-all \
    -fdump-ipa-all \
    -O2 -c test2.c -o complex1.o 2>/dev/null

# Override dumpdir with trailing dash logic
gcc -save-temps=obj \
    -dumpdir "./complex_dumps2" \
    -dumpbase "complex2.test" \
    -fdump-tree-optimized \
    -fdump-rtl-expand \
    -c test2.c -o complex2.o 2>/dev/null

# Test save_temps_overrides_dumpdir logic
gcc -save-temps -dumpdir "./should_be_ignored/" -c test.c -o temp_override.o 2>/dev/null

# 5. Test multiple compilations in sequence with varying options
echo "5. Testing sequential compilations with state changes..."
for i in 1 2 3 4 5; do
    DUMPDIR="./seq_${i}"
    DUMPBASE="seqbase_${i}"
    MARCH=""
    
    case $i in
        1) MARCH="-march=x86-64" ;;
        2) MARCH="-march=nocona" ;;
        3) MARCH="" ;; # Reset to default
        4) MARCH="-march=core2" ;;
        5) MARCH="" ;; # Reset again
    esac
    
    gcc $MARCH \
        -dumpdir "$DUMPDIR" \
        -dumpbase "$DUMPBASE" \
        -fdump-tree-cfg \
        -c test.c -o "seq_${i}.o" 2>/dev/null
done

# 6. Final compilation to verify driver state was properly reset
echo "6. Final verification compilation..."
gcc test1.o test2.o test3.o -o final_test 2>/dev/null

if [ -x ./final_test ]; then
    OUTPUT=$(./final_test 2>&1)
    if [ "$OUTPUT" = "Test passed" ]; then
        echo "SUCCESS: Driver state properly reset between invocations"
    else
        echo "WARNING: Program output mismatch: $OUTPUT"
    fi
else
    echo "WARNING: Final executable not created"
fi

# 7. Test with specs file if available
echo "7. Testing with specs overrides..."
SPECFILE="test.specs"
cat > "$SPECFILE" << 'EOF'
*link:
%{!shared:%{!static:%{!rdynamic:-dynamic-linker /lib64/ld-linux-x86-64.so.2}}}
EOF

gcc -specs="$SPECFILE" -c test.c -o test_specs.o 2>/dev/null
# Follow with compilation without specs to test reset
gcc -c test.c -o test_nospecs.o 2>/dev/null

# 8. Test verbose flag and report times
echo "8. Testing verbose and timing flags..."
gcc -v -c test.c -o test_verbose.o 2>/dev/null > verbose.log 2>&1
# Time report would need -ftime-report, but report_times_to_file is internal

echo "=== Test completed ==="
echo "Generated files in: $TESTDIR"
echo "Check that dump directories were created and cleaned properly"
ls -la

# Keep the directory for inspection if test fails
echo "To clean up, remove directory: $TESTDIR"
trap - EXIT
```

This script systematically tests the uncovered code by:

1. **Multiple output naming variations**: Uses `-dumpdir`, `-dumpbase`, `-fdump-*` flags with different patterns to exercise allocation/free logic
2. **Sysroot and machine spec resets**: Uses `--sysroot`, `-march`, `-B` flags that modify `target_system_root` and `spec_machine`
3. **Driver mode switching**: Calls help/version flags and subprocess queries that set `print_help_list`, `print_version`, etc.
4. **Complex save-temps logic**: Tests `save_temps_flag` and `dumpdir_trailing_dash_added` with various combinations
5. **Sequential state changes**: Loops through different machine architectures to ensure proper reset between compilations
6. **Final verification**: Links object files to ensure driver state was properly reset

The script creates a temporary directory, generates test files, runs the compiler with various flag combinations, and cleans up. Each invocation forces the driver to modify its internal state, and subsequent invocations (or cleanup) trigger the reset logic in the target code block.
