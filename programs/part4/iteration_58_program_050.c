Here's a shell script that implements the requirements to exercise the uncovered reset logic in gcc.cc:

```bash
#!/bin/bash
set -e

# Clean up any previous test artifacts
cleanup() {
    rm -f test*.c test*.o test*.i test*.s test*.out output prog dummy
    rm -rf mydir asm d1 d2 dump
    rm -f opts.txt args.txt wrapper.sh
}
cleanup

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

# Create response files with various options
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

# Create a wrapper script to test GCC_EXEC_PREFIX path
cat > wrapper.sh << 'EOF'
#!/bin/bash
# Simulate a compiler wrapper that adds arguments
exec gcc "$@"
EOF
chmod +x wrapper.sh

# Test 1: Basic state reset with mixed options
echo "Test 1: Basic state reset with mixed options"
gcc -save-temps -dumpdir=./mydir -dumpbase=mybase test1.c -c -o test1.o 2>/dev/null || true
gcc --version test1.o -o prog 2>/dev/null || true

# Test 2: Complex multi-file state transition
echo "Test 2: Complex multi-file state transition"
gcc -E -dumpbase=preproc test1.c -S -dumpdir=./asm test2.c -c -dumpbase=obj test3.c -o combined.o 2>/dev/null || true

# Test 3: Response file and help combination
echo "Test 3: Response file and help combination"
gcc @args.txt test1.c --help=optimizers -o dummy 2>&1 | head -5 || true

# Test 4: Multiple compilation units with conflicting dump options
echo "Test 4: Multiple compilation units with conflicting dump options"
gcc -save-temps=cwd -dumpdir=./d1 test1.c -save-temps=obj -dumpdir=./d2 test2.c -o output 2>/dev/null || true

# Test 5: Mix help/version with actual compilation in different orders
echo "Test 5: Mix help/version with actual compilation"
gcc --help -c test1.c 2>&1 | head -5 || true
gcc test1.c --version -o test1.out 2>&1 | head -5 || true
gcc --target-help test1.c -S 2>&1 | head -5 || true

# Test 6: Response file with additional command-line overrides
echo "Test 6: Response file with command-line overrides"
gcc @opts.txt -dumpbase-ext=.alt test1.c -c 2>/dev/null || true

# Test 7: Mode switches with dump options
echo "Test 7: Mode switches with dump options"
gcc -x c -dumpbase=test1 -E test1.c -x c -dumpdir=./asm -S test2.c 2>/dev/null || true

# Test 8: Environment variables affecting driver behavior
echo "Test 8: With environment variables"
COMPILER_PATH=/usr/bin gcc -save-temps test1.c -c 2>/dev/null || true
GCC_EXEC_PREFIX="" gcc --version 2>&1 | head -5 || true

# Test 9: Using wrapper script
echo "Test 9: Using wrapper script"
PATH=.:$PATH wrapper.sh -save-temps -dumpbase=wrapped test1.c -c 2>/dev/null || true

# Test 10: Multiple response files and source files
echo "Test 10: Multiple response files"
cat > opts2.txt << 'EOF'
-dumpbase=second
-save-temps=obj
EOF
gcc @args.txt test1.c @opts2.txt test2.c -o final 2>/dev/null || true

# Test 11: Exercise all save-temps variants
echo "Test 11: All save-temps variants"
gcc -save-temps test1.c -c -o test1_temp.o 2>/dev/null || true
gcc -save-temps=cwd test2.c -c -o test2_temp.o 2>/dev/null || true
gcc -save-temps=obj test3.c -c -o test3_temp.o 2>/dev/null || true

# Test 12: Combined options that stress the state machine
echo "Test 12: Combined stress test"
gcc -shared -dumpdir=./shared -dumpbase=shared test1.c test2.c --help=warnings -E test3.c 2>&1 | head -5 || true

# Clean up
cleanup

echo "All tests executed successfully"
```

This script systematically tests the uncovered reset logic by:

1. **Multiple compilation units with mixed options** (Tests 4, 7, 10): Uses different `-save-temps`, `-dumpdir`, and `-dumpbase` options for different source files in the same invocation.

2. **Combine help/version with actual compilation** (Tests 3, 5, 12): Mixes `--help`, `--version`, and `--target-help` with source files and compilation options in various orders.

3. **Use `@file` syntax** (Tests 3, 6, 10): Creates response files with dump options and uses them with additional command-line arguments.

4. **Exercise `-save-temps` and dump variants** (Tests 1, 4, 6, 11): Tests all `-save-temps` variants (`cwd`, `obj`, default) with various dump options.

5. **Leverage mode switches** (Tests 2, 7, 12): Uses `-E`, `-S`, `-c`, `-shared`, and `-x` options to change driver modes between files.

6. **Environment variables and wrapper** (Tests 8, 9): Sets `COMPILER_PATH` and `GCC_EXEC_PREFIX`, and uses a wrapper script to test alternative execution paths.

The script redirects output to `/dev/null` or pipes to `head` to prevent excessive output while still executing the driver logic. Each test is designed to trigger the reset block by forcing the driver to process multiple conflicting states within a single invocation.
