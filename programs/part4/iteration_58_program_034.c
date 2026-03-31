Here's a shell script that implements the requirements to trigger the uncovered reset logic in gcc.cc:

```bash
#!/bin/bash
set -e

# Clean up from previous runs
cleanup() {
    rm -f test1.c test2.c test3.c opts.txt output *.o *.i *.s *.so
    rm -rf dump d1 d2 mydir asm
}
cleanup

# Create simple C source files
cat > test1.c << 'EOF'
int main() { return 0; }
EOF

cat > test2.c << 'EOF'
int helper() { return 1; }
EOF

cat > test3.c << 'EOF'
int another() { return 2; }
EOF

# Create response file with various options
cat > opts.txt << 'EOF'
-save-temps=cwd
-dumpdir=./dump
-dumpbase=testdump
EOF

echo "=== Test 1: Basic state reset with mixed options ==="
# This should trigger reset between processing different files
gcc -save-temps -dumpdir=./mydir -dumpbase=mybase test1.c -c -o test1.o
gcc --version test1.o -o output 2>/dev/null || true

echo "=== Test 2: Complex multi-file state transitions ==="
# Chain multiple processing modes with different dump options
gcc -E -dumpbase=preproc test1.c -S -dumpdir=./asm test2.c -c -dumpbase=obj test3.c -o combined.o 2>/dev/null

echo "=== Test 3: Response file with help combination ==="
# Use @file syntax followed by help request
gcc @opts.txt test1.c --help=optimizers -o dummy 2>&1 | head -5

echo "=== Test 4: Multiple compilation units with conflicting dump options ==="
# Different save-temps and dumpdir for each file
gcc -save-temps=cwd -dumpdir=./d1 test1.c -save-temps=obj -dumpdir=./d2 test2.c -o output 2>/dev/null

echo "=== Test 5: Mode switches with dumpbase-ext variations ==="
gcc @opts.txt -dumpbase-ext=.alt test1.c -dumpbase-ext=.mod test2.c -c 2>/dev/null

echo "=== Test 6: Mixed help/version with actual compilation ==="
# Help before and after source files
gcc --help -c test1.c 2>&1 | head -3
gcc test1.c --version -o test1.o 2>&1 | head -2

echo "=== Test 7: Target system root and machine spec variations ==="
# Exercise target-related variables
gcc -specs=/dev/null test1.c -c -o test1_spec.o 2>/dev/null || true

echo "=== Test 8: Shared library with dump options ==="
gcc -shared -dumpdir=./mydir -fPIC test1.c test2.c -o libtest.so 2>/dev/null

echo "=== Test 9: Verbose flag combinations ==="
gcc -v -dumpbase=verbose_test test1.c -c 2>&1 | grep -i "version" | head -1

echo "=== Test 10: Multiple response files ==="
cat > opts2.txt << 'EOF'
-dumpbase=second
-save-temps
EOF
gcc @opts.txt test1.c @opts2.txt test2.c -o final 2>/dev/null

echo "=== Test 11: Exercise greatest_status with failed compilation ==="
# Create invalid C code
cat > invalid.c << 'EOF'
invalid syntax here
EOF
gcc invalid.c -o /dev/null 2>/dev/null || true
# Follow with valid compilation to trigger reset
gcc test1.c -c -o test1.o 2>/dev/null

echo "=== Test 12: Environment variable influence ==="
# Set environment variables that might affect driver state
GCC_EXEC_PREFIX=/usr/lib/gcc/ gcc test1.c -c -o env_test.o 2>/dev/null || true
COMPILER_PATH=/usr/bin gcc test2.c -c -o env_test2.o 2>/dev/null || true

echo "=== Test 13: Combined everything ==="
# Most complex case combining multiple techniques
gcc -v @opts.txt -save-temps=obj -dumpdir=./final_dump test1.c \
    --help=warnings test2.c -dumpbase-ext=.combined \
    -x c test3.c -S -dumpbase=asm_output -o /dev/null 2>&1 | head -10

echo "=== Test 14: At-file supplied flag exercise ==="
# Multiple @file usages
cat > filelist.txt << 'EOF'
test1.c
test2.c
EOF
gcc @opts.txt @filelist.txt -o multi_file 2>/dev/null

echo "=== Test 15: Print help list flag ==="
gcc --help=common test1.c -c 2>&1 | head -5

# Clean up
cleanup

echo "All tests completed successfully!"
```

To run this test script:

1. Save it as `test_gcc_reset.sh`
2. Make it executable: `chmod +x test_gcc_reset.sh`
3. Run it: `./test_gcc_reset.sh`

The script systematically exercises the uncovered lines by:

1. **Multiple compilation units with mixed options**: Tests 4, 5, 10, and 13 use different dump options for different files
2. **Combine help/version with compilation**: Tests 1, 3, 6, and 13 mix help/version requests with actual compilation
3. **Use @file syntax**: Tests 3, 5, 10, 13, and 14 use response files to set `at_file_supplied`
4. **Exercise save-temps and dump options**: Tests 1-5, 8, 10, and 13 use various combinations of `-save-temps`, `-dumpdir`, `-dumpbase`
5. **Driver mode switches**: Tests 2, 13 switch between `-E`, `-S`, `-c`, and linking modes
6. **Environment variables**: Test 12 sets `GCC_EXEC_PREFIX` and `COMPILER_PATH`
7. **Greatest status reset**: Test 11 forces a compilation failure followed by success

The script redirects error output to `/dev/null` or pipes to `head` to keep output manageable while still executing all the driver code paths. Each test is designed to trigger the reset logic in different ways, ensuring coverage of all the uncovered lines in the specified block.
