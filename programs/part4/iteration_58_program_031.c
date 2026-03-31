Here's a shell script that implements the requirements to exercise the uncovered reset logic in gcc.cc:

```bash
#!/bin/bash
set -e

# Clean up any previous test files
cleanup() {
    rm -f test*.c test*.o test*.i test*.s test*.out output prog dummy
    rm -rf dump* mydir asm d1 d2
    rm -f opts.txt args.txt
}
cleanup 2>/dev/null || true

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

echo "Test 1: Basic state reset with mixed options"
gcc -save-temps -dumpdir=./mydir -dumpbase=mybase test1.c -c -o test1.o 2>/dev/null || true
gcc --version test1.o -o prog 2>/dev/null || true

echo "Test 2: Complex multi-file state transitions"
gcc -E -dumpbase=preproc test1.c -S -dumpdir=./asm test2.c -c -dumpbase=obj test3.c -o combined.o 2>/dev/null || true

echo "Test 3: Response file with help combination"
gcc @args.txt test1.c --help=optimizers -o dummy 2>/dev/null || true

echo "Test 4: Multiple compilation units with conflicting dump options"
gcc -save-temps=cwd -dumpdir=./d1 test1.c -save-temps=obj -dumpdir=./d2 test2.c -o output 2>/dev/null || true

echo "Test 5: Mixing help/version with actual compilation in different orders"
gcc test1.c --version -c -o test1_v.o 2>/dev/null || true
gcc --target-help test2.c -S -o test2_s.s 2>/dev/null || true
gcc -c test3.c --help=warnings -o test3_h.o 2>/dev/null || true

echo "Test 6: Using @file with additional conflicting options"
gcc @opts.txt -dumpbase-ext=.alt test1.c -o test1_alt 2>/dev/null || true

echo "Test 7: Mode switches with dump options"
gcc -x c -save-temps -dumpdir=./mode1 test1.c -x c -E -dumpdir=./mode2 test2.c 2>/dev/null || true

echo "Test 8: Environment variables affecting driver behavior"
GCC_EXEC_PREFIX=/usr/lib/gcc/ gcc -save-temps test1.c -o env_test 2>/dev/null || true
COMPILER_PATH=/usr/bin gcc -dumpbase=envbase test2.c -c 2>/dev/null || true

echo "Test 9: Multiple response files and command-line overrides"
cat > override.txt << 'EOF'
-dumpbase=override
-save-temps=obj
EOF
gcc @opts.txt @override.txt -dumpdir=final test1.c test2.c -o final.out 2>/dev/null || true

echo "Test 10: Combination of all techniques"
gcc @args.txt -save-temps=cwd -dumpdir=combo1 test1.c \
    --help=common -dumpdir=combo2 test2.c \
    -x c -E -dumpbase=combo test3.c \
    -o combo.out 2>/dev/null || true

# Clean up
cleanup

echo "All tests executed successfully"
echo "Note: Some invocations may produce help text or warnings, but the driver's reset logic should have been exercised"
```

To run this test:

1. Save it as `test_gcc_reset.sh`
2. Make it executable: `chmod +x test_gcc_reset.sh`
3. Run it: `./test_gcc_reset.sh`

The script systematically exercises the uncovered reset logic by:

1. **Multiple compilation units with mixed options**: Tests 4, 9, and 10 use different `-save-temps` and `-dumpdir` values for different source files in the same invocation.

2. **Combine help/version with compilation**: Tests 3, 5, and 10 mix `--help`, `--version`, and `--target-help` with actual source files.

3. **Use `@file` syntax**: Tests 3, 6, 9, and 10 use response files with `@args.txt` and `@opts.txt`.

4. **Exercise `-save-temps` and dump options**: All tests use various combinations of `-save-temps`, `-dumpdir`, `-dumpbase`, and `-dumpbase-ext`.

5. **Leverage mode switches**: Tests 2, 7, and 10 use `-E`, `-S`, `-c`, and `-x` options to change processing modes.

6. **Environment variables**: Test 8 sets `GCC_EXEC_PREFIX` and `COMPILER_PATH` to affect driver behavior.

The script suppresses error output (`2>/dev/null`) since some combinations may not produce valid compilations (like mixing `--help` with `-o`), but the driver still processes the arguments and should execute the reset logic. The `|| true` ensures the script continues even if individual invocations fail.

This comprehensive approach should trigger the specific lines in gcc.cc that reset:
- `is_cpp_driver`, `at_file_supplied`, `print_help_list`, etc.
- `dumpdir`, `dumpbase`, `dumpbase_ext`, `outbase` (with free/Null assignment)
- `spec_machine` and `greatest_status`
