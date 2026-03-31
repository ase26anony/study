I'll create a comprehensive test that triggers the uncovered lines in gcc.cc by invoking the GCC driver with various configurations that set and reset the state variables.

```bash
#!/bin/bash
# Test script to trigger GCC driver state initialization and cleanup paths
# Specifically targets lines 11228-11250 in gcc.cc

set -e

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gcc_test_XXXXXX)
trap "rm -rf $TEST_DIR" EXIT

cd $TEST_DIR

# Create a minimal valid C source file
cat > test_source.c << 'EOF'
int main(void) {
    return 0;
}
EOF

# Create a minimal C++ source file
cat > test_source.cpp << 'EOF'
int main() {
    return 0;
}
EOF

# Create a syntactically invalid C source file
cat > invalid.c << 'EOF'
int main(void {
    return 0
}
EOF

# Create an at-file with compiler arguments
cat > args.at << 'EOF'
-O2
-Wall
-Wextra
-save-temps=cwd
-dumpdir=./dumpdir_test
-dumpbase=testdump
EOF

# Create another at-file with different options
cat > args2.at << 'EOF'
-g
-O0
-fverbose-asm
-save-temps
EOF

# Test 1: Basic compilation with state-setting flags
echo "Test 1: Basic compilation with dumpdir and dumpbase"
gcc -save-temps -dumpdir ./mydumps -dumpbase mytest -dumpbase-ext .c \
    -o test1.exe test_source.c 2>/dev/null || true

# Test 2: Preprocessor run with extensive flags
echo "Test 2: Preprocessor with state flags"
gcc -E -P -dD -save-temps=cwd -dumpdir . -dumpbase preproc_test \
    -dumpbase-ext .i test_source.c > /dev/null 2>&1

# Test 3: Help and version flags (should trigger early exit but still hit cleanup)
echo "Test 3: Help and version flags"
gcc --help > /dev/null 2>&1
g++ --target-help > /dev/null 2>&1
gcc --version > /dev/null 2>&1

# Test 4: Compilation after help flags (triggers reset)
echo "Test 4: Compilation after help flags"
gcc -c test_source.c -o test_source.o 2>/dev/null

# Test 5: Using at-files
echo "Test 5: Using at-files"
gcc @args.at test_source.c -o test2.exe 2>/dev/null || true

# Test 6: C++ driver with different options
echo "Test 6: C++ driver with state flags"
g++ -save-temps -dumpdir ./cppdumps -dumpbase cpptest \
    -dumpbase-ext .cpp -o test3.exe test_source.cpp 2>/dev/null || true

# Test 7: Failed compilation (to affect greatest_status)
echo "Test 7: Failed compilation"
gcc -c invalid.c -o invalid.o 2>/dev/null || true

# Test 8: Successful compilation after failure
echo "Test 8: Successful compilation after failure"
gcc -c test_source.c -o test4.o 2>/dev/null

# Test 9: Multiple output and dump options
echo "Test 9: Complex output configuration"
gcc -save-temps -dumpdir ./complex -dumpbase complex -dumpbase-ext .mult \
    -o test5.exe test_source.c -E -P > /dev/null 2>&1

# Test 10: Different source file extensions
echo "Test 10: Different source types"
# Create .i (preprocessed) file
gcc -E test_source.c > test_preprocessed.i 2>/dev/null
# Create .s (assembly) file
gcc -S test_source.c -o test_assembly.s 2>/dev/null

# Compile preprocessed file
gcc -save-temps -dumpdir ./preproc -dumpbase preproc test_preprocessed.i \
    -o test6.exe 2>/dev/null || true

# Compile assembly file
gcc -save-temps -dumpdir ./asm -dumpbase asm test_assembly.s \
    -o test7.exe 2>/dev/null || true

# Test 11: Mixed flags and at-file
echo "Test 11: Mixed at-file and command line flags"
gcc @args2.at -dumpdir ./mixed -dumpbase mixed test_source.c \
    -o test8.exe 2>/dev/null || true

# Test 12: Verbose and diagnostic flags
echo "Test 12: Verbose and diagnostic output"
gcc -v -save-temps=cwd test_source.c -o test9.exe 2>/dev/null || true
gcc -### test_source.c 2>/dev/null || true

# Test 13: Target-specific options (affects spec_machine)
echo "Test 13: Target-specific options"
# Try with different machine specs if supported
gcc -specs=/dev/null test_source.c -o test10.exe 2>/dev/null || true

# Test 14: Multiple invocations in sequence (triggers repeated init/cleanup)
echo "Test 14: Sequential invocations"
for i in {1..3}; do
    gcc -save-temps -dumpdir "./seq$i" -dumpbase "seq$i" \
        -o "test_seq$i.exe" test_source.c 2>/dev/null || true
done

# Test 15: Combination of all state-affecting flags
echo "Test 15: Comprehensive flag combination"
gcc -save-temps=cwd -dumpdir ./final -dumpbase final -dumpbase-ext .all \
    -o final.exe test_source.c -E -P -dD -v 2>/dev/null || true

# Test 16: Reset with no output file but dump options
echo "Test 16: Dump options without output file"
gcc -save-temps -dumpdir ./dumponly -dumpbase dumponly \
    -dumpbase-ext .nodash test_source.c -c 2>/dev/null

# Test 17: Test with trailing dash in dumpdir (dumpdir_trailing_dash_added)
echo "Test 17: Dumpdir with potential trailing dash"
gcc -save-temps -dumpdir ./dashdir/ -dumpbase dashbase \
    test_source.c -o dash.exe 2>/dev/null || true

# Test 18: Different driver invocations
echo "Test 18: Different compiler drivers"
# Try to trigger is_cpp_driver variations
cpp -E test_source.c > /dev/null 2>&1 || true
g++ -x c test_source.c -E -P > /dev/null 2>&1
gcc -x c++ test_source.cpp -E -P > /dev/null 2>&1

# Test 19: Time reporting (affects report_times_to_file)
echo "Test 19: Time reporting flags"
gcc -ftime-report test_source.c -o time.exe 2>/dev/null || true
gcc -ftime-report -ftime-report-details test_source.c -c 2>/dev/null || true

# Test 20: Final cleanup trigger
echo "Test 20: Final simple compilation to ensure cleanup"
gcc test_source.c -o final_simple.exe 2>/dev/null

echo "All tests completed. Test directory: $TEST_DIR"
echo "Note: Many commands redirect stderr to /dev/null to suppress warnings"
echo "and focus on triggering the driver's internal state transitions."
```

This script creates a comprehensive test that:

1. **Sets multiple state variables** through various GCC flags:
   - `-save-temps`, `-save-temps=cwd` → affects `save_temps_flag`
   - `-dumpdir`, `-dumpbase`, `-dumpbase-ext` → allocates strings that get freed in the reset block
   - `-o` with different paths → affects `outbase`
   - `--help`, `--version`, `--target-help` → sets `print_help_list`, `print_version`
   - `-v`, `-###` → may affect `verbose_only_flag`
   - `-specs=` → may affect `spec_machine`

2. **Triggers different execution paths**:
   - Successful compilations
   - Failed compilations (syntax errors) → affects `greatest_status`
   - Preprocessor-only runs (`-E -P -dD`)
   - Assembly generation (`-S`)
   - Object file compilation (`-c`)

3. **Uses at-files** (`@file`) → sets `at_file_supplied`

4. **Tests different drivers**:
   - `gcc` (C driver)
   - `g++` (C++ driver) → may affect `is_cpp_driver`
   - `cpp` (preprocessor driver)

5. **Combines flags strategically**:
   - Mixes state-setting flags with actual compilation
   - Sequences commands to trigger repeated initialization/cleanup cycles
   - Uses both early-exit flags (`--help`) and full compilation paths

The script runs each command in a way that should cause the GCC driver to allocate, use, and then reset its internal state variables, specifically hitting the reset block at lines 11228-11250 in gcc.cc. The redirection to `/dev/null` prevents output clutter while still executing the driver's full code paths.
