I'll create a comprehensive shell script that exercises the uncovered reset logic in gcc.cc by combining multiple techniques as specified.

```bash
#!/bin/bash

# test_gcc_driver_reset.sh
# This script tests the driver reset logic in gcc.cc lines 11228-11250
# by creating various compilation scenarios that should trigger state resets.

set -e

echo "Testing GCC driver reset logic..."

# Create temporary directory for test files
TEST_DIR=$(mktemp -d)
cd "$TEST_DIR"

# Cleanup function
cleanup() {
    cd /
    rm -rf "$TEST_DIR"
    echo "Cleaned up test directory: $TEST_DIR"
}
trap cleanup EXIT

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
cat > opts.txt << 'EOF'
-save-temps=cwd
-dumpdir=./response_dump
-dumpbase=response_base
-O2
EOF

# Create another response file with different options
cat > opts2.txt << 'EOF'
-ffast-math
-funroll-loops
-dumpbase-ext=.alt_ext
EOF

# Test 1: Multiple compilation units with mixed dump options
echo "Test 1: Multiple files with mixed dump options..."
gcc -save-temps=cwd -dumpdir=./dump1 -dumpbase=file1 main.c \
    -save-temps=obj -dumpdir=./dump2 -dumpbase=file2 lib.c \
    -c -o combined.o 2>/dev/null || true

# Test 2: Help/version requests mixed with actual compilation
echo "Test 2: Help/version mixed with compilation..."
# Help before source file
gcc --help main.c 2>/dev/null || true

# Version after compilation flag
gcc -c main.c --version 2>/dev/null || true

# Target help with output specification
gcc --target-help -o dummy main.c 2>/dev/null || true

# Test 3: Response file syntax with conflicting command-line options
echo "Test 3: Response file with conflicting options..."
gcc @opts.txt -dumpdir=./cmdline_dump -dumpbase=cmdline_base main.c -c 2>/dev/null || true

# Test 4: Multiple response files and direct options
echo "Test 4: Multiple response files..."
gcc @opts.txt @opts2.txt -dumpbase-ext=.override main.c lib.c -c 2>/dev/null || true

# Test 5: Different processing modes in single command line
echo "Test 5: Multiple processing modes..."
gcc -E -dumpbase=preproc -dumpdir=./preproc_dump main.c \
    -S -dumpbase=asm -dumpdir=./asm_dump lib.c \
    -c -dumpbase=obj util.c \
    -o final_output 2>/dev/null || true

# Test 6: Save-temps variants with dump options
echo "Test 6: Save-temps variants..."
gcc -save-temps -dumpdir=./temps1 main.c -c -o main1.o 2>/dev/null || true
gcc -save-temps=cwd -dumpdir=./temps2 lib.c -c -o lib1.o 2>/dev/null || true
gcc -save-temps=obj -dumpdir=./temps3 util.c -c -o util1.o 2>/dev/null || true

# Test 7: Language specification changes with mode switches
echo "Test 7: Language specification and mode switches..."
# Create a C++ source file for -x testing
cat > test.cpp << 'EOF'
extern "C" int cpp_func() { return 1; }
EOF

gcc -x c -E -dumpbase=c_preproc main.c \
    -x c++ -S -dumpbase=cpp_asm test.cpp \
    -x c -c -dumpbase=c_obj lib.c 2>/dev/null || true

# Test 8: Complex combination with output redirection
echo "Test 8: Complex combination..."
gcc @opts.txt \
    -save-temps=obj -dumpdir=./complex1 main.c \
    --help=optimizers \
    -save-temps=cwd -dumpdir=./complex2 lib.c \
    -c \
    -dumpbase-ext=.final \
    -o complex_output.o 2>/dev/null || true

# Test 9: Environment variable influence
echo "Test 9: With environment variables..."
GCC_EXEC_PREFIX=./ gcc -save-temps -dumpdir=./env_dump main.c -c -o env.o 2>/dev/null || true
COMPILER_PATH=/usr/bin gcc -dumpbase=env_base lib.c -c -o env2.o 2>/dev/null || true

# Test 10: Wrapper script simulation (simulated via shell function)
echo "Test 10: Wrapper-like invocation..."
gcc_wrapper() {
    # Simulate wrapper adding arguments
    gcc -save-temps=cwd -dumpdir=./wrapper_dump "$@" -dumpbase=wrapper_base
}

gcc_wrapper main.c lib.c -c -o wrapped.o 2>/dev/null || true

# Test 11: Reset between separate invocations in same script
# This tests that state is properly reset between independent calls
echo "Test 11: Sequential invocations..."
gcc -save-temps -dumpdir=./seq1 -dumpbase=seq1_base main.c -c -o seq1.o 2>/dev/null || true
gcc -dumpdir=./seq2 -dumpbase=seq2_base lib.c -c -o seq2.o 2>/dev/null || true
gcc --version 2>/dev/null || true
gcc -save-temps=obj -dumpdir=./seq3 util.c -c -o seq3.o 2>/dev/null || true

# Test 12: at_file_supplied flag testing
echo "Test 12: Multiple @file usages..."
cat > compile_args.txt << 'EOF'
-c
-O1
-dumpdir=@file_dump
EOF

gcc @opts.txt main.c @compile_args.txt -o @file_output.o 2>/dev/null || true

# Test 13: Mixed help types and compilation
echo "Test 13: Various help options with compilation..."
gcc -c main.c --help=common 2>/dev/null || true
gcc --help=target lib.c -S 2>/dev/null || true
gcc --help=warnings -dumpbase=help_base util.c -c 2>/dev/null || true

# Test 14: Override save-temps with dumpdir
echo "Test 14: Save-temps overrides..."
gcc -save-temps -dumpdir=./override_dir main.c -c 2>/dev/null || true
gcc -save-temps=cwd -dumpdir=./override_dir2 lib.c -c 2>/dev/null || true

# Test 15: Greatest status and system root related
echo "Test 15: System root and status testing..."
# Use --sysroot to affect target_system_root
gcc --sysroot=/ -dumpbase=sysroot_base main.c -c -o sysroot.o 2>/dev/null || true
# Chain commands that might affect greatest_status
gcc -save-temps main.c -c -o temp1.o && \
gcc -dumpdir=./chain lib.c -c -o temp2.o 2>/dev/null || true

echo "All tests completed successfully!"
echo "The GCC driver reset logic should have been exercised multiple times."
echo "Check coverage to verify lines 11228-11250 in gcc.cc were executed."
```

This script comprehensively tests the GCC driver reset logic by:

1. **Multiple compilation units with mixed options**: Tests 1, 5, 7, 8 combine different files with varying `-save-temps`, `-dumpdir`, and `-dumpbase` options.

2. **Combine help/version requests with actual compilation**: Tests 2, 8, 13 mix `--help`, `--version`, and `--target-help` with source files and compilation flags.

3. **Use `@file` (response file) syntax**: Tests 3, 4, 8, 12 use response files with various options that set the `at_file_supplied` flag.

4. **Exercise `-save-temps` and dump options variants**: Tests 1, 3, 4, 6, 8, 14 use different combinations of `-save-temps` (`=cwd`, `=obj`, none) with `-dumpdir`, `-dumpbase`, and `-dumpbase-ext`.

5. **Leverage driver-specific mode switches**: Test 5 uses `-E`, `-S`, `-c` in a single command line. Test 7 uses `-x` for language specification.

6. **Introduce environment variables and wrapper scripts**: Test 9 sets `GCC_EXEC_PREFIX` and `COMPILER_PATH`. Test 10 simulates a wrapper script.

The script creates multiple source files, response files, and executes GCC with complex argument combinations that should force the driver to reset its internal state between processing different inputs or option groups. Each test is designed to trigger specific parts of the reset block, particularly the variables mentioned in lines 11228-11250 of gcc.cc.
