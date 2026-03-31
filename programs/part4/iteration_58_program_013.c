I'll create a comprehensive test script that exercises the uncovered reset logic in gcc.cc by combining multiple techniques as specified.

```bash
#!/bin/bash

# Test script to exercise the uncovered reset logic in gcc.cc lines 11228-11250
# This script creates multiple C source files and invokes gcc with various
# combinations of options to trigger the driver's internal state reset.

set -e

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcc_driver_test_XXXXXX)
cd "$TEST_DIR"

echo "Test directory: $TEST_DIR"

# Create simple C source files
cat > main.c << 'EOF'
int main() {
    return 0;
}
EOF

cat > lib.c << 'EOF'
int helper() {
    return 42;
}
EOF

cat > util.c << 'EOF'
void util_func() {
    // Do nothing
}
EOF

# Create a response file with various options
cat > args.txt << 'EOF'
-save-temps=cwd
-dumpdir=./response_dump
-dumpbase=response_base
-O2
EOF

# Create another response file with different options
cat > args2.txt << 'EOF'
--version
--help=optimizers
EOF

# Create a wrapper script to test GCC_EXEC_PREFIX
cat > gcc_wrapper.sh << 'EOF'
#!/bin/bash
# Simple wrapper that adds extra arguments
exec gcc -v "$@"
EOF
chmod +x gcc_wrapper.sh

echo "=== Test 1: Basic compilation with dump options ==="
# This should set dumpdir, dumpbase, etc.
gcc -save-temps -dumpdir=./test_dump -dumpbase=test_base main.c -c -o main.o 2>/dev/null || true

echo "=== Test 2: Mixed help and compilation options ==="
# Help followed by actual compilation - may trigger reset after help display
gcc --help -c main.c 2>&1 | head -5
echo "---"
gcc -c main.c --help=warnings 2>&1 | head -5

echo "=== Test 3: Multiple files with different save-temps options ==="
# This forces state reset between processing different files
gcc -save-temps=cwd -dumpdir=./d1 main.c -save-temps=obj -dumpdir=./d2 lib.c -o test_prog 2>/dev/null || true

echo "=== Test 4: Response file with command-line overrides ==="
# Uses @file syntax (sets at_file_supplied) with command-line overrides
gcc @args.txt -dumpbase-ext=.alt -dumpdir=./override_dump main.c -c 2>/dev/null || true

echo "=== Test 5: Chained processing modes with state changes ==="
# Different modes for different files should trigger resets
gcc -E -dumpbase=preproc main.c -S -dumpdir=./asm lib.c -c -dumpbase=obj util.c 2>/dev/null || true

echo "=== Test 6: Response file followed by help ==="
# Mix @file with help options
gcc @args.txt --help=target 2>&1 | head -5

echo "=== Test 7: Version and target help mixed with compilation ==="
gcc --version main.c -c -o version.o 2>&1 | head -2
echo "---"
gcc --target-help -dumpbase=help_base lib.c -S 2>/dev/null || true

echo "=== Test 8: Complex multi-option sequence ==="
# This sequence should exercise multiple state transitions
gcc -shared -dumpdir=./shared_dump lib.c -c \
    -static -dumpbase=static_base util.c -c \
    -save-temps=obj -dumpdir=./obj_dump main.c -c 2>/dev/null || true

echo "=== Test 9: Environment variable with wrapper ==="
# Test with environment variables that might affect driver state
GCC_EXEC_PREFIX="" COMPILER_PATH="" ./gcc_wrapper.sh -save-temps -dumpdir=./wrapper_dump main.c -c 2>/dev/null || true

echo "=== Test 10: Multiple response files ==="
# Use multiple @file arguments
gcc @args.txt @args2.txt main.c 2>&1 | head -5

echo "=== Test 11: Reset between compilation and linking ==="
# Compile separately then link with different options
gcc -save-temps -dumpdir=./compile_dump -c main.c -o main2.o 2>/dev/null || true
gcc -dumpbase=link_base -dumpdir=./link_dump main2.o lib.o -o final_prog 2>/dev/null || true

echo "=== Test 12: Save-temps variants ==="
# Test different save-temps options
gcc -save-temps=cwd -dumpdir=./cwd_dump main.c -c 2>/dev/null || true
gcc -save-temps=obj -dumpdir=./obj_dump2 lib.c -c 2>/dev/null || true

echo "=== Test 13: Dump options with output redirection ==="
# Test with -o option affecting outbase
gcc -save-temps -dumpdir=./out_dump -dumpbase=myprog main.c lib.c -o myprogram 2>/dev/null || true

echo "=== Test 14: Mixed language specifications ==="
# Use -x to specify language (might affect spec_machine)
cat > test.cpp << 'EOF'
extern "C" int cpp_func() { return 0; }
EOF
gcc -x c -dumpbase=c_base main.c -c \
    -x c++ -dumpbase=cpp_base test.cpp -c 2>/dev/null || true

echo "=== Test 15: Verbose and time reporting ==="
# Exercise verbose_only_flag and report_times_to_file
gcc -v -ftime-report -dumpdir=./verbose_dump main.c -c 2>&1 | head -10

# Cleanup
echo -e "\n=== Cleaning up ==="
cd /
rm -rf "$TEST_DIR"

echo "Test completed successfully!"
```

This script exercises the uncovered reset logic by:

1. **Multiple Compilation Units with Mixed Options**: Tests 3, 5, 8, and 14 compile multiple files with different `-save-temps`, `-dumpdir`, and `-dumpbase` options, forcing state resets.

2. **Combine Help/Version Requests with Actual Compilation**: Tests 2, 6, 7, and 10 mix `--help`, `--version`, and `--target-help` with compilation commands.

3. **Use `@file` (Response File) Syntax**: Tests 4, 6, and 10 use response files to set `at_file_supplied`.

4. **Exercise `-save-temps` and Dump Options Variants**: Tests 1, 3, 4, 8, 12, and 13 use various combinations of `-save-temps`, `-dumpdir`, `-dumpbase`, and `-dumpbase-ext`.

5. **Leverage Driver-Specific Mode Switches**: Test 5 chains `-E`, `-S`, and `-c` modes. Test 14 uses `-x` for language specification.

6. **Introduce Environment Variables and Wrapper Scripts**: Test 9 uses environment variables and a wrapper script.

The script creates temporary files, runs multiple gcc invocations that should trigger the reset logic, and cleans up. Each test is designed to exercise different paths through the driver's state machine, particularly focusing on the variables mentioned in the uncovered block.
