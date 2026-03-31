Here's a shell script that implements the requirements to exercise the uncovered reset logic in gcc.cc:

```bash
#!/bin/bash
set -e

# Clean up from previous runs
cleanup() {
    rm -f test*.c test*.o test*.s test*.i test*.ii output* prog* dummy*
    rm -rf dump* mydir* asm* d1 d2
    rm -f opts.txt args.txt
}
cleanup 2>/dev/null || true

# Create simple test source files
cat > test1.c << 'EOF'
int main() { return 0; }
EOF

cat > test2.c << 'EOF'
int helper() { return 1; }
EOF

cat > test3.c << 'EOF'
int another() { return 2; }
EOF

# Create a response file with various options
cat > opts.txt << 'EOF'
-save-temps=cwd
-dumpdir=./dumpdir_test
-dumpbase=testdump
-O2
EOF

# Create another response file for different options
cat > args.txt << 'EOF'
-v
-dumpbase-ext=.special
EOF

echo "=== Test 1: Mixing help/version with compilation ==="
# Help followed by actual compilation
gcc --help -c test1.c 2>/dev/null || true
# Version followed by compilation with output
gcc --version test1.c -o output1 2>/dev/null || true
# Target help mixed with source file
gcc --target-help test2.c 2>/dev/null || true
# Help for specific component followed by compilation
gcc --help=optimizers -c test3.c 2>/dev/null || true

echo "=== Test 2: Multiple files with different dump options ==="
# Different save-temps and dumpdir for each file
gcc -save-temps=cwd -dumpdir=./d1 test1.c -save-temps=obj -dumpdir=./d2 test2.c -o prog1 2>/dev/null || true

echo "=== Test 3: Response file with command-line overrides ==="
# Use response file then override on command line
gcc @opts.txt -dumpbase-ext=.alt test1.c -o output2 2>/dev/null || true

echo "=== Test 4: Complex mode switching ==="
# Chain different processing modes for different files
gcc -E -dumpbase=preproc test1.c -S -dumpdir=./asm test2.c -c -dumpbase=obj test3.c -o combined.o 2>/dev/null || true

echo "=== Test 5: Mixed response files and help ==="
# Response file, compilation, then help request
gcc @args.txt test1.c --help=warnings -o dummy 2>/dev/null || true

echo "=== Test 6: Multiple @file usage ==="
# Multiple response files
gcc @opts.txt @args.txt test1.c test2.c -o output3 2>/dev/null || true

echo "=== Test 7: Save-temps variants ==="
# Different save-temps options in sequence
gcc -save-temps test1.c -save-temps=obj test2.c -save-temps=cwd test3.c -o output4 2>/dev/null || true

echo "=== Test 8: Language specification changes ==="
# Change language modes
gcc -x c test1.c -x assembler -x c test2.c -o output5 2>/dev/null || true

echo "=== Test 9: Environment variable influence ==="
# Set environment variables that might affect driver state
GCC_EXEC_PREFIX=./ gcc -v test1.c -o output6 2>/dev/null || true
COMPILER_PATH=/usr/bin gcc -v test2.c -o output7 2>/dev/null || true

echo "=== Test 10: Complex combination ==="
# Most complex combination to stress the reset logic
gcc -save-temps=cwd -dumpdir=./mydir -dumpbase=base1 test1.c \
    @opts.txt \
    -E -dumpbase=preproc2 test2.c \
    --help=target \
    -S -dumpdir=./asm2 test3.c \
    -c -o final.o \
    --version \
    -o final 2>/dev/null || true

echo "=== Test 11: Separate compilation with linking ==="
# Compile separately then link with different options
gcc -c -save-temps -dumpdir=./compile1 test1.c
gcc -c -save-temps=obj -dumpdir=./compile2 test2.c
gcc --help=common test1.o test2.o -o linked 2>/dev/null || true

echo "=== Test 12: At-file supplied flag testing ==="
# Explicitly test @file syntax variations
echo "-O1 -g" > simple.txt
gcc @simple.txt test1.c -o simple_out 2>/dev/null || true

# Clean up generated files but keep some for inspection
echo "Cleaning up..."
rm -f test*.o test*.s test*.i test*.ii output* prog* dummy* simple_out final.o linked
rm -f opts.txt args.txt simple.txt
rm -rf d1 d2 2>/dev/null || true

echo "All tests executed successfully!"
echo "Note: Some commands may fail due to conflicting options, but the driver's reset logic should have been exercised."
```

To run this test:

1. Save the script to a file (e.g., `test_gcc_reset.sh`)
2. Make it executable: `chmod +x test_gcc_reset.sh`
3. Run it: `./test_gcc_reset.sh`

The script implements all the requirements:

1. **Multiple Compilation Units**: Uses `test1.c`, `test2.c`, `test3.c` with different options for each.

2. **Help/Version with Compilation**: Mixes `--help`, `--version`, `--target-help` with actual source files in various orders.

3. **@file Syntax**: Creates and uses `opts.txt`, `args.txt`, and `simple.txt` response files.

4. **Save-temps and Dump Options**: Uses `-save-temps`, `-save-temps=cwd`, `-save-temps=obj`, `-dumpdir`, `-dumpbase`, `-dumpbase-ext` in various combinations.

5. **Mode Switches**: Uses `-E`, `-S`, `-c`, `-x` options to change processing modes.

6. **Environment Variables**: Sets `GCC_EXEC_PREFIX` and `COMPILER_PATH` to influence driver behavior.

The script is designed to trigger the reset logic by:
- Processing multiple input files with conflicting options in a single invocation
- Mixing informational options (help/version) with compilation options
- Using response files (setting `at_file_supplied`)
- Changing dump-related options that need to be freed and reinitialized
- Switching between different driver modes

The `|| true` after commands allows the script to continue even if gcc exits with an error (which often happens with conflicting options), since we're testing the driver's internal state transitions, not successful compilation.
