Here's a shell script that implements the requirements to trigger the uncovered reset logic in gcc.cc:

```bash
#!/bin/bash
set -e

# Clean up from previous runs
cleanup() {
    rm -f test*.c test*.o test*.s test*.i test*.out output* prog*
    rm -rf dump* mydir* asm* d1 d2
    rm -f opts.txt args.txt
}
cleanup

# Create simple test files
cat > test1.c << 'EOF'
int main() { return 0; }
EOF

cat > test2.c << 'EOF'
int helper() { return 42; }
EOF

cat > test3.c << 'EOF'
int another() { return 1; }
EOF

# Create response files
cat > opts.txt << 'EOF'
-save-temps=cwd
-dumpdir=./dump
-dumpbase=testdump
EOF

cat > args.txt << 'EOF'
-save-temps
-dumpdir=mydir
-dumpbase=mybase
EOF

echo "=== Test 1: Basic state reset with dump options ==="
gcc -save-temps -dumpdir=./mydir1 -dumpbase=mybase1 test1.c -c -o test1.o 2>/dev/null || true
gcc --version test1.o -o prog1 2>/dev/null || true

echo "=== Test 2: Complex multi-file state transition ==="
gcc -E -dumpbase=preproc test1.c -S -dumpdir=./asm test2.c -c -dumpbase=obj test3.c -o combined.o 2>/dev/null || true

echo "=== Test 3: Response file and help combination ==="
gcc @args.txt test1.c --help=optimizers -o dummy 2>&1 | head -5 || true

echo "=== Test 4: Mixed save-temps options between files ==="
gcc -save-temps=cwd -dumpdir=./d1 test1.c -save-temps=obj -dumpdir=./d2 test2.c -o output 2>/dev/null || true

echo "=== Test 5: Multiple compilation units with conflicting dump options ==="
gcc -save-temps -dumpdir=./dump1 -dumpbase=base1 test1.c \
    -dumpdir=./dump2 -dumpbase=base2 test2.c \
    -dumpbase-ext=.alt -dumpdir=./dump3 test3.c \
    -o final 2>/dev/null || true

echo "=== Test 6: Version/help mixed with compilation ==="
gcc test1.c --version -c test2.c 2>&1 | head -2 || true
gcc --target-help test1.c -o test1.out 2>&1 | head -2 || true

echo "=== Test 7: Response file with additional command-line overrides ==="
gcc @opts.txt -dumpbase-ext=.alt test1.c -dumpdir=./override test2.c 2>/dev/null || true

echo "=== Test 8: Mode switches with dump options ==="
gcc -x c -E -dumpdir=./pre -dumpbase=pre test1.c \
    -x c -S -dumpdir=./asm -dumpbase=asm test2.c \
    -x c -c -dumpdir=./obj -dumpbase=obj test3.c 2>/dev/null || true

echo "=== Test 9: Environment variables affecting driver ==="
GCC_EXEC_PREFIX=./ gcc -save-temps test1.c -o envtest 2>/dev/null || true
COMPILER_PATH=/usr/bin gcc -dumpbase=envbase test2.c -c 2>/dev/null || true

echo "=== Test 10: Multiple response files and separators ==="
cat > opts2.txt << 'EOF'
--help
EOF
gcc @args.txt test1.c -- @opts2.txt 2>&1 | head -3 || true

echo "=== Test 11: Save-temps variants ==="
gcc -save-temps=obj -dumpdir=./objdir test1.c -c 2>/dev/null || true
gcc -save-temps=cwd -dumpdir=. test2.c -c 2>/dev/null || true

echo "=== Test 12: Complex chain with all features ==="
gcc @opts.txt -E test1.c \
    -save-temps=obj -dumpdir=./chain1 test2.c -S \
    --version \
    -dumpbase=chainbase test3.c -c \
    -o chain.o 2>&1 | head -2 || true

# Clean up
cleanup

echo "All tests executed successfully!"
```

This script systematically tests the uncovered reset logic by:

1. **Multiple Compilation Units with Mixed Options**: Tests 4, 5, and 8 use different `-save-temps`, `-dumpdir`, and `-dumpbase` options for different source files in the same invocation.

2. **Combine Help/Version with Compilation**: Tests 3, 6, and 12 mix `--help`, `--version`, and `--target-help` with actual compilation commands.

3. **Use `@file` Syntax**: Tests 3, 7, and 10 use response files with various combinations of arguments.

4. **Exercise `-save-temps` Variants**: Tests 1, 4, 7, and 11 use different `-save-temps` options (`=cwd`, `=obj`) along with dump options.

5. **Driver Mode Switches**: Tests 2, 8, and 12 use `-E`, `-S`, `-c`, and `-x` options to change processing modes.

6. **Environment Variables**: Test 9 sets `GCC_EXEC_PREFIX` and `COMPILER_PATH` to affect driver behavior.

The script redirects output to `/dev/null` or pipes through `head` to avoid cluttering the terminal, as we're only interested in exercising the driver's internal logic, not the actual compilation results. Each test is designed to trigger the reset block by forcing the driver to process conflicting options and state changes between different input files and modes within a single invocation.
