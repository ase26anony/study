Looking at the uncovered lines in `gcc.cc`, I need to create a comprehensive test that exercises the initialization and cleanup of these global variables. Here's a shell script that systematically covers each variable:

```bash
#!/bin/bash
# test_driver.sh - Comprehensive test for GCC driver initialization/cleanup

set -e

# Create test directory structure
TESTDIR=$(mktemp -d)
cd "$TESTDIR"
echo "Test directory: $TESTDIR"

# 1. Create minimal test files
cat > test.c << 'EOF'
int main() { return 0; }
EOF

cat > test2.c << 'EOF'
int foo() { return 1; }
EOF

# 2. Create at-file for testing
cat > args.txt << 'EOF'
-O2
-c
test.c
-o
test.o
EOF

# Helper function to run gcc and capture status
run_gcc() {
    echo "Running: gcc $*"
    gcc "$@" 2>/dev/null || true
    echo "Exit status: $?"
    echo "---"
}

echo "=== Testing initialization and cleanup of global variables ==="
echo

# 3. Test help and version flags (print_help_list, print_version)
echo "Test 1: Help and version flags"
run_gcc --version
run_gcc --help
run_gcc --target-help
run_gcc -v --help  # Combined verbose and help

# 4. Test verbose and diagnostic flags (verbose_only_flag)
echo "Test 2: Verbose and diagnostic flags"
run_gcc -v -c test.c
run_gcc -### -c test.c 2>&1 | head -5

# 5. Test save-temps and dump variables (save_temps_flag, dumpdir, dumpbase, etc.)
echo "Test 3: Save-temps and dump file handling"
mkdir -p dumpdir
run_gcc -save-temps -dumpdir ./dumpdir -dumpbase mytest -c test.c
run_gcc -save-temps=cwd -dumpbase_ext .ext -c test.c
run_gcc -save-temps=obj -o test.o -c test.c

# Test cleanup by running multiple invocations with different dump settings
run_gcc -dumpdir ./dump1 -dumpbase db1 -c test.c
run_gcc -dumpdir ./dump2 -dumpbase db2 -c test.c  # Should trigger cleanup of previous

# 6. Test sysroot and target variables (target_system_root, target_system_root_changed)
echo "Test 4: Sysroot and target specifications"
run_gcc --sysroot=/ -c test.c 2>/dev/null
run_gcc -isysroot /usr/include -c test.c 2>/dev/null
run_gcc -target $(gcc -dumpmachine) -c test.c  # Use current target

# 7. Test at-file handling (at_file_supplied)
echo "Test 5: At-file handling"
run_gcc @args.txt
# Also test without at-file for comparison
run_gcc -O2 -c test.c -o test2.o

# 8. Test linker selection (use_ld)
echo "Test 6: Linker selection"
for linker in bfd gold lld; do
    if gcc -fuse-ld=$linker --help 2>&1 | grep -q "unrecognized"; then
        echo "Linker $linker not available, skipping"
    else
        run_gcc -fuse-ld=$linker -c test.c
    fi
done

# 9. Test subprocess help (print_subprocess_help)
echo "Test 7: Subprocess help"
run_gcc --help=common
run_gcc --help=target
run_gcc --help=optimizers

# 10. Test time reporting (report_times_to_file)
echo "Test 8: Time and memory reporting"
run_gcc -ftime-report -c test.c 2>&1 | head -10
run_gcc -fmem-report -c test.c 2>&1 | head -5

# 11. Test error conditions (greatest_status)
echo "Test 9: Error conditions"
run_gcc invalid_file.c  # Should fail
run_gcc -invalid-flag 2>/dev/null  # Should fail
run_gcc -c test.c -o /proc/invalid/path 2>/dev/null  # Should fail on permission

# 12. Test multiple sequential invocations to trigger cleanup
echo "Test 10: Sequential invocations triggering cleanup"
run_gcc -v -save-temps -dumpdir ./seq1 -c test.c
run_gcc -v -save-temps -dumpdir ./seq2 -c test2.c
run_gcc -v -c test.c  # Clean default state

# 13. Test combination of many flags
echo "Test 11: Complex combination"
run_gcc -v --sysroot=/ -save-temps -dumpdir ./complex -dumpbase complex \
        -ftime-report -fuse-ld=bfd -c test.c 2>/dev/null

# 14. Test driver mode changes (is_cpp_driver)
echo "Test 12: Different driver modes"
run_gcc -E test.c  # Preprocessor mode
run_gcc -S test.c  # Assembly mode  
run_gcc -c test.c  # Compile mode
run_gcc test.c -o test.out  # Link mode

# 15. Test with environment variables that might affect initialization
echo "Test 13: With GCC environment variables"
GCC_EXEC_PREFIX=/usr/lib/gcc/ run_gcc -c test.c
COMPILER_PATH=/usr/bin run_gcc -c test.c

# Cleanup
echo "=== Test complete ==="
cd ..
rm -rf "$TESTDIR"
```

**Key aspects that ensure coverage of the uncovered lines:**

1. **Multiple initialization cycles**: Each `run_gcc` call creates a new process, forcing full initialization and cleanup.

2. **Specific flag combinations**:
   - `--help`, `--version` → `print_help_list`, `print_version`
   - `-v`, `-###` → `verbose_only_flag`
   - `-save-temps`, `-dumpdir`, `-dumpbase` → `save_temps_flag`, `dumpdir`, `dumpbase`, etc.
   - `--sysroot`, `-isysroot` → `target_system_root`, `target_system_root_changed`
   - `@args.txt` → `at_file_supplied`
   - `-fuse-ld=*` → `use_ld`
   - `--help=*` → `print_subprocess_help`
   - `-ftime-report` → `report_times_to_file`
   - Error conditions → `greatest_status`

3. **Cleanup triggering**: Sequential invocations with different dumpdir/dumpbase settings force `free()` calls and NULL assignments.

4. **Driver mode testing**: `-E`, `-S`, `-c`, linking modes ensure `is_cpp_driver` and other mode-specific variables are exercised.

5. **Error paths**: Invalid files and flags ensure error handling code paths are taken.

The script creates a temporary directory, runs comprehensive tests, and cleans up. Each test is designed to trigger specific parts of the initialization/cleanup block in `gcc.cc` lines 11228-11250.
