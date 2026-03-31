Here's a shell script that implements the requirements to exercise the uncovered reset logic in gcc.cc:

```bash
#!/bin/bash

# Test script to exercise gcc driver reset logic (lines 11228-11250 in gcc.cc)
# This script creates multiple test files and invokes gcc with various combinations
# of options that should trigger the reset block.

set -e

# Create a temporary directory for test files
TESTDIR=$(mktemp -d /tmp/gcc_reset_test_XXXXXX)
cd "$TESTDIR"

echo "Test directory: $TESTDIR"

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

# Create a response file with various options
cat > opts.txt << 'EOF'
-save-temps=cwd
-dumpdir=./response_dump
-dumpbase=response_base
-O2
EOF

# Create another response file with different options
cat > opts2.txt << 'EOF'
-ffast-math
-ftree-vectorize
-dumpbase-ext=.resp_ext
EOF

# Test 1: Basic state reset with dump options
echo "Test 1: Basic state reset with dump options"
gcc -save-temps -dumpdir=./mydir -dumpbase=mybase test1.c -c -o test1.o 2>/dev/null || true
gcc --version test1.o -o prog1 2>/dev/null || true

# Test 2: Complex multi-file state transition with different modes
echo "Test 2: Complex multi-file state transition"
gcc -E -dumpbase=preproc test1.c -S -dumpdir=./asm test2.c -c -dumpbase=obj test3.c -o combined.o 2>/dev/null || true

# Test 3: Response file and help combination
echo "Test 3: Response file and help combination"
gcc @opts.txt test1.c --help=optimizers -o dummy 2>/dev/null || true

# Test 4: Mixed help/version with actual compilation
echo "Test 4: Mixed help/version with compilation"
gcc test1.c --help -c test2.c 2>/dev/null || true
gcc --target-help test1.c -o test1 2>/dev/null || true
gcc -v test2.c --version test3.c 2>/dev/null || true

# Test 5: Multiple compilation units with conflicting dump options
echo "Test 5: Multiple files with conflicting dump options"
gcc -save-temps=cwd -dumpdir=./dump1 test1.c -save-temps=obj -dumpdir=./dump2 test2.c -o output 2>/dev/null || true

# Test 6: Response file followed by conflicting command-line options
echo "Test 6: Response file with conflicting options"
gcc @opts.txt -dumpbase-ext=.alt test1.c -dumpdir=./override test2.c 2>/dev/null || true

# Test 7: Mode switches with dump options
echo "Test 7: Mode switches with dump options"
gcc -x c -save-temps -dumpbase=switch1 test1.c -x c -S -dumpdir=./switch_dump test2.c 2>/dev/null || true

# Test 8: Multiple response files
echo "Test 8: Multiple response files"
gcc @opts.txt @opts2.txt test1.c test2.c -o multi_response 2>/dev/null || true

# Test 9: Combination of all techniques
echo "Test 9: Comprehensive combination"
gcc -save-temps=cwd -dumpdir=./final1 test1.c @opts.txt --help=warnings -S -dumpbase=final2 test2.c -c -dumpdir=./final3 test3.c --version -o final_output 2>/dev/null || true

# Test 10: Environment variables with complex invocation
echo "Test 10: With environment variables"
GCC_EXEC_PREFIX="" COMPILER_PATH="" gcc -save-temps -dumpdir=./env_dump -dumpbase=env_base test1.c -E test2.c -c test3.c -o env_output 2>/dev/null || true

# Cleanup
echo "Cleaning up..."
cd /
rm -rf "$TESTDIR"

echo "All tests executed successfully (note: many invocations will fail due to conflicting options, but that's expected)"
echo "The goal was to execute the driver reset logic, not necessarily successful compilation"
```

To run this test:

1. Save the script as `test_gcc_reset.sh`
2. Make it executable: `chmod +x test_gcc_reset.sh`
3. Run it: `./test_gcc_reset.sh`

**Key aspects of this test:**

1. **Multiple Compilation Units**: Uses `test1.c`, `test2.c`, `test3.c` with different options for each.

2. **Mixed Help/Version Requests**: Combines `--help`, `--version`, `--target-help`, `--help=optimizers`, `--help=warnings` with actual source files.

3. **Response File Syntax**: Uses `@opts.txt` and `@opts2.txt` to set `at_file_supplied` flag.

4. **Save-temps and Dump Options**: Uses `-save-temps`, `-save-temps=cwd`, `-save-temps=obj`, `-dumpdir`, `-dumpbase`, `-dumpbase-ext` with conflicting values.

5. **Driver Mode Switches**: Uses `-E`, `-S`, `-c`, `-x c` options to change processing modes.

6. **Environment Variables**: Tests with `GCC_EXEC_PREFIX` and `COMPILER_PATH` set.

The script intentionally includes many invocations that will fail (due to conflicting options like mixing `--help` with `-o`), but that's acceptable because the goal is to execute the driver's internal reset logic, not to produce valid output. Each invocation exercises different paths through the driver's state machine, increasing the likelihood of hitting the uncovered reset block.
