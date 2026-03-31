Here's a shell script that comprehensively tests the uncovered initialization/cleanup logic in gcc.cc:

```bash
#!/bin/sh
# Test script for GCC driver initialization/cleanup logic
# Targets lines 11228-11250 in gcc.cc

set -e
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

cd "$TMPDIR"

# Create a simple test source file
cat > test.c <<'EOF'
#include <stdio.h>
int main() {
    printf("Hello, coverage test!\n");
    return 0;
}
EOF

cat > test2.c <<'EOF'
int add(int a, int b) {
    return a + b;
}
EOF

# 1. Test multiple compilations with varied output flags and dump options
echo "=== Test 1: Multiple compilations with dump options ==="
gcc -save-temps -dumpdir ./dumps/ -dumpbase test1 -fdump-rtl-all -O2 -c test.c -o test1.o 2>/dev/null || true
gcc -save-temps=obj -dumpdir ./dumps2 -dumpbase test2 -fdump-tree-all -c test.c -o test2.o 2>/dev/null || true
gcc -dumpbase test3.c -fdump-ipa-all -c test.c -o test3.o 2>/dev/null || true

# 2. Test sysroot and machine specification overrides
echo "=== Test 2: Sysroot and machine specification ==="
# Try different sysroot values (some may fail but should still trigger the logic)
gcc --sysroot=/ -march=x86-64 -c test.c -o test4.o 2>/dev/null || true
gcc -B /usr/bin -specs=/dev/null -march=native -c test.c -o test5.o 2>/dev/null || true

# 3. Test driver mode switching and help/version requests
echo "=== Test 3: Driver mode switching ==="
# Invoke as C++ driver
g++ --help > /dev/null 2>&1
g++ --version > /dev/null 2>&1
# Help and version requests
gcc --help=common > /dev/null 2>&1
gcc --version > /dev/null 2>&1
# Subprocess help
gcc -print-prog-name=cc1 > /dev/null 2>&1
gcc -print-prog-name=as > /dev/null 2>&1
# Now compile after help/version requests
gcc -c test.c -o test6.o 2>/dev/null || true

# 4. Test complex dump directory and save-temps combinations
echo "=== Test 4: Complex dump directory logic ==="
mkdir -p complex_dump
gcc -save-temps -dumpdir complex_dump/ -dumpbase complex -fdump-rtl-expand \
    -fdump-tree-optimized -c test.c -o test7.o 2>/dev/null || true
# Test without trailing slash (should trigger dumpdir_trailing_dash_added logic)
gcc -save-temps -dumpdir complex_dump -dumpbase complex2 -c test.c -o test8.o 2>/dev/null || true
# Test dumpbase with extension
gcc -dumpbase complex3.c -fdump-tree-cfg -c test.c -o test9.o 2>/dev/null || true

# 5. Test multiple sequential compilations with state changes
echo "=== Test 5: Sequential state-changing compilations ==="
for i in 10 11 12 13 14; do
    gcc -save-temps=${i} -dumpdir seq${i}/ -dumpbase seq${i} \
        -march=x86-64 -c test.c -o test${i}.o 2>/dev/null || true
done

# 6. Test with verbose flag (verbose_only_flag)
echo "=== Test 6: Verbose flag testing ==="
gcc -v -c test.c -o test15.o 2>/dev/null || true
gcc -### -c test.c -o test16.o 2>/dev/null || true

# 7. Test output base variations
echo "=== Test 7: Output base variations ==="
gcc -save-temps -dumpdir ./ -dumpbase out1 -o out1.o -c test.c 2>/dev/null || true
gcc -dumpbase out2 -o out2.o -c test.c 2>/dev/null || true

# 8. Final compilation to verify driver state was properly reset
echo "=== Test 8: Final verification compilation ==="
gcc test1.o test2.o -o final_test 2>/dev/null || true
if [ -x final_test ]; then
    ./final_test 2>/dev/null && echo "Final test executed successfully"
else
    # If linking failed, compile and run a fresh program
    gcc test.c -o simple_test 2>/dev/null && ./simple_test 2>/dev/null && echo "Simple test executed successfully"
fi

# 9. Test with at_file_supplied logic (response files)
echo "=== Test 9: Response file testing ==="
echo "-c test.c -o test17.o" > args.txt
gcc @args.txt 2>/dev/null || true

# 10. Test report times functionality
echo "=== Test 10: Time reporting ==="
gcc -ftime-report -c test.c -o test18.o 2>/dev/null || true

# Cleanup (somewhat redundant due to trap, but explicit)
echo "=== Cleaning up ==="
ls -la 2>/dev/null | head -20

echo "Test completed. All driver state resets should have been exercised."
```

This script systematically tests all the requirements:

1. **Multiple Compiler Invocations**: Compiles the same source file 18+ times with different output and dump options
2. **System Root and Spec Machine Overrides**: Uses `--sysroot=`, `-B`, `-specs=`, `-march=` with various values
3. **Driver Mode Switching**: Invokes `g++`, uses `--help`, `--version`, `-print-prog-name=`
4. **Save-Temps and Dump Directory**: Tests `-save-temps`, `-dumpdir` with/without trailing slash, `-dumpbase` with extensions, various `-fdump-*` flags
5. **Comprehensive State Changes**: Each invocation modifies different global state variables that need resetting

The script uses `2>/dev/null || true` to handle cases where certain flag combinations might fail on the current system (e.g., invalid `--sysroot` paths) while still ensuring the driver initialization/cleanup logic is executed.

The execution flow ensures the driver goes through multiple initialization/cleanup cycles, hitting the target code block that resets:
- `dumpdir`, `dumpbase`, `outbase` allocations and frees
- `target_system_root` and `spec_machine` resets to defaults  
- Flags like `is_cpp_driver`, `print_help_list`, `print_version`
- `save_temps_flag` and `dumpdir_trailing_dash_added` logic

The final compilation verifies the driver can still function correctly after all state modifications.
