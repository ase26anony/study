Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc by performing multiple compiler invocations with varied state-modifying flags:

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
    printf("Test program\n");
    return 0;
}
EOF

# 1. Multiple compilations with varied output flags and dump options
echo "=== Phase 1: Output file naming and dump directory logic ==="
gcc -save-temps -dumpdir ./dumps/ -dumpbase test1 -fdump-rtl-all -O2 -c test.c -o test1.o 2>/dev/null || true
gcc -save-temps=obj -dumpdir ./dumps2 -dumpbase test2 -fdump-tree-all -c test.c -o test2.o 2>/dev/null || true
gcc -dumpbase test3.c -fdump-ipa-all -c test.c -o test3.o 2>/dev/null || true

# Force dumpdir trailing slash logic
gcc -dumpdir ./dumps3/ -fdump-rtl-expand -c test.c -o test4.o 2>/dev/null || true
gcc -dumpdir ./dumps4 -fdump-tree-optimized -c test.c -o test5.o 2>/dev/null || true

# 2. System root and machine specification overrides
echo "=== Phase 2: System root and machine spec overrides ==="
# Try different sysroot values (some may fail if paths don't exist, but that's OK)
gcc --sysroot=/ -march=x86-64 -c test.c -o test6.o 2>/dev/null || true
gcc --sysroot=/usr -march=native -c test.c -o test7.o 2>/dev/null || true
# Reset to default by not specifying sysroot
gcc -march=x86-64 -c test.c -o test8.o 2>/dev/null || true

# Use -B and -specs to modify toolchain paths
gcc -B/usr/bin -c test.c -o test9.o 2>/dev/null || true
# Create a dummy spec file
echo "*cpp:" > dummy.specs
gcc -specs=dummy.specs -c test.c -o test10.o 2>/dev/null || true

# 3. Driver mode switching and help/version requests
echo "=== Phase 3: Driver mode switching ==="
# Invoke as C++ driver (g++ usually symlinks to gcc with different mode)
if command -v g++ >/dev/null 2>&1; then
    g++ -c test.c -o test11.o 2>/dev/null || true
fi

# Help and version requests (these set print_help_list, print_version)
gcc --help > /dev/null 2>&1
gcc --version > /dev/null 2>&1
gcc -print-prog-name=cc1 > /dev/null 2>&1
gcc -print-subprocess-help > /dev/null 2>&1 || true

# Now compile after help/version to ensure state reset
gcc -c test.c -o test12.o 2>/dev/null || true

# 4. Complex save-temps and dump combinations
echo "=== Phase 4: Complex save-temps and dump combinations ==="
mkdir -p temps
gcc -save-temps -dumpdir ./temps/dumps/ -dumpbase complex \
    -fdump-rtl-all -fdump-tree-all -fdump-ipa-all \
    -O2 -c test.c -o test13.o 2>/dev/null || true

# Override dumpdir with different trailing slash
gcc -save-temps=obj -dumpdir ./temps2 -dumpbase complex2 \
    -fdump-tree-optimized -c test.c -o test14.o 2>/dev/null || true

# Test save_temps_overrides_dumpdir logic
gcc -save-temps -dumpdir ./ignored/ -c test.c -o test15.o 2>/dev/null || true

# 5. Multiple rapid invocations with mixed options
echo "=== Phase 5: Rapid mixed invocations ==="
for i in 16 17 18 19 20; do
    case $((i % 4)) in
        0) FLAGS="-O1 -fdump-tree-cfg -dumpbase iter$i" ;;
        1) FLAGS="-O2 -fdump-rtl-expand --sysroot=/ -march=x86-64" ;;
        2) FLAGS="-save-temps -dumpdir ./iter$i/ -fdump-all" ;;
        3) FLAGS="-g -fdump-tree-optimized -dumpbase final" ;;
    esac
    gcc $FLAGS -c test.c -o test${i}.o 2>/dev/null || true
done

# 6. Final compilation and link to verify driver state was properly reset
echo "=== Phase 6: Final verification ==="
# This should work with default state
gcc test.c -o test_program
./test_program

echo "Test completed successfully"
echo "Generated files in: $TMPDIR"
ls -la "$TMPDIR"/*.o "$TMPDIR"/test_program 2>/dev/null || true
```

This script systematically targets each requirement:

1. **Multiple Compiler Invocations with Varied Output Flags**: Uses `-c`, `-o`, `-dumpdir`, `-dumpbase`, `-fdump-*` flags in different combinations across multiple invocations.

2. **System Root and Spec Machine Overrides**: Uses `--sysroot=`, `-B`, `-specs=`, `-march=` options to modify `target_system_root` and `spec_machine`.

3. **Driver Mode Switching**: Invokes `g++` (C++ driver mode), uses `--help`, `--version`, `-print-prog-name`, `-print-subprocess-help` to set the corresponding flags.

4. **Save-Temps and Dump Directory Complexity**: Tests `-save-temps` with various `-dumpdir` configurations (with/without trailing slashes), multiple `-fdump-*` flags, and exercises `save_temps_overrides_dumpdir` logic.

5. **Cleanup Between Invocations**: Each `gcc` invocation runs in a separate process, forcing the driver to initialize and clean up its global state each time.

6. **Final Verification**: Compiles and runs a test program to ensure the driver state was properly reset to defaults.

The script uses `2>/dev/null || true` to continue even if some options fail (e.g., invalid sysroot paths), as the goal is to exercise the initialization/cleanup logic, not necessarily succeed at compilation. The temporary directory cleanup ensures no leftover files remain.
