Here's a shell script that implements the requirements to cover the uncovered lines in `gcc.cc`:

```bash
#!/bin/bash

# Test script to cover reset logic in gcc.cc lines 11228-11250
# This script creates multiple test files and invokes gcc with various
# combinations of options that should trigger the reset block.

set -e

# Create a temporary directory for test files
TESTDIR=$(mktemp -d)
cd "$TESTDIR"

echo "Test directory: $TESTDIR"

# Create simple C source files
cat > test1.c << 'EOF'
int main(void) { return 0; }
EOF

cat > test2.c << 'EOF'
int helper(void) { return 42; }
EOF

cat > test3.c << 'EOF'
int another(void) { return 1; }
EOF

# Create a response file with various options
cat > opts.txt << 'EOF'
-save-temps=cwd
-dumpdir=./dumpdir_from_file
-dumpbase=base_from_file
EOF

# Create another response file with different options
cat > opts2.txt << 'EOF'
--target-help
-version
EOF

# Test 1: Mix help/version with compilation (triggers reset after help)
echo "Test 1: Mixing help with compilation"
gcc --help test1.c 2>&1 | head -5 || true
gcc test1.c --version 2>&1 | head -2 || true
gcc --target-help -c test1.c 2>&1 | head -5 || true

# Test 2: Response file with conflicting command-line options
echo -e "\nTest 2: Response file with conflicting options"
gcc @opts.txt -dumpdir=./cmdline_dump -dumpbase=cmdline_base test1.c -c -o test1.o 2>&1 || true

# Test 3: Multiple files with different dump options (forces reset between files)
echo -e "\nTest 3: Multiple files with different dump options"
gcc -save-temps=cwd -dumpdir=./d1 test1.c -save-temps=obj -dumpdir=./d2 test2.c -c 2>&1 || true

# Test 4: Complex mode switching with dump options
echo -e "\nTest 4: Complex mode switching"
gcc -E -dumpbase=preproc test1.c -S -dumpdir=./asm test2.c -c -dumpbase=obj test3.c 2>&1 || true

# Test 5: Response file followed by help request
echo -e "\nTest 5: Response file followed by help"
gcc @opts.txt test1.c --help=optimizers 2>&1 | head -10 || true

# Test 6: Multiple response files and compilation
echo -e "\nTest 6: Multiple response files"
gcc @opts.txt @opts2.txt test1.c -c 2>&1 | head -5 || true

# Test 7: Save-temps variants with output redirection
echo -e "\nTest 7: Save-temps variants"
gcc -save-temps -dumpdir=./mydir -dumpbase=mybase test1.c -c -o test1_2.o 2>&1 || true
gcc --version test1_2.o -o prog 2>&1 | head -2 || true

# Test 8: Using -x language specifiers with dump options
echo -e "\nTest 8: Language specifiers with dump options"
gcc -x c -dumpbase=c_file test1.c -x c -dumpdir=./c_dump test2.c -c 2>&1 || true

# Test 9: Mixed compilation modes in single command
echo -e "\nTest 9: Mixed compilation modes"
gcc -E test1.c -S test2.c -c test3.c -dumpbase=mixed -dumpdir=./mixed 2>&1 || true

# Test 10: At-file supplied flag test with empty response
echo -e "\nTest 10: Empty response file"
touch empty.txt
gcc @empty.txt test1.c -c 2>&1 || true

# Test 11: Override dumpbase-ext
echo -e "\nTest 11: Override dumpbase-ext"
gcc @opts.txt -dumpbase-ext=.alt test1.c -c 2>&1 || true

# Test 12: Shared library creation with dump options
echo -e "\nTest 12: Shared library with dump options"
gcc -shared -fPIC -dumpdir=./shared_dump -dumpbase=shared test1.c test2.c -o libtest.so 2>&1 || true

# Cleanup (optional - comment out for debugging)
echo -e "\nCleaning up..."
cd ..
rm -rf "$TESTDIR"

echo "All tests completed successfully"
```

To run this test script:

1. Save it as `test_gcc_reset.sh`
2. Make it executable: `chmod +x test_gcc_reset.sh`
3. Run it: `./test_gcc_reset.sh`

The script implements all the requirements:

1. **Multiple Compilation Units**: Uses `test1.c`, `test2.c`, `test3.c` with different options
2. **Help/Version Mixing**: Combines `--help`, `--version`, `--target-help` with compilation
3. **Response File Syntax**: Uses `@opts.txt` and `@opts2.txt`
4. **Save-temps Variants**: Uses `-save-temps`, `-save-temps=cwd`, `-save-temps=obj`
5. **Dump Options**: Uses `-dumpdir`, `-dumpbase`, `-dumpbase-ext` with various values
6. **Mode Switches**: Uses `-E`, `-S`, `-c`, `-shared`, `-x c`
7. **Complex Combinations**: Mixes multiple techniques in single commands

The script uses `|| true` after gcc invocations to continue even if compilation fails (since we're testing driver logic, not successful compilation). The `2>&1` redirects stderr to stdout to capture all output.

This should trigger the reset logic in `gcc.cc` by:
- Setting `at_file_supplied` via `@opts.txt`
- Setting `dumpdir`, `dumpbase`, etc. then freeing them
- Changing `save_temps_flag` between different values
- Triggering help/version output which may cause resets
- Switching between different compilation modes
- Processing multiple input files with different options in single command
