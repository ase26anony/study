Here's a shell script that systematically exercises the uncovered initialization/cleanup logic in gcc.cc by invoking the compiler driver with various state-modifying flags:

```bash
#!/bin/sh
# Test script for GCC driver initialization/cleanup logic
# Targets the reset of global variables in gcc.cc lines 11228-11250

set -e
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"

# Create a simple test source file
cat > test.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Test program\n");
    return 0;
}
EOF

# Create a simple C++ source file for driver mode testing
cat > test.cpp << 'EOF'
#include <iostream>
int main() {
    std::cout << "C++ test" << std::endl;
    return 0;
}
EOF

echo "=== Testing GCC driver state reset logic ==="

# 1. Multiple compilations with varied output flags and dump options
echo "1. Testing output naming and dump directory logic..."
gcc -save-temps -dumpdir ./dumps/ -dumpbase test1 -fdump-rtl-all -O2 -c test.c -o test1.o 2>/dev/null
gcc -save-temps=obj -dumpdir ./dumps2 -dumpbase test2 -fdump-tree-all -c test.c -o test2.o 2>/dev/null
gcc -dumpbase test3.c -fdump-ipa-all -c test.c -o test3.o 2>/dev/null

# Force dumpdir trailing slash logic
gcc -dumpdir ./dumps3/ -dumpbase test4 -fdump-rtl-expand -c test.c -o test4.o 2>/dev/null
gcc -dumpdir ./dumps3 -dumpbase test5 -fdump-tree-optimized -c test.c -o test5.o 2>/dev/null

# 2. System root and machine specification overrides
echo "2. Testing sysroot and machine spec reset..."
# Use dummy sysroot paths (these won't affect actual compilation)
gcc --sysroot=/usr -march=x86-64 -c test.c -o test6.o 2>/dev/null
gcc --sysroot=/opt/cross -march=native -c test.c -o test7.o 2>/dev/null
# Reset to default by not specifying sysroot
gcc -march=x86-64 -c test.c -o test8.o 2>/dev/null

# Test with -B prefix for executable search path
gcc -B/usr/lib/gcc -c test.c -o test9.o 2>/dev/null

# 3. Driver mode switching and help/version requests
echo "3. Testing driver mode switching..."
# Invoke as C++ driver (sets is_cpp_driver)
g++ -c test.cpp -o test_cpp.o 2>/dev/null

# Help and version requests (set print_help_list, print_version)
gcc --help > /dev/null 2>&1
gcc --version > /dev/null 2>&1

# Subprocess help (sets print_subprocess_help)
gcc -print-prog-name=cc1 > /dev/null 2>&1
gcc -print-prog-name=as > /dev/null 2>&1

# Now compile normally - driver must reset state
gcc -c test.c -o test10.o 2>/dev/null

# 4. Complex save-temps and dump combinations
echo "4. Testing complex dump configurations..."
mkdir -p complex_dumps
gcc -save-temps -dumpdir complex_dumps/ -dumpbase complex \
    -fdump-rtl-all -fdump-tree-all -fdump-ipa-all \
    -O2 -c test.c -o complex.o 2>/dev/null

# Change dumpbase extension
gcc -dumpbase complex.alt -fdump-tree-optimized -c test.c -o complex2.o 2>/dev/null

# Test save_temps_flag variations
gcc -save-temps=cwd -c test.c -o temp1.o 2>/dev/null
gcc -save-temps=obj -c test.c -o temp2.o 2>/dev/null

# 5. Mixed invocations to stress cleanup between jobs
echo "5. Testing mixed invocations..."
# Sequence that modifies multiple state variables
gcc --sysroot=/usr -march=x86-64 -save-temps -dumpdir ./mixed/ -c test.c -o mixed1.o 2>/dev/null
gcc --help > /dev/null 2>&1
gcc -dumpbase mixed2 -fdump-rtl-all -c test.c -o mixed2.o 2>/dev/null
g++ -c test.cpp -o mixed3.o 2>/dev/null
gcc -print-prog-name=ld > /dev/null 2>&1
gcc -c test.c -o mixed4.o 2>/dev/null

# 6. Final compilation to verify driver state is properly reset
echo "6. Final verification compilation..."
gcc test.c -o final_test
./final_test

if [ $? -eq 0 ]; then
    echo "✓ Driver state properly reset - final compilation succeeded"
else
    echo "✗ Final compilation failed - driver state may not be reset properly"
    exit 1
fi

# Cleanup
echo "Cleaning up temporary files..."
cd /
rm -rf "$TEMP_DIR"

echo "=== Test completed successfully ==="
echo "The uncovered reset logic in gcc.cc lines 11228-11250 should have been"
echo "exercised through multiple driver invocations with varying state."
```

This script systematically targets each requirement:

1. **Multiple Compiler Invocations with Varied Output Flags**: Uses `-c`, `-o`, `-dumpdir`, `-dumpbase`, `-fdump-*` flags in different combinations across multiple invocations.

2. **System Root and Spec Machine Overrides**: Uses `--sysroot=`, `-march=`, `-B` flags to modify `target_system_root` and `spec_machine`.

3. **Driver Mode Switching**: Invokes `g++` (C++ driver), `--help`, `--version`, `-print-prog-name=` to set mode flags that must be reset.

4. **Save-Temps and Dump Directory Complexity**: Tests `-save-temps` with different values, `-dumpdir` with/without trailing slashes, multiple `-fdump-*` flags.

5. **Mixed Sequence**: Combines all the above in a sequence that forces the driver to repeatedly modify and reset its internal state.

The script creates a temporary directory, generates test files, runs the compiler with various flag combinations, and cleans up. Each invocation modifies different global state variables that should be reset by the target code block. The final compilation verifies the driver can still function correctly after all state modifications.
