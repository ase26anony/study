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
cat > main.c << 'EOF'
int main(void) {
    return 0;
}
EOF

cat > lib.c << 'EOF'
int helper(void) {
    return 42;
}
EOF

cat > util.c << 'EOF'
int util_func(void) {
    return 1;
}
EOF

# Create a response file with various options
cat > opts.txt << 'EOF'
-save-temps=cwd
-dumpdir=./dump_from_file
-dumpbase=filebase
-O2
EOF

# Create another response file with different options
cat > opts2.txt << 'EOF'
-ffast-math
-ftree-vectorize
-march=native
EOF

# Test 1: Basic state reset with dump options
echo "=== Test 1: Basic state reset with dump options ==="
gcc -save-temps -dumpdir=./mydir -dumpbase=mybase main.c -c -o main.o 2>&1 || true
gcc --version main.o -o prog1 2>&1 || true

# Test 2: Complex multi-file state transition with different modes
echo -e "\n=== Test 2: Complex multi-file state transition ==="
gcc -E -dumpbase=preproc main.c -S -dumpdir=./asm lib.c -c -dumpbase=obj util.c -o combined.o 2>&1 || true

# Test 3: Response file and help combination
echo -e "\n=== Test 3: Response file and help combination ==="
gcc @opts.txt main.c --help=optimizers -o dummy 2>&1 | head -20

# Test 4: Mixed help/version with actual compilation
echo -e "\n=== Test 4: Mixed help/version with compilation ==="
gcc main.c --help -c lib.o 2>&1 | head -10
gcc --target-help lib.c -S -o lib.s 2>&1 | head -10

# Test 5: Multiple compilation units with conflicting dump options
echo -e "\n=== Test 5: Multiple files with conflicting dump options ==="
gcc -save-temps=cwd -dumpdir=./d1 main.c -save-temps=obj -dumpdir=./d2 lib.c -save-temps -dumpdir=./d3 util.c -o output 2>&1 || true

# Test 6: Response file followed by conflicting command-line options
echo -e "\n=== Test 6: Response file with override options ==="
gcc @opts.txt -dumpbase-ext=.alt -dumpdir=./override_dir main.c -c 2>&1 || true

# Test 7: Multiple response files
echo -e "\n=== Test 7: Multiple response files ==="
gcc @opts.txt @opts2.txt main.c lib.c util.c -o prog2 2>&1 || true

# Test 8: Mode switches with dump options
echo -e "\n=== Test 8: Mode switches ==="
gcc -x c -dumpbase=cfile main.c -x c -dumpdir=cwd lib.c -shared -dumpbase=shared util.c 2>&1 || true

# Test 9: Combination of all techniques
echo -e "\n=== Test 9: Comprehensive test ==="
gcc @opts.txt -save-temps=obj -dumpdir=./final main.c --help=warnings -S lib.c -dumpbase=finalbase -c util.c -o final.o 2>&1 | head -15

# Test 10: Using -- separator
echo -e "\n=== Test 10: Using -- separator ==="
gcc -save-temps --verbose -dumpdir=./sepdir -- -save-temps=cwd main.c 2>&1 || true

# Test 11: Environment variables affecting driver behavior
echo -e "\n=== Test 11: With GCC_EXEC_PREFIX ==="
GCC_EXEC_PREFIX=/usr/lib/gcc/ gcc -save-temps -dumpbase=envtest main.c -c 2>&1 || true

# Test 12: Multiple output specifications
echo -e "\n=== Test 12: Multiple output files ==="
gcc -save-temps -dumpdir=./multi -dumpbase=multi1 main.c -c -o main2.o -dumpbase=multi2 lib.c -c -o lib2.o 2>&1 || true

# Cleanup
echo -e "\n=== Cleaning up ==="
cd /
rm -rf "$TESTDIR"

echo "Test completed successfully!"
```

To run this test:

1. Save the script as `test_gcc_reset.sh`
2. Make it executable: `chmod +x test_gcc_reset.sh`
3. Run it: `./test_gcc_reset.sh`

The script will:
1. Create a temporary directory with multiple C source files
2. Create response files with various GCC options
3. Execute 12 different GCC invocations that combine the techniques needed to trigger the reset logic
4. Clean up all temporary files

Key aspects that should trigger the uncovered reset block:

1. **Multiple compilation units with mixed options** (Tests 1, 5, 12): Different `-save-temps`, `-dumpdir`, and `-dumpbase` options for different files force state resets.

2. **Help/version requests with compilation** (Tests 3, 4, 9): Mixing `--help`, `--version`, `--target-help` with actual source files stresses the state machine.

3. **Response file syntax** (Tests 3, 6, 7, 9): Using `@file` ensures `at_file_supplied` is set and needs resetting.

4. **Save-temps and dump options variants** (All tests): Various combinations of `-save-temps`, `-dumpdir`, `-dumpbase`, and `-dumpbase-ext` manipulate the variables that need freeing.

5. **Driver mode switches** (Tests 2, 8, 9): Using `-E`, `-S`, `-c`, `-shared`, and `-x` options triggers mode changes.

6. **Environment variables** (Test 11): Setting `GCC_EXEC_PREFIX` affects driver behavior.

The script uses `2>&1 || true` to ensure the script continues even if GCC returns an error (which is expected for some invalid combinations). The goal is to execute the driver code paths, not necessarily to produce valid compilation output.
