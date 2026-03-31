Here's a shell script that implements the requirements to cover the reset logic in gcc.cc:

```bash
#!/bin/bash
set -e

# Clean up from previous runs
cleanup() {
    rm -f test1.c test2.c test3.c opts.txt
    rm -rf dump d1 d2 mydir asm
    rm -f *.o *.i *.s *.out output prog dummy combined.o
    rm -f mybase.* preproc.* obj.*
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

# Create response file with dump options
cat > opts.txt << 'EOF'
-save-temps=cwd
-dumpdir=./dump
-dumpbase=response_base
EOF

echo "=== Test 1: Basic state reset with mixed options ==="
# This should trigger reset between processing different files
gcc -save-temps -dumpdir=./mydir -dumpbase=mybase test1.c -c -o test1.o 2>/dev/null
gcc --version test1.o -o prog 2>/dev/null || true

echo "=== Test 2: Complex multi-file state transition ==="
# Chain multiple processing modes with different dump options
gcc -E -dumpbase=preproc test1.c -S -dumpdir=./asm test2.c -c -dumpbase=obj test3.c -o combined.o 2>/dev/null

echo "=== Test 3: Response file with help combination ==="
# Use @file syntax followed by help request
gcc @opts.txt test1.c --help=optimizers -o dummy 2>/dev/null || true

echo "=== Test 4: Multiple compilation units with conflicting dump options ==="
# Force state reset between files with conflicting dump options
gcc -save-temps=cwd -dumpdir=./d1 test1.c -save-temps=obj -dumpdir=./d2 test2.c -o output 2>/dev/null

echo "=== Test 5: Mixed help/version with actual compilation ==="
# Help/version options in different positions
gcc test1.c --help -c 2>/dev/null || true
gcc -o test.out --version test2.c 2>/dev/null || true
gcc --target-help test3.c -S 2>/dev/null || true

echo "=== Test 6: Response file with command-line overrides ==="
# @file followed by conflicting command-line options
gcc @opts.txt -dumpbase-ext=.alt -dumpdir=./override test1.c -c 2>/dev/null

echo "=== Test 7: Mode switches with dump options ==="
# Test driver mode changes
gcc -x c -save-temps -dumpbase=mode_test test1.c -E -o test1.i 2>/dev/null
gcc -shared -dumpdir=./shared test2.c -c -o test2.so 2>/dev/null || true

echo "=== Test 8: Environment variables affecting driver ==="
# Set environment variables that might affect driver state
GCC_EXEC_PREFIX=./ gcc -save-temps test1.c -c 2>/dev/null || true
COMPILER_PATH=/usr/bin gcc --version 2>/dev/null

echo "=== Test 9: Multiple response files ==="
cat > opts2.txt << 'EOF'
-dumpbase=second
-dumpdir=./second_dump
EOF
gcc @opts.txt test1.c @opts2.txt test2.c -o multi_response 2>/dev/null

echo "=== Test 10: Save-temps variants ==="
# Test different save-temps options
gcc -save-temps=obj -dumpdir=obj_dir test1.c -c 2>/dev/null
gcc -save-temps=cwd -dumpbase=cwd_base test2.c -c 2>/dev/null

echo "=== Test 11: Combined complex invocation ==="
# Most complex test combining multiple techniques
gcc @opts.txt -save-temps=obj -dumpdir=final_dump \
    test1.c -E -dumpbase=pre1 \
    test2.c -S -dumpdir=asm_dir \
    test3.c -c -dumpbase=obj3 \
    --help=warnings -o final.out 2>/dev/null || true

echo "=== Test 12: Reset after error conditions ==="
# Trigger errors that might affect state
gcc -dumpdir=./error_dir -dumpbase=error_base nonexistent.c -o error.out 2>/dev/null || true
# Then compile normally to trigger reset
gcc test1.c -c 2>/dev/null

echo "=== Test 13: At-file supplied flag testing ==="
# Ensure at_file_supplied is set and reset
cat > simple.txt << 'EOF'
test1.c
EOF
gcc @simple.txt -c 2>/dev/null
# Follow with non-@file invocation
gcc test2.c -c 2>/dev/null

echo "=== Test 14: Target system root variations ==="
# Test with target system root options
gcc -print-sysroot test1.c -c 2>/dev/null || true
gcc --sysroot=/ test2.c -c 2>/dev/null || true

echo "=== Test 15: Time reporting with dump options ==="
# Combine time reporting with dump options
gcc -ftime-report -dumpdir=time_dir -dumpbase=time_base test1.c -c 2>/dev/null || true

echo "=== All tests completed successfully ==="
cleanup
```

To run this test script:

1. Save it as `test_gcc_reset.sh`
2. Make it executable: `chmod +x test_gcc_reset.sh`
3. Run it: `./test_gcc_reset.sh`

The script will:
1. Create multiple C source files with trivial content
2. Create response files with various options
3. Execute 15 different test scenarios that combine the techniques mentioned in the requirements
4. Clean up all generated files

Key aspects that trigger the uncovered reset logic:

1. **Multiple files with conflicting dump options** (Tests 1, 4, 10): Forces the driver to reset `dumpdir`, `dumpbase`, and `save_temps_flag` between files
2. **Response file usage** (Tests 3, 6, 9, 13): Sets `at_file_supplied` flag that needs resetting
3. **Mixed help/version with compilation** (Tests 3, 5, 11): Causes the driver to process help requests then reset for compilation
4. **Mode switches** (Tests 2, 7): Changes between `-E`, `-S`, `-c` modes triggering state resets
5. **Environment variables** (Test 8): Tests paths where `target_system_root` might be affected
6. **Complex combinations** (Test 11): Maximizes state changes to ensure reset logic is exercised

The script suppresses error output (with `2>/dev/null`) since some combinations will fail (like mixing help with output files), but the driver should still execute the reset logic before exiting. The `|| true` allows the script to continue even when gcc exits with an error.
