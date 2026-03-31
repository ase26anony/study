Looking at the uncovered lines in `gcc.cc`, I need to create a test that triggers the initialization/cleanup logic for these global variables. Here's a comprehensive shell script that exercises all the mentioned scenarios:

```bash
#!/bin/bash
# test_driver.sh - Comprehensive test for GCC driver initialization/cleanup logic

set -e

# Create a minimal test C source file
cat > test.c << 'EOF'
int main() { return 0; }
EOF

# Create an at-file with compilation arguments
cat > args.txt << 'EOF'
-O2
-c
test.c
-o
test.o
EOF

# Create another at-file with different options
cat > args2.txt << 'EOF'
-v
-c
test.c
EOF

echo "=== Testing GCC driver initialization/cleanup logic ==="
echo

# 1. Basic help and version requests (affects print_help_list, print_version)
echo "1. Testing help and version flags:"
gcc --version > /dev/null
gcc --help > /dev/null 2>&1
gcc --target-help > /dev/null 2>&1
echo "   Done (help/version flags)"
echo

# 2. Verbose and diagnostic flags (affects verbose_only_flag)
echo "2. Testing verbose and diagnostic flags:"
gcc -v -c test.c 2>&1 | head -5 > /dev/null
gcc -### -c test.c 2>&1 > /dev/null
echo "   Done (verbose flags)"
echo

# 3. Save-temps and dump options (affects save_temps_flag, dumpdir, dumpbase, etc.)
echo "3. Testing save-temps and dump options:"
# First invocation with dump options
gcc -save-temps -dumpdir ./test_dump -dumpbase mytest -dumpbase-ext .c -c test.c 2>/dev/null || true
# Clean up
rm -f test.i test.s test.o mytest.*
rm -rf ./test_dump
# Second with different save-temps option
gcc -save-temps=cwd -c test.c 2>/dev/null || true
rm -f test.i test.s test.o
echo "   Done (save-temps/dump options)"
echo

# 4. At-file handling (affects at_file_supplied)
echo "4. Testing at-file handling:"
gcc @args.txt 2>/dev/null
rm -f test.o
gcc @args2.txt 2>&1 | head -10 > /dev/null
echo "   Done (at-file handling)"
echo

# 5. Sysroot and target specifications (affects target_system_root, spec_machine)
echo "5. Testing sysroot and target options:"
# Note: Using / as sysroot which should exist on most systems
gcc --sysroot=/ -c test.c 2>/dev/null || true
# Invalid sysroot to trigger cleanup
gcc --sysroot=/nonexistent/path -c test.c 2>/dev/null || true
# Target specification
gcc -target x86_64-linux-gnu -c test.c 2>/dev/null || true
echo "   Done (sysroot/target options)"
echo

# 6. Linker and subprocess control (affects use_ld, print_subprocess_help)
echo "6. Testing linker and subprocess options:"
gcc -fuse-ld=bfd -c test.c 2>/dev/null || true
gcc -fuse-ld=gold -c test.c 2>/dev/null || true
# Try lld if available
gcc -fuse-ld=lld -c test.c 2>/dev/null || true
# Subprocess help
gcc --help=common 2>&1 | head -5 > /dev/null
echo "   Done (linker/subprocess options)"
echo

# 7. Time reporting and resource management (affects report_times_to_file)
echo "7. Testing time and resource reporting:"
gcc -ftime-report -c test.c 2>&1 | head -20 > /dev/null
gcc -fmem-report -c test.c 2>&1 | head -20 > /dev/null
echo "   Done (time/resource reporting)"
echo

# 8. Error conditions (affects greatest_status)
echo "8. Testing error conditions:"
gcc invalid_file_that_does_not_exist.c 2>/dev/null || true
gcc -invalid-flag 2>/dev/null || true
echo "   Done (error conditions)"
echo

# 9. Combined complex scenario
echo "9. Testing combined complex scenario:"
# This should trigger multiple initializations and cleanups
gcc -v --help=target 2>&1 | head -10 > /dev/null
gcc -save-temps -dumpdir ./dump2 -c test.c 2>/dev/null || true
rm -rf ./dump2 test.i test.s test.o
gcc @args.txt 2>/dev/null
rm -f test.o
gcc -ftime-report -fuse-ld=bfd --sysroot=/ -c test.c 2>&1 | head -30 > /dev/null
echo "   Done (combined scenario)"
echo

# 10. Multiple rapid invocations to stress cleanup/reinitialization
echo "10. Stress testing rapid invocations:"
for i in {1..5}; do
    gcc -c test.c -o test_$i.o 2>/dev/null
    gcc -v -c test.c 2>&1 > /dev/null
    gcc --version > /dev/null
done
rm -f test_*.o
echo "   Done (stress test)"
echo

# Cleanup
rm -f test.c args.txt args2.txt test.i test.s test.o mytest.*
rm -rf ./test_dump ./dump2

echo "=== All tests completed ==="
```

This script systematically tests all the aspects mentioned in the requirements:

1. **Help and version flags** - Exercises `print_help_list`, `print_version`
2. **Verbose flags** - Exercises `verbose_only_flag`
3. **Save-temps and dump options** - Exercises `save_temps_flag`, `dumpdir`, `dumpbase`, `dumpbase_ext`, `outbase` and their cleanup
4. **At-file handling** - Exercises `at_file_supplied`
5. **Sysroot and target options** - Exercises `target_system_root`, `target_system_root_changed`, `spec_machine`
6. **Linker options** - Exercises `use_ld`, `print_subprocess_help`
7. **Time reporting** - Exercises `report_times_to_file`
8. **Error conditions** - Exercises `greatest_status`
9. **Combined scenarios** - Tests multiple initializations and cleanups
10. **Stress testing** - Rapid invocations to ensure proper cleanup between runs

The script uses `2>/dev/null` and output redirection to suppress expected error messages and keep the output clean. Each command is designed to trigger specific parts of the initialization/cleanup logic in the uncovered lines, particularly the resetting of variables to their default states.
