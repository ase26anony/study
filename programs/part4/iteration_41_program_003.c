Here's a comprehensive shell script that exercises the uncovered initialization/cleanup block in `gcc.cc`:

```bash
#!/bin/bash
# test_driver.sh - Exercise GCC driver initialization and cleanup logic

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

# Create a directory for dump files
mkdir -p test_dump_dir

echo "=== Testing GCC driver initialization/cleanup block ==="
echo

# 1. Basic help and version flags (affects print_help_list, print_version)
echo "1. Testing help/version flags..."
gcc --version > /dev/null
gcc --help > /dev/null 2>&1
gcc --target-help > /dev/null 2>&1
echo "   Done."

# 2. Verbose and diagnostic flags (affects verbose_only_flag)
echo "2. Testing verbose/diagnostic flags..."
gcc -v -c test.c 2>&1 | head -5 > /dev/null
gcc -### -c test.c 2>&1 > /dev/null
echo "   Done."

# 3. Save-temps and dump options (affects save_temps_flag, dumpdir, dumpbase, etc.)
echo "3. Testing save-temps and dump options..."
gcc -save-temps -dumpdir ./test_dump_dir -dumpbase mytest -dumpbase-ext .ext -c test.c -o test_save.o
# Clean up generated files
rm -f test_save.* mytest.* test.i test.s test.o
echo "   Done."

# 4. At-file handling (affects at_file_supplied)
echo "4. Testing at-file handling..."
gcc @args.txt
rm -f test.o
echo "   Done."

# 5. Sysroot and target specifications (affects target_system_root, spec_machine)
echo "5. Testing sysroot and target flags..."
# Note: Using / as sysroot which should exist on most systems
gcc --sysroot=/ -c test.c 2>/dev/null || true
gcc -isysroot /usr/include -c test.c 2>/dev/null || true
# Try with target specification
gcc -target x86_64-linux-gnu -c test.c 2>/dev/null || true
echo "   Done."

# 6. Linker selection and subprocess help (affects use_ld, print_subprocess_help)
echo "6. Testing linker selection and subprocess help..."
gcc -fuse-ld=bfd -c test.c 2>/dev/null || true
gcc -fuse-ld=gold -c test.c 2>/dev/null || true
gcc -fuse-ld=lld -c test.c 2>/dev/null || true
gcc --help=common > /dev/null 2>&1
gcc --help=target > /dev/null 2>&1
echo "   Done."

# 7. Time reporting and resource management (affects report_times_to_file)
echo "7. Testing time and memory reporting..."
gcc -ftime-report -c test.c 2>&1 | head -10 > /dev/null
gcc -fmem-report -c test.c 2>&1 | head -10 > /dev/null
echo "   Done."

# 8. Error cases and cleanup (affects greatest_status)
echo "8. Testing error cases for cleanup..."
gcc invalid_file_that_does_not_exist.c 2>/dev/null || true
gcc -invalid-flag 2>/dev/null || true
echo "   Done."

# 9. Combination tests to trigger multiple initializations
echo "9. Testing combinations and sequences..."
# Sequence that should trigger full initialization and cleanup
gcc --version > /dev/null
gcc -save-temps=cwd -dumpbase combo_test -c test.c
rm -f combo_test.* test.i test.s test.o
gcc -v --help 2>&1 | head -5 > /dev/null
gcc @args.txt
rm -f test.o
echo "   Done."

# 10. Test with different output base (affects outbase)
echo "10. Testing output base variations..."
gcc -save-temps -dumpbase_ext .myext -dumpbase mydump -c test.c -o custom_out.o
rm -f custom_out.o mydump.* test.i test.s

# Test with trailing dash in dumpdir (affects dumpdir_trailing_dash_added)
gcc -dumpdir "./test_dump_dir/" -dumpbase testdump -c test.c 2>/dev/null || true
echo "   Done."

# Cleanup
rm -f test.c args.txt
rm -rf test_dump_dir

echo
echo "=== All tests completed ==="
echo "The GCC driver has been exercised through various initialization"
echo "and cleanup scenarios covering the target lines in gcc.cc"
```

This script systematically tests the uncovered block by:

1. **Basic initialization**: Help/version flags set `print_help_list`, `print_version`
2. **Verbose flags**: `-v` and `-###` affect `verbose_only_flag`
3. **Dump file handling**: `-save-temps`, `-dumpdir`, `-dumpbase` exercise the allocation and cleanup of `dumpdir`, `dumpbase`, `dumpbase_ext`, `outbase`
4. **At-file handling**: `@args.txt` tests `at_file_supplied`
5. **Sysroot/target**: `--sysroot`, `-isysroot`, `-target` affect `target_system_root`, `target_system_root_changed`, `spec_machine`
6. **Linker control**: `-fuse-ld=` options set `use_ld`, subprocess help affects `print_subprocess_help`
7. **Time reporting**: `-ftime-report`, `-fmem-report` affect `report_times_to_file`
8. **Error handling**: Invalid files/flags trigger error cleanup affecting `greatest_status`
9. **Combinations**: Sequential invocations ensure cleanup between calls
10. **Edge cases**: Trailing dash in dumpdir tests `dumpdir_trailing_dash_added`

The script creates temporary files and directories, runs GCC with various flag combinations, and cleans up after itself. Each invocation triggers the driver's initialization logic, and the sequence ensures the cleanup/reset code is exercised between calls.
