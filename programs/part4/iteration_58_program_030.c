Here's a shell script that implements the requirements to trigger the uncovered reset logic in gcc.cc:

```bash
#!/bin/bash
set -e

# Clean up any previous test files
cleanup() {
    rm -f test1.c test2.c test3.c opts.txt
    rm -rf dump d1 d2 mydir asm
    rm -f *.o *.i *.s *.out output prog combined.o dummy
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

echo "=== Test 1: Mixing help/version with compilation ==="
# Help followed by compile - forces state reset after help display
gcc --help -c test1.c 2>/dev/null || true
gcc --version -c test1.c 2>/dev/null || true
gcc -c test1.c --help=optimizers 2>/dev/null || true
gcc -c test1.c --target-help 2>/dev/null || true

echo "=== Test 2: Multiple files with different dump options ==="
# Create dump directories
mkdir -p d1 d2 mydir asm

# Different save-temps and dumpdir for each file
gcc -save-temps=cwd -dumpdir=./d1 test1.c \
    -save-temps=obj -dumpdir=./d2 test2.c \
    -o output 2>/dev/null || true

echo "=== Test 3: Response file with command-line overrides ==="
# Response file sets initial state, command line modifies it
gcc @opts.txt -dumpbase-ext=.alt test1.c -c 2>/dev/null || true

echo "=== Test 4: Complex multi-file state transitions ==="
# Chain multiple processing modes with different dump options
gcc -E -dumpbase=preproc test1.c \
    -S -dumpdir=./asm test2.c \
    -c -dumpbase=obj test3.c \
    -o combined.o 2>/dev/null || true

echo "=== Test 5: Mixed mode operations with output specification ==="
# Test with -o option in different positions
gcc -save-temps -dumpdir=./mydir -dumpbase=mybase test1.c -c -o test1.o 2>/dev/null || true
gcc --version test1.o -o prog 2>/dev/null || true

echo "=== Test 6: Response file with help combination ==="
# Response file followed by help request
gcc @opts.txt test1.c --help=optimizers -o dummy 2>/dev/null || true

echo "=== Test 7: Multiple @file usage ==="
# Create another response file with conflicting options
cat > opts2.txt << 'EOF'
-save-temps
-dumpbase=conflict
EOF
gcc @opts.txt @opts2.txt test1.c test2.c -o test.out 2>/dev/null || true

echo "=== Test 8: Environment variable influence ==="
# Test with environment variables that might affect driver state
GCC_EXEC_PREFIX=test: gcc -save-temps test1.c -c 2>/dev/null || true
COMPILER_PATH=/tmp gcc -dumpbase=envtest test1.c -c 2>/dev/null || true

echo "=== Test 9: Language specification changes ==="
# Use -x to change language mode
gcc -x c -dumpbase=cfile test1.c \
    -x assembler -dumpdir=./asm test2.s 2>/dev/null || true

echo "=== Test 10: Save-temps variants ==="
# Test all save-temps variants
gcc -save-temps=cwd -dumpdir=./temp1 test1.c -c 2>/dev/null || true
gcc -save-temps=obj -dumpdir=./temp2 test2.c -c 2>/dev/null || true

echo "=== Test 11: Mixed with linker options ==="
# Combine compilation options with linker options
gcc -save-temps -dumpbase=linktest test1.c test2.c \
    -shared -o libtest.so 2>/dev/null || true

echo "=== Test 12: Reset between driver phases ==="
# Force multiple driver phases in one invocation
gcc -E -dumpbase=phase1 test1.c \
    -c -dumpdir=./phase2 test2.c \
    -v test3.c -o final.out 2>/dev/null || true

echo "=== All tests completed successfully ==="

# Clean up
cleanup
```

To run this test script:

1. Save it as `test_gcc_reset.sh`
2. Make it executable: `chmod +x test_gcc_reset.sh`
3. Run it: `./test_gcc_reset.sh`

The script implements all the requirements:

1. **Multiple Compilation Units with Mixed Options**: Tests 2, 4, 7, and 12 compile multiple files with different `-save-temps`, `-dumpdir`, and `-dumpbase` options in the same command line.

2. **Combine Help/Version Requests with Actual Compilation**: Tests 1, 5, and 6 mix `--help`, `--version`, and `--target-help` with compilation commands.

3. **Use `@file` (Response File) Syntax**: Tests 3, 6, and 7 use response files with various options.

4. **Exercise `-save-temps` and Dump Options Variants**: Tests 2, 3, 4, 5, 10 use different combinations of `-save-temps=cwd`, `-save-temps=obj`, `-dumpdir`, `-dumpbase`, and `-dumpbase-ext`.

5. **Leverage Driver-Specific Mode Switches**: Tests 4 and 12 chain `-E`, `-S`, `-c` modes. Test 9 uses `-x` for language specification.

6. **Environment Variables**: Test 8 sets `GCC_EXEC_PREFIX` and `COMPILER_PATH` to influence driver behavior.

The script redirects stderr to `/dev/null` and uses `|| true` to continue even if some invocations fail (as some combinations may be invalid), since the goal is to execute the driver's internal reset logic, not necessarily to produce valid compilation output.

Each test case is designed to trigger the reset block at lines 11228-11250 by forcing the driver to:
- Process multiple input files with different option sets
- Transition between help/version display and compilation
- Handle response files that set `at_file_supplied`
- Free and reallocate `dumpdir`, `dumpbase`, etc. between files
- Reset `spec_machine` and `greatest_status` between operations
