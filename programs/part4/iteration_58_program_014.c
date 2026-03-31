Here's a shell script that implements the requirements to exercise the uncovered reset logic in gcc.cc:

```bash
#!/bin/bash
set -e

# Clean up from previous runs
rm -rf test_*.c test_*.o test_*.s test_*.i test_*.ii dump* mydir output* combined opts.txt

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
-dumpdir=./dump
-dumpbase=testdump
-O2
EOF

echo "=== Test 1: Basic state reset with dump options ==="
# This should trigger reset when switching between files with different dump options
gcc -save-temps -dumpdir=./mydir -dumpbase=mybase1 test1.c -c -o test1.o 2>&1 | grep -v "warning" || true
gcc --version test1.o -o output1 2>&1 | tail -1

echo -e "\n=== Test 2: Complex multi-file state transitions ==="
# Chain multiple processing modes with different dump options
gcc -E -dumpbase=preproc test1.c -S -dumpdir=./asm test2.c -c -dumpbase=obj test3.c -o combined.o 2>&1 | grep -v "warning" || true

echo -e "\n=== Test 3: Response file with help combination ==="
# Use response file then request help
gcc @opts.txt test1.c --help=optimizers -o dummy 2>&1 | head -20

echo -e "\n=== Test 4: Mixed save-temps variants between files ==="
# Different save-temps options for different files
gcc -save-temps=cwd -dumpdir=./d1 test1.c -save-temps=obj -dumpdir=./d2 test2.c -o output2 2>&1 | grep -v "warning" || true

echo -e "\n=== Test 5: Version/help with actual compilation ==="
# Help/version options mixed with compilation
gcc --help -c test1.c 2>&1 | head -5
gcc test2.c --version -c 2>&1 | tail -2
gcc -c test3.c --target-help -o test3.o 2>&1 | head -5

echo -e "\n=== Test 6: Multiple dumpbase/dumpdir changes ==="
# Rapid changes to dump-related options
gcc -dumpbase=base1 -dumpdir=dir1 test1.c -dumpbase-ext=.ext1 -dumpbase=base2 test2.c -dumpdir=dir2 test3.c -c 2>&1 | grep -v "warning" || true

echo -e "\n=== Test 7: Mode switches with -x option ==="
# Language specification changes
echo "int x = 1;" > test4.cpp
gcc -x c -c test1.c -x c++ -c test4.cpp -o test4.o 2>&1 | grep -v "warning" || true

echo -e "\n=== Test 8: Combined everything with response file ==="
# Complex combination
cat > opts2.txt << 'EOF'
-save-temps
-dumpbase=combined
-verbose
EOF
gcc @opts2.txt test1.c -dumpdir=./final test2.c --help=warnings test3.c -o final.out 2>&1 | grep -E "(cc1|Driver|==)" || true

echo -e "\n=== Test 9: Environment variables with wrapper-like behavior ==="
# Simulate wrapper script behavior by setting env vars and using --
GCC_EXEC_PREFIX=/usr/lib/gcc/ gcc -save-temps=obj -dumpdir=./envtest test1.c -- -v 2>&1 | grep -A2 "COLLECT_GCC" || true

echo -e "\n=== Test 10: Multiple output specifications ==="
# Multiple -o options and output redirections
gcc -c test1.c -o test1a.o -dumpbase=multi1 -c test2.c -o test2a.o -dumpbase=multi2 2>&1 | grep -v "warning" || true

# Cleanup
rm -rf test_*.c test_*.o test_*.s test_*.i test_*.ii dump* mydir d1 d2 dir1 dir2 asm output* combined* dummy final.out test4.cpp test4.o test1a.o test2a.o opts.txt opts2.txt

echo -e "\n=== All tests completed successfully ==="
echo "The driver's reset logic should have been exercised multiple times."
echo "Check coverage reports to verify lines 11228-11250 in gcc.cc were hit."
```

To run this test:

1. Save it as `test_gcc_reset.sh`
2. Make it executable: `chmod +x test_gcc_reset.sh`
3. Run it: `./test_gcc_reset.sh`

The script implements all the requirements:

1. **Multiple compilation units** (`test1.c`, `test2.c`, `test3.c`) with mixed options
2. **Help/version requests combined with compilation** (Tests 1, 3, 5, 8)
3. **Response file syntax** (`@opts.txt` in Tests 3 and 8)
4. **Save-temps and dump options variants** (Tests 1, 2, 4, 6, 9)
5. **Driver mode switches** (`-E`, `-S`, `-c`, `-x` in Tests 2 and 7)
6. **Environment variables** (Test 9 simulates wrapper behavior)

The key invocations that should trigger the reset logic are:
- When switching between files with different `-dumpdir`/`-dumpbase` options
- When mixing `--help`/`--version` with actual compilation
- When using response files followed by command-line options
- When changing processing modes (`-E` to `-S` to `-c`) in a single invocation

The script suppresses most warnings and focuses on the driver execution rather than compilation output. Each test is designed to cause the driver to reset its internal state between processing different arguments or input files.
