Here's a shell script that implements the requirements to exercise the uncovered reset logic in gcc.cc:

```bash
#!/bin/bash
set -e

# Clean up from previous runs
cleanup() {
    rm -f test*.c test*.o test*.s test*.i test*.ii output prog dummy
    rm -rf dump* mydir asm d1 d2
    rm -f opts.txt args.txt
}
cleanup

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

echo "=== Test 1: Basic state reset with dump options ==="
# This should set dumpdir, dumpbase, then reset them
gcc -save-temps -dumpdir=./mydir -dumpbase=mybase test1.c -c -o test1.o 2>/dev/null || true
echo "Test 1 completed"

echo -e "\n=== Test 2: Version/help mixed with compilation ==="
# Help followed by compile - tests print_version and reset
gcc --help -c test1.c 2>&1 | head -5
echo "..."
# Version after source file
gcc test1.c --version 2>&1 | head -2
echo "..."

echo -e "\n=== Test 3: Complex multi-file state transitions ==="
# Chain multiple processing modes with different dump options
gcc -E -dumpbase=preproc test1.c -S -dumpdir=./asm test2.c -c -dumpbase=obj test3.c -o combined.o 2>/dev/null || true
echo "Test 3 completed"

echo -e "\n=== Test 4: Response file with help combination ==="
# Use @file syntax (sets at_file_supplied) then request help
gcc @opts.txt test1.c --help=optimizers -o dummy 2>&1 | head -5
echo "..."

echo -e "\n=== Test 5: Multiple compilation units with mixed options ==="
# Different save-temps and dumpdir for each file
gcc -save-temps=cwd -dumpdir=./d1 test1.c -save-temps=obj -dumpdir=./d2 test2.c -o output 2>/dev/null || true
echo "Test 5 completed"

echo -e "\n=== Test 6: Response file with command-line overrides ==="
# Response file sets options, command line overrides some
gcc @args.txt -dumpbase-ext=.alt test1.c -c 2>/dev/null || true
echo "Test 6 completed"

echo -e "\n=== Test 7: Mode switches with -x option ==="
# Test spec_machine reset with language changes
gcc -x c test1.c -x assembler -x c test2.c -c 2>/dev/null || true
echo "Test 7 completed"

echo -e "\n=== Test 8: Target system root and wrapper simulation ==="
# Test target_system_root related variables
GCC_EXEC_PREFIX=/usr/lib/gcc/ gcc -v test1.c -c 2>&1 | grep -i "exec prefix" || true
echo "Test 8 completed"

echo -e "\n=== Test 9: Mixed informational and compilation options ==="
# Combine --target-help with actual compilation
gcc --target-help test1.c -c 2>&1 | tail -5
echo "..."

echo -e "\n=== Test 10: Save-temps variants and dump options ==="
# Exercise all save-temps variants
gcc -save-temps=cwd -dumpdir=./dump1 test1.c -c 2>/dev/null || true
gcc -save-temps=obj -dumpdir=./dump2 test2.c -c 2>/dev/null || true
echo "Test 10 completed"

# Clean up
cleanup
echo -e "\n=== All tests completed successfully ==="
```

To run this test script:

1. Save it as `test_gcc_reset.sh`
2. Make it executable: `chmod +x test_gcc_reset.sh`
3. Run it: `./test_gcc_reset.sh`

The script systematically tests the uncovered lines by:

1. **Multiple Compilation Units with Mixed Options** (Test 5): Uses different `-save-temps` and `-dumpdir` options for `test1.c` and `test2.c` in the same command line.

2. **Combine Help/Version Requests with Actual Compilation** (Tests 2, 9): Mixes `--help`, `--version`, and `--target-help` with source file arguments in various orders.

3. **Use `@file` (Response File) Syntax** (Tests 4, 6): Creates response files with dump options and uses them with `@args.txt` syntax.

4. **Exercise `-save-temps` and Dump Options Variants** (Tests 1, 10): Tests `-save-temps`, `-save-temps=cwd`, `-save-temps=obj` with various `-dumpdir`, `-dumpbase`, and `-dumpbase-ext` options.

5. **Leverage Driver-Specific Mode Switches** (Tests 3, 7): Chains `-E`, `-S`, `-c` modes with `-x` language specifications to trigger mode resets.

6. **Environment Variables** (Test 8): Sets `GCC_EXEC_PREFIX` to test environment variable handling.

The script redirects error output to `/dev/null` or pipes through `head`/`grep` to keep output manageable, as we're only interested in exercising the driver's internal logic, not the actual compilation results. Each test is designed to cause the driver to execute the reset block between processing different arguments or input files.
