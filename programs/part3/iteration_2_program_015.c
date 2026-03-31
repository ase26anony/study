Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc by invoking the compiler driver with various state-modifying options:

```bash
#!/bin/sh

# Test for gcc.cc uncovered lines 11228-11250
# This script triggers driver state initialization and cleanup cycles

set -e
TESTDIR=$(mktemp -d /tmp/gcc_driver_test_XXXXXX)
cd "$TESTDIR"
echo "Test directory: $TESTDIR"

# Cleanup function
cleanup() {
    echo "Cleaning up test directory..."
    cd /
    rm -rf "$TESTDIR"
}
trap cleanup EXIT

# Create a simple test source file
cat > test.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Driver test passed\n");
    return 0;
}
EOF

# Create a second test file for multiple compilations
cat > test2.c << 'EOF'
int add(int a, int b) {
    return a + b;
}
EOF

echo "=== Phase 1: Basic compilation with dump and output options ==="

# 1. Exercise dumpdir/dumpbase allocation and freeing
# This triggers free(dumpdir), free(dumpbase), etc.
gcc -save-temps -dumpdir ./dumps/ -dumpbase test1 -fdump-rtl-all -O2 -c test.c -o test1.o 2>/dev/null || true

# 2. Different dumpbase and dumpdir (tests dumpdir_trailing_dash_added)
gcc -save-temps -dumpdir ./dumps2 -dumpbase test2 -fdump-tree-all -c test.c -o test2.o 2>/dev/null || true

# 3. With dumpbase_ext (implicit from source name)
gcc -save-temps=obj -dumpdir ./out/ -fdump-ipa-all -c test.c -o test3.o 2>/dev/null || true

# 4. Explicit dumpbase with extension
gcc -dumpbase test4.c -dumpdir ./ -fdump-rtl-expand -c test.c -o test4.o 2>/dev/null || true

echo "=== Phase 2: System root and machine specification ==="

# 5. Modify target_system_root and spec_machine
# Note: Using / as sysroot since it should exist on all systems
gcc --sysroot=/ -march=x86-64 -c test.c -o test5.o 2>/dev/null || true

# 6. Different machine spec
gcc -march=native -specs=/dev/null -c test.c -o test6.o 2>/dev/null || true

# 7. Reset to defaults (should trigger DEFAULT_TARGET_MACHINE reset)
gcc -c test.c -o test7.o 2>/dev/null || true

echo "=== Phase 3: Driver mode switching and help/version ==="

# 8. Invoke as C++ driver (tests is_cpp_driver)
g++ -c test.c -o test8.o 2>/dev/null || true

# 9. Help request (tests print_help_list)
gcc --help > /dev/null 2>&1 || true

# 10. Version request (tests print_version)
gcc --version > /dev/null 2>&1 || true

# 11. Subprocess help (tests print_subprocess_help)
gcc -print-prog-name=cc1 > /dev/null 2>&1 || true

# 12. Verbose flag (tests verbose_only_flag)
gcc -v -c test.c -o test9.o 2>/dev/null || true

echo "=== Phase 4: Complex save-temps and dump combinations ==="

# 13. Multiple save-temps modes
gcc -save-temps=cwd -dumpdir ./complex/ -dumpbase complex -fdump-tree-all -fdump-rtl-all -c test.c -o test10.o 2>/dev/null || true

# 14. Override dumpdir with save-temps (tests save_temps_overrides_dumpdir)
gcc -save-temps -dumpdir ./overridden/ -c test.c -o test11.o 2>/dev/null || true

# 15. Multiple compilations in sequence to stress cleanup
for i in 12 13 14 15; do
    gcc -save-temps -dumpdir "./iter$i/" -dumpbase "iter$i" -c test.c -o "test${i}.o" 2>/dev/null || true
    gcc -dumpbase "iter${i}b" -c test2.c -o "test${i}b.o" 2>/dev/null || true
done

echo "=== Phase 5: Final compilation and verification ==="

# 16. Final compilation to verify driver state is properly reset
gcc test.c -o final_test
./final_test

# 17. One more with all options to ensure cleanup happens
gcc -save-temps -dumpdir ./final/ -dumpbase final -fdump-rtl-all --sysroot=/ -march=x86-64 -v -c test.c -o final.o 2>/dev/null || true

echo "=== Test completed successfully ==="
echo "Generated files in $TESTDIR:"
ls -la "$TESTDIR" | head -20

# Verify the specific dump files were created (indicating dumpdir/dumpbase worked)
if [ -d "./dumps" ] && [ -f "./dumps/test1.c.237r.expand" ] 2>/dev/null; then
    echo "✓ Dump directory and files created successfully"
fi

if [ -f "./final_test" ]; then
    echo "✓ Final executable created and runs correctly"
fi
```

**Key aspects that trigger the uncovered code:**

1. **Multiple allocations/frees of dumpdir/dumpbase**: Each `-dumpdir` and `-dumpbase` option causes allocations, and the cleanup between invocations triggers the `free()` calls in lines 11241-11244.

2. **System root and machine spec resets**: Using `--sysroot=/` modifies `target_system_root`, and subsequent compilations without it trigger reset to `DEFAULT_TARGET_SYSTEM_ROOT`. Similarly for `-march` and `spec_machine`.

3. **Driver mode flags**: `--help`, `--version`, `-print-prog-name=` set their respective flags (`print_help_list`, `print_version`, `print_subprocess_help`) which must be reset.

4. **Save-temps variations**: Different `-save-temps` modes exercise `save_temps_flag` and `save_temps_overrides_dumpdir`.

5. **Trailing slash handling**: Using `-dumpdir ./dumps/` (with slash) and `-dumpdir ./dumps2` (without) tests `dumpdir_trailing_dash_added`.

6. **Multiple sequential invocations**: The loop and multiple calls ensure the driver's internal state is modified and must be cleaned up between jobs.

The script creates a temporary directory, generates test files, runs the compiler with various flag combinations, and cleans up after itself. Each compiler invocation modifies the driver's global state, and the cleanup between them (or at program exit) should execute the target code block.
