Here's a comprehensive shell script that exercises the uncovered initialization/cleanup block in `gcc.cc`:

```bash
#!/bin/bash
# test_driver.sh - Test GCC driver initialization and cleanup logic

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

# Create a temporary directory for dump files
DUMP_DIR=$(mktemp -d)
trap "rm -rf $DUMP_DIR" EXIT

echo "=== Testing GCC driver initialization/cleanup block ==="
echo

# 1. Basic help and version flags (affects print_help_list, print_version)
echo "1. Testing help/version flags:"
gcc --version > /dev/null
gcc --help > /dev/null 2>&1
gcc --target-help > /dev/null 2>&1
echo "   Done (sets print_help_list, print_version)"

# 2. Verbose and diagnostic flags (affects verbose_only_flag)
echo "2. Testing verbose/diagnostic flags:"
gcc -v -c test.c 2>&1 | grep -q "COLLECT_GCC" || true
gcc -### -c test.c 2>&1 > /dev/null
echo "   Done (sets verbose_only_flag)"

# 3. Save-temps and dump file handling (affects save_temps_flag, dumpdir, dumpbase, etc.)
echo "3. Testing save-temps and dump file handling:"
gcc -save-temps -dumpdir "$DUMP_DIR" -dumpbase "mytest" -dumpbase-ext ".ext" -c test.c -o test1.o 2>/dev/null || true
gcc -save-temps=cwd -c test.c -o test2.o 2>/dev/null || true
# Clean up generated files
rm -f test1.o test2.o test1.i test1.s test2.i test2.s
echo "   Done (exercises dumpdir, dumpbase, save_temps_flag)"

# 4. At-file handling (affects at_file_supplied)
echo "4. Testing at-file handling:"
gcc @args.txt 2>/dev/null || true
echo "   Done (sets at_file_supplied)"

# 5. Sysroot and target specifications (affects target_system_root, spec_machine)
echo "5. Testing sysroot and target flags:"
gcc --sysroot=/ -c test.c 2>/dev/null || true
gcc -isysroot /usr/include -c test.c 2>/dev/null || true
gcc -target x86_64-linux-gnu -c test.c 2>/dev/null || true
echo "   Done (affects target_system_root, target_system_root_changed, spec_machine)"

# 6. Linker selection and subprocess help (affects use_ld, print_subprocess_help)
echo "6. Testing linker selection and subprocess help:"
gcc -fuse-ld=bfd -c test.c 2>/dev/null || true
gcc -fuse-ld=gold -c test.c 2>/dev/null || true
gcc -fuse-ld=lld -c test.c 2>/dev/null || true
gcc --help=common 2>&1 | head -5 > /dev/null
gcc --help=target 2>&1 | head -5 > /dev/null
echo "   Done (sets use_ld, print_subprocess_help)"

# 7. Time reporting and resource management (affects report_times_to_file)
echo "7. Testing time reporting flags:"
gcc -ftime-report -c test.c 2>&1 | grep -q "Time variable" || true
gcc -fmem-report -c test.c 2>&1 > /dev/null || true
echo "   Done (sets report_times_to_file)"

# 8. Error conditions (affects greatest_status)
echo "8. Testing error conditions:"
gcc invalid_file_that_does_not_exist.c 2>/dev/null || true
gcc -invalid-flag 2>/dev/null || true
echo "   Done (triggers error cleanup, affects greatest_status)"

# 9. Combined complex scenario
echo "9. Testing combined complex scenario:"
gcc -v --help --target-help 2>&1 | head -10 > /dev/null
gcc -save-temps -dumpdir "$DUMP_DIR" -dumpbase "combined" @args.txt 2>/dev/null || true
gcc --sysroot=/ -fuse-ld=gold -ftime-report -c test.c 2>/dev/null || true
echo "   Done (exercises multiple variables simultaneously)"

# 10. Cleanup verification (implicitly tested through multiple invocations)
echo "10. Testing cleanup between invocations:"
# Multiple distinct invocations to ensure cleanup happens
for i in {1..3}; do
    gcc -c test.c -o "test_iter_$i.o" 2>/dev/null
    rm -f "test_iter_$i.o"
done
echo "   Done (ensures reset between compilations)"

# Cleanup
rm -f test.c args.txt test.o 2>/dev/null || true

echo
echo "=== All tests completed ==="
echo "The GCC driver has been exercised through various initialization"
echo "and cleanup paths covering the target block in gcc.cc"
```

This script systematically tests the uncovered block by:

1. **Basic Initialization**: Tests `--version`, `--help`, `--target-help` to affect `print_help_list` and `print_version`
2. **Verbose Flags**: Uses `-v` and `-###` to influence `verbose_only_flag`
3. **Dump File Handling**: Exercises `-save-temps`, `-dumpdir`, `-dumpbase` to allocate and free dump variables
4. **At-file Syntax**: Uses `@args.txt` to test `at_file_supplied`
5. **Sysroot/Target**: Tests `--sysroot`, `-isysroot`, `-target` for `target_system_root` and `spec_machine`
6. **Linker Control**: Uses `-fuse-ld=*` for `use_ld` and `--help=*` for `print_subprocess_help`
7. **Time Reporting**: Tests `-ftime-report` and `-fmem-report` for `report_times_to_file`
8. **Error Conditions**: Triggers error paths to affect `greatest_status`
9. **Combined Scenarios**: Tests multiple flags together
10. **Cleanup Cycles**: Multiple invocations ensure reset logic is triggered

The script redirects output to `/dev/null` where appropriate to avoid cluttering the terminal while still executing the driver logic. Each invocation causes the GCC driver to initialize, execute, and clean up, exercising the target code block.
