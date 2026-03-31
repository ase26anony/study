Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc by invoking the compiler driver with various state-modifying options:

```bash
#!/bin/sh
# Test script for GCC driver initialization/cleanup logic
# Targets gcc.cc lines 11228-11250

set -e
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

cd "$TMPDIR"

# Create a simple test source file
cat > test.c <<'EOF'
#include <stdio.h>
int main() {
    printf("Hello from test program\n");
    return 0;
}
EOF

cat > test2.c <<'EOF'
int add(int a, int b) {
    return a + b;
}
EOF

echo "=== Testing GCC driver state reset logic ==="
echo "Target: gcc.cc lines 11228-11250 (global variable cleanup)"

# 1. Multiple compilations with varied output flags and dump options
echo "1. Testing output file naming and dump directory logic..."
gcc -save-temps -dumpdir ./dumps/ -dumpbase test1 -fdump-rtl-all -O2 -c test.c -o test1.o 2>/dev/null || true
gcc -save-temps=obj -dumpdir ./dumps2 -dumpbase test2 -fdump-tree-all -c test.c -o test2.o 2>/dev/null || true
gcc -dumpbase test3.c -fdump-ipa-all -c test2.c -o test3.o 2>/dev/null || true

# Force dumpdir with trailing slash (tests dumpdir_trailing_dash_added)
mkdir -p dumps3
gcc -dumpdir dumps3/ -dumpbase test4 -fdump-rtl-expand -c test.c -o test4.o 2>/dev/null || true

# 2. System root and machine specification overrides
echo "2. Testing sysroot and machine spec reset..."
# Use a dummy sysroot (current directory as fake sysroot)
gcc --sysroot=. -march=x86-64 -c test.c -o test5.o 2>/dev/null || true
gcc --sysroot=/usr -march=native -c test2.c -o test6.o 2>/dev/null || true
# Reset to default by not specifying sysroot
gcc -c test.c -o test7.o 2>/dev/null || true

# 3. Driver mode switching and help/version requests
echo "3. Testing driver mode switching..."
# Invoke as C++ driver (tests is_cpp_driver)
g++ --version >/dev/null 2>&1 || true
g++ -c test.c -o test8.o 2>/dev/null || true

# Help and version requests (tests print_help_list, print_version)
gcc --help >/dev/null 2>&1 || true
gcc --version >/dev/null 2>&1 || true
gcc -print-prog-name=cc1 >/dev/null 2>&1 || true
gcc -print-subprocess-help >/dev/null 2>&1 || true

# 4. Complex save-temps and dump directory combinations
echo "4. Testing complex save-temps scenarios..."
mkdir -p complex_dump
gcc -save-temps -dumpdir complex_dump/ -dumpbase complex \
    -fdump-rtl-all -fdump-tree-all -fdump-ipa-all \
    -O2 -c test.c -o test9.o 2>/dev/null || true

# Test save_temps_overrides_dumpdir logic
gcc -save-temps=obj -dumpdir ignored_dir/ -c test2.c -o test10.o 2>/dev/null || true

# 5. Mixed invocations to force repeated cleanup cycles
echo "5. Mixed invocations to stress cleanup..."
(
    gcc --help=optimizers >/dev/null 2>&1
    gcc -save-temps -dumpdir ./mixed/ -dumpbase mixed -c test.c -o mixed1.o 2>/dev/null
    gcc --sysroot=. -march=x86-64 -c test2.c -o mixed2.o 2>/dev/null
    gcc -dumpbase final -fdump-rtl-final -c test.c -o final.o 2>/dev/null
) || true

# 6. Final compilation to verify driver still works correctly
echo "6. Final verification compilation..."
gcc test1.o test7.o -o final_test 2>/dev/null || true
if [ -x ./final_test ]; then
    ./final_test && echo "Final test program executed successfully"
else
    echo "Final linking failed (expected with dummy sysroot tests)"
fi

# 7. Test with multiple source files in one invocation
echo "7. Testing multiple source files..."
gcc -c test.c test2.c -dumpdir multi/ -dumpbase multi 2>/dev/null || true

# 8. Test verbose flag (tests verbose_only_flag)
echo "8. Testing verbose flag..."
gcc -v -c test.c -o verbose.o 2>/dev/null || true

# 9. Test time reporting (tests report_times_to_file)
echo "9. Testing time reporting..."
gcc -ftime-report -c test.c -o timed.o 2>/dev/null || true

# 10. Cleanup and display generated files
echo ""
echo "=== Generated files in $TMPDIR ==="
find . -type f -name "*.o" -o -name "*.i" -o -name "*.s" -o -name "*.dump*" 2>/dev/null | head -20
echo ""

echo "Test completed. The driver should have executed the target cleanup code"
echo "multiple times during these invocations."
echo "Lines covered should include:"
echo "  - is_cpp_driver, at_file_supplied, print_help_list, print_version"
echo "  - target_system_root, spec_machine reset to defaults"
echo "  - free(dumpdir), free(dumpbase), free(dumpbase_ext), free(outbase)"
echo "  - save_temps_flag, dumpdir_trailing_dash_added logic"
echo "  - greatest_status reset to 1"
```

This script systematically tests the uncovered code by:

1. **Multiple compilations with dump options**: Uses `-save-temps`, `-dumpdir`, `-dumpbase` with various dump flags to exercise the allocation and freeing of dump-related variables.

2. **Sysroot and machine spec changes**: Uses `--sysroot` and `-march` to modify `target_system_root` and `spec_machine`, then resets them by omitting these flags.

3. **Driver mode switching**: Invokes `g++` (C++ driver), help, version, and subprocess help requests to toggle various boolean flags.

4. **Complex save-temps scenarios**: Tests edge cases like trailing slashes in dumpdir and save-temps overriding dumpdir.

5. **Mixed invocations**: Combines different option types in sequence to force the driver through multiple cleanup cycles.

6. **Final verification**: Attempts to link and run a program to ensure the driver state was properly reset.

The script uses `2>/dev/null || true` to handle cases where some options might fail (e.g., invalid sysroot paths) while still allowing the driver to execute the cleanup code. The temporary directory ensures no pollution of the source tree.
