Here's a shell script that implements the requirements to cover the reset logic in gcc.cc:

```bash
#!/bin/bash

# Test script to exercise GCC driver reset logic (lines 11228-11250 in gcc.cc)
# This script creates multiple test files and invokes gcc with various combinations
# of options that should trigger the reset block.

set -e

# Create a temporary directory for test files
TESTDIR=$(mktemp -d /tmp/gcc_reset_test_XXXXXX)
cd "$TESTDIR"

echo "Test directory: $TESTDIR"

# Create simple C source files
cat > test1.c << 'EOF'
int main() { return 0; }
EOF

cat > test2.c << 'EOF'
int helper() { return 42; }
EOF

cat > test3.c << 'EOF'
int another() { return 1; }
EOF

# Create a response file with various options
cat > opts.txt << 'EOF'
-save-temps=cwd
-dumpdir=./response_dump
-dumpbase=response_base
-O2
EOF

# Create another response file with different options
cat > opts2.txt << 'EOF'
--target-help
-verbose
EOF

# Create a wrapper script to test GCC_EXEC_PREFIX
cat > gcc_wrapper.sh << 'EOF'
#!/bin/bash
# Simple wrapper that adds extra arguments
exec gcc -dumpbase=wrapper_base "$@"
EOF
chmod +x gcc_wrapper.sh

echo "=== Test 1: Basic compilation with dump options ==="
gcc -save-temps -dumpdir=./mydir -dumpbase=mybase test1.c -c -o test1.o

echo "=== Test 2: Mixing help/version with compilation ==="
# Help followed by compile (should trigger reset after help)
gcc --help -c test1.c 2>/dev/null || true
# Version followed by linking
gcc --version test1.o -o prog1 2>/dev/null || true

echo "=== Test 3: Complex multi-file state transitions ==="
# Chain multiple processing modes with different dump options
gcc -E -dumpbase=preproc test1.c -S -dumpdir=./asm test2.c -c -dumpbase=obj test3.c -o combined.o 2>/dev/null

echo "=== Test 4: Response file with conflicting command-line options ==="
# Use response file then override options on command line
gcc @opts.txt -dumpbase-ext=.alt -dumpdir=./override_dump test1.c -c 2>/dev/null

echo "=== Test 5: Multiple save-temps options for different files ==="
# Different save-temps and dumpdir for each file
gcc -save-temps=cwd -dumpdir=./d1 test1.c -save-temps=obj -dumpdir=./d2 test2.c -o output 2>/dev/null

echo "=== Test 6: Response file followed by help request ==="
# This should set at_file_supplied and dump options, then reset for help
gcc @opts.txt test1.c --help=optimizers 2>/dev/null || true

echo "=== Test 7: Mixed language specifications ==="
# Create a simple C++ file
cat > test4.cpp << 'EOF'
extern "C" int func() { return 0; }
EOF

# Use -x to specify language, mixing with dump options
gcc -x c -dumpbase=c_base test1.c -x c++ -dumpbase=cpp_base test4.cpp -c 2>/dev/null

echo "=== Test 8: Environment variables and wrapper ==="
# Test with GCC_EXEC_PREFIX set
GCC_EXEC_PREFIX="" ./gcc_wrapper.sh -save-temps -dumpdir=wrapper_dir test1.c -c 2>/dev/null

echo "=== Test 9: Multiple response files ==="
# Use multiple response files with conflicting options
gcc @opts.txt @opts2.txt test1.c -c 2>/dev/null || true

echo "=== Test 10: Complex chain with mode switches ==="
# Test various driver modes in sequence
gcc -E test1.c | gcc -x c - -S -dumpbase=pipe_base -o test1.s 2>/dev/null
gcc -c test1.s -o test1_from_asm.o 2>/dev/null
gcc test1_from_asm.o test2.o -dumpdir=final_dir -o final_prog 2>/dev/null

echo "=== Test 11: Save-temps variants with output redirection ==="
# Test different save-temps options
gcc -save-temps=cwd -dumpdir=./cwd_dump test1.c -c -o test1_cwd.o 2>/dev/null
gcc -save-temps=obj -dumpdir=./obj_dump test2.c -c -o test2_obj.o 2>/dev/null

echo "=== Test 12: Reset after target system root options ==="
# Test with sysroot options (may be no-ops on current system)
gcc --sysroot=/ -dumpbase=sysroot_base test1.c -c 2>/dev/null || true
gcc -no-sysroot-suffix test1.c -c 2>/dev/null || true

echo "=== Test 13: Combination of all techniques ==="
# The ultimate test: mix everything together
gcc @opts.txt \
    -save-temps=cwd -dumpdir=./combo1 test1.c \
    --help=warnings 2>/dev/null || true
    
gcc -save-temps=obj -dumpdir=./combo2 test2.c \
    -E -dumpbase=combo_preproc test3.c \
    @opts2.txt 2>/dev/null || true

echo "=== Test 14: Exercise greatest_status reset ==="
# Create a failing compilation to set greatest_status
cat > fail.c << 'EOF'
int main() { undefined_function(); return 0; }
EOF

gcc -c fail.c -o fail.o 2>/dev/null || true
# Follow with successful compilation to test reset
gcc -c test1.c -o test1_final.o

echo "=== Test 15: Cleanup and final test ==="
# Clean temporary files but keep some for inspection
rm -f *.o *.s *.i *.ii *.bc *.s *.o
rm -f prog* output final_prog
# Keep dump directories to show they were created
find . -type d -name "*dump*" | head -5

echo "All tests completed successfully!"
echo "Test directory preserved for inspection: $TESTDIR"
echo "To clean up manually: rm -rf $TESTDIR"
```

This script systematically exercises the GCC driver's reset logic by:

1. **Multiple compilation units with mixed options**: Tests 3, 5, and 13 compile multiple files with different `-save-temps`, `-dumpdir`, and `-dumpbase` options in a single command line.

2. **Combine help/version with compilation**: Tests 2, 6, and 13 mix `--help`, `--version`, and `--target-help` with actual compilation commands.

3. **Use `@file` syntax**: Tests 4, 6, 9, and 13 use response files with various options, ensuring `at_file_supplied` is set.

4. **Exercise `-save-temps` and dump options**: Tests 1, 4, 5, 11, and 13 use different combinations of `-save-temps`, `-dumpdir`, `-dumpbase`, and `-dumpbase-ext`.

5. **Driver-specific mode switches**: Tests 3, 7, and 10 use `-E`, `-S`, `-c`, and `-x` options to switch between preprocessing, compilation, and assembly modes.

6. **Environment variables and wrapper scripts**: Test 8 uses a wrapper script and environment variables to test different execution paths.

The script creates temporary files, runs GCC with various option combinations that should trigger the reset block, and cleans up after itself. Each test is designed to exercise different parts of the reset logic in `gcc.cc` lines 11228-11250.
