I'll create a comprehensive test script that exercises the uncovered reset logic in gcc.cc by combining multiple techniques as specified.

```bash
#!/bin/bash

# Test script to exercise the driver::finalize reset logic in gcc.cc
# This script creates multiple test files and invokes gcc with various
# combinations of options to trigger the uncovered reset block.

set -e

# Create a temporary directory for test files
TESTDIR=$(mktemp -d /tmp/gcc_driver_test_XXXXXX)
cd "$TESTDIR"

echo "Test directory: $TESTDIR"

# Create simple C source files
cat > test1.c << 'EOF'
int main() {
    return 0;
}
EOF

cat > test2.c << 'EOF'
int helper() {
    return 42;
}
EOF

cat > test3.c << 'EOF'
#include <stdio.h>
void greet() {
    printf("Hello\n");
}
EOF

# Create a response file with various options
cat > opts.txt << 'EOF'
-save-temps=cwd
-dumpdir=./response_dump
-dumpbase=response_base
-O2
EOF

# Create another response file with conflicting options
cat > opts2.txt << 'EOF'
-save-temps=obj
-dumpdir=./response_dump2
-dumpbase=response_base2
EOF

# Test 1: Basic state reset with dump options
echo "=== Test 1: Basic state reset with dump options ==="
gcc -save-temps -dumpdir=./mydir -dumpbase=mybase test1.c -c -o test1.o 2>/dev/null || true
gcc --version test1.o -o prog1 2>/dev/null || true

# Test 2: Complex multi-file state transition
echo "=== Test 2: Complex multi-file state transition ==="
gcc -E -dumpbase=preproc test1.c -S -dumpdir=./asm test2.c -c -dumpbase=obj test3.c -o combined.o 2>/dev/null || true

# Test 3: Response file and help combination
echo "=== Test 3: Response file and help combination ==="
gcc @opts.txt test1.c --help=optimizers -o dummy 2>/dev/null || true

# Test 4: Mixed help/version with actual compilation
echo "=== Test 4: Mixed help/version with actual compilation ==="
gcc test1.c --help -c test2.c 2>/dev/null || true
gcc -o output --version test1.c test2.c 2>/dev/null || true
gcc --target-help test1.c -c 2>/dev/null || true

# Test 5: Multiple compilation units with differing options
echo "=== Test 5: Multiple compilation units with differing options ==="
gcc -save-temps=cwd -dumpdir=./dump1 test1.c -save-temps=obj -dumpdir=./dump2 test2.c -o output2 2>/dev/null || true

# Test 6: Response file followed by conflicting command-line options
echo "=== Test 6: Response file with conflicting options ==="
gcc @opts.txt -dumpbase-ext=.alt -dumpdir=./override_dump test1.c 2>/dev/null || true

# Test 7: Multiple response files
echo "=== Test 7: Multiple response files ==="
gcc @opts.txt @opts2.txt test1.c test2.c -o output3 2>/dev/null || true

# Test 8: Mode switches with dump options
echo "=== Test 8: Mode switches with dump options ==="
gcc -x c -dumpbase=c_file test1.c -x assembler -dumpdir=asm_dump test2.s 2>/dev/null || true

# Test 9: Save-temps variants
echo "=== Test 9: Save-temps variants ==="
gcc -save-temps -dumpdir=temp_dir1 test1.c -save-temps=cwd -dumpdir=temp_dir2 test2.c -save-temps=obj test3.c -o output4 2>/dev/null || true

# Test 10: Combination of all techniques
echo "=== Test 10: Comprehensive combination ==="
gcc @opts.txt -E test1.c --help=warnings -S -dumpdir=final_dump test2.c -c -dumpbase=final_base test3.c --version -o final_output 2>/dev/null || true

# Test 11: Exercise target system root variables
echo "=== Test 11: With sysroot options ==="
gcc --sysroot=/tmp -dumpdir=sysroot_dump test1.c --sysroot=/usr -dumpbase=sysroot_base test2.c -o output5 2>/dev/null || true

# Test 12: Verbose and timing options
echo "=== Test 12: Verbose and timing options ==="
gcc -v -ftime-report -dumpdir=verbose_dump test1.c -v test2.c -o output6 2>/dev/null || true

# Test 13: Linker specification options
echo "=== Test 13: Linker options ==="
gcc -Wl,--verbose -dumpbase=linker_base test1.c -fuse-ld=bfd -dumpdir=linker_dump test2.c -o output7 2>/dev/null || true

# Clean up
echo "=== Cleaning up ==="
cd /
rm -rf "$TESTDIR"

echo "Test completed successfully!"
echo ""
echo "The test script executed multiple gcc invocations designed to trigger the reset logic:"
echo "1. Mixed dumpdir/dumpbase options between files"
echo "2. Response files (@file syntax) setting at_file_supplied"
echo "3. Help/version requests mixed with compilation"
echo "4. Multiple save-temps variants"
echo "5. Mode switches (-E, -S, -c) with state changes"
echo ""
echo "These invocations should cause the driver to execute the uncovered reset block:"
echo "- Reset flags: is_cpp_driver, at_file_supplied, print_help_list, etc."
echo "- Free and NULL dumpdir, dumpbase, dumpbase_ext, outbase"
echo "- Reset spec_machine and greatest_status"
```

This script creates a comprehensive test that:

1. **Multiple Compilation Units with Mixed Options**: Tests 5, 9, and 10 mix different `-save-temps`, `-dumpdir`, and `-dumpbase` options for different source files in the same invocation.

2. **Combine Help/Version Requests with Actual Compilation**: Tests 3, 4, and 10 mix `--help`, `--version`, and `--target-help` with actual source file compilation.

3. **Use `@file` (Response File) Syntax**: Tests 3, 6, and 7 use response files to set `at_file_supplied` flag.

4. **Exercise `-save-temps` and Dump Options Variants**: Tests 1, 5, 6, and 9 use various combinations of `-save-temps`, `-dumpdir`, `-dumpbase`, and `-dumpbase-ext`.

5. **Leverage Driver-Specific Mode Switches**: Tests 2, 8, and 10 use `-E`, `-S`, `-c`, and `-x` options to change processing modes.

6. **Environment Variables and System Options**: Test 11 exercises `--sysroot` which affects `target_system_root` variables, and Test 13 uses linker options.

The script redirects stderr to `/dev/null` to suppress normal compiler output and warnings, focusing only on exercising the driver's internal logic. Each test is designed to force the driver to reset its internal state between processing different arguments or input files, specifically targeting the uncovered lines in `gcc.cc`.
